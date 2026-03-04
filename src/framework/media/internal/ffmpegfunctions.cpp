/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "ffmpegfunctions.h"

namespace muse::media {

bool FFmpegFunctions::isValid() const
{
    return av_free && av_freep && av_rescale_q
           && av_image_fill_arrays && av_image_get_buffer_size
           && av_opt_set_int
           && av_guess_format && avformat_alloc_context && avformat_new_stream
           && avformat_write_header && av_write_trailer
           && avio_open && avio_close && av_interleaved_write_frame
           && avformat_open_input && avformat_find_stream_info
           && avformat_close_input && avformat_free_context
           && avformat_alloc_output_context2 && av_read_frame
           && avcodec_alloc_context3 && avcodec_find_encoder && avcodec_free_context
           && avcodec_open2 && avcodec_parameters_from_context && avcodec_parameters_copy
           && avcodec_send_frame && avcodec_receive_packet
           && av_frame_alloc
           && av_packet_alloc && av_packet_free && av_packet_unref && av_packet_rescale_ts
           && sws_getCachedContext && sws_scale;
}

}
