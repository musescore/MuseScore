/* -----------------------------------------------------------------------------
Software License for The Fraunhofer FDK AAC Codec Library for Android

© Copyright  1995 - 2018 Fraunhofer-Gesellschaft zur Förderung der angewandten
Forschung e.V. All rights reserved.

 1.    INTRODUCTION
The Fraunhofer FDK AAC Codec Library for Android ("FDK AAC Codec") is software
that implements the MPEG Advanced Audio Coding ("AAC") encoding and decoding
scheme for digital audio. This FDK AAC Codec software is intended to be used on
a wide variety of Android devices.

AAC's HE-AAC and HE-AAC v2 versions are regarded as today's most efficient
general perceptual audio codecs. AAC-ELD is considered the best-performing
full-bandwidth communications codec by independent studies and is widely
deployed. AAC has been standardized by ISO and IEC as part of the MPEG
specifications.

Patent licenses for necessary patent claims for the FDK AAC Codec (including
those of Fraunhofer) may be obtained through Via Licensing
(www.vialicensing.com) or through the respective patent owners individually for
the purpose of encoding or decoding bit streams in products that are compliant
with the ISO/IEC MPEG audio standards. Please note that most manufacturers of
Android devices already license these patent claims through Via Licensing or
directly from the patent owners, and therefore FDK AAC Codec software may
already be covered under those patent licenses when it is used for those
licensed purposes only.

Commercially-licensed AAC software libraries, including floating-point versions
with enhanced sound quality, are also available from Fraunhofer. Users are
encouraged to check the Fraunhofer website for additional applications
information and documentation.

2.    COPYRIGHT LICENSE

Redistribution and use in source and binary forms, with or without modification,
are permitted without payment of copyright license fees provided that you
satisfy the following conditions:

You must retain the complete text of this software license in redistributions of
the FDK AAC Codec or your modifications thereto in source code form.

You must retain the complete text of this software license in the documentation
and/or other materials provided with redistributions of the FDK AAC Codec or
your modifications thereto in binary form. You must make available free of
charge copies of the complete source code of the FDK AAC Codec and your
modifications thereto to recipients of copies in binary form.

The name of Fraunhofer may not be used to endorse or promote products derived
from this library without prior written permission.

You may not charge copyright license fees for anyone to use, copy or distribute
the FDK AAC Codec software or your modifications thereto.

Your modified versions of the FDK AAC Codec must carry prominent notices stating
that you changed the software and the date of any change. For modified versions
of the FDK AAC Codec, the term "Fraunhofer FDK AAC Codec Library for Android"
must be replaced by the term "Third-Party Modified Version of the Fraunhofer FDK
AAC Codec Library for Android."

3.    NO PATENT LICENSE

NO EXPRESS OR IMPLIED LICENSES TO ANY PATENT CLAIMS, including without
limitation the patents of Fraunhofer, ARE GRANTED BY THIS SOFTWARE LICENSE.
Fraunhofer provides no warranty of patent non-infringement with respect to this
software.

You may use this FDK AAC Codec software or modifications thereto only for
purposes that are authorized by appropriate patent licenses.

4.    DISCLAIMER

This FDK AAC Codec software is provided by Fraunhofer on behalf of the copyright
holders and contributors "AS IS" and WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
including but not limited to the implied warranties of merchantability and
fitness for a particular purpose. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE for any direct, indirect, incidental, special, exemplary,
or consequential damages, including but not limited to procurement of substitute
goods or services; loss of use, data, or profits, or business interruption,
however caused and on any theory of liability, whether in contract, strict
liability, or tort (including negligence), arising in any way out of the use of
this software, even if advised of the possibility of such damage.

5.    CONTACT INFORMATION

Fraunhofer Institute for Integrated Circuits IIS
Attention: Audio and Multimedia Departments - FDK AAC LL
Am Wolfsmantel 33
91058 Erlangen, Germany

www.iis.fraunhofer.de/amm
amm-info@iis.fraunhofer.de
----------------------------------------------------------------------------- */

/************************* System integration library **************************

   Author(s):

   Description:

*******************************************************************************/

/** \file   machine_type.h
 *  \brief  Type defines for various processors and compiler tools.
 */

#if !defined(MACHINE_TYPE_H)
#define MACHINE_TYPE_H

#include <stddef.h> /* Needed to define size_t */

#if defined(__ANDROID__) && (__GNUC__ == 4) && (__GNUC_MINOR__ == 4) && \
    (__GNUC_GNU_INLINE__ == 1)
typedef unsigned long long uint64_t;
#include <sys/types.h>
#endif

/* Library calling convention spec. __cdecl and friends might be added here as
 * required. */
#define LINKSPEC_H
#define LINKSPEC_CPP

/* for doxygen the following docu parts must be separated */
/** \var  SCHAR
 *        Data type representing at least 1 byte signed integer on all supported
 * platforms.
 */
/** \var  UCHAR
 *        Data type representing at least 1 byte unsigned integer on all
 * supported platforms.
 */
