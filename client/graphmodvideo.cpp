#include "graphmodvideo.h"
#include <QDebug>
extern "C" {
#include <libavcodec/avcodec.h>
}

GraphModVideo::GraphModVideo() :
    outFmt(NULL), fmt(NULL), swsCtx(NULL), frameRgb(NULL), frameYuv(NULL)
{
    avcodec_register_all();
    av_register_all();
}

bool GraphModVideo::setupFormat(const char *format)
{
    int ret;

    outFmt = av_guess_format(format, NULL, NULL);
    if (!outFmt) {
        qDebug() << "Could not find format";
        return false;
    }

    if (outFmt->video_codec == AV_CODEC_ID_NONE)
        return false;

    ret = avformat_alloc_output_context2(&fmt, outFmt, NULL, NULL);
    if (ret < 0) {
        qDebug() << "Error format alloc";
        return false;
    }

    return true;
}

bool GraphModVideo::setupVideoStream(int sWidth, int sHeight, int dWidth, int dHeight,
                                     int bitrate, int frameRate)
{
    AVCodecContext  *codecCtx;
    AVCodec         *vCodec;

    AVStream *stream = av_new_stream(fmt, 0);
    if (!stream) {
        qDebug() << "Could not alloc stream";
        return false;
    }

    codecCtx            = stream->codec;
    codecCtx->codec_id  = outFmt->video_codec;
    codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;

    codecCtx->bit_rate  = bitrate;

    codecCtx->width     = dWidth;
    codecCtx->height    = dHeight;

    codecCtx->time_base.den = frameRate;
    codecCtx->time_base.num = 1;
    codecCtx->gop_size      = frameRate * 4;
    codecCtx->pix_fmt       = PIX_FMT_YUV420P;

    vCodec = avcodec_find_encoder(codecCtx->codec_id);
    if (!vCodec) {
        qDebug() << "Could not found";
        return false;
    }

    ret = avcodec_open2(codecCtx, vCodec, NULL);
    if (ret < 0) {
        qDebug() << "Could not open codec";
        return false;
    }

    frameRgb = new dFrame(PIX_FMT_RGB32, sWidth, sHeight);
    frameYuv = new dFrame(PIX_FMT_YUV420P, dWidth, dHeight);

    return true;
}


dFrame::dFrame(PixelFormat pixFmt, int width, int height) :
    pixfmt(pixFmt)
{
    avcodec_get_frame_defaults(&frame);

    bufSize = avpicture_get_size(pixFmt, width, height);
    buffer = new unsigned char [bufSize];
    avpicture_fill((AVPicture*)&frame, buffer, pixfmt, width, height);
}

dFrame::~dFrame()
{
    delete buffer;
}
