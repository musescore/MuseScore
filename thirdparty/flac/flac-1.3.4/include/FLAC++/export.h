/* libFLAC++ - Free Lossless Audio Codec library
 * Copyright (C) 2002-2009  Josh Coalson
 * Copyright (C) 2011-2016  Xiph.Org Foundation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FLACPP__EXPORT_H
#define FLACPP__EXPORT_H

/** \file include/FLAC++/export.h
 *
 *  \brief
 *  This module contains \#defines and symbols for exporting function
 *  calls, and providing version information and compiled-in features.
 *
 *  See the \link flacpp_export export \endlink module.
 */

/** \defgroup flacpp_export FLAC++/export.h: export symbols
 *  \ingroup flacpp
 *
 *  \brief
 *  This module contains \#defines and symbols for exporting function
 *  calls, and providing version information and compiled-in features.
 *
 *  If you are compiling with MSVC and will link to the static library
 *  (libFLAC++.lib) you should define FLAC__NO_DLL in your project to
 *  make sure the symbols are exported properly.
 *
 * \{
 */

#if defined(FLAC__NO_DLL)
#define FLACPP_API

#elif defined(_WIN32)
#ifdef FLACPP_API_EXPORTS
#define	FLACPP_API __declspec(dllexport)
#else
#define FLACPP_API __declspec(dllimport)
#endif

#elif defined(FLAC__USE_VISIBILITY_ATTR)
#define FLACPP_API __attribute__ ((visibility ("default")))

#else
#define FLACPP_API

#endif

/* These \#defines will mirror the libtool-based library version number, see
 * http://www.gnu.org/software/libtool/manual/libtool.html#Libtool-versioning
 */
#define FLACPP_API_VERSION_CURRENT 9
#define FLACPP_API_VERSION_REVISION 0
#define FLACPP_API_VERSION_AGE 3

/* \} */

#endif
