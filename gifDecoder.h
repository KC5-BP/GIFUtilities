/*..--------------------------------------------------------------------------.
../ .------------------------------------------------------------------------. \
./´/
|x| __--""'\
|x|  ,__  -'""`;
|x| /   \  /"'  \
|x|   __// \-"-_/
|x| ´"   \  |           > Module: gifDecoder
|x| \     |  \  _.-"',
|x| "^,-´\/\  '" ,--. \
|x|  \|\| | | , /    | |
|x|     '`'\|._ |   / /
|x|         '\),/  / |          > Creation: July 2023
|x|           |/.-"_/           > By: KC5-BP
|x| .__---+-_/'|--"
|x|         _| |_--,            > Description :
|x|        ',/ |   /                > Functions to fully extract informations
|x|        /|| |  /                 > of GIF Picture or Animation.
|x|     |\| |/ |- |
|x| .-,-/ | /  '/-"
|x| -\|/-/\/ ^,'|
|x| _-     |  |/
|x|  .  --"  /
|x| /--__,.-/
.\`\_________________________________________________________________________/´/
..`___________________________________________________________________________´
===============================================================================>
============================================================================= */

#ifndef __GIFDECODER_H__
#define __GIFDECODER_H__

#include <stdio.h>

#define GIF_HEADER_SIZE     		 6
#define GIF_APPLICATION_NAME_SIZE	11
#define N_DIMENSION_BYTE    		 2

#define GIF_GCE_START 		'!'
#define GIF_FRAME_START     ','
#define GIF_SUBBLOCK_END      0
#define GIF_EOF             ';'

#define FMT_BYTE            "%02x(%c) "

/**********************************************
 * Type redefinition to ease modulation
 *********************************************/
typedef int position_t;
typedef unsigned char color_t;

/**********************************************
 *********************************************/
typedef enum {
	GIF_PIC_EXT=0xF9, GIF_ANIMATION_EXT=0xFF
} tagGifExt;

/**********************************************
 *********************************************/
struct dimension {
    position_t width;
    position_t height;
};

/**********************************************
 *********************************************/
struct location {
    position_t x;
    position_t y;
};

/**********************************************
 *********************************************/
struct rgb {
    color_t r;
    color_t g;
    color_t b;
};

/**********************************************
 *********************************************/
struct logicalScreenDescriptor {
    struct dimension logicDim;
    unsigned char gctInfos;
    unsigned char bitDepth : 4;	/* Bit depth -1 => Largest = 7 + 1 */
    unsigned char hasGct   : 1;
    unsigned char backgroundIndex;
    unsigned char pxAspectRatio;
};

/**********************************************
 *********************************************/
struct colorTable {
    int nPal;
    struct rgb *palette;
};

/**********************************************
 *********************************************/
struct gceGifPicture {
	
};

/**********************************************
 *********************************************/
struct imgDescriptor {
};

/**********************************************
 *********************************************/
struct gceGifAnimation {
	char applicationName[GIF_APPLICATION_NAME_SIZE+1]; /* + '\0' */
};


/**********************************************
 *********************************************/
struct gifFile {
    char header[GIF_HEADER_SIZE+1]; /* + '\0' */
	struct logicalScreenDescriptor lsd;
	struct colorTable gct;
    tagGifExt extCode;
    int nGceDatas;
    int hasTransparency;
    int frameDelay; /* in hundreth */
};

/**********************************************
 *********************************************/
struct gifFile *gifStructAllocate(void);

/**********************************************
 *********************************************/
void gifStructFree(struct gifFile *gf);

/**********************************************
 *********************************************/
void gifGetHeader(FILE *fp, struct gifFile *gf);

/**********************************************
 *********************************************/
void gifGetLogicalDims(FILE *fp, struct gifFile *gf);

/**********************************************
 *********************************************/
void gifGetLogicalScreenDescr(FILE *fp, struct gifFile *gf);

/**********************************************
 *********************************************/
void gifGetGct(FILE *fp, struct gifFile *gf);

/**********************************************
 *********************************************/
void gifGetCommonGce(FILE *fp, struct gifFile *gf);

/**********************************************
 *********************************************/
void gifGetSpecificGce(FILE *fp, struct gifFile *gf);

/**********************************************
 *********************************************/
void gifGetImgDescr(FILE *fp, struct gifFile *gf);

#endif /* __GIFDECODER_H__ */

