/*
 * Mistika demuxer
 * Copyright (c) 2015 Bengt Ove Sannes
 *
 * Based on YUV4MPEG demuxer
 * Copyright (c) 2001, 2002, 2003 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "avformat.h"
#include "libavutil/intfloat_readwrite.h"
#include "libavutil/intreadwrite.h"
#include "internal.h"
#include "mistika.h"

#define MAX_MISTIKA_HEADER 300


static int mistika_read_header(AVFormatContext *s)
{
	int32_t bytesPerFrame;
    int32_t 	magicNumber;	//must be 395726
    int32_t	version;    	//only version 3 currently used
    int32_t 	numFrames;  	
    int32_t	blockSize;  	//should be 4096
    int32_t	truncMode;  	//irrelevant for reading
    int32_t	sizeX;
    int32_t	sizeY;
    int32_t 	packing;    	//should be one of the defined DM_PACKING values
    double  	rate;	    	
    int32_t	interlacing;	//0: progressive, 1: interlaced.
    char    	timeCode[128];  //null terminated string containing timecode, as frame or tc
    char    	tapeName[128];  //null terminated tape name string 	
	int bottomUp = 0;
	
	
    enum PixelFormat pix_fmt = PIX_FMT_NONE;
    enum AVChromaLocation chroma_sample_location = AVCHROMA_LOC_UNSPECIFIED;
    AVStream *st;
	
	magicNumber = avio_rb32(s->pb);
	version = avio_rb32(s->pb);
	numFrames = avio_rb32(s->pb);
	blockSize = avio_rb32(s->pb);
	truncMode = avio_rb32(s->pb);
	sizeX = avio_rb32(s->pb);
	sizeY = avio_rb32(s->pb);
	packing = avio_rb32(s->pb);
	rate = av_int2dbl(avio_rb64(s->pb));
	interlacing = avio_rb32(s->pb);
	avio_read(s->pb, timeCode, 128);
	avio_read(s->pb, tapeName, 128);
	
	// DM_PACKING_RGB      	    (1000)
	// DM_PACKING_RGBA             (1002)
	// DM_PACKING_YUV422           (1008)
	// DM_PACKING_YUV422X2         (1100)
	// DM_PACKING_RGB10    	    (8001)
	// DM_PACKING_YUV422_10	    (8100)
	// DM_PACKING_YUV422_10X2      (8101)
	// DM_PACKING_RGBA_16          (8200)
	// DM_PACKING_RGBA_EXR         (8300)
	// DM_PACKING_YUV420           (8400)
	
	switch(packing)
    {
	case 1000:
		av_log(s, AV_LOG_ERROR, "RGB8 streams are not yet supported.\n" );
        return -1;
		bottomUp = 1;
		pix_fmt =  PIX_FMT_BGR24;
		bytesPerFrame = sizeY * sizeX * 3;
		break;
	case 1002:
		av_log(s, AV_LOG_ERROR, "RGBA8 streams are not yet supported.\n" );
        return -1;
		bottomUp = 1;
		pix_fmt =  PIX_FMT_BGRA;
		bytesPerFrame = sizeY * sizeX * 4;
		break;
	case 1008:
		pix_fmt =  PIX_FMT_UYVY422;
		bytesPerFrame = sizeY * sizeX * 2;
		break;
	case 1100:
		av_log(s, AV_LOG_ERROR, "Dual YUV422 8bit streams are not yet supported.\n" );
        return -1;
	case 8001:
		av_log(s, AV_LOG_ERROR, "RGB10 streams are not yet supported.\n" );
        return -1;
		bottomUp = 1;
		bytesPerFrame = sizeY * sizeX * 4;
		break;
	case 8101:
		av_log(s, AV_LOG_ERROR, "Dual YUV422 10bit streams are not yet supported.\n" );
        return -1;
	case 8100:
		av_log(s, AV_LOG_ERROR, "YUV422 10bit streams are not yet supported.\n" );
        return -1;
		pix_fmt =  PIX_FMT_YUV422P10BE;
		bytesPerFrame = (sizeY * sizeX * 5) / 2;
		break;
	case 8200:
		av_log(s, AV_LOG_ERROR, "RGBA16 streams are not yet supported.\n" );
        return -1;
		bottomUp = 1;
		bytesPerFrame = sizeY * sizeX * 8;
		break;
	case 8300:
		av_log(s, AV_LOG_ERROR, "JS EXR streams are not yet supported.\n" );
        return -1;
	case 8400:
		av_log(s, AV_LOG_ERROR, "JS 420 streams are not yet supported.\n" );
        return -1;
		pix_fmt =  PIX_FMT_YUV420P;
		bytesPerFrame = (sizeY * sizeX * 3 ) / 2;
		break;
	default:    av_log(s, AV_LOG_ERROR, "Mistika stream contains an unsupported pixel format.\n" );
                return -1;
    }

	bytesPerFrame = ((bytesPerFrame + blockSize - 1) / blockSize) * blockSize;
	
    st = av_new_stream(s, 0);
    if (!st)
        return AVERROR(ENOMEM);
    st->codec->width  = sizeX;
    st->codec->height = sizeY;
    av_set_pts_info(st, 64, 1, rate);
    st->r_frame_rate.num = rate;
    st->r_frame_rate.den = 1;
    // st->time_base                = av_d2q(1.0, rate);
    // st->avg_frame_rate                = av_d2q(rate, INT_MAX);
    st->nb_frames = st->duration      = numFrames;
    st->codec->pix_fmt                = pix_fmt;
	if (bottomUp == 1)
		st->codec->extradata  = av_strdup("BottomUp");
    st->codec->codec_type             = AVMEDIA_TYPE_VIDEO;
    st->codec->codec_id               = CODEC_ID_RAWVIDEO;
    st->codec->chroma_sample_location = chroma_sample_location;
	if (interlacing == 0)
		st->codec->interlaced = -1;
	else
		st->codec->interlaced = 2;
	av_dict_set(&st->metadata, "timeCode", timeCode, 0);
	av_dict_set(&st->metadata, "tapeName", tapeName, 0);
	
	avio_seek(s->pb, bytesPerFrame, SEEK_SET ); // header is padded to frame size
    return 0;
}

static int mistika_read_packet(AVFormatContext *s, AVPacket *pkt)
{
   int packet_size, ret, width, height;
    AVStream *st = s->streams[0];

    width = st->codec->width;
    height = st->codec->height;

    packet_size = avpicture_get_size(st->codec->pix_fmt, width, height);
	packet_size = ((packet_size + MISTIKA_BLOCK_SIZE - 1) / MISTIKA_BLOCK_SIZE) * MISTIKA_BLOCK_SIZE;
    if (packet_size < 0)
        return -1;

    ret = av_get_packet(s->pb, pkt, packet_size);
    pkt->pts = pkt->dts = (pkt->pos - packet_size ) / packet_size;

    if (ret < 0)
        return ret;
	else if (ret != packet_size)
        return s->pb->eof_reached ? AVERROR_EOF : AVERROR(EIO);
	
    return 0;
}

static int mistika_probe(AVProbeData *p)
{

	const uint8_t *b = p->buf;

    if (AV_RB32(b) == MISTIKA_MAGIC)
        return AVPROBE_SCORE_MAX;
    return 0;
}

AVInputFormat ff_mistika_demuxer = {
    .name           = "mistika",
    .long_name      = NULL_IF_CONFIG_SMALL("SGO Mistika .js format"),
    .read_probe     = mistika_probe,
    .read_header    = mistika_read_header,
    .read_packet    = mistika_read_packet,
    .extensions     = "js",
};
