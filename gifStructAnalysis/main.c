
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include "gifStructure.h"

#define ARF(VAR) (unsigned char)VAR, (unsigned char)VAR

int main(int argc, char **argv) {
	char *fPath;
	uint8_t c;
	FILE *fp;
	fpos_t tmpPos;
	struct gifStructure gs;
	int rc;

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
	gifGetFileStructure(fp, &gs);

	if (gs.extCode == GIF_PIC_EXT_CODE) {
		printf("--- Simple Frame ---\n");
		printf("-- Header\n");
		printf("'-> Start byte: %#02x(%c)\n", ARF(gs.header.startByte));
		printf("'-> Size: %d\n", gs.header.subBlockSize);

		printf("-- Logical Screen Descriptor\n");
		printf("'-> Start byte: %#02x(%c)\n", ARF(gs.lsd.startByte));
		printf("'-> Size: %d\n", gs.lsd.subBlockSize);

		if (gs.hasGct) {
			printf("'-> Presence of a Global Color Table (GCT)\n");
			printf("   '-> Start byte: %#02x(%c)\n", ARF(gs.gct->startByte));
			printf("   '-> Size: %d\n", gs.gct->subBlockSize);
		} else {
			printf("'-> No Global Color Table (GCT)\n");
		}

		printf("-- GCE\n");
		printf("'-> Starting byte: %#02x(%c)\n", 							\
				ARF(gs.dataComposition.imgFrame.gce.startByte));
		printf("'-> Size: %d\n", gs.dataComposition.imgFrame.gce.subBlockSize);

		printf("-- Img Descriptor\n");
		printf("'-> Starting byte: %#02x(%c)\n", 			\
				ARF(gs.dataComposition.imgFrame.imgDescriptor.startByte));
		if (gs.dataComposition.imgFrame.hasLct) {
			printf("'-> Size: %d (%d + %d from LCT)\n", 								 \
					gs.dataComposition.imgFrame.imgDescriptor.subBlockSize + \
					gs.dataComposition.imgFrame.lct->subBlockSize, 			 \
					gs.dataComposition.imgFrame.imgDescriptor.subBlockSize,  \
					gs.dataComposition.imgFrame.lct->subBlockSize);
		} else {
			printf("'-> Size: %d (without LCT)\n", 							 \
					gs.dataComposition.imgFrame.imgDescriptor.subBlockSize);
		}

		printf("-- Img Datas\n");
		printf("'-> LZW Minimum Code\n");
		printf("   '-> Starting byte: %#02x(%c)\n", 						 \
				ARF(gs.dataComposition.imgFrame.imgDatas.lzwMinCode.startByte));
		printf("   '-> Datas splitted in: %d sub-blocks\n", 				 \
				gs.dataComposition.imgFrame.imgDatas.lzwMinCode.subBlockSize);
		printf("'-> Raw datas\n");
		for (int i = 0; 													   \
			 i < gs.dataComposition.imgFrame.imgDatas.lzwMinCode.subBlockSize; \
			 ++i) {
			printf("   '-> [%02d] Starting byte: %#02x(%c)\n", i,			   \
			ARF(gs.dataComposition.imgFrame.imgDatas.rawDatas[i].startByte));
			printf("   '-> [%02d] Size: %d\n", i,			   				   \
			gs.dataComposition.imgFrame.imgDatas.rawDatas[i].subBlockSize);
		}

		printf("-- Trailer\n");
		printf("'-> Start byte: %#02x(%c)\n", ARF(gs.trailer.startByte));
		printf("'-> Size: %d\n", gs.trailer.subBlockSize);
	} else if (gs.extCode == GIF_ANIM_EXT_CODE) {
		printf("--- Animated GIF ---\n");
		printf("GCE\n'-> Starting byte: %#02x(%c)\n", 		\
				gs.dataComposition.imgFrame.gce.startByte,	\
				gs.dataComposition.imgFrame.gce.startByte);
	}


	free(gs.gct);
	fclose(fp);
	printf("--- Stop reading file ---\n");

	return 0;
}

