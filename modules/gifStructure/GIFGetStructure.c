
#include "GIFGetStructure.h"

#include "../gifcsts.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>

#include <errno.h>
#include <string.h>

/* EN_DBG in ../GIFDefines.h */
#ifdef EN_DBG
#define DBG(FMT, ...)	\
do {							\
	printf(FMT, ##__VA_ARGS__);	\
} while(0)
#else
#define DBG(FMT, ...)
#endif

#define SAVE_CURRENT_FILE_POS(FP)	\
	fpos_t curPos;					\
	fgetpos(FP, &curPos)

#define RESTORE_CURRENT_FILE_POS(FP)	\
	/* Return to current position */	\
	fsetpos(FP, &curPos)

void gifFreeStructure(struct gifStructure *gs) {
	free(gs->gct);
	if (gs->ec == GIF_PIC_EXT_CODE) {
		free(gs->dataComposition.img.lct);
		free(gs->dataComposition.img.dataSec.rawDatas);
	} else if (gs->ec == GIF_ANIM_EXT_CODE) {
		for (int i = 0; i < gs->dataComposition.anim.nFrames; i++) {
			free(gs->dataComposition.anim.frames[i].lct);
			free(gs->dataComposition.anim.frames[i].dataSec.rawDatas);
		}
		free(gs->dataComposition.anim.frames);
	}
}

int gifGetHeaderInfos(FILE *fp, struct gifStructure *gs) {
	int rc = 0;
 
	SAVE_CURRENT_FILE_POS(fp);

	/* Restart file */
	rc = fseek(fp, 0, SEEK_SET);
	if (rc)
		DBG("%s: fseek failed (%s)\n", __func__, strerror(errno));
	
	/* Get infos */
	fgetpos(fp, &gs->header.filePos);
	gs->header.subBlockSize = GIF_HEADER_SIZE;
	gs->header.firstByte = fgetc(fp);

	RESTORE_CURRENT_FILE_POS(fp);

	rc = (gs->header.firstByte == GIF_HEADER_START_BYTE) ? rc : -1;

	return rc;
}

int gifGetLSDInfos(FILE *fp, struct gifStructure *gs) {
	int rc = 0;

	SAVE_CURRENT_FILE_POS(fp);

	rc = fseek(fp, gs->header.subBlockSize, SEEK_SET);
	if (rc)
		DBG("%s: fseek failed (%s)\n", __func__, strerror(errno));
	
	fgetpos(fp, &gs->lsd.filePos);
	gs->lsd.subBlockSize = GIF_LSD_SIZE;
	gs->lsd.firstByte = fgetc(fp); /* Expected 1st Width byte */

	RESTORE_CURRENT_FILE_POS(fp);

	return rc;
}

int ctAllocation(FILE *fp, uint8_t byte, struct sectionInfos **ct, uint8_t offset) {
	int rc = 0;
	int hasCt = -1;

	hasCt = (byte & CT_BITFIELD_CT_PRESENCE) >> CT_PRESENCE_BIT;
	*ct = (struct sectionInfos *) calloc(1, sizeof(struct sectionInfos));

	if ( ! ct ) {
		rc = -1;	/* Error during allocation */
	} else {
		/* *3 <=> RGB */
		(*ct)->subBlockSize = pow(2, (byte & CT_BITFIELD_PAL_BITS) + 1) * \
									CT_ENTRY_SIZE;

		if (hasCt && offset) {
			/* Skip bytes */
			rc = fseek(fp, offset, SEEK_CUR);
			if (rc)
				DBG("%s: fseek failed (%s)\n", __func__, strerror(errno));
			
			fgetpos(fp, &(*ct)->filePos);
			(*ct)->firstByte = fgetc(fp); /* Expected RED byte */
		}
	}

	return rc;
}

int gifGetGCTInfos(FILE *fp, struct gifStructure *gs) {
	int rc = 0;
	uint8_t byte;

	SAVE_CURRENT_FILE_POS(fp);

	/* Reach GCT Infos byte */
	rc = fseek(fp, gs->header.subBlockSize + GIF_LSD_OFFSET_GCT_INFOS, SEEK_SET);
	if (rc)
		DBG("%s: fseek failed (%s)\n", __func__, strerror(errno));

	byte = fgetc(fp);

	/* Check GCT presence */
	if (byte)
		/* Offset 2: Skip Background + Transparency bytes */
		rc = ctAllocation(fp, byte, &gs->gct, 2);

	RESTORE_CURRENT_FILE_POS(fp);

	return rc;
}

int gifGetExtCode(FILE *fp, struct gifStructure *gs) {
	int rc = 0;
	uint8_t byte;
	fpos_t tmpPos;

	SAVE_CURRENT_FILE_POS(fp);

	/* Think from LSD position 
	 * -> Get to it
     * -> Skip it  				*/
	fsetpos(fp, &gs->lsd.filePos);
	rc = fseek(fp, GIF_LSD_SIZE, SEEK_CUR);
	if (rc)
		DBG("%s: fseek failed (%s)\n", __func__, strerror(errno));

	if (gs->gct) {
		/* If possible, skip GCT */
		fseek(fp, gs->gct->subBlockSize, SEEK_CUR);
		if (rc)
			DBG("%s: fseek on GCT skip failed (%s)\n", __func__, strerror(errno));
	}
	
	fgetpos(fp, &tmpPos);
	byte = fgetc(fp);
	if (byte != GIF_GCE_START_BYTE) {
		rc = -1;
	} else {
		gs->ec = fgetc(fp);
		if ((gs->ec != GIF_PIC_EXT_CODE) && \
			(gs->ec != GIF_ANIM_EXT_CODE))
			rc = -1;
	}

	RESTORE_CURRENT_FILE_POS(fp);

	return rc;
}

int gifGetGceInfos(FILE *fp, gifExtCode extCode, struct sectionInfos *gceSection) {
	fgetpos(fp, &gceSection->filePos);

	if (extCode == GIF_PIC_EXT_CODE)
		gceSection->subBlockSize = GIF_GCE_PIC_SIZE;
	else if (extCode == GIF_ANIM_EXT_CODE)
		gceSection->subBlockSize = GIF_GCE_ANIM_SIZE;

	gceSection->firstByte = fgetc(fp);

	return (gceSection->firstByte == GIF_GCE_START_BYTE) ? (0) : (-1);
}

int gifCountFrameSubBlocks(FILE *fp, fpos_t *imgDataPos, uint32_t *lastSize) {
	int rc = 0;
	uint8_t byte;
	uint32_t nSubBlocks = 0;

	/* Get to Img Data beginning */
	fsetpos(fp, imgDataPos);

	/* Skip LZW Minimum Code */
	rc = fseek(fp, 1, SEEK_CUR);
	if (rc)
		DBG("%s: fseek on LWZ failed (%s)\n", __func__, strerror(errno));

	byte = fgetc(fp);
	while (byte) {
		++nSubBlocks;
		rc = fseek(fp, byte, SEEK_CUR);
		if (rc)
			printf("%s: fseek failed(%s)\n", __func__, strerror(errno));
		*lastSize = byte;
		byte = fgetc(fp);
	}
	return ( ! rc ) ? (nSubBlocks) : (-1);
}

int gifCountAnimFrames(FILE *fp, ...) {
	return 0;
}

int gifGetFrameInfos(FILE *fp, struct gifStructure *gs, struct frameSections *fs) {
	int rc = 0;
	uint8_t byte;
	fpos_t tmpPos;

	rc = gifGetGceInfos(fp, gs->ec, &fs->gce);

	/* Skip GCE */
	rc = fseek(fp, fs->gce.subBlockSize - 1, SEEK_CUR);
	if (rc)
		DBG("%s: fseek on GCE failed (%s)\n", __func__, strerror(errno));
	
	fgetpos(fp, &tmpPos); /* Supposed to start on ',' byte */
	/* Check image descriptor starting byte */
	byte = fgetc(fp);
	if (byte != GIF_IMG_DESCR_START_BYTE) {
		rc = -1;
	} else {
		/* Fill infos */
		fs->descriptor.firstByte = byte;
		fs->descriptor.filePos = tmpPos;
		fs->descriptor.subBlockSize = GIF_IMG_DESCR_SIZE;

		/* Reach LCT byte infos */
		rc = fseek(fp, GIF_IMG_DESCR_OFFSET_LCT - 1, SEEK_CUR);
		if (rc)
			DBG("%s: fseek on GCE failed (%s)\n", __func__, strerror(errno));

		/* Check LCT presence */
		byte = fgetc(fp);
		if (byte)
			rc = ctAllocation(fp, byte, &fs->lct, 0);

		if (rc) return rc;

		if (fs->lct) /* If possible, skip LCT */
			fseek(fp, fs->lct->subBlockSize - 1, SEEK_CUR);

		fgetpos(fp, &tmpPos);
		fs->dataSec.lzwMinCode.filePos = tmpPos;
		fs->dataSec.lzwMinCode.firstByte = fgetc(fp);
		
		uint32_t lastBlockSize;
		uint32_t subBlocks = gifCountFrameSubBlocks(fp, &tmpPos, &lastBlockSize);
		fs->dataSec.lzwMinCode.subBlockSize = subBlocks;

		/* Allocate each sectionInfos */
		fs->dataSec.rawDatas = (struct sectionInfos *) calloc(subBlocks, sizeof(struct sectionInfos));
		
		if ( ! fs->dataSec.rawDatas ) {
			rc = -1;
		} else {
			fsetpos(fp, &tmpPos);
			fseek(fp, 1, SEEK_CUR);

			fgetpos(fp, &tmpPos);
			byte = fgetc(fp);
			for (int i = 0; i < subBlocks; ++i) {
				fs->dataSec.rawDatas[i].filePos = tmpPos;
				fs->dataSec.rawDatas[i].firstByte = byte;
				fs->dataSec.rawDatas[i].subBlockSize = ((int)byte) + 1;

				printf("%s: %#02x(%c)\n", __func__, (unsigned char)byte, byte);
				fseek(fp, byte, SEEK_CUR);

				fgetpos(fp, &tmpPos);
				byte = fgetc(fp);
			}
		}

		gs->trailer.subBlockSize = 1;
		fgetpos(fp, &gs->trailer.filePos);
		gs->trailer.firstByte = fgetc(fp);
	}
	return rc;
}

int gifGetAnimInfos(FILE *fp, struct gifStructure *gs, struct animSections *as) {
	gifGetGceInfos(fp, gs->ec, &as->gce);

	return 0;
}

int gifGetDatasInfos(FILE *fp, struct gifStructure *gs) {
	int rc = 0;

	SAVE_CURRENT_FILE_POS(fp);

	if (gs->ec == GIF_PIC_EXT_CODE) {
		gifGetFrameInfos(fp, gs, &gs->dataComposition.img);
	} else if (gs->ec == GIF_ANIM_EXT_CODE) {
		gifGetAnimInfos(fp, gs, &gs->dataComposition.anim);
	} else {
		rc = -1;
	}

	RESTORE_CURRENT_FILE_POS(fp);

	return rc;
}

int gifGetFileStructure(FILE *fp, struct gifStructure *gs) {
	uint32_t rc = 0;
	fpos_t inbetweenPos;

	/* *** Header *** */
	gifGetHeaderInfos(fp, gs);

	/* *** Logical Screen Descriptor (LSD) *** */
	gifGetLSDInfos(fp, gs);

	/* *** Global Color Table (GCT) *** */
	rc = gifGetGCTInfos(fp, gs);
	if (rc)	return rc;

	if (gs->gct) {
		/* Go to GCT beginning */
		fsetpos(fp, &gs->gct->filePos);

		/* Skip table */
		fseek(fp, gs->gct->subBlockSize, SEEK_CUR);
	} else {
		/* Go to LSD beginning */
		fsetpos(fp, &gs->lsd.filePos);

		/* Skip LSD */
		fseek(fp, GIF_LSD_SIZE, SEEK_CUR);
	}

	/* *** Global Control Extension (GCE) *** */
	fgetpos(fp, &inbetweenPos); /* Save temporarily position */

	/* Firstly, read extension code (is it an IMG or an ANIMATION?) */
	rc = gifGetExtCode(fp, gs);
	if (rc)	return rc;

	/* Then, get file composition infos */
	fsetpos(fp, &inbetweenPos);
	gifGetDatasInfos(fp, gs);

	return rc;
}

/*printf("%s: starting byte: \n", __func__);
for (int i = 0; i < 4; ++i) {
	byte = fgetc(fp);
	printf("%#02x(%c) ", (unsigned char)byte, byte);
}*/
