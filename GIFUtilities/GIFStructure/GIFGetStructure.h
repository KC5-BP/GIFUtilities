
#ifndef __GIF_GET_STRUCTURE__
#define __GIF_GET_STRUCTURE__

#include <stdio.h>
#include <inttypes.h>

struct sectionInfos {
	fpos_t pos;
	uint32_t subBlockSize; /* in Bytes */
	uint8_t startByte;
};

struct imgDatasSection {
	struct sectionInfos lzwMinCode;
	struct sectionInfos *rawDatas; /* size stocked in lzwMinCode.subBlockSize */
};

struct frameSections {
	struct sectionInfos gce;
	struct sectionInfos imgDescriptor;
	uint8_t hasLct;
	struct sectionInfos *lct; /* Pointer because it is optionnal */
	struct imgDatasSection imgDatas;
};

struct animSections {
	uint32_t nFrames;
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
	uint8_t hasGct;
	struct sectionInfos *gct; /* Pointer because it is optionnal */
	gifExtCode extCode;
	union dataSection dataComposition;
	struct sectionInfos trailer;
};

void gifFreeStructure(struct gifStructure *gs);

int gifGetFileStructure(FILE *fp, struct gifStructure *gs);

#endif /* __GIF_GET_STRUCTURE__ */

