/* plugin_common - Routines common to several plugins
 * Copyright (C) 2002-2009  Josh Coalson
 * Copyright (C) 2011-2016  Xiph.Org Foundation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef FLAC__PLUGIN_COMMON__DITHER_H
#define FLAC__PLUGIN_COMMON__DITHER_H

#include <stdlib.h> /* for size_t */
#include "defs.h" /* buy FLAC_PLUGIN__MAX_SUPPORTED_CHANNELS for the caller */
#include "FLAC/ordinals.h"

size_t FLAC__plugin_common__pack_pcm_signed_big_endian(FLAC__byte *data, const FLAC__int32 * const input[], uint32_t wide_samples, uint32_t channels, uint32_t source_bps, uint32_t target_bps);
size_t FLAC__plugin_common__pack_pcm_signed_little_endian(FLAC__byte *data, const FLAC__int32 * const input[], uint32_t wide_samples, uint32_t channels, uint32_t source_bps, uint32_t target_bps);

#endif
