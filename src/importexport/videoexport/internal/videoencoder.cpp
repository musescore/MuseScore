/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "videoencoder.h"

#include "ffmpeg.h"

#include "log.h"

using namespace mu::iex::videoexport;

struct mu::iex::videoexport::FFmpeg {
    int width = 0;
    int height = 0;
    unsigned int ptsCounter = 0;

    // FFmpeg stuff
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(59, 16, 100)
    AVOutputFormat* outputFormat = nullptr;
#else
    const AVOutputFormat* outputFormat = nullptr;
#endif
    AVFormatContext* formatCtx = nullptr;
    AVStream* videoStream = nullptr;
    AVCodecContext* codecCtx = nullptr;
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(57, 33, 100)
    AVCodec* codec = nullptr;
#else
    const AVCodec* codec = nullptr;
#endif
    // Frame data
    AVFrame* ppicture = nullptr;
    uint8_t* picture_buf = nullptr;
    // Conversion
    SwsContext* img_convert_ctx = nullptr;
    // Packet
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 133, 100)
    AVPacket pkt;
#else
    AVPacket* pkt;
#endif

    bool opened = false;
};

VideoEncoder::VideoEncoder()
{
    m_ffmpeg = new FFmpeg();

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 10, 100)
    avcodec_register_all();
#endif
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100)
    av_register_all();
#endif
}

VideoEncoder::~VideoEncoder()
{
    delete m_ffmpeg;
}

bool VideoEncoder::open(const muse::io::path_t& fileName, unsigned width, unsigned height, unsigned bitrate, unsigned gop, unsigned fps)
{
    m_ffmpeg->ptsCounter = 0;

    m_ffmpeg->width = width;
    m_ffmpeg->height = height;

    m_ffmpeg->outputFormat = av_guess_format("mp4", NULL, NULL);

    m_ffmpeg->formatCtx = avformat_alloc_context();
    if (!m_ffmpeg->formatCtx) {
        LOGE() << "failed allocate format context";
        return false;
    }
    m_ffmpeg->formatCtx->oformat = m_ffmpeg->outputFormat;
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 7, 100)
    snprintf(m_ffmpeg->formatCtx->filename, sizeof(m_ffmpeg->formatCtx->filename), "%s", fileName.c_str());
#else
    m_ffmpeg->formatCtx->url = strdup(fileName.c_str());
#endif

    // Add the video stream
    m_ffmpeg->videoStream = avformat_new_stream(m_ffmpeg->formatCtx, 0);
    if (!m_ffmpeg->videoStream) {
        LOGE() << "failed allocate stream";
        return false;
    }

    m_ffmpeg->videoStream->time_base.den = fps;
    m_ffmpeg->videoStream->time_base.num = 1;

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(57, 33, 100)
    m_ffmpeg->codecCtx = m_ffmpeg->videoStream->codec;
    m_ffmpeg->codecCtx->codec_id = AV_CODEC_ID_H264;
    m_ffmpeg->codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
#else
    // find the video encoder
    m_ffmpeg->codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!m_ffmpeg->codec) {
        LOGE() << "not found codec";
        return false;
    }
    m_ffmpeg->codecCtx = avcodec_alloc_context3(m_ffmpeg->codec);
    if (m_ffmpeg->codecCtx == nullptr) {
        LOGE() << "failed to allocate AV context";
        return false;
    }
#endif
    m_ffmpeg->codecCtx->profile = FF_PROFILE_H264_HIGH;

    m_ffmpeg->codecCtx->bit_rate = bitrate;
    m_ffmpeg->codecCtx->width = width;
    m_ffmpeg->codecCtx->height = height;

    // Avoid bug with missing frames
    m_ffmpeg->codecCtx->gop_size = gop;
    m_ffmpeg->codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    m_ffmpeg->codecCtx->thread_count = 10;

    m_ffmpeg->codecCtx->time_base.den = fps;
    m_ffmpeg->codecCtx->time_base.num = 1;
    m_ffmpeg->codecCtx->max_b_frames = 3;
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 25, 100)
    m_ffmpeg->codecCtx->b_frame_strategy = 1;
