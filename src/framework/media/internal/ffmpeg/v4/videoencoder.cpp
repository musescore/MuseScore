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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "videoencoder.h"
#include "ffmpeglibhandler.h"

#include "io/file.h"
#include "io/path.h"
#include "defer.h"
#include "log.h"

namespace muse::media::ffmpeg::v4 {
struct FFmpeg {
    int width = 0;
    int height = 0;
    unsigned int ptsCounter = 0;

    // FFmpeg stuff
    AVFormatContext* formatCtx = nullptr;
    AVStream* videoStream = nullptr;
    AVCodecContext* codecCtx = nullptr;
    const AVCodec* codec = nullptr;
    // Frame data
    AVFrame* ppicture = nullptr;
    uint8_t* picture_buf = nullptr;
    // Conversion
    SwsContext* img_convert_ctx = nullptr;
    // Packet
    AVPacket* pkt;

    bool opened = false;
    bool finished = false;
};

VideoEncoder::VideoEncoder()
{
    m_ffmpeg = new FFmpeg();
}

VideoEncoder::~VideoEncoder()
{
    delete m_ffmpeg;
}

bool VideoEncoder::load(const FFmpegLibPaths& paths)
{
    auto handler = std::make_shared<FFmpegLibHandler>();
    if (!handler->loadLib(paths.avUtilPath, paths.avCodecPath, paths.avFormatPath,
                          paths.swScalePath, paths.swResamplePath)
        || !handler->loadApi()) {
        LOGW() << "FFmpeg libraries not found";
        return false;
    }
    m_ffmpegHandler = std::move(handler);
    return true;
}

bool VideoEncoder::open(const muse::io::path_t& fileName, const Options& options)
{
    m_ffmpeg->ptsCounter = 0;

    m_ffmpeg->width = options.width;
    m_ffmpeg->height = options.height;

    if (m_ffmpegHandler->avformat_alloc_output_context2(&m_ffmpeg->formatCtx, nullptr, options.format.c_str(), fileName.c_str()) < 0
        || !m_ffmpeg->formatCtx) {
        LOGE() << "failed to allocate output context";
        return false;
    }

    // Add the video stream
    m_ffmpeg->videoStream = m_ffmpegHandler->avformat_new_stream(m_ffmpeg->formatCtx, nullptr);
    if (!m_ffmpeg->videoStream) {
        LOGE() << "failed allocate stream";
        return false;
    }

    m_ffmpeg->videoStream->time_base.den = options.fps;
    m_ffmpeg->videoStream->time_base.num = 1;

    // find the video encoder
    m_ffmpeg->codec = m_ffmpegHandler->avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!m_ffmpeg->codec) {
        LOGE() << "not found codec";
        return false;
    }
    m_ffmpeg->codecCtx = m_ffmpegHandler->avcodec_alloc_context3(m_ffmpeg->codec);
    if (m_ffmpeg->codecCtx == nullptr) {
        LOGE() << "failed to allocate AV context";
        return false;
    }

    m_ffmpeg->videoStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    m_ffmpeg->videoStream->codecpar->codec_id = AV_CODEC_ID_H264;
    m_ffmpeg->videoStream->codecpar->width = options.width;
    m_ffmpeg->videoStream->codecpar->height = options.height;
    m_ffmpeg->videoStream->codecpar->format = AV_PIX_FMT_YUV420P;
    m_ffmpeg->videoStream->codecpar->bit_rate = options.bitrate;

    if (m_ffmpegHandler->avcodec_parameters_to_context(m_ffmpeg->codecCtx, m_ffmpeg->videoStream->codecpar) < 0) {
        m_ffmpegHandler->avcodec_free_context(&m_ffmpeg->codecCtx);
        LOGE() << "failed to set codec parameters from stream";
        return false;
    }

