
#ifndef __GIF_GET_STRUCTURE_H__
#define __GIF_GET_STRUCTURE_H__

#include <stdio.h>
#include <inttypes.h>

/* HEADER */
#define GIF_HEADER_START_BYTE 			'G'

/* LOGICAL SCREEN DESCRIPTOR (LSD) */

/* GRAPHICAL CONTROL EXTENSION (GCE) */
#define GIF_GCE_START_BYTE 				'!'

/* IMAGE DESCRIPTOR */
#define GIF_IMG_DESCR_START_BYTE 		','

/* IMAGE DATAS */

/* TRAILER */
#define GIF_EOF ';'

typedef enum {
	GIF_PIC_EXT_CODE=0xF9, GIF_ANIM_EXT_CODE=0xFF
} gifExtCode;

struct sectionInfos {
	fpos_t pos;
	uint8_t startByte;
	uint32_t subBlockSize;	/* in Bytes */
};

struct imgDatasSection {
	struct sectionInfos lzwMinCode;	/* .subBlockSize gives 
									 * the full section size */
	uint32_t nSubBlocks;
	struct sectionInfos *rawDatas;	/* Array based on nSubBlocks */
};

struct frameSections {
	struct sectionInfos gce;
	struct sectionInfos imgDescriptor;
	uint8_t hasLct;
	struct sectionInfos *lct;		/* Pointer for memory size
									 * when not present 	   */
	struct imgDatasSection dataSec;
};

struct animSections {
	uint32_t nFrames;
	struct sectionInfos gce;
	struct frameSections *frames;
};

union dataSection {
	struct frameSections img;
	struct animSections anim;
};

struct gifStructure {
	struct sectionInfos header;
	struct sectionInfos lsd;
	uint8_t hasGct;
	struct sectionInfos *gct;	/* Pointer for memory size
								 * when not present 	   */
	gifExtCode extCode;
	union dataSection dataComposition;
	struct sectionInfos trailer;
};

void gifFreeStructure(struct gifStructure *gs);

int gifGetFileStructure(FILE *fp, struct gifStructure *gs);

#endif /* __GIF_GET_STRUCTURE_H__ */

