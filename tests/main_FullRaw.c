
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include "../modules/gifcsts.h"
#include "../modules/giftypes.h"

/* *** *** */
#define LIGHT_CYAN "\033[1;36m"
#define ORANGE     "\033[0;33m"
#define GREEN      "\033[0;32m"
#define RED        "\033[0;31m"
#define RED_BOLD   "\033[1;31m"
#define NC         "\033[0m"

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
        ct->palette[i].r = (unsigned char) getc(fp);
        ct->palette[i].g = (unsigned char) getc(fp);
        ct->palette[i].b = (unsigned char) getc(fp);
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
    } else if (byte == GIF_ANIM_EXT_CODE) {
        datas->anim.gce.extCode = byte;
    } else {
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
    printf("Header: %s\n", version);

    /* *** *** */
    rc = readLsd(fp, &lsd);
    if (rc) {
        printf("Error during LOGICAL SCREEN DESCRIPTOR reading\n");
        fclose(fp);
        free(version);
        return -1;
    }
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
        printf("File does have a Global Color Table\n");
        printf("# of colors (2^%d): %d\n", lsd.bitDepth, gct.nCol);
        printf(" i ) -R- -G- -B-\n");
        for (i = 0; i < gct.nCol; ++i) {
            printf("%3d) %3d %3d %3d\n", i, gct.palette[i].r, \
                    gct.palette[i].g, \
                    gct.palette[i].b);
        }
    } else {
        printf("File does %sNOT%s have a Global Color Table\n", RED_BOLD, NC);
    }

    /* *** *** */
    rc = readDatas(fp, gct.nCol * sizeof(struct rgb), &datas);
    if (rc) {
        printf("Error during DATAS reading\n");
        fclose(fp);
        free(version);
        free(gct.palette);
        return -1;
    }
    printf("File type: ");
    if (datas.img.gce.extCode == GIF_PIC_EXT_CODE) {
        printf("Picture (Code: %#x)\n", datas.img.gce.extCode);
    } else if (datas.anim.gce.extCode == GIF_ANIM_EXT_CODE) {
        printf("Animation (Code: %#x)\n", datas.anim.gce.extCode);
    } else {
        printf("Unknown\n");
    }

    /* *** *** */
    fclose(fp);
    free(version);
    free(gct.palette);

    return 0;
}
