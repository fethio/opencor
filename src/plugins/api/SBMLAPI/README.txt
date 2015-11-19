We build our own copy of libSBML, which requires the following:
 - libSBML's core source code (see http://sourceforge.net/projects/sbml/files/libsbml/):
    - Linux: replace "isnan" with "std::isnan" in src/sbml/math/ASTNumber.cpp (lines 1732 and 1741)
 - libSBML dependencies (Windows only; the contents of which is to be located under [libSBML]/dependencies):
    - Windows 32-bit:
       - Release: http://sourceforge.net/projects/sbml/files/libsbml/win-dependencies/libSBML_dependencies_vs12_release_x86.zip
       - Debug:   http://sourceforge.net/projects/sbml/files/libsbml/win-dependencies/libSBML_dependencies_vs12_debug_x86.zip
    - Windows 64-bit:
       - Release: http://sourceforge.net/projects/sbml/files/libsbml/win-dependencies/libSBML_dependencies_vs12_release_x64.zip
       - Debug:   http://sourceforge.net/projects/sbml/files/libsbml/win-dependencies/libSBML_dependencies_vs12_debug_x64.zip

From there, using cmake-gui on Windows or ccmake on Linux / OS X, we want to:
 - Customise libSBML:
    ===> WITH_BZIP2=ON
    ===> WITH_CPP_NAMESPACE=ON
    ===> WITH_EXPAT=OFF
    ===> WITH_LIBXML=ON
    ===> WITH_SWIG=OFF
    ===> WITH_XERCES=OFF
    ===> WITH_ZLIB=ON
 - Ask for a 'Release' build (or a 'Debug' build on Windows since both 'Release' and 'Debug' binaries are needed on that platform):
    ===> CMAKE_BUILD_TYPE=Release
 - Ask for C++11 compilation to be used on Linux / OS X (on Windows, MSVC uses C++11 by default) (this is part of the advanced settings):
    - Linux:
       ===> CMAKE_CXX_FLAGS=-std=c++0x
    - OS X:
       ===> CMAKE_CXX_FLAGS=-std=c++0x -stdlib=libc++
 - Update the installation destination so that, upon 'installation', we have a ready to use version of the libSBML binaries (this is particularly useful on OS X since the 'installation' will result in 'clean' binaries):
    ===> CMAKE_INSTALL_PREFIX=<InstallationDestination>

Once we have built and 'installed' libSBML, we can replace the OpenCOR version of the include and binary files, which are located in the 'installed' include and lib folders, respectively.
