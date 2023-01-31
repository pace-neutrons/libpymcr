Installation
============


.. contents:: Contents
   :local:


``libpymcr`` is available on the Python Package Index `(PyPI) <https://pypi.org/project/pace-neutrons>`__,
and can in principle be installed with any (CPython) distributions.

.. code-block:: sh

   pip install libpymcr



Matlab Compiler Runtime
-----------------------

``libpymcr`` relies on the Matlab Compiler Runtime (MCR).
You need to either install the full distribution from 
`here <https://uk.mathworks.com/products/compiler/matlab-runtime.html>`__,
ensuring that the MCR version matches the Matlab version you used to
compile your ``ctf`` archive, or you could also create 
`an installer <https://uk.mathworks.com/help/compiler_sdk/ml_code/compiler.package.installer.html>`__
for a dummy stand-alone package to install only the components you need.