#else
    av_opt_set_int(m_ffmpeg->codecCtx, "b_strategy", 1, 0);
#endif

    m_ffmpeg->codecCtx->me_cmp = 1;
    m_ffmpeg->codecCtx->me_range = 16;

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(56, 49, 100)
    m_ffmpeg->codecCtx->me_method = ME_HEX;
#else
    av_opt_set_int(m_ffmpeg->codecCtx, "hex", 1, 0);
#endif

    m_ffmpeg->codecCtx->qmin = 10;
    m_ffmpeg->codecCtx->qmax = 51;
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 25, 100)
    m_ffmpeg->codecCtx->scenechange_threshold = 40;
#else
    av_opt_set_int(m_ffmpeg->codecCtx, "sc_threshold", 40, 0);
#endif
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(56, 56, 100)
    m_ffmpeg->codecCtx->flags |= CODEC_FLAG_LOOP_FILTER;
#else
    m_ffmpeg->codecCtx->flags |= AV_CODEC_FLAG_LOOP_FILTER;
#endif
    m_ffmpeg->codecCtx->me_subpel_quality = 5;
    m_ffmpeg->codecCtx->i_quant_factor = 0.71;
    m_ffmpeg->codecCtx->qcompress = 0.6;
    m_ffmpeg->codecCtx->max_qdiff = 4;

    // some formats want stream headers to be separate
    if (m_ffmpeg->formatCtx->oformat->flags & AVFMT_GLOBALHEADER) {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(56, 56, 100)
        m_ffmpeg->codecCtx->flags |= CODEC_FLAG_LOOP_FILTER;
#else
        m_ffmpeg->codecCtx->flags |= AV_CODEC_FLAG_LOOP_FILTER;
#endif
    }

    //av_dump_format(m_ffmpeg->formatCtx, 0, fileName.c_str(), 1);

    // open_video
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(57, 33, 100)
    // find the video encoder
    m_ffmpeg->codec = avcodec_find_encoder(m_ffmpeg->codecCtx->codec_id);
    if (!m_ffmpeg->codec) {
        LOGE() << "not found codec";
        return false;
    }
#else
    if (avcodec_parameters_from_context(m_ffmpeg->videoStream->codecpar, m_ffmpeg->codecCtx) < 0) {
        avcodec_free_context(&m_ffmpeg->codecCtx);
        LOGE() << "failed to set AV parameters from context";
        return false;
    }
#endif
    // open the codec
    if (avcodec_open2(m_ffmpeg->codecCtx, m_ffmpeg->codec, 0) < 0) {
        LOGE() << "failed open codec";
        return false;
    }

    // Allocate the YUV frame
    m_ffmpeg->ppicture = av_frame_alloc();
    if (!m_ffmpeg->ppicture) {
        LOGE() << "failed allocate frame";
        return false;
    }

    m_ffmpeg->ppicture->width = width;
    m_ffmpeg->ppicture->height = height;
    m_ffmpeg->ppicture->format = AV_PIX_FMT_YUV420P;

#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(55, 4, 100)
    int size = avpicture_get_size(m_ffmpeg->codecCtx->pix_fmt, m_ffmpeg->codecCtx->width, m_ffmpeg->codecCtx->height);
#else
    int size = av_image_get_buffer_size(m_ffmpeg->codecCtx->pix_fmt, m_ffmpeg->codecCtx->width, m_ffmpeg->codecCtx->height, 1);
#endif
    m_ffmpeg->picture_buf = new uint8_t[size];
    if (!m_ffmpeg->picture_buf) {
        LOGE() << "failed allocate frame buf";
        av_free(m_ffmpeg->ppicture);
        return false;
    }

    // Setup the planes
#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(55, 4, 100)
    avpicture_fill((AVPicture*)m_ffmpeg->ppicture, m_ffmpeg->picture_buf, m_ffmpeg->codecCtx->pix_fmt, m_ffmpeg->codecCtx->width,
                   m_ffmpeg->codecCtx->height);
