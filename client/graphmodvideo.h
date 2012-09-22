#pragma once

#include <QSize>
#include <QByteArray>
#include <QImage>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

struct dFrame;

class GraphModVideo
{
public:
    GraphModVideo();
    ~GraphModVideo();
    bool setupFormat(const char* format);
    bool setupVideoStream(QSize srcSize, QSize dstSize,
                          int bitrate, int frameRate = 20);

    QByteArray getSendData(const QImage& pix);

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
