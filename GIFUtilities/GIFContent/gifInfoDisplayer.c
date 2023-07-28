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

#include "gifInfoDisplayer.h"
#include <stdio.h>
#include "gifDecoder.h"

/******************************************************************************/
void gifPrintSignature(char *fmt, struct gifFile *gf) {
    printf(fmt, gf->header);
}

/******************************************************************************/
void gifPrintCt(struct colorTable *ct) {
    printf("   -R-   -G-   -B-\n");
    for (int i = 0; i < ct->nCol; ++i)
        printf("   x%02X   x%02X   x%02X\n", ct->palette[i].r, \
                                             ct->palette[i].g, \
                                             ct->palette[i].b);
}

/******************************************************************************/
void gifPrintLogicalScreenDescriptor(struct gifFile *gf, int printGct) {
    printf("Logical dimension: %dx%d\n",    gf->lsd.logicDim.width, \
            gf->lsd.logicDim.height);
    printf("Global Color Table infos: %#02x\n", gf->lsd.gctInfos);
    printf("'-> Bit depth: %d\n", gf->lsd.bitDepth);
    printf("'-> Presence of a GCT: %s\n", gf->lsd.hasGct ? "YES" : "NO");
    printf("Pixel aspect ratio: %d\n", gf->lsd.pxAspectRatio);

    if (printGct) {
        if ( (! gf->lsd.hasGct) || (! gf->gct.nCol) || (! gf->gct.palette) ) {
            printf("Could not print GCT because file doesn't have one.\n");
            return;
        }

        printf("%s--- Global Color Table ---%s\n", LIGHT_CYAN, NC);
        gifPrintCt(&gf->gct);
    }
}

/******************************************************************************/
void gifPrintGce(struct gifFile *gf) {
    if (gf->gceFile.extCode == GIF_PIC_EXT_CODE) {
        printf("Extension code: %#02x (%s)\n",  gf->gceFile.extCode, \
                                                GIF_PIC_EXT_STRING);
        printf("Amount of GCE Datas: %d\n", gf->gceFile.nGceDatas);
        printf("Has transparency: %s", \
                gf->gceFile.gceSpecs.gcePic.hasTransparency ? "YES" : "NO");
        printf("Frame duration: %d[ms]\n", gf->gceFile.gceSpecs.gcePic.frameDelay);
        printf("Trasparent Color Nbr: %#02x%s", \
                    gf->gceFile.gceSpecs.gcePic.transpColNbr,
                    gf->gceFile.gceSpecs.gcePic.transpColNbr ? "\n" : "(None)\n");
    } else if (gf->gceFile.extCode == GIF_ANIM_EXT_CODE) {
        printf("Extension code: %#02x (%s)\n",  gf->gceFile.extCode, \
                                                GIF_ANIM_EXT_STRING);
        printf("Amount of GCE Datas: %d\n", gf->gceFile.nGceDatas);
        printf("Application Name: %s\n", gf->gceFile.gceSpecs.gceAnim.appliName);
        printf("Animation with \"%d\" frames\n", gf->gceFile.gceSpecs.gceAnim.nFrames);
        printf("Current Sub-block index: %d\n", gf->gceFile.gceSpecs.gceAnim.currentSubBlockIndex);
        printf("Repetitions: %d\n", gf->gceFile.gceSpecs.gceAnim.nRepetitions);
    } else {
        printf("Extension code: %#02x (%s)\n",  gf->gceFile.extCode, \
                                                "Unknown extension code");
    }


    if (gf->gceFile.extCode == GIF_PIC_EXT_CODE) {
    } else if (gf->gceFile.extCode == GIF_ANIM_EXT_CODE) {
    } else {
    }
}

/******************************************************************************/
void gifPrintGcePicture(struct gifFile *gf);

/******************************************************************************/
void gifPrintGceAnimation(struct gifFile *gf);

/******************************************************************************/
void gifPrintImgDescr(struct gifFile *gf) {
    printf("Position from North-West corner: (%d, %d)\n", gf->datas.img.descr.pos.x, gf->datas.img.descr.pos.y);
    printf("Dimension: %dx%d\n", gf->datas.img.descr.dim.width, gf->datas.img.descr.dim.height);

    printf("Local Color Table infos: %#02x\n", gf->datas.img.descr.lctInfos);
    printf("'-> Bit depth: %d\n", gf->datas.img.descr.bitDepth);
    printf("'-> Pic is interlaced: %s\n", gf->datas.img.descr.isInterlaced ? "YES" : "NO");
    printf("'-> Presence of a LCT: %s", gf->datas.img.descr.hasLct ? "YES" : "NO");
}