#else
    av_image_fill_arrays(m_ffmpeg->ppicture->data, m_ffmpeg->ppicture->linesize, m_ffmpeg->picture_buf, m_ffmpeg->codecCtx->pix_fmt,
                         m_ffmpeg->codecCtx->width, m_ffmpeg->codecCtx->height, 1);
#endif

    if (avio_open(&m_ffmpeg->formatCtx->pb, fileName.c_str(), AVIO_FLAG_WRITE) < 0) {
        LOGE() << "failed open file: " << fileName;
        return false;
    }

    int ret = avformat_write_header(m_ffmpeg->formatCtx, NULL);
    if (ret < 0) {
        LOGE() << "failed write AV header";
        return false;
    }

    m_ffmpeg->opened = true;

    return true;
}

void VideoEncoder::close()
{
    if (!m_ffmpeg->opened) {
        return;
    }

    int ret = avcodec_send_frame(m_ffmpeg->codecCtx, NULL);
    if (ret < 0) {
        LOGE() << "failed to flush encoder buffer";
        return;
    }

    while (true) {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 133, 100)
        av_init_packet(&m_ffmpeg->pkt);
        int ret = avcodec_receive_packet(m_ffmpeg->codecCtx, &m_ffmpeg->pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            LOGE() << "error during encoding";
            return;
        }

        m_ffmpeg->pkt.pts = av_rescale_q(m_ffmpeg->ptsCounter, m_ffmpeg->codecCtx->time_base, m_ffmpeg->videoStream->time_base);
        m_ffmpeg->pkt.dts = av_rescale_q(m_ffmpeg->ptsCounter, m_ffmpeg->codecCtx->time_base, m_ffmpeg->videoStream->time_base);

        m_ffmpeg->ptsCounter++;

        ret = av_interleaved_write_frame(m_ffmpeg->formatCtx, &m_ffmpeg->pkt);
        if (ret < 0) {
            return;
        }
        av_packet_unref(&m_ffmpeg->pkt);
#else
        m_ffmpeg->pkt = av_packet_alloc();
        int ret = avcodec_receive_packet(m_ffmpeg->codecCtx, m_ffmpeg->pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            LOGE() << "error during encoding";
            return;
        }

        m_ffmpeg->pkt->pts = av_rescale_q(m_ffmpeg->ptsCounter, m_ffmpeg->codecCtx->time_base, m_ffmpeg->videoStream->time_base);
        m_ffmpeg->pkt->dts = av_rescale_q(m_ffmpeg->ptsCounter, m_ffmpeg->codecCtx->time_base, m_ffmpeg->videoStream->time_base);

        m_ffmpeg->ptsCounter++;

        ret = av_interleaved_write_frame(m_ffmpeg->formatCtx, m_ffmpeg->pkt);
        if (ret < 0) {
            return;
        }
        av_packet_unref(m_ffmpeg->pkt);
#endif
    }

    av_write_trailer(m_ffmpeg->formatCtx);

    // close_video

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(57, 33, 100)
    avcodec_close(m_ffmpeg->videoStream->codec);
#else
    avcodec_free_context(&m_ffmpeg->codecCtx);
#endif

    if (m_ffmpeg->picture_buf) {
        delete[] m_ffmpeg->picture_buf;
        m_ffmpeg->picture_buf = 0;
    }

    if (m_ffmpeg->ppicture) {
        av_free(m_ffmpeg->ppicture);
        m_ffmpeg->ppicture = 0;
    }

    /* free the streams */

    for (unsigned int i = 0; i < m_ffmpeg->formatCtx->nb_streams; i++) {
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(57, 33, 100)
        av_freep(&m_ffmpeg->formatCtx->streams[i]->codec);
#endif
        av_freep(&m_ffmpeg->formatCtx->streams[i]);
    }

    // Close file
    avio_close(m_ffmpeg->formatCtx->pb);

    // Free the stream
    av_free(m_ffmpeg->formatCtx);
}

