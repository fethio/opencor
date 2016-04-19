/*******************************************************************************

Licensed to the OpenCOR team under one or more contributor license agreements.
See the NOTICE.txt file distributed with this work for additional information
regarding copyright ownership. The OpenCOR team licenses this file to you under
the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

*******************************************************************************/

//==============================================================================
// Compiler mathematical functions
//==============================================================================

#pragma once

//==============================================================================

#include "compilerglobal.h"

//==============================================================================

#include <QtGlobal>

//==============================================================================

extern "C" double COMPILER_EXPORT compiler_fabs(double pNb);

extern "C" double COMPILER_EXPORT compiler_log(double pNb);
extern "C" double COMPILER_EXPORT compiler_exp(double pNb);

extern "C" double COMPILER_EXPORT compiler_floor(double pNb);
extern "C" double COMPILER_EXPORT compiler_ceil(double pNb);

extern "C" double COMPILER_EXPORT compiler_factorial(double pNb);

extern "C" double COMPILER_EXPORT compiler_sin(double pNb);
extern "C" double COMPILER_EXPORT compiler_sinh(double pNb);
extern "C" double COMPILER_EXPORT compiler_asin(double pNb);
extern "C" double COMPILER_EXPORT compiler_asinh(double pNb);

extern "C" double COMPILER_EXPORT compiler_cos(double pNb);
extern "C" double COMPILER_EXPORT compiler_cosh(double pNb);
extern "C" double COMPILER_EXPORT compiler_acos(double pNb);
extern "C" double COMPILER_EXPORT compiler_acosh(double pNb);

extern "C" double COMPILER_EXPORT compiler_tan(double pNb);
extern "C" double COMPILER_EXPORT compiler_tanh(double pNb);
extern "C" double COMPILER_EXPORT compiler_atan(double pNb);
extern "C" double COMPILER_EXPORT compiler_atanh(double pNb);

extern "C" double COMPILER_EXPORT compiler_sec(double pNb);
extern "C" double COMPILER_EXPORT compiler_sech(double pNb);
extern "C" double COMPILER_EXPORT compiler_asec(double pNb);
extern "C" double COMPILER_EXPORT compiler_asech(double pNb);

extern "C" double COMPILER_EXPORT compiler_csc(double pNb);
extern "C" double COMPILER_EXPORT compiler_csch(double pNb);
extern "C" double COMPILER_EXPORT compiler_acsc(double pNb);
extern "C" double COMPILER_EXPORT compiler_acsch(double pNb);

extern "C" double COMPILER_EXPORT compiler_cot(double pNb);
extern "C" double COMPILER_EXPORT compiler_coth(double pNb);
extern "C" double COMPILER_EXPORT compiler_acot(double pNb);
extern "C" double COMPILER_EXPORT compiler_acoth(double pNb);

extern "C" double COMPILER_EXPORT compiler_arbitrary_log(double pNb, double pBase);

extern "C" double COMPILER_EXPORT compiler_pow(double pNb1, double pNb2);

extern "C" double COMPILER_EXPORT compiler_multi_min(int pCount, ...);
extern "C" double COMPILER_EXPORT compiler_multi_max(int pCount, ...);

extern "C" double COMPILER_EXPORT compiler_gcd_multi(int pCount, ...);
extern "C" double COMPILER_EXPORT compiler_lcm_multi(int pCount, ...);

//==============================================================================
// End of file
//==============================================================================
