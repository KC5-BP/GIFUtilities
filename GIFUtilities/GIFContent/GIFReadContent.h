/*..-------------------------------------------------------------------------.
../ .-----------------------------------------------------------------------. \
./´/
|x| ____--""'\
|x| .  ,__  -'""`;
|x|  \/   \  /"'  \
|x|     __// \-"-_/
|x| "`´"   \  |         > Module: GIF Read Content
|x|  /\     |  \  _.-"',
|x| ^""^,-´\/\  '" ,--. \
|x| /  \|\| | | , /    | |
|x|       '`'\|._ |   / /
|x|           '\),/  / |    > Creation: July 2023
|x|             |/.-"_/     > By: KC5-BP
|x| _..__---+-_/'|--"
|x|           _| |_--,      > Description:
|x|          ',/ |   /          ° Function protos to fully read content
|x|          /|| |  /           ° of a GIF Picture or an Animation.
|x|       |\| |/ |- |
|x| -..-,-/ | /  '/-"
|x| /--\|/-/\/ ^,'|
|x| -__-     |  |/
|x| .  .  --"  /
|x| -\/--__,.-/
.\`\________________________________________________________________________/´/
..`__________________________________________________________________________´
==============================================================================>
============================================================================ */

#ifndef __GIFDECODER_H__
#define __GIFDECODER_H__

#include <stdio.h>

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
    GIF_PIC_EXT_CODE=0xF9, GIF_ANIM_EXT_CODE=0xFF
} gifExtCode;

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
    //unsigned char bitDepth : 4; /* Bit depth -1 => Largest = 7 + 1 */
    unsigned char bitDepth;     /* Can't pass bit field as arg fn */
    unsigned char hasGct   : 1;
    unsigned char backgroundIndex;
    unsigned char pxAspectRatio;
};

/**********************************************
 * Color Table (CT) with number of palettes
 * (for both Global & Local CT)
 *********************************************/
struct colorTable {
    int nCol;
    struct rgb *palette;
};

/**********************************************
 * Specifications of a Picture's GCE
 *********************************************/
struct gcePicture {
    int hasTransparency;
    int frameDelay;     /* in hundreth of a second <=> 9 => 90[ms] */
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
struct gce {
    gifExtCode extCode;
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
    //unsigned char bitDepth     : 4; /* Bit depth -1 => Largest = 7 + 1 */
    unsigned char bitDepth;         /* Can't pass bit field as arg fn */
    unsigned char isInterlaced : 1;
    unsigned char hasLct       : 1;
};

/**********************************************
 *
 *********************************************/
struct imgDatas {
    int minLzwCodeSize;
    int rawDataSize;
    unsigned char *rawDatas;
};

/**********************************************
 * A Frame is composed of:
 *      a frame's gce (picture's gce)
 *      & an image (see structure above)
 *********************************************/
struct frame {
    struct gce gce;
    //struct img img; /* Simplifies writing => img.img.descr. */
    struct imgDescriptor descr;
    struct colorTable lct;
    struct imgDatas *datas;
};

/**********************************************
 * A Data is either:
 * - a simple image
 * or
 * - a group of Frames (animation GIF usage)
 *********************************************/
union gifComposition {
    struct frame img;
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
struct gifContent {
    char header[GIF_SIGNATURE_SIZE+1]; /* + '\0' */
    struct logicalScreenDescriptor lsd;
    struct colorTable gct;
    union gifComposition datas;
};

/**********************************************
 *********************************************/
struct gifFile *gifStructAllocate(void);

/**********************************************
 *********************************************/
void gifStructFree(struct gifFile *gf);

/**********************************************
 *********************************************/
void gifReadHeader(FILE *fp, struct gifFile *gf);

/**********************************************
 *********************************************/
void gifReadLSD(FILE *fp, struct gifFile *gf);

/**********************************************
 *********************************************/
struct rgb *gifReadCt(FILE *fp, struct colorTable *ct, unsigned char *bitDepth);

/**********************************************
 *********************************************/
void gifReadGce(FILE *fp, struct gifFile *gf);

/**********************************************
 *********************************************/
void gifReadImgDescr(FILE *fp, struct gifFile *gf, struct frame *fr);

#endif /* __GIFDECODER_H__ */

