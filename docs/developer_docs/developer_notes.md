# libpymcr Developer notes

## Overview

`libpymcr` is a CPython extension module to run a compiled Matlab (`mcr`) app as an alternative to the
[official Matlab-Python interface](https://www.mathworks.com/help/compiler_sdk/python_packages.html)
using the Matlab Compiler SDK C++ API directly.

There are two main C++ classes, and a mex function:

* `libpymcr::pymat_converter` in [type_converter.hpp](../../src/type_converter.hpp)
  which converts data from Python to Matlab and vice versa
* `libpymcr::matlab_env` in [libpymcr.hpp](../../src/libpymcr.hpp)
  which initializes the Matlab environment and runs command via `feval`.
* `call_python` mex file in [call_python.cpp](../../src/call_python.cpp)
  which allows Python callable objects to be called from within Matlab

as well as Python wrappers for these.

The `feval` API method can execute **functions** exported by the user
when they "compile" a Matlab app using the `mcc` command.

"Compilation" generates a `.ctf` (Component Technology File)
which is a zip archive containing metainformation and encrypted m-files as described in detail below.

There are some restrictions associated with the `ctf` archives:

* A `ctf` archive generated by one Matlab release (e.g. R2020a)
  cannot be read by the Matlab Compiler Runtime (MCR) for another release (e.g. R2022b)
* You must compile all the m-files you need with a single run of `mcc`
  e.g. you cannot run `mcc` on half the files, then `mcc` again on the other half
  and manually combine the two sets of encrypted m-files in a single zipped `ctf`.

`mcc` allows you to include any type of Matlab files including class definitions,
but only specific named functions in the `mcc` command are exported and are callable by `feval`.
The list of these "`public-functions`" is given in `.META/manifest.xml` and can be extracted
by the Python function `libpymcr.Matlab.get_matlab_functions()`.

This restriction does not apply to the compiled Matlab m-files which can internally call
any Matlab system function or classes as well as all included m-files.
We thus get around the "`public-functions`" restriction,
and also allow things like class constructors to be called
by defining a gateway function [`call.m`](../../src/call.m)
which internally calls the Matlab `feval`.

`call.m` in addition also:

* Wraps and unwraps "old-style" Matlab classes
  (defined only by `@`-files or directories not by `classdef`) which would otherwise be
  interpreted as a `struct` and converted to a Python `dict` on output,
  losing connection with their methods.
  The wrapping is done by creating a `containers.Map` with one key-value pair where the
  value is the old-style object and the key is `'wrapped_oldstyle_class'`.
  A `containers.Map` is interpreted as a new-style `object` so is wrapped in a Python
  opaque class (see type conversion below).
* Unwraps Python callables and calls them using the `call_python` mex file.
  The wrapping is done in `libpymcr::matlab_env` and involves creating a `struct` with two fields:
  + `func_ptr` which holds the memory address (pointer) of the Python function.
  + `converter` which holds the memory address (pointer) of the `libpymcr::pymat_converter` object
  This is passed to the `call_python` mex file which converts them into a `PyObject`
  and `pymat_converter` instance, executes the Python code and converts the results to Matlab format.

Finally, there is a Python class, `libpymcr.MatlabProxyObject`
which overloads various Python dunder methods with equivalent calls to Matlab methods
to allow the "opaque" Matlab objects to be inspected from Python.


## Type conversion

The core of the module is the `libpymcr::pymat_converter` [class](../../src/type_converter.hpp)
which converts data from Python to Matlab and vice versa, according to:

| Python      |to| Matlab  ||  Matlab     |to| Python    |
| ----------: |-| -------- |-| ----------: |-| --------- |
| `list`       || `cell`   || `cell`       || `list`     |
| `tuple`      || `cell`   ||              ||            |
| `dict`       || `struct` || `struct`     || `dict`     |
| `str`        || `char`   || `char`       || `str`      |
|              ||          || `string`     || `str`      |
| callable     || `handle` ||              ||            |
|              ||          || `object`     || opaque     |
| `None`       || `[]`     ||              ||            |
|numeric scalar|| `double` ||numeric scalar|| `np.array` |
| `np.array`   || `array`  || `array`      || `np.array` |


Note that:

* Single type `list`s or `tuple`s are converted to the corresponding type Matlab `array` and _not_ `cell`.
* Elements within containers (`list`, `tuple`, `cell`, `dict`, `struct`) are converted recursively.
* Matlab tends to use `double` for all numeric types so any Python numeric scalars (regardless of type) are
  converted to `double`. Because elements in containers are converted recursively, a Python `list` or `tuple`
  of numbers (of one or mixed types) will be converted to a Matlab `double array`.
* Matlab objects are wrapped in a Python-opaque object (unless the `call.m` gateway function is used, see below).
  This object can be passed back to Matlab for further processing but cannot be processed in Python
* Python objects cannot be passed to Matlab, except for callable Python classes (e.g. those with `__call__`)
  if the `call.m` gateway function and the `call_python` mex function are used.
* Numpy `np.array` and Matlab `array` have their underlying data wrapped in the output format.
    + Numpy arrays support views where the data is not owned by the numpy object. `libpymcr` implements the
      wrapping as a view with the owner being the (Python-opaque) Matlab `array` object.
    + As such all conversions from Matlab-to-Python will be wrapped with no data copies.
    + For the reverse (Python-to-Matlab), the Matlab API does not support views, so a hack is used 
      as detailed below.
    + In addition, Matlab has limited support for non-contiguous arrays, so only contiguous arrays are
      wrapped. Non-contiguous numpy arrays are instead copied into a new Matlab `array`.
    + Another restriction is that Matlab does not support general strides like numpy
      ([details here](https://numpy.org/doc/stable/reference/arrays.ndarray.html#internal-memory-layout-of-an-ndarray))
      but rather only the specific stride patterns corresponding to numpy's `C-contiguous` or `F-contigous`.
      This only applies for higher dimensional (D>2) arrays, but in cases where the strides are not
      in either of these formats `libpymcr` will copy the data when converting from Python-to-Matlab.
    + Finally, whilst `libpymcr` will wrap C-contiguous (row-major) arrays, many internal Matlab numeric
      functions only work with F-contiguous (column-major) arrays so Matlab may force a data transpose copy


## Mac OS notes

On Mac OS, the Matlab GUI uses the Cocoa framework, and requires:

1. A main event loop to be run using the `mclRunMain` (C API) or `cpplib::runMain` (C++ API).
   This should be passed a `main(argc, argv)` function which when it exits will end the Matlab session.
2. Associated property list (`Info.plist`) metadata.

The first requirement seems to be an implementation detail of Matlab -
it's not clear that the Cocoa framework actually requires this.
Apple does require the second condition for GUI application, however.
Unfortunately, Python is not strictly a GUI application so it does not provide an associated `Info.plist`.

The only way to use the approved API with Matlab graphics on Mac OS thus seems to be to
create a standalone executable "app" to then initialise the Python interpreter and
run the main event loop in a thread.
Some [code](https://github.com/pace-neutrons/libpymcr/commit/eec2a1cab354d5e0a6c71c2ef859a8caf93b9a15)
to this effect has been written,
but it was decided that this was too much effort for now,
and instead a workaround which disables graphics (by passing the `-nojvm` option on startup) was adopted.

The official Matlab-Python compiler SDK *does* allow graphics, apparently without a standalone app,
so it maybe that there are unofficial functions in the Matlab libraries to accomplish this,
which will require further investigations for us to make use of.


## Encryption notes

As described in the overview, the "compiled" Matlab app is shipped as a Component Technology File (`ctf`),
which is a zip archive containing metadata and encrypted m-files.

It seems that Matlab uses a relatively standard two step process for the encryption:

1. An 256-bit AES `session-key` is generated each time `mcc` is run and is used to encrypt the m-files.
2. A public 2048-bit (1024-bit before R2020a) RSA `public-key` is used to encrypt the `session-key`.
3. Both the `session-key` and `public-key` is stored in the `.META/manifest.xml` file as raw HEX strings.

For decryption:

0. The zipped `ctf` is inflated in a temporary folder with the m-files still encrypted.
1. An internally stored 2048-bit RSA `private-key` is used to decrypt the `session-key` in the `ctf`.
2. The decrypted `session-key` is used to decrypt the m-files _in memory_ and to run them.

Inspection of generated `ctf` show that the `public-key` did not change from R2020a to R2022b,
but was different for R2019b (and presumably had not changed before)
when it was changed from a 1024-bit to a 2048-bit key.

Since only the public key is needed to encrypt, we could in principle generate our own `ctf` by
creating a session key, encrypting the m-files and then encrypt the session key with the public key.
In addition, since R2022b, `mcc` allows an option (`-k`) for the user to specify their own AES encryption key.
Thus, it seems that the purpose of the encryption in the `ctf` is more to keep the m-file code private
(from the contents of the m-file alone it cannot be decrypted without the private key)
rather than to lock the system down entirely such that `mcc` must be used.

Actually generating our own valid `ctf` archives will need further investigation
because it seems that the m-file contents is compressed first somehow before encryption.

In addition, there is an unknown header for each encryped m-file which varies from Matlab release to release.
If this header could be determined, it may also be possible to modify a `ctf` (without decryption)
to work for a different Matlab release from which it was "compiled".

The `session-key` in the `.META/manifest.xml` file is a 2048-bit HEX encoded number,
the direct output of the RSA encryption of the AES-256 key.

The `public-key` in the `.META/manifest.xml` is a HEX encoded ASN.1 serialised PKCS#1 public key.
It can be extracted and used to generate a random AES-256 key with the following Python code:

```python
# pip install pycryptodome
import zipfile
from Crypto.PublicKey import RSA
from Crypto.Random import get_random_bytes
from Crypto.Cipher import AES, PKCS1_OAEP

with zipfile.ZipFile('file.ctf', 'r') as ctf:
    manifest = ctf.read('.META/manifest.xml').decode('ascii')

tags = manifest.split('><')
ky_str = [t for t in tags if 'public-key' in t][0].split('>')[1].split('<')[0]
rsa_key = RSA.import_key(bytearray.fromhex(ky_str))

session_key = get_random_bytes(32)
enc_session_key = PKCS1_OAEP.new(rsa_key).encrypt(session_key)
sk_str = ''.join(f'{x:02x} for x in enc_session_key)
```
