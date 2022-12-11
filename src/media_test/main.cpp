#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/dict.h>
}
#include "logger.h"

int main()
{
    AVFormatContext* fmtCtx = avformat_alloc_context();
    AVDictionary* options;
    av_dict_set(&options, "input_format", "mjpeg", 0);
    av_dict_set(&options, "video_size", "640*480", 0);
    int rst = avformat_open_input(&fmtCtx, "/dev/video0", NULL, &options);
    if (rst != 0) {
        LOG_DEADLY << "avformat_open_input ERROR" << rst;
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

    
    return 0;
}