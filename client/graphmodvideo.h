#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

struct dFrame;

class GraphModVideo
{
public:
    GraphModVideo();
    bool setupFormat(const char* format);
    bool setupVideoStream(int sWidth, int sHeight, int dWidth, int dHeight,
                          int bitrate, int frameRate = 20);

private:
    AVOutputFormat  *outFmt;
    AVFormatContext *fmt;
    SwsContext      *swsCtx;

    dFrame  *frameRgb, *frameYuv;
};


struct dFrame {
    AVFrame     frame;
    size_t      bufSize;
    uint8_t     *buffer;
    PixelFormat pixfmt;

    dFrame(PixelFormat pixFmt, int width, int height);
    ~dFrame();
};