/** \var  INT
 *        Data type representing at least 4 byte signed integer on all supported
 * platforms.
 */
/** \var  UINT
 *        Data type representing at least 4 byte unsigned integer on all
 * supported platforms.
 */
/** \var  LONG
 *        Data type representing 4 byte signed integer on all supported
 * platforms.
 */
/** \var  ULONG
 *        Data type representing 4 byte unsigned integer on all supported
 * platforms.
 */
/** \var  SHORT
 *        Data type representing 2 byte signed integer on all supported
 * platforms.
 */
/** \var  USHORT
 *        Data type representing 2 byte unsigned integer on all supported
 * platforms.
 */
/** \var  INT64
 *        Data type representing 8 byte signed integer on all supported
 * platforms.
 */
/** \var  UINT64
 *        Data type representing 8 byte unsigned integer on all supported
 * platforms.
 */
/** \def  SHORT_BITS
 *        Number of bits the data type short represents. sizeof() is not suited
 * to get this info, because a byte is not always defined as 8 bits.
 */
/** \def  CHAR_BITS
 *        Number of bits the data type char represents. sizeof() is not suited
 * to get this info, because a byte is not always defined as 8 bits.
 */
/** \var  INT_PCM
 *        Data type representing the width of input and output PCM samples.
 */

typedef signed int INT;
typedef unsigned int UINT;
#ifdef __LP64__
/* force FDK long-datatypes to 4 byte  */
/* Use defines to avoid type alias problems on 64 bit machines. */
#define LONG INT
#define ULONG UINT
#else  /* __LP64__ */
typedef signed long LONG;
typedef unsigned long ULONG;
#endif /* __LP64__ */
typedef signed short SHORT;
typedef unsigned short USHORT;
typedef signed char SCHAR;
typedef unsigned char UCHAR;

#define SHORT_BITS 16
#define CHAR_BITS 8

/* Define 64 bit base integer type. */
#ifdef _MSC_VER
typedef __int64 INT64;
typedef unsigned __int64 UINT64;
#else
typedef long long INT64;
typedef unsigned long long UINT64;
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

#if ((defined(__i686__) || defined(__i586__) || defined(__i386__) ||  \
      defined(__x86_64__)) ||                                         \
     (defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64)))) && \
    !defined(FDK_ASSERT_ENABLE)
#define FDK_ASSERT_ENABLE
#endif

#if defined(FDK_ASSERT_ENABLE)
#include <assert.h>
#define FDK_ASSERT(x) assert(x)
#else
#define FDK_ASSERT(ignore)
#endif

typedef SHORT INT_PCM;
#define MAXVAL_PCM MAXVAL_SGL
#define MINVAL_PCM MINVAL_SGL
#define WAV_BITS 16
#define SAMPLE_BITS 16
#define SAMPLE_MAX ((INT_PCM)(((ULONG)1 << (SAMPLE_BITS - 1)) - 1))
#define SAMPLE_MIN (~SAMPLE_MAX)

/*!
* \def    RAM_ALIGN
*  Used to align memory as prefix before memory declaration. For example:
   \code
   RAM_ALIGN
   int myArray[16];
   \endcode

   Note, that not all platforms support this mechanism. For example with TI
compilers a preprocessor pragma is used, but to do something like

   \code
   #define RAM_ALIGN #pragma DATA_ALIGN(x)
   \endcode

   would require the preprocessor to process this line twice to fully resolve
it. Hence, a fully platform-independant way to use alignment is not supported.

* \def    ALIGNMENT_DEFAULT
*         Default alignment in bytes.
*/

#define ALIGNMENT_DEFAULT 8

/* RAM_ALIGN keyword causes memory alignment of global variables. */
#if defined(_MSC_VER)
#define RAM_ALIGN __declspec(align(ALIGNMENT_DEFAULT))
#elif defined(__GNUC__)
#define RAM_ALIGN __attribute__((aligned(ALIGNMENT_DEFAULT)))
#else
#define RAM_ALIGN
#endif

/*!
 * \def  RESTRICT
 *       The restrict keyword is supported by some platforms and RESTRICT maps
 * to either the corresponding keyword on each platform or to void if the
 *       compiler does not provide such feature. It tells the compiler that a
 * pointer points to memory that does not overlap with other memories pointed to
 * by other pointers. If this keyword is used and the assumption of no
 * overlap is not true the resulting code might crash.
 *
 * \def  WORD_ALIGNED(x)
 *       Tells the compiler that pointer x is 16 bit aligned. It does not cause
 * the address itself to be aligned, but serves as a hint to the optimizer. The
 * alignment of the pointer must be guarranteed, if not the code might
 * crash.
 *
 * \def  DWORD_ALIGNED(x)
 *       Tells the compiler that pointer x is 32 bit aligned. It does not cause
 * the address itself to be aligned, but serves as a hint to the optimizer. The
 * alignment of the pointer must be guarranteed, if not the code might
 * crash.
 *
 */
