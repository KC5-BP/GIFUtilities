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
void gifPrintLogicalScreenDescriptor(struct gifFile *gf, int printGct) {
    printf("Logical dimension: %dx%d\n",    gf->lsd.logicDim.width, \
            gf->lsd.logicDim.height);
    printf("Global Color Table infos: %#02x\n", gf->lsd.gctInfos);
    printf("'-> Bit depth: %d\n", gf->lsd.bitDepth);
    printf("'-> Presence of a GCT: %s\n", gf->lsd.hasGct ? "YES" : "NO");
    printf("Pixel aspect ratio: %d\n", gf->lsd.pxAspectRatio);

    if (printGct) {
        if ( (! gf->lsd.hasGct) || (! gf->gct.nPal) || (! gf->gct.palette) ) {
            printf("Could not print GCT because file doesn't have one.\n");
            return;
        }

        printf("%s--- Global Color Table ---%s\n", LIGHT_CYAN, NC);
        printf("    -R-    -G-    -B-\n");
        for (int i = 0; i < gf->gct.nPal; ++i)
            printf("    x%02X    x%02X    x%02X\n", gf->gct.palette[i].r, \
                    gf->gct.palette[i].g, \
                    gf->gct.palette[i].b);
    }
}

/******************************************************************************/
void gifPrintGce(struct gifFile *gf) {
    printf("Extension code: %s\n",  (gf->gce.extCode == GIF_PIC_EXT_CODE) ? \
            (GIF_PIC_EXT_STRING) :                                          \
            ((gf->gce.extCode == GIF_ANIM_EXT_CODE) ? \
             (GIF_ANIM_EXT_STRING) :                  \
             ("Unknown extension code")));
    printf("Amount of GCE Datas: %d\n", gf->gce.nGceDatas);

    if (gf->gce.extCode == GIF_PIC_EXT_CODE) {
        printf("Has transparency: %s\n", \
                gf->gce.gceSpecs.gcePic.hasTransparency ? "YES" : "NO");
    } else if (gf->gce.extCode == GIF_ANIM_EXT_CODE) {
    } else {
    }
}

/******************************************************************************/
void gifPrintGcePicture(struct gifFile *gf);

/******************************************************************************/
void gifPrintGceAnimation(struct gifFile *gf);

/******************************************************************************/
void gifPrintImgDescr(struct gifFile *gf) {
    printf("Position from North-West corner: (%d, %d)\n", gf->datas.pic.descr.pos.x, gf->datas.pic.descr.pos.y);
    printf("Dimension: %dx%d\n", gf->datas.pic.descr.dim.width, gf->datas.pic.descr.dim.height);

    printf("Local Color Table infos: %#02x\n", gf->datas.pic.descr.lctInfos);
    printf("'-> Bit depth: %d\n", gf->datas.pic.descr.bitDepth);
    printf("'-> Pic is interlaced: %s\n", gf->datas.pic.descr.isInterlaced ? "YES" : "NO");
    printf("'-> Presence of a LCT: %s\n", gf->datas.pic.descr.hasLct ? "YES" : "NO");
}
