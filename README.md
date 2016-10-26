# Introduction {#mainpage}

## Install prerequisites

### Windows
- [Visual Studio](https://www.visualstudio.com/downloads/)

  Note: In Visual Studio 2015, Visual C++ is not installed by default.
  When installing, be sure to choose <strong>Custom</strong>
  installation and then choose the C++ components you require. Or, if
  Visual Studio is already installed, choose <strong>File | New |
  Project | C++</strong> and you will be prompted to install the
  necessary components.

- [Latest Python 2 Release](https://www.python.org/downloads/windows/)

- [SCons](http://scons.org/pages/download.html)

  Note: The SCons installer will not detect the 64-bit installation of
  Python.  Instead, download the zip file and follow the installation
  instructions in SCons README.txt.

- [libuv](http://dist.libuv.org/dist/)

- [SWIG](http://www.swig.org/download.html)

- [Doxygen](http://www.stack.nl/~dimitri/doxygen/download.html)

### Documentation
Building the documentation requires the
[DoxygenBuilder](https://bitbucket.org/scons/scons/wiki/DoxygenBuilder)
tool.  This [page](https://bitbucket.org/scons/scons/wiki/ToolsIndex)
provides instructions on how to install the builder.

## Build
To build run 'scons'.

    $ scons

