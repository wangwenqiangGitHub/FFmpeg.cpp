//source: keyframe.cpp
#include <stdio.h>
#include <string.h>
#include <iostream>

#define __STDC_CONSTANT_MACROS
extern "C" {

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavutil/pixfmt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <jpeglib.h>
}

using namespace std;

char errbuf[256];
char timebuf[256];
// 利用AVFormatContext, 可以获取相关格式(容器)的信息
static AVFormatContext *fmt_ctx = NULL;
// 编解码器上下文
static AVCodecContext *video_dec_ctx = NULL;
static int width, height;
static enum AVPixelFormat pix_fmt;
// 存储每一个视频/音频流信息的结构体
static AVStream *video_stream = NULL;
static const char *src_filename = NULL;
static const char *output_dir = NULL;
static int video_stream_idx = -1;
// AVFrame是包含码流参数较多的结构体
static AVFrame *frame = NULL;
static AVFrame *pFrameRGB = NULL;
// AVPacket是存储压缩编码数据相关信息的结构体
static AVPacket pkt;
static struct SwsContext *pSWSCtx = NULL;
static int video_frame_count = 0;

/* Enable or disable frame reference counting. You are not supposed to support
 * both paths in your application but pick the one most appropriate to your
 * needs. Look for the use of refcount in this example to see what are the
 * differences of API usage between them. */
static int refcount = 0;
static void jpg_save(uint8_t *pRGBBuffer, int iFrame, int width, int height);

static int decode_packet(int *got_frame, int cached)
{
    int ret = 0;
    int decoded = pkt.size;
    *got_frame = 0;

    if (pkt.stream_index == video_stream_idx)
    {
        /* decode video frame */
        ret = avcodec_decode_video2(video_dec_ctx, frame, got_frame, &pkt);
        if (ret < 0)
        {
            fprintf(stderr, "Error decoding video frame (%s)\n", av_make_error_string(errbuf, sizeof(errbuf), ret));
            return ret;
        }
        if (*got_frame)
        {
            if (frame->width != width || frame->height != height ||
                frame->format != pix_fmt)
            {
                /* To handle this change, one could call av_image_alloc again and
                 * decode the following frames into another rawvideo file. */
                fprintf(stderr, "Error: Width, height and pixel format have to be "
                                "constant in a rawvideo file, but the width, height or "
                                "pixel format of the input video changed:\n"
                                "old: width = %d, height = %d, format = %s\n"
                                "new: width = %d, height = %d, format = %s\n",
                        width, height, av_get_pix_fmt_name(pix_fmt),
                        frame->width, frame->height,
                        av_get_pix_fmt_name((AVPixelFormat)frame->format));
                return -1;
            }

            video_frame_count++;
            static int iFrame = 0;
			printf("iFramke:%d\n", frame->key_frame);
            if (frame->key_frame == 1) //如果是关键帧
            {
				printf("<%s %d>\n",__func__, __LINE__);
				
                sws_scale(pSWSCtx, frame->data, frame->linesize, 0,
                          video_dec_ctx->height,
                          pFrameRGB->data, pFrameRGB->linesize);
                // 保存到磁盘
                iFrame++;
                jpg_save(pFrameRGB->data[0], iFrame, width, height);
            }
        }
    }
    /* If we use frame reference counting, we own the data and need
     * to de-reference it when we don't use it anymore */
    if (*got_frame && refcount)
        av_frame_unref(frame);
    return decoded;
}

static int open_codec_context(int *stream_idx,
                              AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
	// AVCodec组件能让我们知道如何编解码这个流
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;
    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(type), src_filename);
        return ret;
    }
    else
    {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];
        /* find decoder for the stream */
		// 找到已经注册的解码器返回AVCodec，让我们知道如何编解码这个流
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec)
        {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }
        /* Allocate a codec context for the decoder */
		// 编解码器上下文
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx)
        {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                    av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }
        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0)
        {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(type));
            return ret;
        }
        /* Init the decoders, with or without reference counting */
        av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
        if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0)
        {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }
    return 0;
}

static int get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry
    {
        enum AVSampleFormat sample_fmt;
        const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
            {AV_SAMPLE_FMT_U8, "u8", "u8"},
            {AV_SAMPLE_FMT_S16, "s16be", "s16le"},
            {AV_SAMPLE_FMT_S32, "s32be", "s32le"},
            {AV_SAMPLE_FMT_FLT, "f32be", "f32le"},
            {AV_SAMPLE_FMT_DBL, "f64be", "f64le"},
    };
    *fmt = NULL;
    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++)
    {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt)
        {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }
    fprintf(stderr,
            "sample format %s is not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return -1;
}

