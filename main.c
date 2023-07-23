
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gifDecoder.h"

#if 1
#define DBG(FMT, ...) 				\
	do {							\
		printf(FMT, ##__VA_ARGS__);	\
	} while(0)
#else
#define DBG(FMT, ...)
#endif

#define LIGHT_CYAN "\033[1;36m"
#define ORANGE     "\033[0;33m"
#define GREEN      "\033[0;32m"
#define RED        "\033[0;31m"
#define NC         "\033[0m"

int main(int argc, char **argv) {
    FILE *fp;
    char *fPath;
    struct gifFile *gf;
    unsigned char c;
    int i;
    int width = 0, height = 0;
    int posX = 0, posY = 0;
    int lzwCodeSize, nDatasSubBlock;

    if (argc < 2) {
        printf("%sFile to open not given... Aborted!%s\n", RED, NC);
        return -1;
    }
    
    gf = gifStructAllocate();
    if ( ! gf ) {
        printf("%sCouldn't allocate \"struct gifFile\"... Aborted!%s\n", RED, NC);
        return -1;
    }

    fPath = argv[1];
    fp = fopen(fPath, "r");

    if ( ! fp ) {
        printf("%sCouldn't open \"%s\"%s\n", RED, fPath, NC);
        return -1;
    }

    printf("%s--- Start reading file ---%s\n", GREEN, NC);
    printf("%s--- GIF Signature ---%s\n", LIGHT_CYAN, NC);
    gifGetHeader(fp, gf);
    printf("%s\n", gf->header);

    printf("%s--- Logical Screen Descriptor ---%s\n", LIGHT_CYAN, NC);
    gifGetLogicalScreenDescr(fp, gf);
    printf("Logical dimension: %dx%d\n", gf->lsd.logicDim.width, \
										 gf->lsd.logicDim.height);
    printf("Global Color Table: %#0x\n", gf->lsd.gctInfos);
    printf("'-> Bit depth: %d\n", gf->lsd.bitDepth);
    printf("'-> Presence of a GCT: %s\n", gf->lsd.hasGct ? "YES" : "NO");
    printf("Default pixel aspect ratio: %d\n", gf->lsd.pxAspectRatio);

    if (gf->lsd.hasGct) {
        printf("%s--- Global Color Table ---%s\n", LIGHT_CYAN, NC);
        gifGetGct(fp, gf);
        printf("-R- -G- -B-\n");
        for (i = 0; i < gf->gct.nPal; ++i)
            printf("x%02X x%02X x%02X\n",	gf->gct.palette[i].r, \
											gf->gct.palette[i].g, \
											gf->gct.palette[i].b);
    }

    printf("%s--- Graphic Control Extension ---%s\n", LIGHT_CYAN, NC);
    gifGetCommonGce(fp, gf);
    gifGetSpecificGce(fp, gf);
    printf("Extension code: %s\n",  (gf->gce.extCode == GIF_PIC_EXT) ?          \
                                    ("Graphic Control Extension") :         \
                                    ((gf->gce.extCode == GIF_ANIMATION_EXT) ?   \
                                        ("Application Extension") :         \
                                        ("Not recognized")));
    printf("Amount of GCE Datas: %d\n", gf->gce.nGceDatas);
    //printf("Has transparency: %s\n", gf->hasTransparency ? "YES" : "NO");

    if (gf->gce.extCode == GIF_PIC_EXT) {        /* Simple GIF   */
        printf("\n%s--- Image Descriptor ---%s\n", LIGHT_CYAN, NC);
        for (i = 0; i < 10; ++i) {
            c = fgetc(fp);
            printf(FMT_BYTE, (unsigned int)c, c);
            switch (i) {
                case 0: /* Image descriptor start ',' */
                    printf("\n");
                    break;

                case 1: /* From North-West position: X */
                    posX += c;
                    break;
                case 2:
                    posX += (c * 256);
                    break;

                case 3: /* From North-West position: Y */
                    posY += c;
                    break;
                case 4:
                    posY += (c * 256);
                    printf("(=> (%d, %d) )\n", posX, posY);
                    break;

                case 5: /* Image dimension: W */
                    width += c;
                    break;
                case 6:
                    width += (c * 256);
                    printf("(=> (W:%d) )\n", width);
                    break;

                case 7: /* Image dimension: H */
                    height += c;
                    break;
                case 8:
                    height += (c * 256);
                    printf("(=> (H:%d) )\n", height);
                    break;

                case 9: /* Local color table bit */
                    printf("\n");
                    if (c) {
                        unsigned char c2 = 0;
                        int nPal = pow(2, (c & 0x07) + 1);
                        printf("nPal: %d\n", nPal);
                        for (int j = 0; j < nPal * 3; j++) {
                            if (j && ((j % 3) == 0)) printf("\n");
                            c2 = fgetc(fp);
                            printf(FMT_BYTE, c2, c2);
                        }
                    }
                    break;

                default:
                    printf("\n");
                    break;
            }
        }

        printf("\n%s--- Image Datas ---%s\n", LIGHT_CYAN, NC);
        lzwCodeSize = (int) fgetc(fp);
        printf(FMT_BYTE "(LZW Code size)\n", (unsigned int)lzwCodeSize, lzwCodeSize);

        nDatasSubBlock = (int) fgetc(fp);
        printf(FMT_BYTE "(Sublock size)\n", (unsigned int)nDatasSubBlock, nDatasSubBlock);
        
        for (i = 0; i < nDatasSubBlock; i++) {
            c = fgetc(fp);
            printf(FMT_BYTE, (unsigned int)c, c);
            if (i && ((i % 10) == 0))   printf("\n");
        }
        
        c = fgetc(fp);
        printf("\n" FMT_BYTE "\n", (unsigned int)c, c);
        if (c != 0) {
            printf("Supposed \"End of GCE block\" (0), got %0x\n", c);
            fclose(fp);
            gifStructFree(gf);
            return -1;
        }
    } else if (gf->gce.extCode == GIF_ANIMATION_EXT) { /* Animated GIF */
    } else {
        printf("Not managed for now ... Abort!\n");
        fclose(fp);
        gifStructFree(gf);
        return -1;
    }

    printf("%sWait user input ...%s\n", ORANGE, NC);
    getchar();

    c = fgetc(fp);
    printf(FMT_BYTE, (unsigned int)c, c);

    if (c != ';') {
        printf("Supposed \"File Terminaison\" (%c), got %c\n", c, c);
        fclose(fp);
        gifStructFree(gf);
        return -1;
    }
    
    printf("\n%s--- Stop reading file ---%s\n", GREEN, NC);

    fclose(fp);
    gifStructFree(gf);

    return 0;
}