#define RESTRICT
#define WORD_ALIGNED(x) C_ALLOC_ALIGNED_CHECK2((const void *)(x), 2);
#define DWORD_ALIGNED(x) C_ALLOC_ALIGNED_CHECK2((const void *)(x), 4);

/*-----------------------------------------------------------------------------------
 * ALIGN_SIZE
 *-----------------------------------------------------------------------------------*/
/*!
 * \brief  This macro aligns a given value depending on ::ALIGNMENT_DEFAULT.
 *
 * For example if #ALIGNMENT_DEFAULT equals 8, then:
 * - ALIGN_SIZE(3) returns 8
 * - ALIGN_SIZE(8) returns 8
 * - ALIGN_SIZE(9) returns 16
 */
#define ALIGN_SIZE(a)                                                          \
  ((a) + (((INT)ALIGNMENT_DEFAULT - ((size_t)(a) & (ALIGNMENT_DEFAULT - 1))) & \
          (ALIGNMENT_DEFAULT - 1)))

/*!
 * \brief  This macro aligns a given address depending on ::ALIGNMENT_DEFAULT.
 */
#define ALIGN_PTR(a)                                      \
  ((void *)((unsigned char *)(a) +                        \
            ((((INT)ALIGNMENT_DEFAULT -                   \
               ((size_t)(a) & (ALIGNMENT_DEFAULT - 1))) & \
              (ALIGNMENT_DEFAULT - 1)))))

/* Alignment macro for libSYS heap implementation */
#define ALIGNMENT_EXTRES (ALIGNMENT_DEFAULT)
#define ALGN_SIZE_EXTRES(a)                                               \
  ((a) + (((INT)ALIGNMENT_EXTRES - ((INT)(a) & (ALIGNMENT_EXTRES - 1))) & \
          (ALIGNMENT_EXTRES - 1)))

/*!
 * \def  FDK_FORCEINLINE
 *       Sometimes compiler do not do what they are told to do, and in case of
 * inlining some additional command might be necessary depending on the
 * platform.
 *
 * \def  FDK_INLINE
 *       Defines how the compiler is told to inline stuff.
 */
#ifndef FDK_FORCEINLINE
#if defined(__GNUC__) && !defined(__SDE_MIPS__)
#define FDK_FORCEINLINE inline __attribute((always_inline))
#else
#define FDK_FORCEINLINE inline
#endif
#endif

#define FDK_INLINE static inline

/*!
 * \def  LNK_SECTION_DATA_L1
 *       The LNK_SECTION_* defines allow memory to be drawn from specific memory
 *       sections. Used as prefix before variable declaration.
 *
 * \def  LNK_SECTION_DATA_L2
 *       See ::LNK_SECTION_DATA_L1
 * \def  LNK_SECTION_L1_DATA_A
 *       See ::LNK_SECTION_DATA_L1
 * \def  LNK_SECTION_L1_DATA_B
 *       See ::LNK_SECTION_DATA_L1
 * \def  LNK_SECTION_CONSTDATA_L1
 *       See ::LNK_SECTION_DATA_L1
 * \def  LNK_SECTION_CONSTDATA
 *       See ::LNK_SECTION_DATA_L1
 * \def  LNK_SECTION_CODE_L1
 *       See ::LNK_SECTION_DATA_L1
 * \def  LNK_SECTION_CODE_L2
 *       See ::LNK_SECTION_DATA_L1
 * \def  LNK_SECTION_INITCODE
 *       See ::LNK_SECTION_DATA_L1
 */
/**************************************************
 * Code Section macros
 **************************************************/
#define LNK_SECTION_CODE_L1
#define LNK_SECTION_CODE_L2
#define LNK_SECTION_INITCODE

/* Memory section macros. */

/* default fall back */
#define LNK_SECTION_DATA_L1
#define LNK_SECTION_DATA_L2
#define LNK_SECTION_CONSTDATA
#define LNK_SECTION_CONSTDATA_L1

#define LNK_SECTION_L1_DATA_A
#define LNK_SECTION_L1_DATA_B

/**************************************************
 * Macros regarding static code analysis
 **************************************************/
#ifdef __cplusplus
#if !defined(__has_cpp_attribute)
#define __has_cpp_attribute(x) 0
#endif
#if defined(__clang__) && __has_cpp_attribute(clang::fallthrough)
#define FDK_FALLTHROUGH [[clang::fallthrough]]
#endif
#endif

#ifndef FDK_FALLTHROUGH
#if defined(__GNUC__) && (__GNUC__ >= 7)
#define FDK_FALLTHROUGH __attribute__((fallthrough))
#else
#define FDK_FALLTHROUGH
#endif
#endif

#ifdef _MSC_VER
/*
 * Sometimes certain features are excluded from compilation and therefore the
 * warning 4065 may occur: "switch statement contains 'default' but no 'case'
 * labels" We consider this warning irrelevant and disable it.
 */
#pragma warning(disable : 4065)
#endif

#endif /* MACHINE_TYPE_H */
