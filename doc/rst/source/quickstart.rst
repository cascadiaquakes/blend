Quick Start
===========

Build and install BLEND
-----------------------

Download the source, copy the user configuration template once, edit it if
needed, and configure from a separate build directory:

.. code-block:: sh

   git clone https://github.com/cascadiaquakes/blend.git
   cd blend
   cp cmake/ConfigUserTemplate.cmake cmake/ConfigUser.cmake
   mkdir build
   cd build
   cmake ..
   cmake --build .
   cmake --build . --target install

After installation, add the installation ``bin`` directory to your ``PATH`` and
check the following commands:

Show version
------------

.. code-block:: sh

   blend --version

Show modules
------------

.. code-block:: sh

   blend --show-modules

Show windows
------------

.. code-block:: sh

   blend --show-windows

Make a 1-D window
-----------------

.. code-block:: sh

   blend window1d -R0/10 -I1 -Fcosine -T0.2/0.2

Query a 1-D window at arbitrary coordinates
-------------------------------------------

.. code-block:: sh

   printf "2.5\n" | blend window1d -R0/10 -I1 -Fcosine -T0.2/0.2

Make a 2-D window
-------------------------

.. code-block:: sh

   printf "1 1\n4 1\n4 4\n1 4\n" > polygon.txt
   printf "polygon.txt cosine/cosine 0.2/0.2/0.2/0.2\n" > supports.txt
   blend window2d -R0/5/0/5 -I1 -Bsupports.txt

If the polygon does not satisfy the xy-monotonicity requirement, use
``-ME`` or ``-MB`` to let BLEND refine the support:

.. code-block:: sh

   blend window2d -R0/5/0/5 -I1 -Bsupports.txt -MB
