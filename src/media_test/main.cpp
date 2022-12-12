
#include <cstdio>
#include <iostream>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/dict.h>
#include "libavcodec/codec.h"
#include "libavcodec/packet.h"
#include "libavutil/frame.h"
#include "libavdevice/avdevice.h"
#include "libavutil/error.h"
}
#include "logger.h"

int main()
{
    avdevice_register_all();
    AVFormatContext* fmtCtx = avformat_alloc_context();
    AVDictionary* options = nullptr;
    // av_dict_set(&options, "input_format", "mjpeg", 0);
    av_dict_set(&options, "video_size", "640*480", 0);
    av_dict_set(&options, "framerate", "30", 0);
    av_dict_set(&options, "pixel_format", "yuyv422", 0);
    auto inputFormat = av_find_input_format("avfoundation");
    int rst = avformat_open_input(&fmtCtx, "FaceTime HD Camera", inputFormat, &options);
    if (rst != 0) {
        LOG_DEADLY << "avformat_open_input ERROR:" << rst;
        return -1;
    }

    rst = avformat_find_stream_info(fmtCtx, NULL);
    if (rst != 0) {
        LOG_DEADLY << "avformat_find_stream_info ERROR" << rst;
        return -1;
    }

    int video_stream_index = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_stream_index < 0) {
        LOG_DEADLY << "Can not find video stream:" << video_stream_index;
        return -1;
    }

    auto codecpar = fmtCtx->streams[video_stream_index]->codecpar;
    LOG_INFO << "frame width:" << codecpar->width;
    LOG_INFO << "frame height:" << codecpar->height;
    auto streamFrameRate = fmtCtx->streams[video_stream_index]->avg_frame_rate;
    if (streamFrameRate.den != 0) {
        LOG_INFO << "stream frame rate:" << streamFrameRate.num / (double)streamFrameRate.den;
    }
    LOG_INFO << "stream frame format:" << codecpar->format;

    auto decoder = avcodec_find_decoder(codecpar->codec_id);

    auto decodecCtx = avcodec_alloc_context3(decoder);
    rst = avcodec_parameters_to_context(decodecCtx, codecpar);
    if (rst != 0) {
        LOG_DEADLY << "avcodec_parameters_to_context ERROR" << rst;
        return -1;
    }

    rst = avcodec_open2(decodecCtx, decoder, nullptr);
    if (rst != 0) {
        LOG_DEADLY << "avcodec_open2 ERROR" << rst;
        return -1;
    }

    FILE* file = fopen("out.yuv", "wb");
    if (!file) {
        LOG_DEADLY << "fopen ERROR" << errno;
    }

    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    int frameCount = 0;
    for (int i = 0; i < 120; i++) {
        rst = av_read_frame(fmtCtx, packet);
        if (rst < 0) {
            LOG_ERROR << "av_read_frame ERROR:" << rst;
            continue;
        }
        if (packet->stream_index != video_stream_index) {
            continue;
        }
        rst = avcodec_send_packet(decodecCtx, packet);
        if (rst < 0) {
            LOG_ERROR << "avcodec_send_packet ERROR" << rst;
            continue;
        }
        av_packet_unref(packet);
        while (rst >= 0) {
            rst = avcodec_receive_frame(decodecCtx, frame);
            if (rst == AVERROR(EAGAIN) || rst == AVERROR_EOF) {
                break;
            } else if (rst < 0) {
                LOG_ERROR << "avcodec_receive_frame ERROR" << rst;
                break;
            } else if (rst >= 0) {
                fwrite(frame->data[0], 1, frame->linesize[0] * frame->height, file);
                fwrite(frame->data[1], 1, frame->linesize[1] * frame->height / 2, file);
                fwrite(frame->data[2], 1, frame->linesize[2] * frame->height / 2, file);
                fflush(file);
                LOG_INFO << "save frame:" << frameCount++;
            }
            memset(frame, 0, sizeof(AVFrame));
        }
    }
    fclose(file);
    LOG_INFO << "Over";

    return 0;
}
