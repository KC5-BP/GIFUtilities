
#include "GIFPrintStructure.h"

#include "GIFGetStructure.h"

#define DOUBLE_VAR(VAR) VAR, VAR

void GIFPrintHeader(struct gifStructure *gs) {
	printf("*** HEADER ***\n");
	printf("-> First byte: %#02x(%c)\n", DOUBLE_VAR(gs->header.firstByte));
	printf("-> Size: %d", gs->header.subBlockSize);
}

void GIFPrintLsd(struct gifStructure *gs) {
	printf("*** Logical Screen Descriptor ***\n");
	printf("-> First byte: %#02x(%c)\n", DOUBLE_VAR(gs->lsd.firstByte));
	printf("-> Size: %d", gs->lsd.subBlockSize);
}

void GIFPrintGct(struct gifStructure *gs) {
	printf("*** Global Color Table ***\n");
	if (gs->gct) {
		printf("-> Presence of a Global Color Table (GCT)\n");
		printf("   -> Start byte: %#02x(%c)\n", DOUBLE_VAR(gs->gct->firstByte));
		printf("   -> Size: %d", gs->gct->subBlockSize);
	} else {
		printf("-> No Global Color Table (GCT)");
	}
}

void GIFPrintFrameInfos(struct frameSections *fs) {
	printf("** Global Control Extension **\n");
	printf("-> Starting byte: %#02x(%c)\n", DOUBLE_VAR(fs->gce.firstByte));
	printf("-> Size: %d\n", fs->gce.subBlockSize);
	
	printf("** Image Descriptor **\n");
	printf("-> Starting byte: %#02x(%c)\n", 		\
			DOUBLE_VAR(fs->descriptor.firstByte));
	if (fs->lct) {
		printf("-> Size: %d (%d + %d from LCT)", \
				fs->descriptor.subBlockSize + 	\
				fs->lct->subBlockSize, 				\
				fs->descriptor.subBlockSize,  	\
				fs->lct->subBlockSize);
	} else {
		printf("-> Size: %d (without LCT)", fs->descriptor.subBlockSize);
	}
}

void GIFPrintLct(struct gifStructure *gs) {
	
}

void GIFPrintData(struct gifStructure *gs) {
	if (gs->ec == GIF_PIC_EXT_CODE) {
		printf("*** SIMPLE IMAGE (Single frame) ***\n");
		GIFPrintFrameInfos(&gs->dataComposition.img);
	} else if (gs->ec == GIF_ANIM_EXT_CODE) {
		printf("*** ANIMATED GIF ***\n");
	}
}

void GIFPrintTrailer(struct gifStructure *gs) {
}

/*************************************************
 * This function allows to print choosen sections
 * by the use of a: 
 * 	- Ptr pointing on a functions array
 ************************************************/
void GIFPrintChoosenSections(struct gifStructure *gs, 						\
							void (* fnSections[]) (struct gifStructure *),	\
							int nFns) {
	for (int i = 0; i < nFns; i++) {
		fnSections[i](gs);
		printf("\n");
	}
}

void GIFPrintStructure(struct gifStructure *gs) {
	
}

