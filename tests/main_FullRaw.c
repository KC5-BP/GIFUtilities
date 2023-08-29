
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "../modules/gifcsts.h"

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

    /* Restart file */
    rc = fseek(fp, 0, SEEK_SET);
    if (rc) DBG("%s: fseek failed (%s)\n", __func__, strerror(errno));
    rc = 0;

    ptrRc = fgets(*version, GIF_HEADER_SIZE + 1, fp);
    /* Error: fgets successful? */
    rc = ( ptrRc ) ? rc : -1;

    /* Error: Does the Header starts correctly? */
    rc = (*version[0] == GIF_HEADER_START_BYTE) ? rc : -1;

    RESTORE_CURRENT_FILE_POS(fp);

    return rc;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("No file has been selected... Abort!\n");
        return -1;
    }

    char *fPath = argv[1];
    FILE *fp;
    int rc;
    char *version;

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

    rc = readHeader(fp, &version);
    if (rc) {
        printf("Error during HEADER acquisition\n");
        fclose(fp);
		free(version);
        return -1;
    }

    printf("Header: %s\n", version);

    fclose(fp);
	free(version);

    return 0;
}
