
#ifndef __GIF_UTILITIES__
#define __GIF_UTILITIES__

/* HEADER */
#define GIF_HEADER_SIZE 6
#define GIF_HEADER_START_BYTE 		'G'

/* LOGICAL SCREEN DESCRIPTOR (LSD) */
#define GIF_LSD_SIZE 7
#define GIF_LSD_WIDTH_OFFSET 		 0
#define GIF_LSD_HEIGHT_OFFSET 		 2
#define GIF_LSD_GCT_INFOS_OFFSET	 4
#define GIF_LSD_BACKGROUND_OFFSET	 5
#define GIF_LSD_TRANSPARENT_OFFSET	 6

/* GRAPHICAL CONTROL EXTENSION (GCCE) */
#define GIF_GCE_PIC_SIZE 8
#define GIF_GCE_ANIM_SIZE 19
#define GIF_GCE_START_BYTE 			'!'

/* IMAGE DESCRIPTOR */
#define GIF_IMG_DESCR_SIZE 10
#define GIF_IMG_DESCR_LCT_OFFSET 	 9
#define GIF_IMG_DESCR_START_BYTE 	','

/* IMAGE DATAS */
#define GIF_IMG_DATAS_SIZE 2
#define GIF_IMG_DATAS_SUBBLOCK_OFFSET	1

/* TRAILER */
#define GIF_EOF ';'

typedef enum {
	GIF_PIC_EXT_CODE=0xF9, GIF_ANIM_EXT_CODE=0xFF
} gifExtCode;

/* *** Sub-modules *** */
#include "GIFStructure/GIFGetStructure.h"
#include "GIFReader/GIFReader.h"

#endif /* __GIF_UTILITIES__ */

