//=====================================================================
//
// rtsp_2_mp4.cpp - 
//
// Created by wwq on 2022/11/09
// Last Modified: 2022/11/09 13:10:59
// ffmpeg -re -i cut_bb.mp4 -vcodec libx264 -f rtsp -rtsp_transport tcp rtsp://192.168.11.137:8554/live/streamid
//
//=====================================================================
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
 
#ifdef __cplusplus
extern "C" {
#endif
 
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


#ifdef __cplusplus
}
#endif

#include <string.h>
#include <pthread.h>
#include <unistd.h>
 
 
// #include "gh_rtsp2mp4.h"
 
 
#define RTSP "rtsp://admin:quantum7@192.168.1.144"
 
//经过实验，这个值最好
#define PTS_VALUE 4500
#define MAX_FRAMES 300
 
static bool g_RunningFlag = true;
 
int rtsp2mp4(const char* pInputFileName, const char* pOutputFileName)
{
    AVOutputFormat *ofmt = NULL;
    //Input AVFormatContext and Output AVFormatContext
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
 
    int ret=0;
    int video_index = -1;
    int frame_index = 0;
    int dts = 0;
    int pts = 0;
 
    // av_register_all();
    //Network
    avformat_network_init();
    //Input
    if ((ret = avformat_open_input(&ifmt_ctx, pInputFileName, 0, 0)) < 0)
    {
        printf("Could not open input file.");
        goto end;
    }
 
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
    {
        printf("Failed to retrieve input stream information");
        goto end;
    }
 
    for (size_t i = 0; i<ifmt_ctx->nb_streams; i++)
    {
        if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_index = i;
            break;
        }
    }
 
    av_dump_format(ifmt_ctx, 0, pInputFileName, 0);
 
    //Output
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, pOutputFileName); //RTMP
    if (!ofmt_ctx)
    {
        printf("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
 
    ofmt = ofmt_ctx->oformat;
    for (size_t i = 0; i < ifmt_ctx->nb_streams; i++)
    {
        //Create output AVStream according to input AVStream
        AVStream *in_stream  = ifmt_ctx->streams[i];
		// 这个接口传参由于instream->codec改为instream->codecpar，需要改写
        // AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
		AVCodec *codec = avcodec_find_encoder(in_stream->codecpar->codec_id);
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, codec);
        if (!out_stream)
        {
            printf("Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }
 
        //Copy the settings of AVCodecContext
		 //ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
		 // //过时的写法
        //// ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
        //if (ret < 0)
        //{
            //printf("Failed to copy context from input to output stream codec context\n");
            //goto end;
        //}
 
        //// out_stream->codec->codec_tag = 0;
		///*ffmpeg中，avformat_write_header 调用init_muxer时会判断par->codec_tag，如果不为0，会进行附加验证*/
		//out_stream->codecpar->codec_tag = 0;
        //if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        //{
            //out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        //}
	
		AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
		ret = avcodec_parameters_to_context(codec_ctx, in_stream->codecpar);
		if (ret < 0){
		    printf("Failed to copy in_stream codecpar to codec context\n");
			goto end;
		}
		 
		codec_ctx->codec_tag = 0;
		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			    codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		 
		ret = avcodec_parameters_from_context(out_stream->codecpar, codec_ctx);
		if (ret < 0){
			printf("Failed to copy codec context to out_stream codecpar context\n");
			goto end;
		}
    }
 
    //Dump Format------------------
    av_dump_format(ofmt_ctx, 0, pOutputFileName, 1);
    //Open output URL
    if (!(ofmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&ofmt_ctx->pb, pOutputFileName, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            printf("Could not open output URL '%s'", pOutputFileName);
            goto end;
        }
    }
 
    //Write file header
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0)
    {
        printf("Error occurred when opening output URL\n");
        goto end;
    }
 
    #if USE_H264BSF
        AVBitStreamFilterContext* h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
    #endif
 
    while (g_RunningFlag)
    {
        AVStream *in_stream, *out_stream;
        //Get an AVPacket
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
        {
            break;
        }
 
        in_stream  = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
 
        //Convert PTS/DTS
#if 1
        if (pts == 0)
        {
            pts = pkt.pts;
            dts = pkt.dts;
        }
        else
        {
            pkt.pts += pts;
            pkt.dts += dts;
        }
        if (pkt.dts < pkt.pts)
        {
            pkt.dts = pkt.pts;
        }
#else
        pkt.pts += 4500*frame_index;
        pkt.dts += 4500*frame_index;
#endif
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
 
        if (pkt.stream_index == video_index)
        {
            //printf("Receive %8d video frames from RTSP\n", frame_index);
            frame_index++;
    #if USE_H264BSF
                av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
    #endif
        }
 
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0 && frame_index > 3)
        {
            printf("Error muxing packet\n");
            break;
        }
 
        // av_free_packet(&pkt);
		av_packet_unref(&pkt);
 
        if (frame_index > MAX_FRAMES)
        {
            break;
        }
    }
 
    #if USE_H264BSF
        av_bitstream_filter_close(h264bsfc);
    #endif
 
    //Write file trailer
    av_write_trailer(ofmt_ctx);
 
end:
    avformat_close_input(&ifmt_ctx);
    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
    {
        avio_close(ofmt_ctx->pb);
    }
    avformat_free_context(ofmt_ctx);
 
    if (ret < 0 && ret != AVERROR_EOF)
    {
        printf("Error occurred.\n");
        return -1;
    }
    return 0;
}
 
// int main(int argc, char **argv)
// {
//     rtsp2mp4(RTSP, "rtsp.mp4");
//     return 0;
// }
