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

/* *** STRUCTURE **************************** */
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

/* ****************************************** */

/* *** CONTENT ****************************** */
/* ****************************************** */

#endif /* __GIF_TYPES_H__ */

