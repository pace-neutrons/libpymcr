# [v0.2.1](https://github.com/pace-neutrons/libpymcr/compare/v0.1.8...v0.2.1)

## New Features

Adds a convenience function to create Matlab `function_handle`s in Python using the "@" (matrix-multiplication) operator:

```
sqw_fun = m @ spinwobj.horace_sqw
```

Note that the LHS must be the Matlab gateway object (`m = libpymcr.Matlab()`) and the RHS must be a proxied method of a Matlab object.
Also, note that Matlab syntax like `fun = @(x) x+1` is not valid in Python, and should be constructed as `m.str2func('@(x)x+1')`.

## Bugfixes

* Fix changed `mxArray` layout in R2023b and newer versions of Matlab, so that wrapping arrays work again.
* Nested calls to Matlab in Python functions called from Matlab now results in a `RuntimeError` rather than just hanging.
* Pressing `Ctrl+C` in Python will now interrupt Matlab execution (but will not be instantaneous)
* Matlab outputs now don't have extraneous newlines and look more like in Matlab.
* Update code to handle numpy v2
* Update code to handle new Matlab buffer syntax change in R2024b
* Refactor `call_python` to avoid ABI change between Python 3.11 and 3.12
* Fix a GIL deadlock error when using `call_python` with new interrupt (`Ctrl+C`) mechanism.

## For Developers

A pair of new utility functions, `stripmex` and `recombinemex` to remove non-encoded files from the `ctf` archive was added.
This is needed because different Matlab releases use different encoding keys so a `ctf` is needed for each release.
Bundling all these into a single wheel can be wasteful as large files (e.g. `mex`) are not encoded.
So, they can be removed from the `ctf`, bundled separately and recombined in the user's installation later.


# [v0.1.8](https://github.com/pace-neutrons/libpymcr/compare/v0.1.7...v0.1.8)

## Bugfixes

* Implements a workaround for a changed `mxArray` layout in R2023b and newer (disables wrapping of Matlab arrays in Python).
* Fixes issues with inline plots in Jupyter notebooks
* Fix an issue with errors for Matlab functions which do not return any values, and an issue with recursive dot indexing

# [v0.1.7](https://github.com/pace-neutrons/libpymcr/compare/v0.1.6...v0.1.7)

## New Features

Add a command `matlab2python` which generates Python wrappers for Matlab code in a specified directory (syntax is similar to `mcc`).
By default the Python wrappers are created in a package folder called `matlab_wrapped`.
If you have a package and want to call the Matlab functions without the `m.` prefix, you can do:

```
from matlab_wrapped import *
```

Matlab class properties which are standard types (`dict`s, `list`s and numpy arrays) are now better supported by a fix to `DictPropertyWrapper` and a new `VectorPropertyWrapper`, which allows syntax like:

```
obj.prop['a'] = 1
obj.prop[1] = 2
```

Matlab column vectors can be indexed with a single index (rather than requiring a tuple as in `obj.prop[1,2]` which numpy requires).

## Bugfixes

Bugfixes for some issues arising from testing in preparation for EDATC2

* Update matlab checker to be more robust and to output a warning of a licensed Matlab was found but the Compiler SDK toolbox is not installed.
* Fix `MatlabProxyObject` dunder methods to work with libpymcr.
* Fix `MatlabProxyObject` indexing bug where libpymcr converted the old list to a vector instead of a cell-array (now uses a tuple).
* Fix a bug in `call.m` where `nargout` is confused if the called function is shadowed (e.g. `fit`) and it could not determined the maximum `nargout`.

# [v0.1.6](https://github.com/pace-neutrons/libpymcr/compare/v0.1.5...v0.1.6)

## Bugfixes

Bugfixes for various user reported issues when used with pace-python and PySpinW.

