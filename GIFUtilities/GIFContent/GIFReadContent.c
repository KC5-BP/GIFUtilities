/*..-------------------------------------------------------------------------.
../ .-----------------------------------------------------------------------. \
./´/
|x| ____--""'\
|x| .  ,__  -'""`;
|x|  \/   \  /"'  \
|x|     __// \-"-_/
|x| "`´"   \  |         > Module: gifDecoder
|x|  /\     |  \  _.-"',
|x| ^""^,-´\/\  '" ,--. \
|x| /  \|\| | | , /    | |
|x|       '`'\|._ |   / /
|x|           '\),/  / |    > Creation: July 2023
|x|             |/.-"_/     > By: KC5-BP
|x| _..__---+-_/'|--"
|x|           _| |_--,      > Description:
|x|          ',/ |   /          ° Function bodies to fully extract content
|x|          /|| |  /           ° of GIF Picture or Animation.
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

#include "GIFReadContent.h"
#include "../GIFDefines.h"
#include "../GIFStructure/GIFGetStructure.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/******************************************************************************/
struct gifContent *gifStructAllocate(void) {
    return (struct gifContent *) calloc(1, sizeof(struct gifContent));
}

/******************************************************************************/
void gifStructFree(struct gifContent *gf) {
    free(gf->gct.palette);
    if (gf->gce.extCode == GIF_PIC_EXT_CODE) {
        free(gf->datas.img.lct.palette);
        // TODO free to add for Raw + RGB datas
    } else if (gf->gce.extCode == GIF_ANIM_EXT_CODE) {
        for (int i = 0; i < gf->gce.gceSpecs.gceAnim.nFrames; i++) {
            free(gf->datas.frames[i].lct.palette);
            // TODO free to add for Raw + RGB datas
        }
    }
    free(gf);
}

/******************************************************************************/
void gifReadHeader(FILE *fp, struct gifContent *gf) {
    for (int i = 0; i < GIF_HEADER_SIZE; ++i)
        gf->header[i] = fgetc(fp);
}

/******************************************************************************/
void gifReadDims(FILE *fp, struct dimension *dim) {
    unsigned char c;
    int i;

    for (i = 0; i < N_DIM_BYTES; ++i) {
        c = fgetc(fp);
        if (i)  dim->width += ((int)c * 256);
        else    dim->width += c;
    }

    for (i = 0; i < N_DIM_BYTES; ++i) {
        c = fgetc(fp);
        if (i)  dim->height += ((int)c * 256);
        else    dim->height += c;
    }
}

/******************************************************************************/
void gifReadLSD(FILE *fp, struct gifContent *gf) {
    /* *** Logical Dimensions *** */
    gifReadDims(fp, &gf->lsd.logicDim);

    /* *** Global Color Table Descriptor *** */
    gf->lsd.gctInfos = (unsigned char) fgetc(fp);
    gf->lsd.bitDepth = (gf->lsd.gctInfos & CT_BITFIELD_PAL_BITS) + 1;
    gf->lsd.hasGct   = (gf->lsd.gctInfos & CT_BITFIELD_CT_PRESENCE) \
                       >> CT_PRESENCE_BIT;

    /* *** Background Color Index *** */
    gf->lsd.backgroundIndex = (unsigned char) fgetc(fp);

    /* *** Pixel Aspect Ratio *** */
    gf->lsd.pxAspectRatio = (unsigned char) fgetc(fp);
}

/******************************************************************************/
struct rgb *gifReadCt(FILE *fp, struct colorTable *ct, unsigned char *bits) {
    ct->nCol = pow(2, *bits);
    ct->palette = (struct rgb *) malloc(ct->nCol * sizeof(struct rgb));

    if ( ! ct->palette )    return ct->palette;

    for (int i = 0; i < ct->nCol; ++i) {
        ct->palette[i].r = (unsigned char) getc(fp);
        ct->palette[i].g = (unsigned char) getc(fp);
        ct->palette[i].b = (unsigned char) getc(fp);
    }

    return ct->palette;
}

