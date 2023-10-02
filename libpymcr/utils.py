import os
import sys
import glob
import platform
import zipfile
import ast
import inspect
import dis
import linecache
from pathlib import Path


OPERATOR_NAMES = {
    "CALL_FUNCTION", "CALL_FUNCTION_VAR", "CALL_FUNCTION_KW", "CALL_FUNCTION_VAR_KW", "CALL",
    "UNARY_POSITIVE", "UNARY_NEGATIVE", "UNARY_NOT", "UNARY_CONVERT", "UNARY_INVERT", "GET_ITER",
    "BINARY_POWER", "BINARY_MULTIPLY", "BINARY_DIVIDE", "BINARY_FLOOR_DIVIDE", "BINARY_TRUE_DIVIDE",
    "BINARY_MODULO", "BINARY_ADD", "BINARY_SUBTRACT", "BINARY_SUBSCR", "BINARY_LSHIFT",
    "BINARY_RSHIFT", "BINARY_AND", "BINARY_XOR", "BINARY_OR",
    "INPLACE_POWER", "INPLACE_MULTIPLY", "INPLACE_DIVIDE", "INPLACE_TRUE_DIVIDE", "INPLACE_FLOOR_DIVIDE",
    "INPLACE_MODULO", "INPLACE_ADD", "INPLACE_SUBTRACT", "INPLACE_LSHIFT", "INPLACE_RSHIFT",
    "INPLACE_AND", "INPLACE_XOR", "INPLACE_OR", "COMPARE_OP", "SET_UPDATE", "BUILD_CONST_KEY_MAP",
    "CALL_FUNCTION_EX", "LOAD_METHOD", "CALL_METHOD", "DICT_MERGE", "DICT_UPDATE", "LIST_EXTEND",
}


def get_nret_from_dis(frame):
    # Tries to get the number of return values for a function
    # Code adapted from Mantid/Framework/PythonInterface/mantid/kernel/funcinspect.py
    ins_stack = []
    call_fun_locs = {}
    start_i = 0
    start_offset = 0
    last_i = frame.f_lasti
    for index, ins in enumerate(dis.get_instructions(frame.f_code)):
        ins_stack.append(ins)
        if ins.opname in OPERATOR_NAMES:
            call_fun_locs[start_offset] = start_i
            start_i = index
            start_offset = ins.offset
    call_fun_locs[start_offset] = start_i
    last_fun_offset = call_fun_locs[last_i]
    last_i_name = ins_stack[last_fun_offset].opname
    next_i_name = ins_stack[last_fun_offset + 1].opname
    if last_i_name == 'DICT_MERGE' and next_i_name in OPERATOR_NAMES:
        last_fun_offset += 1
        last_i = ins_stack[last_fun_offset + 1].offset
    res_i_name = ins_stack[last_fun_offset + 1].opname
    if res_i_name == 'POP_TOP':
        return 0
    elif res_i_name == 'STORE_FAST' or res_i_name == 'STORE_NAME':
        return 1
    elif res_i_name == 'UNPACK_SEQUENCE':
        return ins_stack[last_fun_offset + 1].argval
    elif res_i_name == 'LOAD_FAST' or res_i_name == 'LOAD_NAME':
        return 1  # Dot-assigment to a member or in a multi-line call
    elif res_i_name == 'DUP_TOP':
        raise NotImplementedError('libpymcr does not support multiple assignment')
    else:
        return 1  # Probably in a multi-line call


def get_nlhs(name):
    # Tries to get the number of return values for a named (Matlab) function
    # Assumes that it's called as a method of the `m` or Matlab() object

    def get_branch_of_call(astobj, parent=[]):
        if isinstance(astobj, ast.Call) and isinstance(astobj.func, ast.Attribute) and astobj.func.attr == name:
            return astobj
        for x in ast.iter_child_nodes(astobj):
            rv = get_branch_of_call(x, parent)
            if rv:
                parent.append(astobj)
                return parent
        raise SyntaxError('Empty syntax tree')

    def get_nret_from_call(caller):
        if isinstance(caller, ast.Call):
            return 1                              # f1(m.<func>())
        elif isinstance(caller, ast.Assign):
            targ = caller.targets[0]
            if isinstance(targ, ast.Tuple):
                return len(targ.elts)             # x, y = m.<func>()
            elif isinstance(targ, ast.Name):
                return 1                          # x = m.<func>()
        elif isinstance(caller, ast.Expr):
            return 0                              # m.<func>()
        elif isinstance(caller, ast.Compare):
            return 1                              # x == m.<func>()
        else:
            return 1

    # First gets the Python line where its called, then convert it to an abstract syntax
    # tree and parse that to get the branch which leads to this call (in reverse order)
    # The first element of this branch is the direct caller of this function
    frame = inspect.currentframe().f_back.f_back
    call_line = linecache.getline(frame.f_code.co_filename, frame.f_lineno)
    try:
        ast_branch = get_branch_of_call(ast.parse(call_line))
    except SyntaxError:
        return get_nret_from_dis(frame)
    else:
        return get_nret_from_call(ast_branch[0])


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
