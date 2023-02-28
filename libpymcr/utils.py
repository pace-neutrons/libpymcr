import os
import sys
import glob
import platform
import zipfile
import traceback
from pathlib import Path


def get_nlhs():
    caller = traceback.extract_stack()[-3].line
    retvals = (0, '')
    if '=' in caller:
        return len(caller.split('=')[0].split(','))
    else:
        return 1


def get_version_from_ctf(ctffile):
    with zipfile.ZipFile(ctffile, 'r') as ctf:
        manifest = ctf.read('.META/manifest.xml').decode('ascii')
        for tag in manifest.split('><'):
            if 'mcr-major-version' in tag:
                ver = dict([v.split("=") for v in tag.split() if 'mcr' in v])
                ver = [ver[v].replace('"', '') for v in ['mcr-major-version', 'mcr-minor-version']]
                return "{}.{}".format(*ver)


def get_matlab_from_registry(version=None):
    # Searches for the Mathworks registry key and finds the Matlab path from that
    retval = []
    try:
        import winreg
    except ImportError:
        return retval
    for installation in ['MATLAB', 'MATLAB Runtime', 'MATLAB Compiler Runtime']:
        try:
            with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, f'SOFTWARE\\MathWorks\\{installation}') as key:
                versions = [winreg.EnumKey(key, k) for k in range(winreg.QueryInfoKey(key)[0])]
        except (FileNotFoundError, OSError):
            pass
        else:
            if version is not None:
                versions = [v for v in versions if v == version]
            for v in versions:
                with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, f'SOFTWARE\\MathWorks\\{installation}\\{v}') as key:
                    retval.append(winreg.QueryValueEx(key, 'MATLABROOT')[0])
    return retval