/******************************************************************************/
void gifReadCommonGce(FILE *fp, struct gifContent *gf) {
    unsigned char c;

    c = fgetc(fp);
    if (c != GIF_GCE_START_BYTE) {
        printf("Couldn't identify Graphic Control introduction ('%c')\n", \
                GIF_GCE_START_BYTE);
        gifStructFree(gf);
        fclose(fp);
        exit(-1);
    }

    gf->gce.extCode   = (gifExtCode) getc(fp);
    gf->gce.nGceDatas = (int) getc(fp);
}

/******************************************************************************/
void gifReadSpecificGce(FILE *fp, struct gifContent *gf) {
    unsigned char c;
    int i;

    if (gf->gce.extCode == GIF_PIC_EXT_CODE) {
        for (i = 0; i < gf->gce.nGceDatas; i++) {
            c = fgetc(fp);
            switch(i) {
                case 0:
                    gf->gce.gceSpecs.gcePic.hasTransparency = \
                        ((int) c) & CT_TRANSPARENCY_BIT;
                    break;
                case 1: case 2: /* Frame delay not used for picture */
                    gf->gce.gceSpecs.gcePic.frameDelay = (int)c;
                    break;
                case 3: /* Color number of transparent pixel in GCT */
                    gf->gce.gceSpecs.gcePic.transpColNbr = (int)c;
                    break;
                default:
                    break;
            }
        }
    } else if (gf->gce.extCode == GIF_ANIM_EXT_CODE) {
        //for (i = 0; i < GIF_APPLICATION_NAME_SIZE; i++)
        for (i = 0; i < gf->gce.nGceDatas; i++)
            gf->gce.gceSpecs.gceAnim.appliName[i] = fgetc(fp);

        /* *** Nbr of Frames *** */
        gf->gce.gceSpecs.gceAnim.nFrames = (int) fgetc(fp);

        /* *** Current Sub-block index *** */
        gf->gce.gceSpecs.gceAnim.currentSubBlockIndex = (int) fgetc(fp);

        /* *** Nbr of Frames *** */
        for (i = 0; i < 2; i++) {
            if (i) gf->gce.gceSpecs.gceAnim.nRepetitions += \
                       ((int) fgetc(fp)) * 256;
            else   gf->gce.gceSpecs.gceAnim.nRepetitions += \
                       ((int) fgetc(fp));
        }
    }

    c = fgetc(fp);
    if (c != 0) {
        printf("Couldn't identify Sub-block end sequence\n");
        gifStructFree(gf);
        fclose(fp);
        exit(-1);
    }
}

/******************************************************************************/
void gifReadGce(FILE *fp, struct gifContent *gf) {
    gifReadCommonGce(fp, gf);
    gifReadSpecificGce(fp, gf);
}

/******************************************************************************/
void gifReadImgDescr(FILE *fp, struct gifContent *gf, struct frame *fr) {
    unsigned char c;

    for (int i = 0; i < 7; ++i) {
        switch (i) {
            case 0: /* Image descriptor start ',' */
                c = fgetc(fp);
                break;

            case 1: /* From North-West position: X */
                c = fgetc(fp);
                fr->descr.pos.x += c;
                break;
            case 2:
                c = fgetc(fp);
                fr->descr.pos.x += (c * 256);
                break;

            case 3: /* From North-West position: Y */
                c = fgetc(fp);
                fr->descr.pos.y += c;
                break;
            case 4:
                c = fgetc(fp);
                fr->descr.pos.y += (c * 256);
                break;

            case 5: /* Image dimension: W */
                gifReadDims(fp, &fr->descr.dim);
                break;

            case 6: /* Local color table bit */
                c = fgetc(fp);
                fr->descr.lctInfos = c;
                fr->descr.bitDepth = \
                    (fr->descr.lctInfos & CT_BITFIELD_PAL_BITS) + 1;

                fr->descr.isInterlaced =                            \
                    (fr->descr.lctInfos & CT_BITFIELD_INTERLACED)  \
                    >> CT_INTERLACED_BIT;

                fr->descr.hasLct =                                  \
                    (fr->descr.lctInfos & CT_BITFIELD_CT_PRESENCE) \
                    >> CT_PRESENCE_BIT;

                if (fr->descr.hasLct)
                    gifReadCt(fp, &fr->lct, &fr->descr.bitDepth);
                break;

            default:
                break;
        }
    }
}

