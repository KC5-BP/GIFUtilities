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
|x|        ',/ |   /                >   Give the ability to use GIF's decoder
|x|        /|| |  /                 >   functions from associated 'C' file.
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

#include "gifDecoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**********************************************************************************/
struct gifFile *gifStructAllocate(void) {
    return (struct gifFile *) calloc(1, sizeof(struct gifFile));
}

void gifStructFree(struct gifFile *gf) {
    free(gf->gct.palette);
    free(gf);
}
void gifGetHeader(FILE *fp, struct gifFile *gf) {
    for (int i = 0; i < GIF_SIGNATURE_SIZE; ++i)
        gf->header[i] = fgetc(fp);
}

void gifGetLogicalDims(FILE *fp, struct gifFile *gf) {
    unsigned char c;
    int i;

    for (i = 0; i < N_DIMENSION_BYTE; ++i) {
        c = fgetc(fp);
        if (i)  gf->lsd.logicDim.width += ((int)c * 256);
        else    gf->lsd.logicDim.width += c;
    }

    for (i = 0; i < N_DIMENSION_BYTE; ++i) {
        c = fgetc(fp);
        if (i)  gf->lsd.logicDim.height += ((int)c * 256);
        else    gf->lsd.logicDim.height += c;
    }
}

void gifGetLogicalScreenDescr(FILE *fp, struct gifFile *gf) {
    /* *** Logical Dimensions *** */
    gifGetLogicalDims(fp, gf);

    /* *** Global Color Table Descriptor *** */
    gf->lsd.gctInfos = (unsigned char) fgetc(fp);
    gf->lsd.bitDepth = (gf->lsd.gctInfos & GIF_BITFIELD_PAL_BITS) + 1;
    gf->lsd.hasGct   = (gf->lsd.gctInfos & GIF_BITFIELD_CT_PRESENCE) >> CT_PRESENCE_BIT;

    /* *** Background Color Index *** */
    gf->lsd.backgroundIndex = (unsigned char) fgetc(fp);

    /* *** Pixel Aspect Ratio *** */
    gf->lsd.pxAspectRatio = (unsigned char) fgetc(fp);
}

void gifGetGct(FILE *fp, struct gifFile *gf) {
    gf->gct.nPal = pow(2, gf->lsd.bitDepth);
    gf->gct.palette = (struct rgb *) malloc(gf->gct.nPal * sizeof(struct rgb));

    if ( ! gf->gct.palette ) {
        printf("Failed rgb allocation...\n");
        return;
    }
    
    for (int i = 0; i < gf->gct.nPal; ++i) {
        gf->gct.palette[i].r = (unsigned char) getc(fp);
        gf->gct.palette[i].g = (unsigned char) getc(fp);
        gf->gct.palette[i].b = (unsigned char) getc(fp);
    }
}

void gifGetCommonGce(FILE *fp, struct gifFile *gf) {
    unsigned char c;

    c = fgetc(fp);
    if (c != GIF_GCE_START) {
        printf("Couldn't identify Graphic Control introduction ('%c')\n", \
               GIF_GCE_START);
        fclose(fp);
        gifStructFree(gf);
        exit(-1);
    }

    gf->gce.extCode   = (tagGifExt) getc(fp);
    gf->gce.nGceDatas = (int) getc(fp);
}

void gifGetSpecificGce(FILE *fp, struct gifFile *gf) {
    unsigned char c;

    if (gf->gce.extCode == GIF_PIC_EXT_CODE) {
        for (int i = 0; i < gf->gce.nGceDatas; ++i) {
            c = fgetc(fp);
            switch(i) {
                case 0:
                    gf->gce.gceSpecs.gcePic.hasTransparency = \
										((int) c) & TRANSPARENCY_BIT;
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
        c = fgetc(fp);
        if (c != GIF_SUBBLOCK_END) {
            printf("Couldn't identify Sub-block end sequence\n");
            fclose(fp);
            gifStructFree(gf);
            exit(-1);
        }
    } else if (gf->gce.extCode == GIF_ANIM_EXT_CODE) {
        for (int i = 0; i < GIF_APPLICATION_NAME_SIZE; ++i) {
            c = fgetc(fp);
            switch(i) {
                case 0:
                    break;

                case 1:
                    break;
            }
            printf("%02x(%c) ", (unsigned int)c, c);
        }
    }
}

void gifGetImgDescr(FILE *fp, struct gifFile *gf) {
    gf->datas.pic.descr.isInterlaced = (gf->datas.pic.descr.lctInfos & GIF_BITFIELD_INTERLACED) >> INTERLACED_BIT;
}