* Add search of `matlab` executable on path for Linux.
* Add a `type` method to interrogate Matlab type (fixes issue with [pace-python-demo example](https://github.com/pace-neutrons/pace-python-demo/blob/main/demo.py#L86))
* Add a proxy to allow plain struct properties of a class to be manipulated like in Matlab (so `class.prop.subprop = 1` will work if `class.prop` is a plain Matlab `struct`).
* Change to using `evalAsync` in Matlab and as part of this polls the output streams every 1ms and prints output to Python - this allows synchronous output to both console, Jupyter and Spyder without additional code.


# [v0.1.5](https://github.com/pace-neutrons/libpymcr/compare/v0.1.4...v0.1.5)

## Bugfixes for PySpinW

Bugfixes for PySpinW identified by users and during the RAL-India workshop.

* Fix a segfault when temporary numpy arrays are re-used multiple times in definition of SpinW objects
* Change behaviour of tuples and lists. Python tuples now always convert to Matlab cells. Nested lists will convert to Matlab numeric arrays if they are consistent in shape and contain only numbers. This allows Python `([1,2,3], [4,5,6])` to convert to Matlab `{[1 2 3] [4 5 6]}` whereas before it would have converted to Matlab `[1 2 3; 4 5 6]`. Python `[[1,2,3], [4,5,6]]` will still convert to Matlab `[1 2 3; 4 5 6]`.
* Fix bug where Matlab commands which return zero outputs fail, e.g. `m.axis([0,1,0,1])` due to incorrectly given `nargout` in Matlab.py / call.m
* New `get_nlhs` algorithm to set `nargout` parameters uses `ast` to better parse cases of nested Matlab calls, like `m.eig(m.rand(3))` but this only works if the full command is on one line; and also does not work for the basic interpreter (but should work in Mantid, Jupyter and Spyder). For those cases, the old `dis` algorithm is used, updated to work with Python 3.11 but does not handle nested calls.
* Fix bugs in `call_python`. Changed mechanism back to using a global `dict` as direct memory address was failing in some nested calls. `call_python` now creates its own instance of `type_converter` to avoid a heap memory error when using the Python initialized object from the `libpymcr` instance. Re-add `DEEPBIND` library loading for Linux Lapack/BLAS to avoid conflict with Matlab.


# [v0.1.4](https://github.com/pace-neutrons/libpymcr/compare/v0.1.3...v0.1.4)

## Add IPython magics

Add IPython magics from pace-python (to allow in-line text output and figures in Jupyter notebooks).


# [v0.1.3](https://github.com/pace-neutrons/libpymcr/compare/v0.1.2...v0.1.3)

## Bugfixes for PySpinW

Further bugfixes for PySpinW - now automatically converts unnested lists and numpy vectors to *row-vectors* in Matlab to be consistent with Matlab convention (previously had converted to column vectors using Fortran convention).


# [v0.1.2](https://github.com/pace-neutrons/libpymcr/compare/v0.1.1...v0.1.2)

## Bugfixes for PySpinW

A set of bugfixes and a new feature for the PySpinW beta

* Fix error when caller is `a.bc(de, f=gh)`
* Fix typo in nested dot indexing call
* Add new automatic conversion of nested lists into N-D matlab arrays

E.g.


```
m.eig([[1,2,3], [4,5,6], [7,8,9]])
```

now correctly converts the nested list into a 3x3 matrix and the `eig` function no longer gives an error.


# [v0.1.1](https://github.com/pace-neutrons/libpymcr/compare/v0.1.0...v0.1.1)

## Initial public beta of libpymcr

Basic support for running compiled Matlab apps within Python. Example:

```
from libpymcr import _libpymcr
m = _libpymcr('/path/to/ctf')
m.magic(3)
```

would yield:

```
array([[8., 1., 6.],
       [3., 5., 7.],
       [4., 9., 2.]])
```

if you included the `magic` function in your `ctf` archive.

There is a gateway function `call.m` which if you include in your `ctf` archive
will allow any Matlab built-in functions to be called (internally using `feval`).

`libpymcr` runs on Python 3.7 -- 3.11, and Matlab releases R2020a -- R2022b.

# [v0.1.0](https://github.com/pace-neutrons/libpymcr/compare/08b2c72...v0.1.0)

Extracted the core Matlab-Python interface and moved code from [pace-python](https://github.com/pace-neutrons/pace-python).

`pace-python` history from before fork below:

### [v0.2.0](https://github.com/pace-neutrons/pace-python/compare/v0.1.4...v0.2.0)

Updated to Horace 3.6.2

* New features
  - Add support for Horace parallel framework. Use `m.hpc('on')` to activate and then set the cluster type `m.hpc_config().parallel_cluster = <x>` where `<x> = 'parpool'`, `'herbert'` or `'mpiexec_mpi'`.

* Bugfixes
  - Fix bug where Matlab `+namespaces` were not accessible in Python
  - Several small bugs when using with Spyder.

### [v0.1.4](https://github.com/pace-neutrons/pace-python/compare/v0.1.3...v0.1.4)

* New features
  - New `pace_neutrons` wrapper script to launch PACE, setting all needed paths
  - Can also launch jupyter or spyder with `pace_neutrons --spyder` or `pace_neutrons --jupyter`
  - Facility to download and install MCR automatically or on first use

* Bugfixes
  - If neither spyder or jupyter installed will use IPython (now a dependency)
  - Fix logic error in searching for MCR runtime DLL. Will use registry on Windows.

### [v0.1.3](https://github.com/pace-neutrons/pace-python/compare/v0.1.1...v0.1.3)

Updated to Horace 3.6.1.

* Bugfixes
  - Fix MatlabProxyObject `__setattr__` - can now do e.g. `h = m.herbert_config(); h.use_mex = True`
  - Fix `worker_v2 not found in initialisation` error
  - Fix DataTypes.py encoding list recursion.

### [v0.1.1](https://github.com/pace-neutrons/pace-python/compare/v0.1.0...v0.1.1)

Initial public beta version of pace_python.

Please download the `pace_python_matlab_installer` file for your OS and run it. This will install the Matlab Runtime and a GUI installer app. Run the installer app (`pace_python_installer`) which will install miniconda and the `pace_python` module (if you have an existing Python installation you wish to use, select `Custom Installation` in the app).

If you select `Jupyter` and/or `Spyder` (both recommended) in the `Default Installation` option, then after the installation finishes, you will see links to Jupyter and Spyder in your start menu (on Windows) which you can use to start Jupyter/Spyder with pace-python. Then run:

```
import pace_python
m = pace_python.Matlab()
m.horace()
```

To start the Horace GUI.

If you have any problems, please create an issue on this repository.
