import os, sys
import platform
import shutil
import zipfile
from .MatlabProxyObject import wrap, unwrap
from .utils import get_version_from_ctf, checkPath, get_nlhs

# Store the Matlab engine as a module global wrapped inside a class
# When the global ref is deleted (e.g. when Python exits) the __del__ method is called
# Which then gracefully shutsdown Matlab, else we get a segfault.
_global_matlab_ref = None
_has_registered_magic = None

# On Linux we need to load the BLAS/LAPACK libraries with the DEEPBIND
# flag so it doesn't conflict with Matlab's BLAS/LAPACK.
# This only works if users `import libpymcr` before they import scipy...
if platform.system() == 'Linux':
    old_flags = sys.getdlopenflags()
    sys.setdlopenflags(os.RTLD_NOW | os.RTLD_DEEPBIND)
    try:
        import scipy.linalg
    except ImportError:
        pass
    sys.setdlopenflags(old_flags)


class _MatlabInstance(object):
    def __init__(self, ctffile, matlab_dir=None, options=None):
        ctffile = os.path.abspath(ctffile)
        if not os.path.exists(ctffile):
            raise RuntimeError('CTF file {} does not exist'.format(ctffile))
        if matlab_dir is None:
            matlab_dir = checkPath(get_version_from_ctf(ctffile))
        if options is None:
            # macOS machines need a main function to run the Cocoa interface
            # so, at least at present, they can not create a library instance
            # of MATLAB with plotting capabilities
            from platform import system
            options = ["-nojvm" if system() == 'Darwin' else ""]
        os.environ["LIBPYMCR_MATLAB_ROOT"] = matlab_dir
        self.ctf = ctffile
        from . import _libpymcr
        self.interface = _libpymcr.matlab(ctffile, matlab_dir, options)
        print('Interface opened')

    def __getattr__(self, name):
        if self.interface:
            return getattr(self.interface, name)
        else:
            raise RuntimeError('Matlab interface is not open')


class NamespaceWrapper(object):
    def __init__(self, interface, name):
        self._interface = interface
        self._name = name

    def __getattr__(self, name):
        return NamespaceWrapper(self._interface, f'{self._name}.{name}')

    def __call__(self, *args, **kwargs):
        nargout = kwargs.pop('nargout') if 'nargout' in kwargs.keys() else None
        nreturn = get_nlhs(self._name)
        if nargout is None:
            mnargout, undetermined = self._interface.call('getArgOut', self._name, nargout=2)
            if not undetermined:
                nargout = min(int(mnargout), nreturn)
            else:
                nargout = nreturn
        args += sum(kwargs.items(), ())
        args = unwrap(args, self._interface)
        return wrap(self._interface.call(self._name, *args, nargout=nargout), self._interface)

    def getdoc(self):
        # To avoid error message printing in Spyder
        raise NotImplementedError


class Matlab(object):
    def __init__(self, ctffile=None, mlPath=None):
        """
        Create an interface to a matlab compiled python library and treat the objects in a python/matlab way instead of
        the ugly way it is done by default.

        :param mlPath: Path to the SDK i.e. '/MATLAB/MATLAB_Runtime/v96' or to the location where matlab is installed
                       (MATLAB root directory). If omitted, will attempt to find Matlab folder automatically
        """

        global _global_matlab_ref
        if _global_matlab_ref is None:
            if ctffile is None:
                raise RuntimeError('Matlab is not initialised, please provide a CTF path')
            _global_matlab_ref = _MatlabInstance(ctffile, mlPath)
        self._interface = _global_matlab_ref.interface

    def __getattr__(self, name):
        """
        Override for the get attribute. We don't want to call the process but the interface, so redirect calls there and
        return a MatlabProxyObject

        :param name: The function/class to be called.
        :return: MatlabProxyObject of class/function given by name
        """
        return NamespaceWrapper(self._interface, name)

    def get_matlab_functions(self):
        """
        Returns a list of public functions in this CTF archive
        """
        def _get_xml_val(in_str):
            return in_str.split('=')[1].replace('"', '').replace('/', '')
        def _get_xml_tag(lis, tag_start, tag_end):
            id0 = lis.index(tag_start) + 1
            return lis[id0:lis.index(tag_end, id0)]

        with zipfile.ZipFile(_global_matlab_ref.ctf, 'r') as ctf:
            manifest = ctf.read('.META/manifest.xml').decode('ascii')
            tags = manifest.split('><')
            funcs = tags[(tags.index('public-functions') + 1):tags.index('/public-functions')]
            for fn in funcs:
                name = _get_xml_val(fn)
                info = _get_xml_tag(tags, 'function id="{}"'.format(name), '/function')
                ml_name = info[0].replace('name>', '').replace('</name', '')
                try:
                    inputs = [_get_xml_val(v) for v in _get_xml_tag(info, 'inputs', '/inputs')]
                except ValueError:
                    inputs = []
                try:
                    outputs = [_get_xml_val(v) for v in _get_xml_tag(info, 'outputs', '/outputs')]
                except ValueError:
                    outputs = []
                print('[{}] = {}({})'.format(','.join(outputs), ml_name, ','.join(inputs)))


def register_ipython_magics():
    try:
        import IPython
    except ImportError:
        return None
    else:
        running_kernel = IPython.get_ipython()
        # Only register these magics when running in a notebook / lab
        # Other values seen are: 'TerminalInteractiveShell' and 'InteractiveShellEmbed'
        if (running_kernel.__class__.__name__ != 'ZMQInteractiveShell'
            and running_kernel.__class__.__name__ != 'SpyderShell'):
            return None
    global _has_registered_magic
    _has_registered_magic = True
    if running_kernel is None or sys.__stdout__ is None or sys.__stderr__ is None:
        return None
    from . import IPythonMagics
    from traitlets import Instance
    shell = Instance('IPython.core.interactiveshell.InteractiveShellABC', allow_none=True)
    magics = IPythonMagics.MatlabMagics(shell, None)
    running_kernel.register_magics(magics)
    running_kernel.events.register('post_run_cell', IPythonMagics.showPlot)
    # Only do redirection for Jupyter notebooks - causes errors on Spyder
    if running_kernel == 'ZMQInteractiveShell':
        redirect_stdout = IPythonMagics.Redirection(target='stdout')
        running_kernel.events.register('pre_run_cell', redirect_stdout.pre)
        running_kernel.events.register('post_run_cell', redirect_stdout.post)


if not _has_registered_magic:
    register_ipython_magics()
