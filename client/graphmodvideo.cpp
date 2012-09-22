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

GraphModVideo::~GraphModVideo()
{
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

bool GraphModVideo::setupVideoStream(QSize srcSize, QSize dstSize,
                                     int bitrate, int frameRate)
{
    int ret;
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

    codecCtx->width     = dstSize.width();
    codecCtx->height    = dstSize.height();

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

    frameRgb = new dFrame(PIX_FMT_RGB32, srcSize.width(), srcSize.height());
    frameYuv = new dFrame(PIX_FMT_YUV420P, dstSize.width(), dstSize.height());

    swsCtx = sws_getCachedContext(swsCtx, srcSize.width(), srcSize.height(), frameRgb->pixfmt,
                                  dstSize.width(), dstSize.height(), frameYuv->pixfmt, SWS_BICUBIC, NULL, NULL, NULL);

    return true;
}

QByteArray GraphModVideo::getSendData(const QImage &pix)
{
    int size, gotPacket;
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    const void *imgBytes = pix.bits();
    sws_scale(swsCtx, (const uint8_t* const*)&imgBytes, frameRgb->frame.linesize,
              0, frameYuv->frame.height, frameYuv->frame.data, frameYuv->frame.linesize);

    size = avcodec_encode_video2(fmt->streams[0]->codec, &pkt, &frameYuv->frame, &gotPacket);
    if (size >= 0) {

    }
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