bool VideoEncoder::encodeImage(const QImage& img)
{
    if (!m_ffmpeg->opened) {
        return false;
    }

    convertImage_sws(img);

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(60, 3, 100)
    m_ffmpeg->ppicture->pts = av_rescale_q(m_ffmpeg->codecCtx->frame_num, m_ffmpeg->codecCtx->time_base,
                                           m_ffmpeg->videoStream->time_base);
#else
    m_ffmpeg->ppicture->pts = av_rescale_q(m_ffmpeg->codecCtx->frame_number, m_ffmpeg->codecCtx->time_base,
                                           m_ffmpeg->videoStream->time_base);
#endif

    int ret = avcodec_send_frame(m_ffmpeg->codecCtx, m_ffmpeg->ppicture);
    if (ret < 0) {
        return false;
    }

    while (ret >= 0) {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 133, 100)
        av_init_packet(&m_ffmpeg->pkt);
        ret = avcodec_receive_packet(m_ffmpeg->codecCtx, &m_ffmpeg->pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0) {
            LOGE() << "error during encoding";
            return false;
        }

        m_ffmpeg->pkt.pts = av_rescale_q(m_ffmpeg->ptsCounter, m_ffmpeg->codecCtx->time_base, m_ffmpeg->videoStream->time_base);
        m_ffmpeg->pkt.dts = av_rescale_q(m_ffmpeg->ptsCounter, m_ffmpeg->codecCtx->time_base, m_ffmpeg->videoStream->time_base);

        m_ffmpeg->ptsCounter++;

        ret = av_interleaved_write_frame(m_ffmpeg->formatCtx, &m_ffmpeg->pkt);
        if (ret < 0) {
            return false;
        }
        av_packet_unref(&m_ffmpeg->pkt);
#else
        m_ffmpeg->pkt = av_packet_alloc();
        ret = avcodec_receive_packet(m_ffmpeg->codecCtx, m_ffmpeg->pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0) {
            LOGE() << "error during encoding";
            return false;
        }

        m_ffmpeg->pkt->pts = av_rescale_q(m_ffmpeg->ptsCounter, m_ffmpeg->codecCtx->time_base, m_ffmpeg->videoStream->time_base);
        m_ffmpeg->pkt->dts = av_rescale_q(m_ffmpeg->ptsCounter, m_ffmpeg->codecCtx->time_base, m_ffmpeg->videoStream->time_base);

        m_ffmpeg->ptsCounter++;

        ret = av_interleaved_write_frame(m_ffmpeg->formatCtx, m_ffmpeg->pkt);
        if (ret < 0) {
            return false;
        }
        av_packet_unref(m_ffmpeg->pkt);
#endif
    }

    return true;
}

bool VideoEncoder::convertImage_sws(const QImage& img)
{
    // Check if the image matches the size
    if (img.width() != m_ffmpeg->width || img.height() != m_ffmpeg->height) {
        LOGE() << "wrong image size!";
        return false;
    }

    if (img.format() != QImage::Format_RGB32 && img.format() != QImage::Format_ARGB32) {
        LOGE() << "wrong image format";
        return false;
    }

    m_ffmpeg->img_convert_ctx = sws_getCachedContext(m_ffmpeg->img_convert_ctx, m_ffmpeg->width, m_ffmpeg->height, AV_PIX_FMT_BGRA,
                                                     m_ffmpeg->width, m_ffmpeg->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

    if (!m_ffmpeg->img_convert_ctx) {
        LOGE() << "failed initialize the conversion context";
        return false;
    }

    uint8_t* srcplanes[3];
    srcplanes[0] = (uint8_t*)img.bits();
    srcplanes[1] = 0;
    srcplanes[2] = 0;

    int srcstride[3];
    srcstride[0] = img.bytesPerLine();
    srcstride[1] = 0;
    srcstride[2 ]= 0;

    sws_scale(m_ffmpeg->img_convert_ctx, srcplanes, srcstride, 0, m_ffmpeg->height, m_ffmpeg->ppicture->data, m_ffmpeg->ppicture->linesize);

    return true;
}