int save_picture()
{
    int ret = 0, got_frame;
    int numBytes = 0;
    uint8_t *buffer;
    // if (argc != 3 && argc != 4)
    // {
    //     fprintf(stderr, "usage: %s [-refcount] input_file ouput_dir\n"
    //                     "API example program to show how to read frames from an input file.\n"
    //                     "This program reads frames from a file, decodes them, and writes bmp keyframes\n"
    //                     "If the -refcount option is specified, the program use the\n"
    //                     "reference counting frame system which allows keeping a copy of\n"
    //                     "the data for longer than one decode call.\n"
    //                     "\n",
    //             argv[0]);
    //     exit(1);
    // }

    // if (argc == 4 && !strcmp(argv[1], "-refcount"))
    // {
    //     refcount = 1;
    //     argv++;
    // }

    // src_filename = argv[1];
    // output_dir = argv[2];

    src_filename = "test.flv";
    output_dir = "test_a";
    /* open input file, and allocate format context */
	// 打开一个文件读取文件的头信息，利用相关格式的简要信息填充AVFormatContext(注意，编解码器通常不会打开),需要使用
	// avformat_open_input函数，该函数需要AVFormatContext，文件名和两个可选参数:AVInputFormat(如果为NULL,FFMPEG将参测格式)
	// AVDictionary(解封装参数)
    if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0)
    {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        exit(1);
    }
	// 输出视频的格式和时长
	printf("Format %s, duration %lld us", fmt_ctx->iformat->long_name, fmt_ctx->duration);
    /* retrieve stream information */
	// 为了访问数据流，我们需要从媒体文件中读取数据，需要利用函数avformat_find_stream_info完成此步骤
	// fmt_ctx->nb_streams将获取所有的流信息，并且通过fmt_ctx->stream[i]将获取到指定的i数据流(AVStream)
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
    {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }

	// 打开视频解码器
    if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0)
    {
        video_stream = fmt_ctx->streams[video_stream_idx];
        /* allocate image where the decoded image will be put */
        width = video_dec_ctx->width;
        height = video_dec_ctx->height;
        pix_fmt = video_dec_ctx->pix_fmt;
    }
    else
    {
        goto end;
    }

    /* dump input information to stderr */
    av_dump_format(fmt_ctx, 0, src_filename, 0);
    if (!video_stream)
    {
        fprintf(stderr, "Could not find video stream in the input, aborting\n");
        ret = 1;
        goto end;
    }

    pFrameRGB = av_frame_alloc();
    numBytes = avpicture_get_size(AV_PIX_FMT_BGR24, width, height);
    buffer = (uint8_t*)av_malloc(numBytes);
    avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_BGR24, width, height);
    pSWSCtx = sws_getContext(width, height, pix_fmt, width, height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

    frame = av_frame_alloc();
    if (!frame)
    {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    if (video_stream)
        printf("Demuxing video from file '%s' to dir: %s\n", src_filename, output_dir);

    /* read frames from the file */
	// 使用av_read_frame函数读取帧数据来填充数据包
    while (av_read_frame(fmt_ctx, &pkt) >= 0)
    {
        AVPacket orig_pkt = pkt;
        do
        {
            ret = decode_packet(&got_frame, 0);
            if (ret < 0)
                break;
            pkt.data += ret;
            pkt.size -= ret;
        } while (pkt.size > 0);
        av_packet_unref(&orig_pkt);
    }

    /* flush cached frames */
    pkt.data = NULL;
    pkt.size = 0;

    end:
    if (video_dec_ctx)
        avcodec_free_context(&video_dec_ctx);
    if (fmt_ctx)
        avformat_close_input(&fmt_ctx);
    if (buffer)
        av_free(buffer);
    if (pFrameRGB)
        av_frame_free(&pFrameRGB);
    if (frame)
        av_frame_free(&frame);
    return ret < 0;
}

static void jpg_save(uint8_t *pRGBBuffer, int iFrame, int width, int height)
{

    struct jpeg_compress_struct cinfo;

    struct jpeg_error_mgr jerr;

    char szFilename[1024];
    int row_stride;

    FILE *fp;
    JSAMPROW row_pointer[1]; // 一行位图
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    sprintf(szFilename, "%s/image-%03d.jpg", output_dir, iFrame); //图片名字为视频名+号码
    fp = fopen(szFilename, "wb");

    if (fp == NULL)
        return;

    jpeg_stdio_dest(&cinfo, fp);

    cinfo.image_width = width; // 为图的宽和高，单位为像素
    cinfo.image_height = height;
    cinfo.input_components = 3;     // 在此为1,表示灰度图， 如果是彩色位图，则为3
    cinfo.in_color_space = JCS_RGB; //JCS_GRAYSCALE表示灰度图，JCS_RGB表示彩色图像

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 80, 1);

    jpeg_start_compress(&cinfo, TRUE);

    row_stride = cinfo.image_width * 3; //每一行的字节数,如果不是索引图,此处需要乘以3

    // 对每一行进行压缩
    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &(pRGBBuffer[cinfo.next_scanline * row_stride]);
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fclose(fp);
}
