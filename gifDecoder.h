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

#define GIF_SIGNATURE_SIZE           6
#define GIF_APPLICATION_NAME_SIZE   11
#define N_DIMENSION_BYTE             2

#define TRANSPARENCY_BIT    0
#define GIF_BITFIELD_TRANSPARENCY   0x01
#define GIF_BITFIELD_PAL_BITS       0x07
#define INTERLACED_BIT      6
#define GIF_BITFIELD_INTERLACED     0x40
#define CT_PRESENCE_BIT     7
#define GIF_BITFIELD_CT_PRESENCE    0x80

#define GIF_GCE_START       '!'
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
 * GIF Extension code / tag
 * to identify the file type
 *********************************************/
typedef enum {
    GIF_PIC_EXT=0xF9, GIF_ANIMATION_EXT=0xFF
} tagGifExt;

/**********************************************
 * Dimension of a frame/picture
 *********************************************/
struct dimension {
    position_t width;
    position_t height;
};

/**********************************************
 * Position from North-West corner's frame
 *********************************************/
struct location {
    position_t x;
    position_t y;
};

/**********************************************
 * RGB Palette
 *********************************************/
struct rgb {
    color_t r;
    color_t g;
    color_t b;
};

/**********************************************
 * Logical Screen Descriptor gives infos on:
 * - Logical Dimensions
 * - Global Color Table (GCT)
 * - Background index in GCT
 * - Pixel Aspect Ratio
 *********************************************/
struct logicalScreenDescriptor {
    struct dimension logicDim;
    unsigned char gctInfos;
    unsigned char bitDepth : 4; /* Bit depth -1 => Largest = 7 + 1 */
    unsigned char hasGct   : 1;
    unsigned char backgroundIndex;
    unsigned char pxAspectRatio;
};

/**********************************************
 * Color Table (CT) with number of palettes
 * (for both Global & Local CT)
 *********************************************/
struct colorTable {
    int nPal;
    struct rgb *palette;
};

/**********************************************
 * Specifications of a Picture's GCE
 *********************************************/
struct gcePicture {
    int hasTransparency;
    int frameDelay;     /* in hundreth of a second */
    int transpColNbr;   /* in GCT */
};

/**********************************************
 * Specifications of an Animation's GCE
 *********************************************/
struct gceAnimation {
    char appliName[GIF_APPLICATION_NAME_SIZE+1]; /* + '\0' */
    int nFrames; /* TODO To Be Verified */
    int currentSubBlockIndex;
    unsigned int nRepetitions;
};

/**********************************************
 * Grouped GCE
 *********************************************/
union gceSpecs {
    struct gcePicture gcePic;
    struct gceAnimation gceAnim;
};

/**********************************************
 * Graphical Control Extension (GCE)
 *********************************************/
struct graphicalCtrlExt {
    tagGifExt extCode;
    int nGceDatas;
    union gceSpecs gceSpecs;
};

/**********************************************
 *
 *********************************************/
struct imgDescriptor {
    struct location pos;
    struct dimension dim;
    unsigned char lctInfos;
    unsigned char bitDepth : 4; /* Bit depth -1 => Largest = 7 + 1 */
	unsigned char isInterlaced : 1;
    unsigned char hasLct   : 1;
};

/**********************************************
 *
 *********************************************/
struct imgDatas {
    
};

/**********************************************
 * An Image is composed of:
 *      a description & a data sections
 *********************************************/
struct img {
    struct imgDescriptor descr;
    struct imgDatas *datas;
};

/**********************************************
 * A Frame is composed of:
 *      a frame's gce (picture's gce)
 *      & an image (see structure above)
 *********************************************/
struct frame {
    struct graphicalCtrlExt gceFrame;
    struct img pic;
};

/**********************************************
 * A Data is either a simple image
 * or
 * A group of Frames (animation GIF usage)
 *********************************************/
union data {
    struct img pic;
    struct frame *frames;
};

/**********************************************
 * A GIF File is based on the following sections:
 * - GIF's header / signature
 * - Logical Screen Descriptor
 * - Global Color Table
 * - Graphical Control Extension
 * - Data (Picture or Animation)
 *********************************************/
struct gifFile {
    char header[GIF_SIGNATURE_SIZE+1]; /* + '\0' */
    struct logicalScreenDescriptor lsd;
    struct colorTable gct;
    struct graphicalCtrlExt gce;
    union data datas;
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