class DetectMatlab(object):
    def __init__(self, version):
        self.ver = version
        self.PLATFORM_DICT = {'Windows': ['PATH', 'dll', ''], 'Linux': ['LD_LIBRARY_PATH', 'so', 'libmw'],
                 'Darwin': ['DYLD_LIBRARY_PATH', 'dylib', 'libmw']}
        # Note that newer Matlabs are 64-bit only
        self.ARCH_DICT = {'Windows': {'64bit': 'win64', '32bit': 'pcwin32'},
                          'Linux': {'64bit': 'glnxa64', '32bit': 'glnx86'},
                          'Darwin': {'64bit': 'maci64', '32bit': 'maci'}}
        # https://uk.mathworks.com/help/compiler/mcr-path-settings-for-run-time-deployment.html
        DIRS = ['runtime', os.path.join('sys', 'os'), 'bin', os.path.join('extern', 'bin')]
        self.REQ_DIRS = {'Windows':[DIRS[0]], 'Darwin':DIRS[:3], 'Linux':DIRS}
        self.system = platform.system()
        if self.system not in self.PLATFORM_DICT:
            raise RuntimeError('{0} is not a supported platform.'.format(self.system))
        (self.path_var, self.ext, self.lib_prefix) = self.PLATFORM_DICT[self.system]
        self.arch = self.ARCH_DICT[self.system][platform.architecture()[0]]
        self.required_dirs = self.REQ_DIRS[self.system]
        if self.system == 'Windows':
            self.file_to_find = ''.join((self.lib_prefix, 'mclmcrrt', self.ver.replace('.','_'), '.', self.ext))
            self.sep = ';'
        elif self.system == 'Linux':
            self.file_to_find = ''.join((self.lib_prefix, 'mclmcrrt', '.', self.ext, '.', self.ver))
            self.sep = ':'
        elif self.system == 'Darwin':
            self.file_to_find = ''.join((self.lib_prefix, 'mclmcrrt', '.', self.ver, '.', self.ext))
            self.sep = ':'
        else:
            raise RuntimeError(f'Operating system {self.system} is not supported.')

    def find_version(self, root_dir):
        print(f'Searching for Matlab in {root_dir}')
        def find_file(path, filename, max_depth=3):
            """ Finds a file, will return first match"""
            for depth in range(max_depth + 1):
                dirglobs = f'*{os.sep}'*depth
                files = glob.glob(f'{path}{os.sep}{dirglobs}{filename}')
                files = list(filter(os.path.isfile, files))
                if len(files) > 0:
                    return files[0]
            return None
        lib_file = find_file(root_dir, self.file_to_find)
        if lib_file is not None:
            lib_path = Path(lib_file)
            arch_dir = lib_path.parts[-2]
            self.arch = arch_dir
            ml_subdir = lib_path.parts[-3]
            if ml_subdir != 'runtime':
                self.ver = ml_subdir
            ml_path = os.path.abspath(lib_path.parents[2])
            print(f'Found Matlab {self.ver} {self.arch} at {ml_path}')
            return ml_path
        else:
            return None

    def guess_path(self, mlPath=[]):
        GUESSES = {'Windows': [r'C:\Program Files\MATLAB', r'C:\Program Files (x86)\MATLAB', 
                               r'C:\Program Files\MATLAB\MATLAB Runtime', r'C:\Program Files (x86)\MATLAB\MATLAB Runtime'],
                   'Linux': ['/usr/local/MATLAB', '/opt/MATLAB', '/opt', '/usr/local/MATLAB/MATLAB_Runtime'],
                   'Darwin': ['/Applications/MATLAB', '/Applications/']}
        if self.system == 'Windows':
            mlPath += get_matlab_from_registry(self.ver) + GUESSES['Windows']
        if 'MATLABEXECUTABLE' in os.environ: # Running in CI
            ml_env = os.environ['MATLABEXECUTABLE']
            if self.system == 'Windows' and ':' not in ml_env:
                pp = ml_env.split('/')[1:]
                ml_env = pp[0] + ':\\' + '\\'.join(pp[1:])
            mlPath += [os.path.abspath(os.path.join(ml_env, '..', '..'))]
            print(f'mlPath={mlPath}')
        for possible_dir in mlPath + GUESSES[self.system]:
            if os.path.isdir(possible_dir):
                rv = self.find_version(possible_dir)
                if rv is not None:
                   return rv
        return None

    def guess_from_env(self):
        ld_path = os.getenv(self.path_var)
        if ld_path is None: return None
        for possible_dir in ld_path.split(self.sep):
            if os.path.exists(os.path.join(possible_dir, self.file_to_find)):
                return os.path.abspath(os.path.join(possible_dir, '..', '..'))
        return None

    def env_not_set(self):
        # Determines if the environment variables required by the MCR are set
        if self.path_var not in os.environ:
            return True
        rt = os.path.join('runtime', self.arch)
        pv = os.getenv(self.path_var).split(self.sep)
        for path in [dd for dd in pv if rt in dd]:
            if self.find_version(os.path.join(path,'..','..')) is not None:
                return False
        return True

    def set_environment(self, mlPath=None):
        if mlPath is None:
            mlPath = self.guess_path()
        if mlPath is None:
            raise RuntimeError('Could not find Matlab')
        req_matlab_dirs = self.sep.join([os.path.join(mlPath, sub, self.arch) for sub in self.required_dirs])
        if self.path_var not in os.environ:
            os.environ[self.path_var] = req_matlab_dirs
        else:
            os.environ[self.path_var] += self.sep + req_matlab_dirs
        return None


def checkPath(runtime_version, mlPath=None):
    """
    Sets the environmental variables for Win, Mac, Linux

    :param mlPath: Path to the SDK i.e. '/MATLAB/MATLAB_Runtime/v96' or to the location where matlab is installed
    (MATLAB root directory)
    :return: None
    """

    # We use a class to try to get the necessary variables.
    obj = DetectMatlab(runtime_version)

    if mlPath:
        if not os.path.exists(os.path.join(mlPath)):
            if not os.path.exists(mlPath):
                raise FileNotFoundError(f'Input Matlab folder {mlPath} not found')
    else:
        mlPath = obj.guess_from_env()
        if mlPath is None:
            mlPath = obj.guess_path()
            if mlPath is None:
                raise RuntimeError('Cannot find Matlab')
            else:
                ld_path = obj.sep.join([os.path.join(mlPath, sub, obj.arch) for sub in obj.required_dirs])
                os.environ[obj.path_var] = ld_path
                #print('Set ' + os.environ.get(obj.path_var))
        #else:
        #    print('Found: ' + os.environ.get(obj.path_var))

    return mlPath
