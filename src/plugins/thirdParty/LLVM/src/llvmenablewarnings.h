/*******************************************************************************

Copyright The University of Auckland

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*******************************************************************************/

//==============================================================================
// LLVM enable warnings
//==============================================================================

#if defined(Q_OS_WIN)
    #pragma warning(pop)
#elif defined(Q_OS_LINUX)
    #pragma GCC diagnostic error "-Wmissing-field-initializers"
    #pragma GCC diagnostic error "-Wstrict-aliasing"
    #pragma GCC diagnostic error "-Wunused-parameter"
#elif defined(Q_OS_MAC)
    #pragma GCC diagnostic error "-Wunused-parameter"
#else
    #error Unsupported platform
#endif

//==============================================================================
// End of file
//==============================================================================
