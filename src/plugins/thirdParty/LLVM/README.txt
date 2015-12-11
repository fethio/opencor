We need to build LLVM (incl. Clang) ourselves so that it can be used as a 'proper' plugin within OpenCOR. The problem with the latter is that it's not possible to build LLVM as a shared library using MSVC (because of the need to expose classes/methods as __declspec(dllexport) or __declspec(dllimport) depending on whether LLVM is being built or referenced, respectively).

To address this shortcoming, we must 'manually' expose the classes/methods that are needed by OpenCOR (grep for ---OPENCOR--- to find out which ones those are). Should OpenCOR ever need direct access to another class/method, then that class/method will obviously have to be exposed, and LLVM rebuilt. In addition to exposing some classes/methods, we also had to 'fix' a few things. Again, those are highlighted (just look for ---OPENCOR---).

In case LLVM is to be built (by setting the USE_PREBUILT_LLVM_PLUGIN option to OFF), then keep in mind the following:
 - Configuration files are to be generated using CMake (on Windows, this may require installing Python 2.7.x, which can be downloaded from http://www.python.org/download/):
    1) Download the LLVM and Clang source codes from http://llvm.org/releases/download.html
    2) Uncompress their corresponding tar.xz file
    3) Move the contents of [Clang] to [LLVM]/tools/clang
    4) From the command line:
           cd [LLVM]
           mkdir build
           cd build
           cmake -G "Ninja" ..   OR   cmake -G "Unix Makefiles" ..   OR   cmake -G "NMake Makefiles JOM" ..
           ninja                 OR   make                           OR   jom
       From there, the configuration files are to be manually copied over to the windows, linux and osx folders of the various folders.
 - To keep the LLVM build as simple as possible (and this without removing any of LLVM's features), [LLVM]/include/llvm/Config/[PLATFORM_DIR]/config.h must be edited so that neither HAVE_LIBEDIT nor HAVE_TERMINFO are set, and LLVM_ENABLE_ZLIB is set to 0. To do anything else would require a more complicated build process and it is not worth it.
 - OpencOR is currently supported on Windows, Linux and OS X, so we only need to target X86 at this stage.
