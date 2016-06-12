/*
 * MISTIKA common definitions
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

#ifndef AVFORMAT_MISTIKA_H
#define AVFORMAT_MISTIKA_H

#define MISTIKA_MAGIC 395726
#define MISTIKA_BLOCK_SIZE 4096

#define DM_PACKING_RGB      	    (1000)
#define DM_PACKING_RGBA             (1002)
#define DM_PACKING_YUV422           (1008)
#define DM_PACKING_YUV422X2         (1100)
#define DM_PACKING_RGB10    	    (8001)
#define DM_PACKING_YUV422_10	    (8100)
#define DM_PACKING_YUV422_10X2      (8101)
#define DM_PACKING_RGBA_16          (8200)
#define DM_PACKING_RGBA_EXR         (8300)
#define DM_PACKING_YUV420           (8400)

#endif /* AVFORMAT_MISTIKA_H */
