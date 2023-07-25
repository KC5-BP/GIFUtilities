
#ifndef __GIF_STRUCTURE__
#define __GIF_STRUCTURE__

#include <stdio.h>

/* HEADER */
#define GIF_HEADER_SIZE 6
#define GIF_HEADER_START_BYTE 		'G'

/* LOGICAL SCREEN DESCRIPTOR (LSD) */
#define GIF_LSD_SIZE 7
#define GIF_LSD_WIDTH_OFFSET 		 0
#define GIF_LSD_HEIGHT_OFFSET 		 2
#define GIF_LSD_GCT_INFOS_OFFSET	 4
#define GIF_LSD_BACKGROUND_OFFSET	 5
#define GIF_LSD_TRANSPARENT_OFFSET	 6

/* GRAPHICAL CONTROL EXTENSION (GCCE) */
#define GIF_GCE_PIC_SIZE 8
#define GIF_GCE_ANIM_SIZE 19
#define GIF_GCE_START_BYTE 			'!'

/* IMAGE DESCRIPTOR */
#define GIF_IMG_DESCR_SIZE 10
#define GIF_IMG_DESCR_LCT_OFFSET 	 9
#define GIF_IMG_DESCR_START_BYTE 	','

/* IMAGE DATAS */
#define GIF_IMG_DATAS_SIZE 2
#define GIF_IMG_DATAS_SUBBLOCK_OFFSET	1

/* TRAILER */
#define GIF_EOF ';'

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
	struct sectionInfos *rawDatas; /* size stocked in lzwMinCode.subBlockSize */
};

struct frameSections {
	struct sectionInfos gce;
	struct sectionInfos imgDescriptor;
	char hasLct;
	struct sectionInfos *lct; /* Pointer because it is optionnal */
	struct imgDatasSection imgDatas;
};

struct animSections {
	int nFrames;
	struct sectionInfos animGce;
	struct frameSections *animFrames;
};

union dataSection {
	struct frameSections imgFrame;
	struct animSections animFrames;
};

struct gifStructure {
	struct sectionInfos header;
	struct sectionInfos lsd;
	char hasGct;
	struct sectionInfos *gct; /* Pointer because it is optionnal */
	gifExtCode extCode;
	union dataSection dataComposition;
	struct sectionInfos trailer;
};

int gifGetHeaderInfos(FILE *fp, struct gifStructure *gs);

void gifGetLSDInfos(FILE *fp, struct gifStructure *gs);

int gifGetGCTInfos(FILE *fp, struct gifStructure *gs);

int gifGetExtCode(FILE *fp, struct gifStructure *gs);

int gifGetDatasInfos(FILE *fp, struct gifStructure *gs);

#endif /* __GIF_STRUCTURE__ */