    m_ffmpeg->codecCtx->profile = FF_PROFILE_H264_HIGH;
    m_ffmpeg->codecCtx->time_base.den = options.fps;
    m_ffmpeg->codecCtx->time_base.num = 1;
    m_ffmpeg->codecCtx->gop_size = options.gop;
    m_ffmpeg->codecCtx->thread_count = 10;
    m_ffmpeg->codecCtx->max_b_frames = 3;
    m_ffmpegHandler->av_opt_set_int(m_ffmpeg->codecCtx, "b_strategy", 1, 0);

    m_ffmpeg->codecCtx->me_cmp = 1;
    m_ffmpeg->codecCtx->me_range = 16;
    m_ffmpegHandler->av_opt_set_int(m_ffmpeg->codecCtx, "hex", 1, 0);

    m_ffmpeg->codecCtx->qmin = 10;
    m_ffmpeg->codecCtx->qmax = 51;
    m_ffmpegHandler->av_opt_set_int(m_ffmpeg->codecCtx, "sc_threshold", 40, 0);
    m_ffmpeg->codecCtx->flags |= AV_CODEC_FLAG_LOOP_FILTER;
    m_ffmpeg->codecCtx->me_subpel_quality = 5;
    m_ffmpeg->codecCtx->i_quant_factor = 0.71f;
    m_ffmpeg->codecCtx->qcompress = 0.6f;
    m_ffmpeg->codecCtx->max_qdiff = 4;

