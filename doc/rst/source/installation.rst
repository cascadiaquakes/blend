Installation
============

Dependencies
------------

To build BLEND, install:

* Git, to download the source repository
* CMake 3.15 or newer
* A C compiler
* A POSIX shell
* Ninja, optional but recommended for faster builds

To build the documentation, install:

* Sphinx
* The Read the Docs Sphinx theme, provided by ``sphinx-rtd-theme``
* LaTeX and ``latexmk`` for PDF output

Install the Read the Docs theme in the same Python environment that provides
``sphinx-build``. For example:

.. code-block:: sh

   python -m pip install sphinx-rtd-theme

or, with conda:

.. code-block:: sh

   conda install -c conda-forge sphinx_rtd_theme

If you use a conda environment or another Python environment for documentation
tools, activate it before configuring BLEND:

.. code-block:: sh

   conda activate my-docs-env
   mkdir build
   cd build
   cmake ..

Alternatively, point CMake directly to the ``sphinx-build`` executable from
that environment:

.. code-block:: sh

   cmake .. -DSPHINX_BUILD_EXECUTABLE=/path/to/conda/env/bin/sphinx-build

The important point is that ``sphinx-build`` and ``sphinx-rtd-theme`` must come
from the same Python environment.

Download BLEND
--------------

Download the source from GitHub:

.. code-block:: sh

   git clone https://github.com/cascadiaquakes/blend.git
   cd blend

To update an existing source tree later, run ``git pull`` from the BLEND source
directory.

Configuration
-------------

BLEND follows a GMT-like configuration style. Defaults live in
``cmake/ConfigDefault.cmake``. Local user choices live in
``cmake/ConfigUser.cmake``. Do not edit ``ConfigDefault.cmake``; create and
edit ``ConfigUser.cmake`` instead.

Create the user configuration file:

.. code-block:: sh

   cp cmake/ConfigUserTemplate.cmake cmake/ConfigUser.cmake

Set the installation prefix in ``cmake/ConfigUser.cmake`` if you do not want
to install under the default prefix:

.. code-block:: cmake

   set (CMAKE_INSTALL_PREFIX "/path/to/blend/install")

Other common settings include:

.. code-block:: cmake

   set (BLEND_BUILD_TESTS ON)
   set (BLEND_BUILD_EXAMPLES ON)
   set (BLEND_BUILD_DOCS ON)

See the comments in ``cmake/ConfigUserTemplate.cmake`` for additional install
directories and static/shared library options.

Build
-----

Configure from a separate build directory:

.. code-block:: sh

   mkdir build
   cd build
   cmake ..
   cmake --build .

To use Ninja:

.. code-block:: sh

   mkdir build
   cd build
   cmake .. -G Ninja
   cmake --build .

This builds the BLEND static library, shared library, command-line programs, and
any optional targets enabled in ``cmake/ConfigUser.cmake``.

Run Tests
---------

If ``BLEND_BUILD_TESTS`` is ``ON`` in ``cmake/ConfigUser.cmake``, run tests
from the build directory after ``cmake --build .`` has completed:

.. code-block:: sh

   ctest

If ``BLEND_BUILD_TESTS`` was turned on after the build directory already
existed, rerun ``cmake ..`` and ``cmake --build .`` before running ``ctest``.

Build Documentation
-------------------

If ``BLEND_BUILD_DOCS`` is ``ON`` in ``cmake/ConfigUser.cmake``, build the
documentation from the build directory.

To build both the HTML documentation and the PDF manual:

.. code-block:: sh

   cmake --build . --target docs

To build only the HTML documentation:

.. code-block:: sh

   cmake --build . --target docs_html

To build only the PDF manual:

.. code-block:: sh

   cmake --build . --target docs_pdf

The HTML documentation is written to ``doc/html`` inside the build directory.
Open ``doc/html/index.html`` in a browser to view it. The PDF manual is copied
into the HTML tree as ``_downloads/blend.pdf``, and the front page includes a
download link.

If ``BLEND_BUILD_DOCS`` was turned on after the build directory already
existed, rerun ``cmake ..`` before building the documentation targets. The
``docs_pdf`` target is available only when ``latexmk`` is found during
configuration.

Install
-------

.. code-block:: sh

   cmake --build . --target install

This installs the command-line program, libraries, public headers, CMake
package files, examples, and documentation files.

Depending on the installation location, you may need write permission for this
step.

After installation, make sure the installation ``bin`` directory is in your
shell search path. If ``CMAKE_INSTALL_PREFIX`` is set to
``/path/to/blend/install``, add this to your shell startup file:

.. code-block:: sh

   export PATH="/path/to/blend/install/bin:$PATH"

Then open a new terminal, or reload the startup file, and check the command:

.. code-block:: sh

   blend --version

Update an Existing Installation
-------------------------------

If BLEND was already installed and you want to update to the latest version
from Git, pull the latest changes, update the build directory, rebuild, and
install again:

.. code-block:: sh

   cd /path/to/blend
   git pull
   cd build
   cmake ..
   cmake --build .
   cmake --build . --target install

If you use a local ``cmake/ConfigUser.cmake``, keep it in place so the updated
installation uses the same install prefix and options as before.

Uninstall
---------

The install step also installs a helper script. Run it from the installation
prefix to remove BLEND files:

.. code-block:: sh

   ./share/tools/blend_uninstall.sh

You can also preview the files that would be removed without actually deleting them
by adding the ``--dry-run`` option:

.. code-block:: sh

   ./share/tools/blend_uninstall.sh --dry-run

Use BLEND from Another CMake Project
------------------------------------

Installed CMake projects can import BLEND with:

.. code-block:: cmake

   find_package(blend CONFIG REQUIRED)
   target_link_libraries(my_target PRIVATE BLEND::blend)

Use the installed public header as:

.. code-block:: c

   #include <blend/blend.h>
