
#include <stdio.h>
#include <math.h>

typedef enum {
	GIF_PIC_EXT_CODE=0xF9, GIF_ANIM_EXT_CODE=0xFF
} gifExtCode;

struct sectionInfos {
	fpos_t pos;
	int subBlockSize; /* in Bytes */
	char startByte;
};

struct imgDatasSection {
	struct sectionInfos lzwMinCode;
	int nSubBlocks;
	struct sectionInfos *rawDatas;
};

struct frameSections {
	struct sectionInfos gce;
	struct sectionInfos imgDescriptor;
	struct imgDatasSection imgDatas;
};

union dataSections {
	struct frameSections imgFrame;
	struct frameSections *animFrames;
};

struct gifSections {
	struct sectionInfos header;
	struct sectionInfos lsd;
	struct sectionInfos gct;
	gifExtCode extCode;
	union dataSections;
};

int main(int argc, char **argv) {
	FILE *fp;
	char *fPath;
	char c;
	struct gifSections gs;

	if (argc < 2) {
        printf("File to open not given... Aborted!\n");
        return -1;
    }

    fPath = argv[1];
    fp = fopen(fPath, "r");

    if ( ! fp ) {
        printf("Couldn't open \"%s\"\n", fPath);
        return -1;
    }

    printf("--- Start reading file ---\n");
	fgetpos(fp, &gs.header.pos);
	gs.header.subBlockSize = 6;
	gs.header.startByte = fgetc(fp);
	printf("Read '%c' expected '%c'\n", gs.header.startByte, 'G');

	/* Skip header from beginning, given by SEEK_SET */
	fseek(fp, gs.header.subBlockSize, SEEK_SET);
	fgetpos(fp, &gs.lsd.pos);
	gs.lsd.subBlockSize = 7;
	gs.lsd.startByte = fgetc(fp); /* Expected 1st Width byte */
	printf("Read '%#02x' expected '%#02x' (with ../pics/img_2x2.gif)\n", gs.lsd.startByte, 2);

	/* Skip remaining Width byte + Height bytes to reach GCT's */
	fseek(fp, 3, SEEK_CUR);
	c = fgetc(fp);
	gs.gct.subBlockSize = pow(2, (c & 0x07) + 1);
	/* Skip Background + Transparent */
	fseek(fp, 2, SEEK_CUR);
	fgetpos(fp, &gs.gct.pos);
	gs.gct.startByte = fgetc(fp); /* Expected 1st color RED byte (with order RGB) */
	printf("Read '%#02x' expected '%#02x' (with ../pics/img_2x2.gif)\n", gs.gct.startByte, 0);

	if (gs.gct.subBlockSize) {
		printf("Present of a GCT, skipping it...\n");
		fseek(fp, gs.gct.subBlockSize * 3 - 1, SEEK_CUR);
	}
	
	if ()

	c = fgetc(fp);
	printf("Read '%#02x'(%c) expected '%c' (with ../pics/img_2x2.gif)\n", c, c, '!');

	fclose(fp);
    printf("--- Stop reading file ---\n");

	return 0;
}

