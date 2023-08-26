
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include "../modules/gifStructure/GIFGetStructure.h"
#include "../modules/gifStructure/GIFPrintStructure.h"

#define ARF(VAR) (unsigned char)VAR, (unsigned char)VAR

int main(int argc, char **argv) {
	char *fPath;
	FILE *fp;
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
	rc = gifGetFileStructure(fp, &gs);

	if (rc) {
		gifFreeStructure(&gs);
		fclose(fp);
		printf("Error occured, aborting program!\n");
		return rc;
	}

	if (gs.ec == GIF_PIC_EXT_CODE) {
		printf("--- Simple Frame ---\n");
		GIFPrintHeader(&gs);
		printf("\n");

		GIFPrintLsd(&gs);
		printf("\n");

		GIFPrintGct(&gs);
		printf("\n");

		GIFPrintData(&gs);
		printf("\n");

		printf("-- Img Datas\n");
		printf("'-> LZW Minimum Code\n");
		printf("   '-> Starting byte: %#02x(%c)\n", 						 \
				ARF(gs.dataComposition.img.dataSec.lzwMinCode.firstByte));
		printf("   '-> Datas splitted in: %d sub-blocks\n", 				 \
				gs.dataComposition.img.dataSec.lzwMinCode.subBlockSize);
		printf("'-> Raw datas\n");
		for (int i = 0; 													   \
			 i < gs.dataComposition.img.dataSec.lzwMinCode.subBlockSize; \
			 ++i) {
			printf("   '-> [%02d] Starting byte: %#02x(%c)\n", i,			   \
			ARF(gs.dataComposition.img.dataSec.rawDatas[i].firstByte));
			printf("   '-> [%02d] Size: %d\n", i,			   				   \
			gs.dataComposition.img.dataSec.rawDatas[i].subBlockSize);
		}

		printf("-- Trailer\n");
		printf("'-> Start byte: %#02x(%c)\n", ARF(gs.trailer.firstByte));
		printf("'-> Size: %d\n", gs.trailer.subBlockSize);
	} else if (gs.ec == GIF_ANIM_EXT_CODE) {
		printf("--- Animated GIF ---\n");
		printf("GCE\n'-> Starting byte: %#02x(%c)\n", 		\
				gs.dataComposition.img.gce.firstByte,	\
				gs.dataComposition.img.gce.firstByte);
	}

	gifFreeStructure(&gs);
	fclose(fp);
	printf("--- Stop reading file ---\n");

	return 0;
}

