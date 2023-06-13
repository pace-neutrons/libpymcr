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
