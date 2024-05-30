import sys, os, re
import argparse
import platform
import glob
import tempfile
import subprocess
import json


OS = platform.system()
if OS == 'Windows':
    EXE, OSSEP = ('.exe', '\\')
else:
    EXE, OSSEP = ('', '/')

RESERVED = ['False', 'await', 'else', 'import', 'pass', 'None', 'break', 'except', 'in', 'raise', 'True', 'class', \
            'finally', 'is', 'return', 'and', 'continue', 'for', 'lambda', 'try', 'as', 'def', 'from', 'nonlocal', \
            'while', 'assert', 'del', 'global', 'not', 'with', 'async', 'elif', 'if', 'or', 'yield']

def _get_args():
    parser = argparse.ArgumentParser(description='A script to generate Python wrappers for Matlab functions/classes',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-a', metavar='mfolder', action='append', help='Add a folder of Matlab files to be parsed')
    parser.add_argument('--preamble', default='import libpymcr; m = libpymcr.Matlab()', help='Premable string to initialize libpymcr')
    parser.add_argument('--prefix', default='m', help='Prefix for Matlab commands')
    parser.add_argument('--package', default='matlab_functions', help='Name of Python package to put wrappers into')
    parser.add_argument('--matlabexec', default=argparse.SUPPRESS, metavar='MLPATH',
                        help='Full path to Matlab executable (default: autodetect)')
    parser.add_argument('-o', '--outputdir', default='matlab_wrapped', help='Output folder for Python files')
    parser.add_argument('mfilename', nargs='*', help='Add a Matlab m-file to be parsed')
    return parser


def _parse_addedfolders(folder_list):
    # Parses added folders list to get a list of files and "@" folders
    if not folder_list:
        return [], []
    from os.path import isfile, isdir
    mfiles = []
    classdirs = []
    for folder in folder_list:
        mfiles += list(filter(lambda x: isfile(x) and '\@' not in x, glob.glob(f'{folder}/**/*.m', recursive=True)))
        classdirs += list(filter(isdir, glob.glob(f'{folder}/**/\@*', recursive=True)))
    return mfiles, classdirs


def _parse_mfuncstring(funcstring):
    # Parses the function header to get input and output variable names
    funcstring = funcstring.replace('...', '')
    if '(' in funcstring:
        if '=' in funcstring:
            m = re.search('function ([\[\]A-z0-9_,\s]*)=([A-z0-9_\s]*)\(([A-z0-9_,\s~]*)\)', funcstring)
            outvars = [v.strip() for v in m.group(1).replace('[','').replace(']','').split(',')]
        else:
            outvars = None
            m = re.search('function ()([A-z0-9_\s]*)\(([A-z0-9_,\s~]*)\)', funcstring)
        invars = [v.strip() for v in m.group(3).strip().split(',')]
    else:
        invars = None
        if '=' in funcstring:
            m = re.search('function ([\[\]A-z0-9_,\s]*)=([A-z0-9_\s]*)', funcstring)
            outvars = [v.strip() for v in m.group(1).replace('[','').replace(']','').split(',')]
        else:
            outvars = None
    return invars, outvars


def _parse_single_mfile(mfile):
    # Parses a single m-file to check if it is a function, classdef or script
    with open(mfile, 'r') as f:
        inblockcomment = False
        predocstring, postdocstring = ([], [])
        funcstring = []
        outside_funcdef = False
        for line in f:
            line = line.strip()
            if line.startswith('%{'):
                inblockcomment = True
            if not inblockcomment:
                if outside_funcdef:
                    if line.startswith('%'):
                        postdocstring.append(line)
                    else:
                        break
                if not outside_funcdef and len(funcstring) > 0:
                    funcstring.append(line)
                    if ')' in line:
                        funcstring = ''.join(funcstring)
                        outside_funcdef = True
                elif line.startswith('function'):
                    if ')' in line or '(' not in line:
                        funcstring = line
                        outside_funcdef = True
                    else:
                        funcstring = [line]
                elif line.startswith('classdef'):
                    return 'classdef', [mfile]
                elif not line.startswith('%'):
                    # If the line does not start with a comment we are in the body
                    break
                else:
                    predocstring.append(line)
            elif line.startswith('%}'):
                inblockcomment = False
    if len(funcstring) > 0:
        docstring = predocstring if len(predocstring) > 0 else postdocstring
        return 'function', [[mfile, _parse_mfuncstring(funcstring), '\n'.join(docstring)]]
    return 'script', []


def _parse_mfilelist(mfile_list):
    # Parses a list of mfiles to see if they are functions, classdefs or scripts
    typelist = {'function':[], 'classdef':[], 'script':[]}
    for mfile in mfile_list:
        filetype, fileinfo = _parse_single_mfile(mfile)
        typelist[filetype] += fileinfo
    return typelist['function'], typelist['classdef']


def _conv_path_to_matlabpack(pth):
    pth = pth.replace('.m', '')
    if '+' in pth:
        pth = ''.join(pth.split('+')[1:]).replace(os.path.sep, '.')
    else:
        pth = os.path.split(pth)[1]
    return pth


def _matlab_parse_classes(classdirs, classfiles, mlexec, addeddirs, addedfiles):
    # Uses Matlab metaclass to parse classes
    dirs = set([os.path.dirname(f) for f in addedfiles])
    dirs = list(filter(lambda x: not any([x.startswith(d) for d in addeddirs]), dirs))
    # Strips out package directories
    dirs = list(set([d.split('+')[0] for d in dirs]))
    # Parses the class list
    classlist = []
    for cls in classdirs + classfiles:
        cls = _conv_path_to_matlabpack(cls)
        cls = cls.replace('@', '')
        classlist.append(cls)
    with tempfile.TemporaryDirectory() as d_name:
        parsemfile = os.path.join(d_name, 'parsescript.m')
        jsonout = os.path.join(d_name, 'classdata.json')
        with open(parsemfile, 'w') as f:
            f.write(f"addpath('{os.path.dirname(__file__)}');\n")
            for folder in addeddirs:
                f.write(f"addpath(genpath('{folder}'));\n")
            for folder in dirs:
                f.write(f"addpath('{folder}');\n")
            idx = 1
            for classname in classlist:
                f.write(f"outstruct({idx}) = parseclass('{classname}');\n")
                idx += 1
            f.write("jsontxt = jsonencode(outstruct);\n")
            f.write(f"fid = fopen('{jsonout}', 'w');\n")
            f.write("fprintf(fid, '%s', jsontxt);\n")
            f.write("fclose(fid);\n")
        mlcmd = [mlexec, '-batch', f"addpath('{d_name}'); parsescript; exit"]
        res = subprocess.run(mlcmd, capture_output=True)
        if res.returncode != 0:
            raise RuntimeError(f'Matlab parsing of classes failed with error:\n{res.stdout}\n{res:stderr}')
        with open(jsonout, 'r') as f:
            classinfo = json.load(f)
        return classinfo


def _python_parse_classes(classdirs, classfiles):
    raise NotImplementedError('Python parsing of classes not implemented')


def _parse_args(fn):
    if fn[1][0] is None or (len(fn[1][0]) == 1 and fn[1][0][0] == ''):
        return '', 'args = []'
    if len(fn[1][0]) == 1 and fn[1][0][0] == 'varargin':
        return '*args, ', ''
    argp = ', '.join([v for v in fn[1][0] if v != '' and 'varargin' not in v and v != '~'])
    args = ''.join([f'{v.replace("~","_")}=None, ' for v in fn[1][0] if v!= '']).replace('varargin=None, ', '*args, ')
    argline = f'args = tuple(v for v in [{argp}] if v is not None)'
    if '*args' in args:
        argline += ' + args'
    return args, argline


def _get_funcname(fname):
    # Checks Python functions against reserved keywords
    if '.' in fname:
        fname = fname.split('.')[-1]
    return fname + '_' if fname in RESERVED else fname


def _write_function(f, fn, prefix):
    args, argline = _parse_args(fn)
    fnname = _get_funcname(fn[0])
    mname = fnname if '.' not in fn[0] else fn[0]
    f.write(f'def {fnname}({args}**kwargs):\n')
    f.write(f'    """\n')
    f.write(f'{fn[2]}\n')
    f.write(f'    """\n')
    f.write(f'    {argline}\n')
    f.write(f'    return {prefix}.{mname}(*args, **kwargs)\n\n\n')


def _generate_wrappers(funcfiles, classinfo, outputdir, preamble, prefix):
    # Generates Python wrappers from Matlab function and class info
    #
    # Puts all the functions into output/__init__.py and all classes into their own files,
    # preserving Matlab directory/package structures
    preamble = re.sub(';\s*', '\n', preamble) + '\n\n'
    if not os.path.exists(outputdir):
        os.mkdir(outputdir)
    elif not os.path.isdir(outputdir):
        raise RuntimeError(f'Output {outputdir} is not a folder')
    classes = []
    class_in_packages = {}
    for cls in classinfo:
        if '.' in cls['name']:
            package = os.path.join(*cls['name'].split('.')[:-1])
            clsname = cls['name'].split('.')[-1]
            if package in class_in_packages:
                class_in_packages[package].append(clsname)
            else:
                class_in_packages[package] = [clsname]
            classfile = os.path.join(outputdir, package, f"{clsname}.py")
            packdir = os.path.join(outputdir, package)
            if not os.path.exists(packdir):
                os.makedirs(packdir)
        else:
            classes.append(cls['name'])
            classfile = os.path.join(outputdir, f"{cls['name']}.py")
        with open(classfile, 'w') as f:
            clsname = _get_funcname(cls['name'])
            f.write(preamble)
            f.write("from libpymcr import MatlabProxyObject\n")
            f.write("from libpymcr.utils import get_nlhs\n\n")
            f.write(f"class {clsname}(MatlabProxyObject):\n")
            f.write('    """\n')
            f.write(f"{cls['doc']}\n")
            f.write('    """\n')
            if not isinstance(cls['methods'], list):
                cls['methods'] = [cls['methods']]
            clscons = [c for c in cls['methods'] if c['name'] == cls['name']]
            if clscons:
                clscons = clscons[0]
                args, argline = _parse_args([[], [clscons['inputs'], []], []])
                f.write(f"    def __init__(self, {args}**kwargs):\n")
                f.write( '        """\n')
                f.write(f"{clscons['doc']}\n")
                f.write( '        """\n')
                f.write(f"        self.__dict__['interface'] = {prefix}._interface\n")
                f.write( "        self.__dict__['_methods'] = []\n")
                f.write(f"        self.__dict__['__name__'] = '{clsname}'\n")  # Needed for pydoc
                f.write(f"        self.__dict__['__origin__'] = {clsname}\n")  # Needed for pydoc
                f.write(f'        {argline}\n')
                f.write(f'        args += sum(kwargs.items(), ())\n')
                f.write(f"        self.__dict__['handle'] = self.interface.call('{cls['name']}', *args, nargout=1)\n")
                f.write( '\n')
            else:
                f.write(f"    def __init__(self):\n")
                f.write(f"        self.__dict__['interface'] = {prefix}._interface\n")
                f.write( "        self.__dict__['methods'] = []\n")
                f.write(f"        self.__dict__['handle'] = self.interface.call('{cls['name']}', [], nargout=1)\n")
                f.write( '\n')
            # Create a "help" method so it doesn't try to call the Matlab (and so crash)
            if not isinstance(cls['properties'], list):
                cls['properties'] = [cls['properties']]
            for prop in cls['properties']:
                propname = _get_funcname(prop['name'])
                f.write( '    @property\n')
                f.write(f"    def {propname}(self):\n")
                f.write( '        """\n')
                f.write(f"{prop['doc']}\n")
                f.write( '        """\n')
                f.write(f"        return self.__getattr__('{prop['name']}')\n")
                f.write( '\n')
                f.write(f"    @{propname}.setter\n")
                f.write(f"    def {propname}(self, val):\n")
                f.write(f"        self.__setattr__('{prop['name']}', val)\n")
                f.write( '\n')
            for mth in cls['methods']:
                if mth['name'] == cls['name']:
                    continue
                args, argline = _parse_args([[], [mth['inputs'], []], []])
                mthname = _get_funcname(mth['name'])
                f.write(f"    def {mthname}(self, {args}**kwargs):\n")
                f.write( '        """\n')
                f.write(f"{mth['doc']}\n")
                f.write( '        """\n')
                f.write(f'        {argline}\n')
                f.write(f'        args += sum(kwargs.items(), ())\n')
                f.write(f"        nout = max(min({len(mth['outputs'])}, get_nlhs('{mthname}')), 1)\n")
                f.write(f"        return {prefix}.{mthname}(self.handle, *args, nargout=nout)\n")
                f.write( '\n')
    funcs_in_packages = {}
    with open(os.path.join(outputdir, '__init__.py'), 'w') as f:
        f.write(preamble)
        for cls in classes:
            f.write(f"from .{cls} import {cls}\n")
        singlefuncs = []
        for fn in funcfiles:
            fn[0] = _conv_path_to_matlabpack(fn[0])
            if '.' in fn[0]:  # Put packages into separate files
                package = os.path.join(*fn[0].split('.')[:-1])
                if package in funcs_in_packages:
                    funcs_in_packages[package].append(fn)
                else:
                    funcs_in_packages[package] = [fn]
            else:
                singlefuncs.append(fn)
        packages = list(funcs_in_packages.keys()) + list(class_in_packages.keys())
        # Imports only first level packages in the main __init__.py
        for cls in [p for p in packages if os.path.sep not in p]:
            f.write(f"from . import {cls}\n")
        f.write('\n')
        for fn in singlefuncs:
            # Assume that varargin if present is always the last argument
            _write_function(f, fn, prefix)
    for pack, fns in funcs_in_packages.items():
        packdir = os.path.join(outputdir, pack)
        if not os.path.exists(packdir):
            os.makedirs(packdir)
        with open(os.path.join(packdir, '__init__.py'), 'w') as f:
            f.write(preamble)
            if pack in class_in_packages:
                for cls in class_in_packages.pop(pack):
                    f.write(f"from .{cls} import {cls}\n")
            for cls in [p for p in packages if p != pack and p.startswith(pack)]:
                f.write(f"from . import {cls.split(pack)[1][1:].split(os.path.sep)[0]}\n")
            f.write('\n')
            for fn in fns:
                _write_function(f, fn, prefix)
    for pack, clss in class_in_packages.items():
        packdir = os.path.join(outputdir, pack)
        with open(os.path.join(packdir, '__init__.py'), 'w') as f:
            for cls in clss:
                f.write(f"from .{cls} import {cls}\n")


def main(args=None):
    args = _get_args().parse_args(args if args else sys.argv[1:])
    if args.a is None and len(args.mfilename) == 0:
        print('No mfiles or folders specified. Exiting')
        return
    # Checks if we have Matlab installed
    if hasattr(args, 'matlabexec'):
        if os.path.isfile(args.matlabexec):
            mlexec = args.matlabexec
        else:
            raise RuntimeError(f'Matlab executable "{args.matlabexec}" does not exist or is not a file')
    else:
        import libpymcr.utils
        mlpath = libpymcr.utils.checkPath(runtime_version=None)
        mlexec = os.path.join(mlpath, 'bin', f'matlab{EXE}') if mlpath else None

    mfiles, classdirs = _parse_addedfolders(args.a)
    funcfiles, classfiles = _parse_mfilelist(mfiles + args.mfilename)

    if mlexec:
        # Use Matlab to parse classes.
        classinfo = _matlab_parse_classes(classdirs, classfiles, mlexec, args.a, classfiles)
    else:
        classinfo = _python_parse_classes(classdirs, classfiles)

    _generate_wrappers(funcfiles, classinfo, args.outputdir, args.preamble, args.prefix)


if __name__ == '__main__':
    main()
