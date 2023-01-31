# libpymcr

*libpymcr* is a Python module for loading a compiled Matlab `ctf` archive and running functions within it in Python.

Whilst there is an [official Mathworks binding for Python](https://uk.mathworks.com/help/compiler_sdk/python_packages.html),
*libpymcr* provides a few benefits over the official package:

* It is compatible with versions of Python not supported by Matlab (e.g. 3.10, 3.11, 3.12). Moreover, the supported Python
  versions are not locked to the Matlab versions, unlike with the official packages.
* When converting data between Matlab and Python it avoids data copies where ever possible by wrapping the underlying arrays
  in the target type (`numpy` or Matlab `mxArray`). In the official bindings, a data copy is required when converting data
  from Python to Matlab (inputs to functions).
* It provides a simpler syntax, and if you include the provided `call.m` and `call_python.mex` files in your compiled
  package, you will also be able to access Matlab objects transparently in Python, and pass Python callables to Matlab
  to evaluate (e.g. in a fitting routine).
  
## Getting started

You can install the package using:

```
pip install libpymcr
```

You must create a compiled Matlab archive (`ctf` file) of your program using the
[Matlab Compiler SDK toolbox](https://uk.mathworks.com/help/compiler_sdk/index.html),
using the [mcc](https://uk.mathworks.com/help/compiler_sdk/index.html) command:

```
mcc -W CTF:your_program_name -U mfile1 mfile2 mfile3
```

Then in Python, you can load this and call the Matlab functions with:

```python
import libpymcr
m = libpymcr.Matlab('your_program_name.ctf')
m.mfile1()
```

The functions, `mfile1`, `mfile2` etc. are exposed to Python can can be called as methods of the `Matlab()` object.