    if (m_ffmpeg->formatCtx->oformat->flags & AVFMT_GLOBALHEADER) {
        m_ffmpeg->codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (m_ffmpegHandler->avcodec_open2(m_ffmpeg->codecCtx, m_ffmpeg->codec, nullptr) < 0) {
        LOGE() << "failed open codec";
        return false;
    }

    // Copy codec params to stream AFTER open - codec adds extradata (SPS/PPS) during open
    if (m_ffmpegHandler->avcodec_parameters_from_context(m_ffmpeg->videoStream->codecpar, m_ffmpeg->codecCtx) < 0) {
        m_ffmpegHandler->avcodec_free_context(&m_ffmpeg->codecCtx);
        LOGE() << "failed to copy codec parameters to stream";
        return false;
    }

    // Allocate the YUV frame
    m_ffmpeg->ppicture = m_ffmpegHandler->av_frame_alloc();
    if (!m_ffmpeg->ppicture) {
        LOGE() << "failed allocate frame";
        return false;
    }

    m_ffmpeg->ppicture->width = options.width;
    m_ffmpeg->ppicture->height = options.height;
    m_ffmpeg->ppicture->format = AV_PIX_FMT_YUV420P;

    int size = m_ffmpegHandler->av_image_get_buffer_size(AV_PIX_FMT_YUV420P, m_ffmpeg->width, m_ffmpeg->height, 1);
    m_ffmpeg->picture_buf = new uint8_t[size];
    if (!m_ffmpeg->picture_buf) {
        LOGE() << "failed allocate frame buf";
        m_ffmpegHandler->av_frame_free(&m_ffmpeg->ppicture);
        return false;
    }

    // Setup the planes
    m_ffmpegHandler->av_image_fill_arrays(m_ffmpeg->ppicture->data, m_ffmpeg->ppicture->linesize, m_ffmpeg->picture_buf,
                                          AV_PIX_FMT_YUV420P, m_ffmpeg->width, m_ffmpeg->height, 1);

    if (m_ffmpegHandler->avio_open(&m_ffmpeg->formatCtx->pb, fileName.c_str(), AVIO_FLAG_WRITE) < 0) {
        LOGE() << "failed open file: " << fileName;
        return false;
    }

    int ret = m_ffmpegHandler->avformat_write_header(m_ffmpeg->formatCtx, NULL);
    if (ret < 0) {
        LOGE() << "failed write AV header";
        return false;
    }

    //av_dump_format(m_ffmpeg->formatCtx, 0, fileName.c_str(), 1);

    m_ffmpeg->opened = true;
    m_outputPath = fileName;

    return true;
}

void VideoEncoder::finishEncode()
{
    if (!m_ffmpeg->opened || m_ffmpeg->finished) {
        return;
    }

    int ret = m_ffmpegHandler->avcodec_send_frame(m_ffmpeg->codecCtx, NULL);
    if (ret < 0) {
        LOGE() << "failed to flush encoder buffer";
        return;
    }

    while (true) {
        m_ffmpeg->pkt = m_ffmpegHandler->av_packet_alloc();
        int ret = m_ffmpegHandler->avcodec_receive_packet(m_ffmpeg->codecCtx, m_ffmpeg->pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            m_ffmpegHandler->av_packet_free(&m_ffmpeg->pkt);
            break;
        } else if (ret < 0) {
            m_ffmpegHandler->av_packet_free(&m_ffmpeg->pkt);
            LOGE() << "error during encoding";
            return;
        }

        m_ffmpeg->pkt->pts = m_ffmpegHandler->av_rescale_q(m_ffmpeg->ptsCounter, m_ffmpeg->codecCtx->time_base,
                                                           m_ffmpeg->videoStream->time_base);
        m_ffmpeg->pkt->dts = m_ffmpegHandler->av_rescale_q(m_ffmpeg->ptsCounter, m_ffmpeg->codecCtx->time_base,
                                                           m_ffmpeg->videoStream->time_base);

        m_ffmpeg->ptsCounter++;

        ret = m_ffmpegHandler->av_interleaved_write_frame(m_ffmpeg->formatCtx, m_ffmpeg->pkt);
        if (ret < 0) {
            m_ffmpegHandler->av_packet_free(&m_ffmpeg->pkt);
            return;
        }
        m_ffmpegHandler->av_packet_free(&m_ffmpeg->pkt);
    }

    m_ffmpegHandler->av_write_trailer(m_ffmpeg->formatCtx);

    if (m_ffmpeg->formatCtx->pb) {
        m_ffmpegHandler->avio_close(m_ffmpeg->formatCtx->pb);
        m_ffmpeg->formatCtx->pb = nullptr;
    }

    m_ffmpeg->finished = true;
}

void VideoEncoder::close()
{
    if (!m_ffmpeg->opened) {
        return;
    }

    if (!m_ffmpeg->finished) {
        finishEncode();
    }

    m_ffmpegHandler->avcodec_free_context(&m_ffmpeg->codecCtx);

    if (m_ffmpeg->picture_buf) {
        delete[] m_ffmpeg->picture_buf;
        m_ffmpeg->picture_buf = 0;
    }

    if (m_ffmpeg->ppicture) {
        m_ffmpegHandler->av_frame_free(&m_ffmpeg->ppicture);
        m_ffmpeg->ppicture = 0;
    }

    if (m_ffmpeg->formatCtx) {
        if (m_ffmpeg->formatCtx->pb) {
            m_ffmpegHandler->avio_close(m_ffmpeg->formatCtx->pb);
            m_ffmpeg->formatCtx->pb = nullptr;
        }
        m_ffmpegHandler->avformat_free_context(m_ffmpeg->formatCtx);
        m_ffmpeg->formatCtx = nullptr;
    }
    m_ffmpeg->opened = false;
    m_ffmpeg->finished = false;

    m_outputPath = io::path_t();
}

bool VideoEncoder::encodeImage(const QImage& img)
{
    if (!m_ffmpeg->opened) {
        return false;
    }

    convertImage_sws(img);

    m_ffmpeg->ppicture->pts = m_ffmpegHandler->av_rescale_q(m_ffmpeg->codecCtx->frame_number, m_ffmpeg->codecCtx->time_base,
                                                            m_ffmpeg->videoStream->time_base);

    int ret = m_ffmpegHandler->avcodec_send_frame(m_ffmpeg->codecCtx, m_ffmpeg->ppicture);
    if (ret < 0) {
        return false;
    }

    while (ret >= 0) {
        m_ffmpeg->pkt = m_ffmpegHandler->av_packet_alloc();
        ret = m_ffmpegHandler->avcodec_receive_packet(m_ffmpeg->codecCtx, m_ffmpeg->pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            m_ffmpegHandler->av_packet_free(&m_ffmpeg->pkt);
            return true;
        } else if (ret < 0) {
            m_ffmpegHandler->av_packet_free(&m_ffmpeg->pkt);
            LOGE() << "error during encoding";
            return false;
        }

        m_ffmpeg->pkt->pts = m_ffmpegHandler->av_rescale_q(m_ffmpeg->ptsCounter, m_ffmpeg->codecCtx->time_base,
                                                           m_ffmpeg->videoStream->time_base);
        m_ffmpeg->pkt->dts = m_ffmpegHandler->av_rescale_q(m_ffmpeg->ptsCounter, m_ffmpeg->codecCtx->time_base,
                                                           m_ffmpeg->videoStream->time_base);

        m_ffmpeg->ptsCounter++;

        ret = m_ffmpegHandler->av_interleaved_write_frame(m_ffmpeg->formatCtx, m_ffmpeg->pkt);
        if (ret < 0) {
            m_ffmpegHandler->av_packet_free(&m_ffmpeg->pkt);
            return false;
        }
        m_ffmpegHandler->av_packet_free(&m_ffmpeg->pkt);
    }

    return true;
}

bool VideoEncoder::addAudio(const io::path_t& audioPath, double audioOffsetSec)
{
    if (m_outputPath.empty()) {
        LOGE() << "addAudio: encoder was not opened or path not set";
        return false;
    }

    const io::path_t videoPath = m_outputPath;
    const io::path_t tmpPath = m_outputPath + ".tmp";

    AVFormatContext* videoFmtCtx = nullptr;
    AVFormatContext* audioFmtCtx = nullptr;
    AVFormatContext* outputFmtCtx = nullptr;

    bool cleaned = false;
    auto cleanup = [&]() {
        if (cleaned) {
            return;
        }

        if (videoFmtCtx) {
            m_ffmpegHandler->avformat_close_input(&videoFmtCtx);
        }
        if (audioFmtCtx) {
            m_ffmpegHandler->avformat_close_input(&audioFmtCtx);
        }
        if (outputFmtCtx) {
            if (outputFmtCtx->pb) {
                m_ffmpegHandler->avio_close(outputFmtCtx->pb);
            }
            m_ffmpegHandler->avformat_free_context(outputFmtCtx);
        }

        cleaned = true;
    };

    DEFER {
        cleanup();
    };

    if (m_ffmpegHandler->avformat_open_input(&videoFmtCtx, videoPath.c_str(), nullptr, nullptr) < 0) {
        LOGE() << "addAudio: failed to open video input: " << videoPath;
        return false;
    }
    if (m_ffmpegHandler->avformat_find_stream_info(videoFmtCtx, nullptr) < 0) {
        LOGE() << "addAudio: failed to find video stream info";
        return false;
    }

    if (m_ffmpegHandler->avformat_open_input(&audioFmtCtx, audioPath.c_str(), nullptr, nullptr) < 0) {
        LOGE() << "addAudio: failed to open audio input: " << audioPath;
        return false;
    }
    if (m_ffmpegHandler->avformat_find_stream_info(audioFmtCtx, nullptr) < 0) {
        LOGE() << "addAudio: failed to find audio stream info";
        return false;
    }

    if (m_ffmpegHandler->avformat_alloc_output_context2(&outputFmtCtx, nullptr, "mp4", tmpPath.c_str()) < 0 || !outputFmtCtx) {
        LOGE() << "addAudio: failed to allocate output context";
        return false;
    }

    int videoInIdx = -1;
    int audioInIdx = -1;
    int outVideoIdx = -1;
    int outAudioIdx = -1;

    for (unsigned i = 0; i < videoFmtCtx->nb_streams; i++) {
        if (videoFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoInIdx = static_cast<int>(i);
            AVStream* outStream = m_ffmpegHandler->avformat_new_stream(outputFmtCtx, nullptr);
            if (!outStream) {
                LOGE() << "addAudio: failed to create output video stream";
                return false;
            }
            m_ffmpegHandler->avcodec_parameters_copy(outStream->codecpar, videoFmtCtx->streams[i]->codecpar);
            outStream->codecpar->codec_tag = 0;
            outStream->time_base = videoFmtCtx->streams[i]->time_base;
            outVideoIdx = outStream->index;
            break;
        }
    }

    for (unsigned i = 0; i < audioFmtCtx->nb_streams; i++) {
        if (audioFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioInIdx = static_cast<int>(i);
            AVStream* outStream = m_ffmpegHandler->avformat_new_stream(outputFmtCtx, nullptr);
            if (!outStream) {
                LOGE() << "addAudio: failed to create output audio stream";
                return false;
            }
            m_ffmpegHandler->avcodec_parameters_copy(outStream->codecpar, audioFmtCtx->streams[i]->codecpar);
            outStream->codecpar->codec_tag = 0;
            outStream->time_base = audioFmtCtx->streams[i]->time_base;
            outAudioIdx = outStream->index;
            break;
        }
    }

    if (videoInIdx < 0 || audioInIdx < 0) {
        LOGE() << "addAudio: could not find video or audio stream";
        return false;
    }

    if (m_ffmpegHandler->avio_open(&outputFmtCtx->pb, tmpPath.c_str(), AVIO_FLAG_WRITE) < 0) {
        LOGE() << "addAudio: failed to open output file: " << tmpPath;
        return false;
    }

    if (m_ffmpegHandler->avformat_write_header(outputFmtCtx, nullptr) < 0) {
        LOGE() << "addAudio: failed to write header";
        return false;
    }

    int64_t audioOffsetPts = 0;
    {
        AVStream* audioOutStream = outputFmtCtx->streams[outAudioIdx];
        audioOffsetPts = static_cast<int64_t>(audioOffsetSec * audioOutStream->time_base.den
                                              / audioOutStream->time_base.num);
    }

    AVPacket* pkt = m_ffmpegHandler->av_packet_alloc();

    while (m_ffmpegHandler->av_read_frame(videoFmtCtx, pkt) >= 0) {
        if (pkt->stream_index == videoInIdx) {
            pkt->stream_index = outVideoIdx;
            m_ffmpegHandler->av_packet_rescale_ts(pkt, videoFmtCtx->streams[videoInIdx]->time_base,
                                                  outputFmtCtx->streams[outVideoIdx]->time_base);
            pkt->pos = -1;
            m_ffmpegHandler->av_interleaved_write_frame(outputFmtCtx, pkt);
        }
        m_ffmpegHandler->av_packet_unref(pkt);
    }

    while (m_ffmpegHandler->av_read_frame(audioFmtCtx, pkt) >= 0) {
        if (pkt->stream_index == audioInIdx) {
            pkt->stream_index = outAudioIdx;
            m_ffmpegHandler->av_packet_rescale_ts(pkt, audioFmtCtx->streams[audioInIdx]->time_base,
                                                  outputFmtCtx->streams[outAudioIdx]->time_base);
            pkt->pts += audioOffsetPts;
            pkt->dts += audioOffsetPts;
            pkt->pos = -1;
            m_ffmpegHandler->av_interleaved_write_frame(outputFmtCtx, pkt);
        }
        m_ffmpegHandler->av_packet_unref(pkt);
    }

    m_ffmpegHandler->av_packet_free(&pkt);
    m_ffmpegHandler->av_write_trailer(outputFmtCtx);

    cleanup();

    if (!io::File::copy(tmpPath, videoPath, true)) {
        LOGE() << "addAudio: failed to replace with muxed file";
        return false;
    }

    io::File::remove(tmpPath);

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

    m_ffmpeg->img_convert_ctx = m_ffmpegHandler->sws_getCachedContext(m_ffmpeg->img_convert_ctx, m_ffmpeg->width, m_ffmpeg->height,
                                                                      AV_PIX_FMT_BGRA,
                                                                      m_ffmpeg->width, m_ffmpeg->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC,
                                                                      NULL, NULL, NULL);

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

    m_ffmpegHandler->sws_scale(m_ffmpeg->img_convert_ctx, srcplanes, srcstride, 0, m_ffmpeg->height, m_ffmpeg->ppicture->data,
                               m_ffmpeg->ppicture->linesize);

    return true;
}
}
