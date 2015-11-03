/*=========================================================================
*
*  Copyright Insight Software Consortium
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*         http://www.apache.org/licenses/LICENSE-2.0.txt
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*=========================================================================*/
#ifndef __sitkConfigure_h
#define __sitkConfigure_h


/* #undef SITK_BUILD_SHARED_LIBS */
#ifdef SITK_BUILD_SHARED_LIBS
#define SITKDLL
#else
#define SITKSTATIC
#endif
/* #undef SITK_SimpleITKExplit_STATIC */


// defined if the system has C++0x "static_assert" keyword
#define SITK_HAS_CXX11_STATIC_ASSERT
// defined if the compiler has C++11 "nullptr" keyword
#define SITK_HAS_CXX11_NULLPTR
#define SITK_HAS_CXX11_FUNCTIONAL
#define SITK_HAS_CXX11_TYPE_TRAITS
#define SITK_HAS_CXX11_UNORDERED_MAP

/* #undef SITK_HAS_TR1_SUB_INCLUDE */

/* #undef SITK_HAS_TR1_FUNCTIONAL */
/* #undef SITK_HAS_TR1_TYPE_TRAITS */
/* #undef SITK_HAS_TR1_UNORDERED_MAP */

#define SITK_INT64_PIXELIDS
/* #undef SITK_4D_IMAGES */

#define SITK_EXPLICIT_INSTANTIATION

/* #undef SITK_EXPRESS_INSTANTIATEDPIXELS */

#if defined(__clang__)
#define CLANG_TEMPLATE template
#else
#define CLANG_TEMPLATE
#endif

// Include ITK version reported in CMake with SITK prefix, so that
// SimpleITK dosen't need ITK header in our headers.
#define SITK_ITK_VERSION_MAJOR 4
#define SITK_ITK_VERSION_MINOR 8
#define SITK_ITK_VERSION_PATCH 1

#endif // __sitkConfigure_h
