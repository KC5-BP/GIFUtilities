
#ifndef __GIF_GET_STRUCTURE_H__
#define __GIF_GET_STRUCTURE_H__

#include <stdio.h>
#include <inttypes.h>

#include "../giftypes.h"

#define GIF_HEADER_START_BYTE 			'G'
#define GIF_GCE_START_BYTE 				'!'
#define GIF_IMG_DESCR_START_BYTE 		','
#define GIF_EOF ';'

void gifFreeStructure(struct gifStructure *gs);

int gifGetFileStructure(FILE *fp, struct gifStructure *gs);

#endif /* __GIF_GET_STRUCTURE_H__ */

