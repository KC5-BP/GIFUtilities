
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include "limits.h"
#include "../modules/gifcsts.h"
#include "../modules/giftypes.h"

/* *** *** */
#define LIGHT_CYAN_BOLD "\033[1;36m"
#define ORANGE          "\033[0;33m"
#define GREEN           "\033[0;32m"
#define GREEN_BOLD      "\033[1;32m"
#define RED             "\033[0;31m"
#define RED_BOLD        "\033[1;31m"
#define NC              "\033[0m"

/* *** *** */
#define SAVE_CURRENT_FILE_POS(FP)   \
    fpos_t curPos;                  \
    fgetpos(FP, &curPos)

#define RESTORE_CURRENT_FILE_POS(FP)    \
    fsetpos(FP, &curPos)

#if 1
#define DBG(FMT, ...)               \
    do {                            \
        printf(FMT, ##__VA_ARGS__); \
    } while(0)
#else
#define DBG(FMT, ...)
#endif

/* *** *** */
int readHeader(FILE *fp, char **version) {
    int rc;
    char *ptrRc;

    SAVE_CURRENT_FILE_POS(fp);

    /* Restart at file's beginning */
    rc = fseek(fp, 0, SEEK_SET);
    if (rc) DBG("%s%s: fseek failed (%s)%s\n", RED, __func__, \
                                               strerror(errno), NC);

    /* Read at most count - 1 */
    ptrRc = fgets(*version, GIF_HEADER_SIZE + 1, fp);
    /* Error: fgets successful? */
    rc = ( ptrRc ) ? rc : -1;

    /* Error: Does the Header starts correctly? */
    rc = (*version[0] == GIF_HEADER_SENTINEL) ? rc : -1;

    RESTORE_CURRENT_FILE_POS(fp);

    return rc;
}

/* *** *** */
void gifReadDims(FILE *fp, struct dimension *dims) {
    int i;
    unsigned byte;

    for (i = 0; i < N_DIM_BYTES; ++i) {
        byte = fgetc(fp);

        if (i)  dims->width += ((unsigned)byte * 256);
        else    dims->width = byte;
    }

    for (i = 0; i < N_DIM_BYTES; ++i) {
        byte = fgetc(fp);

        if (i)  dims->height += ((unsigned)byte * 256);
        else    dims->height = byte;
    }
}

int readLsd(FILE *fp, struct logicalScreenDescriptor *lsd) {
    int rc;

    SAVE_CURRENT_FILE_POS(fp);

    /* Restart file after header section */
    rc = fseek(fp, GIF_HEADER_SIZE, SEEK_SET);
    if (rc) DBG("%s%s: fseek failed (%s)%s\n", RED, __func__, \
                                               strerror(errno), NC);

    gifReadDims(fp, &lsd->logicDim);

    /* *** Global Color Table Descriptor *** */
    lsd->gctInfos = (unsigned char) fgetc(fp);
    lsd->bitDepth = (lsd->gctInfos & CT_BITFIELD_PAL_BITS) + 1;

    /* *** Background Color Index *** */
    lsd->backgroundIndex = (unsigned char) fgetc(fp);

    /* *** Pixel Aspect Ratio *** */
    lsd->pxAspectRatio = (unsigned char) fgetc(fp);

    RESTORE_CURRENT_FILE_POS(fp);

    return rc;
}

/* *** *** */
int allocateAndReadCt(FILE *fp, unsigned char bitDepth, struct colorTable *ct) {
    ct->nCol = pow(2, bitDepth);
    ct->palette = (struct rgb *) malloc(ct->nCol * sizeof(struct rgb));

    if ( ! ct->palette )    return -1;

    for (int i = 0; i < ct->nCol; ++i) {
        /* Using fgets without testing return gives a smaller executable */
        /*ptrRc = */fgets((char *) (ct->palette+i), sizeof(struct rgb) + 1, fp);
    }

    return 0;
}

int readGct(FILE *fp, char ctInfos, unsigned char bitDepth, \
            struct colorTable *ct) {
    int rc;

    if ((ctInfos & CT_BITFIELD_CT_PRESENCE) >> CT_PRESENCE_BIT) {
        SAVE_CURRENT_FILE_POS(fp);

        /* Restart file after header section */
        rc = fseek(fp, GIF_HEADER_SIZE + GIF_LSD_SIZE, SEEK_SET);
        if (rc) DBG("%s%s: fseek failed (%s)%s\n", RED, __func__, \
                                                   strerror(errno), NC);

        rc = (allocateAndReadCt(fp, bitDepth, ct) < 0) ? -1 : rc;

        RESTORE_CURRENT_FILE_POS(fp);
    } else {
        ct->nCol = 0;
        ct->palette = NULL;
        rc = 0;
    }

    return rc;
}

/* *** *** */
int readImgDescr(FILE *fp, struct frame *fr) {
    int rc;
    unsigned byte;

    byte = fgetc(fp);
    if (byte != GIF_IMG_DESCR_SENTINEL) {
        DBG("%s%s: Read %c(%d) instead of %c(%d)%s\n", RED, __func__,       \
                                            (char) byte, byte,              \
                                            GIF_IMG_DESCR_SENTINEL,         \
                                            (int) GIF_IMG_DESCR_SENTINEL,   \
                                            NC);
    }

    /* TODO: transform gifReadDims param into "void *" */
    gifReadDims(fp, (struct dimension *)&fr->descr.pos);

    gifReadDims(fp, &fr->descr.dim);
    
    /* *** Local Color Table Descriptor *** */
    fr->descr.lctInfos = (unsigned char) fgetc(fp);
    fr->descr.bitDepth = (fr->descr.lctInfos & CT_BITFIELD_PAL_BITS) + 1;

    if ((fr->descr.lctInfos & CT_BITFIELD_CT_PRESENCE) >> CT_PRESENCE_BIT) {
        rc = allocateAndReadCt(fp, fr->descr.bitDepth, &fr->lct);
        if (rc)
            DBG("%s%s: Failure during allocating and reading LCT%s\n", \
                                                    RED, __func__, NC);
    } else {
        fr->lct.nCol = 0;
        fr->lct.palette = NULL;
        rc = 0;
    }

    return rc;
}

int countImgDatasSize(FILE *fp) {
    int rc, size = 0;
    unsigned byte;

    SAVE_CURRENT_FILE_POS(fp);

    do {
        byte = fgetc(fp);
        size += byte;

        rc = fseek(fp, byte, SEEK_CUR);
        if (rc) DBG("%s%s: fseek failed (%s)%s\n", RED, __func__, \
                                                   strerror(errno), NC);
    } while (byte != GIF_END_OF_SECTION);

    RESTORE_CURRENT_FILE_POS(fp);

    return size;
}

int readImgDatas(FILE *fp, struct frame *fr) {
    unsigned byte;

    fr->minLzwCodeSize = fgetc(fp);

    fr->fullDatasLength = countImgDatasSize(fp);

    fr->datas = (char *) malloc(fr->fullDatasLength);
    if ( ! fr->datas )  return -1;

    byte = fgetc(fp);   /* Read first size */
    for (int i = 0; byte != GIF_END_OF_SECTION; i++) {
        fgets(fr->datas + i * UCHAR_MAX, byte + 1, fp); /* Read DATAS */

        byte = fgetc(fp);                               /* Read next size */
    }

    return 0;
}

int readFrame(FILE* fp, struct frame *fr, char extCodeFoundOutside) {
    unsigned byte;

    /* GCE */
    if ( ! extCodeFoundOutside ) {
        /* Read SENTINEL + EXTENSION CODE */
        byte = fgetc(fp);
        if (byte != GIF_GCE_SENTINEL) {
            DBG("%s%s: Read %c(%d) instead of %c(%d)%s\n", RED, __func__, \
                                                (char) byte, byte,        \
                                                GIF_GCE_SENTINEL,         \
                                                (int) GIF_GCE_SENTINEL,   \
                                                NC);
        }

        fr->gce.extCode = fgetc(fp);
    }

    fr->gce.nGceDatas = fgetc(fp);

    fr->gce.gceSpecs.gcePic.hasTransparency = fgetc(fp);

    for (int i = 0; i < N_DIM_BYTES; ++i) {
        byte = fgetc(fp);

        if (i)  fr->gce.gceSpecs.gcePic.frameDelay += ((unsigned)byte * 256);
        else    fr->gce.gceSpecs.gcePic.frameDelay = byte;
    }
    fr->gce.gceSpecs.gcePic.frameDelay *= 10; /* To milliseconds */

    fr->gce.gceSpecs.gcePic.transpColNbr = fgetc(fp);

    byte = fgetc(fp);
    if (byte != GIF_END_OF_SECTION) {
        DBG("%s%s: Read %c(%d) instead of %c(%d)%s\n", RED, __func__, \
                                            (char) byte, byte,        \
                                            GIF_END_OF_SECTION,       \
                                            (int) GIF_END_OF_SECTION, \
                                            NC);
    }

    readImgDescr(fp, fr);

    readImgDatas(fp, fr);

    return 0;
}

/* *** *** */
int readAnimation(...) {
    /* GCE */

    // Count frames

    // for each ( frames )
    //      readFrame(..., false);
    return 0;
}

/* *** *** */
int readDatas(FILE *fp, long gctOffset, union gifComposition *datas) {
    int rc;
    unsigned char byte;

    SAVE_CURRENT_FILE_POS(fp);

    /* Restart file after header section */
    rc = fseek(fp, GIF_HEADER_SIZE + GIF_LSD_SIZE + gctOffset, SEEK_SET);
    if (rc) DBG("%s%s: fseek failed (%s)%s\n", RED, __func__, \
                                               strerror(errno), NC);

    byte = fgetc(fp);
    if (byte != GIF_GCE_SENTINEL) {
        RESTORE_CURRENT_FILE_POS(fp);
        return rc ? rc : -1;
    }

    byte = fgetc(fp);
    if (byte == GIF_PIC_EXT_CODE) {
        datas->img.gce.extCode = byte;

        readFrame(fp, &datas->img, 1);

    } else if (byte == GIF_ANIM_EXT_CODE) {
        datas->anim.gce.extCode = byte;

        // readAnimation(fp, &datas->anim);

        /* TODO Move followings inside readAnimation() */
        datas->anim.gce.nGceDatas = fgetc(fp);

        /* Dedicated part */
    } else {
        rc = rc ? rc : -1;
    }

    byte = fgetc(fp);
    if (byte != GIF_EOF) {
        DBG("%s%s: Read %c(%d) instead of %c(%d)%s\n", RED, __func__,   \
                                            (char) byte, byte, GIF_EOF, \
                                            (int) GIF_EOF, NC);
        rc = rc ? rc : -1;
    }

    RESTORE_CURRENT_FILE_POS(fp);

    return rc;
}

/* *** *** */
int main(int argc, char **argv) {
    //printf("EOF: %d (%#x)\n", EOF, EOF);
    if (argc < 2) {
        printf("No file has been selected... Abort!\n");
        return -1;
    }

    char *fPath = argv[1];
    FILE *fp;
    int rc, i;
    char *version;
    struct logicalScreenDescriptor lsd;
    struct colorTable gct;
    union gifComposition datas;

    version = (char *) malloc(GIF_HEADER_SIZE + 1);
    if ( ! version ) {
        printf("Couldn't allocate \"version\". Abort!\n");
        return -1;
    }

    fp = fopen(fPath, "r");
    if ( ! fp ) {
        printf("Couldn't open \"%s\". Abort!\n", fPath);
        free(version);
        return -1;
    }

    /* *** ACTUAL READING FILE PART *** */
    rc = readHeader(fp, &version);
    if (rc) {
        printf("Error during HEADER reading\n");
        fclose(fp);
        free(version);
        return -1;
    }
    printf("%s --- Header --- %s\n", LIGHT_CYAN_BOLD, NC);
    printf("Version: %s\n", version);

    /* *** *** */
    rc = readLsd(fp, &lsd);
    if (rc) {
        printf("Error during LOGICAL SCREEN DESCRIPTOR reading\n");
        fclose(fp);
        free(version);
        return -1;
    }
    printf("%s --- Logical Screen Descriptor --- %s\n", LIGHT_CYAN_BOLD, NC);
    printf("Logical Dimension: %dx%d\n", lsd.logicDim.width, lsd.logicDim.height);
    printf("GCT info: %#x\n", lsd.gctInfos);
    printf("Bit depth: %d\n", lsd.bitDepth);
    printf("Transparent background index: %d\n", lsd.backgroundIndex);
    printf("Pixel aspect ratio: %d\n", lsd.pxAspectRatio);

    /* *** *** */
    rc = readGct(fp, lsd.gctInfos, lsd.bitDepth, &gct);
    if (rc) {
        printf("Error during GLOBAL COLOR TABLE reading\n");
        fclose(fp);
        free(version);
        free(gct.palette);
        return -1;
    }
    if (gct.nCol) {
        printf("File %sdoes have%s a Global Color Table\n", GREEN_BOLD, NC);
        printf("# of colors (2^%d): %d\n", lsd.bitDepth, gct.nCol);
        printf(" i ) -R- -G- -B-\n");
        for (i = 0; i < gct.nCol; ++i) {
            printf("%3d) %3d %3d %3d\n", i, gct.palette[i].r, \
                                            gct.palette[i].g, \
                                            gct.palette[i].b);
        }
    } else {
        printf("File %sdoes NOT have%s a Global Color Table\n", RED_BOLD, NC);
    }

    /* *** *** */
    rc = readDatas(fp, gct.nCol * sizeof(struct rgb), &datas);
    if (rc) {
        printf("Error during DATAS reading\n");
        fclose(fp);
        free(version);
        free(gct.palette);
        if (datas.img.gce.extCode == GIF_PIC_EXT_CODE) {
            free(datas.img.lct.palette);
            free(datas.img.datas);
        } else if (datas.anim.gce.extCode == GIF_ANIM_EXT_CODE) {
        } else {
        }
        return -1;
    }
    printf("%s --- Datas --- %s\n", LIGHT_CYAN_BOLD, NC);
    printf("File type: ");
    if (datas.img.gce.extCode == GIF_PIC_EXT_CODE) {
        printf("Picture (Code: %#x)\n", datas.img.gce.extCode);
        printf("\t# of datas in sub-block: %d\n", datas.img.gce.nGceDatas);
        printf("\tTransparent field: %#x\n",  \
                                datas.img.gce.gceSpecs.gcePic.hasTransparency);
        printf("\tFrame delay: %d[ms] (Useless for this file)\n", \
                                datas.img.gce.gceSpecs.gcePic.frameDelay);
        printf("\tTransparent index in Color Table: %d\n\n",      \
                                datas.img.gce.gceSpecs.gcePic.transpColNbr);

        printf("\tPosition from North/West: (%d, %d)\n",                    \
                                                    datas.img.descr.pos.x,  \
                                                    datas.img.descr.pos.y);
        printf("\tDimension: %dx%d\n", datas.img.descr.dim.width, \
                                       datas.img.descr.dim.height);
        printf("\tLCT info: %#x\n", datas.img.descr.lctInfos);
        printf("\tBit Depth: %d\n", datas.img.descr.bitDepth);
        if (datas.img.lct.nCol) {
            printf("\tFrame %sdoes have%s a Local Color Table\n",   \
                                                                GREEN_BOLD, NC);
            printf("\t# of colors (2^%d): %d\n", datas.img.descr.bitDepth,  \
                                                 datas.img.lct.nCol);
            printf("\t i ) -R- -G- -B-\n");
            for (i = 0; i < datas.img.lct.nCol; ++i) {
                printf("\t%3d) %3d %3d %3d\n", i, datas.img.lct.palette[i].r, \
                                                  datas.img.lct.palette[i].g, \
                                                  datas.img.lct.palette[i].b);
            }
        } else {
            printf("\tFrame %sdoes NOT have%s a Local Color Table\n",   \
                                                                RED_BOLD, NC);
        }

        printf("\n\tLZW (Lempel Ziv Welch) Minimum code length: %d\n",  \
                                                datas.img.minLzwCodeSize);
        printf("\tDatas length: %d\n\t", datas.img.fullDatasLength);
        for (i = 0; i < datas.img.fullDatasLength; i++) {
            printf("%02x ", (unsigned char) datas.img.datas[i]);
            if ((i + 1) % 10 == 0)  printf("\n\t");
        }
    } else if (datas.anim.gce.extCode == GIF_ANIM_EXT_CODE) {
        printf("Animation (Code: %#x)\n", datas.anim.gce.extCode);
        printf("\t# of datas in sub-block: %d\n", datas.anim.gce.nGceDatas);
    } else {
        printf("Unknown\n");
    }

    /* *** *** */
    fclose(fp);
    free(version);
    free(gct.palette);

    if (datas.img.gce.extCode == GIF_PIC_EXT_CODE) {
        free(datas.img.lct.palette);
        free(datas.img.datas);
    } else if (datas.anim.gce.extCode == GIF_ANIM_EXT_CODE) {
    } else {
    }

    return 0;
}
