/*..-------------------------------------------------------------------------.
../ .-----------------------------------------------------------------------. \
./´/
|x| ____--""'\
|x| .  ,__  -'""`;
|x|  \/   \  /"'  \
|x|     __// \-"-_/
|x| "`´"   \  |         > Module: GIF Types
|x|  /\     |  \  _.-"',
|x| ^""^,-´\/\  '" ,--. \
|x| /  \|\| | | , /    | |
|x|       '`'\|._ |   / /
|x|           '\),/  / |    > Creation: August 2023
|x|             |/.-"_/     > By: D3ep
|x| _..__---+-_/'|--"
|x|           _| |_--,      > Description:
|x|          ',/ |   /          ° Structure & Union types
|x|          /|| |  /           ° of a GIF file's representation
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

#ifndef __GIF_TYPES_H__
#define __GIF_TYPES_H__

#include <stdio.h>
#include <inttypes.h>

/**********************************************
 * GIF Extension code / tag                   *
 * to identify the file type                  *
 ******************************************** */
typedef enum {
    GIF_PIC_EXT_CODE=0xF9, GIF_ANIM_EXT_CODE=0xFF
} gifExtCode;

/* *** STRUCTURE ************************************* */
/**********************************************
 * Section's infos                            *
 ******************************************** */
struct sectionInfos {
    fpos_t filePos;
    uint8_t firstByte;
    uint32_t subBlockSize;  /* in Bytes */
};

/**********************************************
 *                                            *
 ******************************************** */
struct frameDataSections {
    struct sectionInfos lzwMinCode; /* .subBlockSize gives 
                                     * the full section size */
    uint32_t nSubBlocks;
    struct sectionInfos *rawDatas;  /* Array based on nSubBlocks */
};

/**********************************************
 *                                            *
 ******************************************** */
struct frameSections {
    struct sectionInfos gce;
    struct sectionInfos descriptor;
    struct sectionInfos *lct;       /* NULL if no Local Color Table */
    struct frameDataSections dataSec;
};

/**********************************************
 *                                            *
 ******************************************** */
struct animSections {
    struct sectionInfos gce;
    uint32_t nFrames;
    struct frameSections *frames;
};

/**********************************************
 * Union to determine whether the file's      *
 * content is a simple image or an animation  *
 ******************************************** */
union dataSections {
    struct frameSections img;
    struct animSections anim;
};

/**********************************************
 *                                            *
 ******************************************** */
struct gifStructure {
    struct sectionInfos header;
    struct sectionInfos lsd;
    struct sectionInfos *gct;   /* NULL if no Global Color Table */
    gifExtCode ec;
    union dataSections dataComposition;
    struct sectionInfos trailer;
};

/* *************************************************** */

/* *** CONTENT *************************************** */
/**********************************************
 * Type redefinition to ease modulation
 **********************************************/
typedef int position_t;
typedef unsigned char color_t;

/**********************************************
 * Dimension of a frame/picture
 **********************************************/
struct dimension {
    position_t width;
    position_t height;
};

/**********************************************
 * Position from North-West corner's frame
 **********************************************/
struct location {
    position_t x;
    position_t y;
};

/**********************************************
 * RGB Palette
 **********************************************/
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
 **********************************************/
struct logicalScreenDescriptor {
    struct dimension logicDim;
    unsigned char gctInfos;
    //unsigned char bitDepth : 4; /* Bit depth -1 => Largest = 7 + 1 */
    unsigned char bitDepth;     /* Can't pass bit field as arg fn */
    unsigned char hasGct   : 1; // TODO Check if still needed
    unsigned char backgroundIndex;
    unsigned char pxAspectRatio;
};

/**********************************************
 * Color Table (CT) with number of palettes
 * (for both Global & Local CT)
 **********************************************/
struct colorTable {
    int nCol;
    struct rgb *palette;
};

/**********************************************
 * Specifications of a Picture's GCE
 **********************************************/
struct gcePicture {
    int hasTransparency;
    int frameDelay;     /* in hundreth of a second <=> 9 => 90[ms] */
    int transpColNbr;   /* in GCT */
};

/**********************************************
 * Specifications of an Animation's GCE
 **********************************************/
struct gceAnimation {
    char appliName[GIF_GCE_ANIM_APPLI_NAME_SIZE+1]; /* + '\0' */
    int nFrames; /* TODO Verified! Not number of frame at all */
    int currentSubBlockIndex;
    unsigned int nRepetitions;
};

/**********************************************
 * Grouped GCE
 **********************************************/
union gceSpecs {
    struct gcePicture gcePic;
    struct gceAnimation gceAnim;
};

/**********************************************
 * Graphical Control Extension (GCE)
 **********************************************/
struct gce {
    gifExtCode extCode;
    int nGceDatas;
    union gceSpecs gceSpecs;
};

/**********************************************
 *
 **********************************************/
struct imgDescriptor {
    struct location pos;
    struct dimension dim;
    unsigned char lctInfos;
    //unsigned char bitDepth     : 4; /* Bit depth -1 => Largest = 7 + 1 */
    unsigned char bitDepth;         /* Can't pass bit field as arg fn */
    unsigned char isInterlaced : 1;
    unsigned char hasLct       : 1; // TODO Check if still needed
};

/**********************************************
 *
 **********************************************/
struct imgDatas {
    int rawDataSize;
    unsigned char *rawDatas;
};

/**********************************************
 * A Frame is composed of:
 *      a frame's gce (picture's gce)
 *      & an image (see structure above)
 **********************************************/
struct frame {
    struct gce gce;
    //struct img img; /* Simplifies writing => img.img.descr. */
    struct imgDescriptor descr;
    struct colorTable lct;
    int minLzwCodeSize;
    struct imgDatas *datas;
};

/**********************************************
 *
 **********************************************/
struct animation {
	struct gce gce;
	unsigned int nFrames;
	struct frame *frames;
};

/**********************************************
 * Data is either composed of:
 * - a simple image
 * or
 * - a group of images (animation GIF usage)
 **********************************************/
union gifComposition {
    struct frame img;
    struct animation anim;
};

/**********************************************
 * A GIF File is based on the following sections:
 * - GIF's header / signature
 * - Logical Screen Descriptor
 * - Global Color Table
 * - Graphical Control Extension
 * - Data (Picture or Animation)
 **********************************************/
struct gifContent {
    char header[GIF_HEADER_SIZE+1]; /* + '\0' */
    struct logicalScreenDescriptor lsd;
    struct colorTable gct;
    union gifComposition datas;
};

/* *************************************************** */

#endif /* __GIF_TYPES_H__ */

