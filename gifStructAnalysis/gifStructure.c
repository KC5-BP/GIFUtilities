
#include "gifStructure.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#define SAVE_CURRENT_FILE_POS(FP)	\
	fpos_t curPos;					\
	fgetpos(FP, &curPos)

#define RESTORE_CURRENT_FILE_POS(FP)	\
	/* Return to current position */	\
	fsetpos(FP, &curPos)

int gifGetHeaderInfos(FILE *fp, struct gifStructure *gs) {
	SAVE_CURRENT_FILE_POS(fp);

	/* Restart file */
	fseek(fp, 0, SEEK_SET);
	
	/* Get infos */
	fgetpos(fp, &gs->header.pos);
	gs->header.subBlockSize = GIF_HEADER_SIZE;
	gs->header.startByte = fgetc(fp);

	RESTORE_CURRENT_FILE_POS(fp);

	return (gs->header.startByte == GIF_HEADER_START_BYTE) ? 0 : -1;
}

void gifGetLSDInfos(FILE *fp, struct gifStructure *gs) {
	SAVE_CURRENT_FILE_POS(fp);

	fseek(fp, gs->header.subBlockSize, SEEK_SET);
	fgetpos(fp, &gs->lsd.pos);
	gs->lsd.subBlockSize = GIF_LSD_SIZE;
	gs->lsd.startByte = fgetc(fp); /* Expected 1st Width byte */

	RESTORE_CURRENT_FILE_POS(fp);
}

int ctAllocation(FILE *fp, char byte, char *hasCt, struct sectionInfos **ct, char offset) {
	int rc = 0;

	*hasCt = (byte & 0x80) >> 7;
	*ct = (struct sectionInfos *) calloc(1, sizeof(struct sectionInfos));

	if ( ! ct ) {
		rc = -1;	/* Error during allocation */
	} else {
		/* *3 <=> RGB */
		(*ct)->subBlockSize = pow(2, (byte & 0x07) + 1) * 3;

		if (*hasCt) {
			/* Skip 2 bytes (background + transparent) */
			fseek(fp, offset, SEEK_CUR);
			fgetpos(fp, &(*ct)->pos);
			(*ct)->startByte = fgetc(fp); /* Expected 1st color RED byte (with order RGB) */
		}
	}

	return rc;
}

int gifGetGCTInfos(FILE *fp, struct gifStructure *gs) {
	int rc;
	char byte;

	SAVE_CURRENT_FILE_POS(fp);

	/* Reach GCT Infos byte */
	fseek(fp, gs->header.subBlockSize + GIF_LSD_GCT_INFOS_OFFSET, SEEK_SET);

	byte = fgetc(fp);

	/* Check GCT presence */
	if (byte)
		rc = ctAllocation(fp, byte, &gs->hasGct, &gs->gct, 2);

	RESTORE_CURRENT_FILE_POS(fp);

	return rc;
}

int gifGetExtCode(FILE *fp, struct gifStructure *gs) {
	volatile int rc = 0;
	char c;
	fpos_t tmpPos;

	SAVE_CURRENT_FILE_POS(fp);

	/* Think from LSD position 
	 * -> Get to it
     * -> Skip it  				*/
	fsetpos(fp, &gs->lsd.pos);
	fseek(fp, GIF_LSD_SIZE, SEEK_CUR);

	if (gs->hasGct && gs->gct) /* If possible, skip GCT */
		fseek(fp, gs->gct->subBlockSize, SEEK_CUR);
	
	fgetpos(fp, &tmpPos);
	c = fgetc(fp);
	if (c != GIF_GCE_START_BYTE) {
		rc = -1;
	} else {
		gs->extCode = (unsigned char) fgetc(fp);
	}

	RESTORE_CURRENT_FILE_POS(fp);

	return rc;
}

void gifGetGceInfos(FILE *fp, gifExtCode extCode, struct sectionInfos *gceSection) {
	fgetpos(fp, &gceSection->pos);

	if (extCode == GIF_PIC_EXT_CODE)
		gceSection->subBlockSize = GIF_GCE_PIC_SIZE;
	else if (extCode == GIF_ANIM_EXT_CODE)
		gceSection->subBlockSize = GIF_GCE_ANIM_SIZE;

	gceSection->startByte = fgetc(fp);
}

int gifCountImgDatasSubBlocks(FILE *fp, fpos_t *imgDataPos, int *lastSize) {
	char c;
	int nSubBlocks = 0;

	/* Get to Img Data beginning */
	fsetpos(fp, imgDataPos);

	/* Skip LZW Minimum Code */
	fseek(fp, 1, SEEK_CUR);

	c = fgetc(fp);
	printf("%s: %#02x(%c)\n", __func__, (unsigned char)c, c);
	while (c) {
		++nSubBlocks;
		printf("fseek of: %#02x(%c) ", (unsigned char)c, c);
		int rc = fseek(fp, (long int)c, SEEK_CUR);
		if ( rc ) {
			printf("fssek failed!! (%s)\n", strerror(errno));
		}
		*lastSize = c;
		c = fgetc(fp);
		printf("fseek of: %#02x(%c) ", (unsigned char)c, c);
	}
	return nSubBlocks;
}

int gifCountAnimFrames(FILE *fp, ...) {
	
}

int gifGetFrameInfos(FILE *fp, struct gifStructure *gs, struct frameSections *fs) {
	volatile int rc = 0;
	char byte;
	fpos_t tmpPos;

	gifGetGceInfos(fp, gs->extCode, &fs->gce);

	/* Skip GCE */
	fseek(fp, fs->gce.subBlockSize - 1, SEEK_CUR);
	
	fgetpos(fp, &tmpPos); /* Supposed to start on ',' byte */
	/* Check image descriptor starting byte */
	byte = fgetc(fp);
	if (byte != GIF_IMG_DESCR_START_BYTE) {
		rc = -1;
	} else {
		/* Fill infos */
		fs->imgDescriptor.startByte = byte;
		fs->imgDescriptor.pos = tmpPos;
		fs->imgDescriptor.subBlockSize = GIF_IMG_DESCR_SIZE;

		/* Reach LCT byte infos */
		fseek(fp, GIF_IMG_DESCR_LCT_OFFSET - 1, SEEK_CUR);
		byte = fgetc(fp);

		/* Check LCT presence */
		if (byte)
			rc = ctAllocation(fp, byte, &fs->hasLct, &fs->lct, 0);

		if (rc) return rc;

		if (fs->hasLct && fs->lct) /* If possible, skip LCT */
			fseek(fp, fs->lct->subBlockSize - 1, SEEK_CUR);

		fgetpos(fp, &tmpPos);
		fs->imgDatas.lzwMinCode.pos = tmpPos;
		fs->imgDatas.lzwMinCode.startByte = fgetc(fp);
		
		int lastBlockSize;
		int subBlocks = gifCountImgDatasSubBlocks(fp, &tmpPos, &lastBlockSize);
		//fs->imgDatas.subBlockSize = (subBlocks-1) * 255 + lastBlockSize;
		fs->imgDatas.lzwMinCode.subBlockSize = subBlocks;

		/* Allocate each sectionInfos */
		fs->imgDatas.rawDatas = (struct sectionInfos *) calloc(subBlocks, sizeof(struct sectionInfos));
		
		if ( ! fs->imgDatas.rawDatas ) {
			rc = -1;
		} else {
			fsetpos(fp, &tmpPos);
			fseek(fp, 1, SEEK_CUR);

			fgetpos(fp, &tmpPos);
			byte = fgetc(fp);
			printf("%s: %#02x(%c)\n", __func__, (unsigned char)byte, byte);
			for (int i = 0; i < subBlocks; ++i) {
				fs->imgDatas.rawDatas[i].pos = tmpPos;
				fs->imgDatas.rawDatas[i].startByte = byte;
				fs->imgDatas.rawDatas[i].subBlockSize = ((int)byte) + 1;

				fseek(fp, byte - 1, SEEK_CUR);
				//fseek(fp, byte, SEEK_CUR);

				fgetpos(fp, &tmpPos);
				byte = fgetc(fp);
			}
		}
		
		fseek(fp, 1, SEEK_CUR); /* Skip end of sub-blocks (0x00) */

		gs->trailer.subBlockSize = 1;
		fgetpos(fp, &gs->trailer.pos);
		gs->trailer.startByte = fgetc(fp);

		for (int i = 0; i < 4; ++i) {
			byte = fgetc(fp);
			printf("%#02x(%c) ", (unsigned char)byte, byte);
		}
	}
	return rc;
}

int gifGetAnimInfos(FILE *fp, struct gifStructure *gs, struct animSections *as) {
	gifGetGceInfos(fp, gs->extCode, &as->animGce);
}

int gifGetDatasInfos(FILE *fp, struct gifStructure *gs) {
	int rc = 0;

	SAVE_CURRENT_FILE_POS(fp);

	if (gs->extCode == GIF_PIC_EXT_CODE) {
		gifGetFrameInfos(fp, gs, &gs->dataComposition.imgFrame);
	} else if (gs->extCode == GIF_ANIM_EXT_CODE) {
		gifGetAnimInfos(fp, gs, &gs->dataComposition.animFrames);
	} else {
		rc = -1;
	}

	RESTORE_CURRENT_FILE_POS(fp);

	return rc;
}

/*printf("%s: starting byte: \n", __func__);
for (int i = 0; i < 4; ++i) {
	byte = fgetc(fp);
	printf("%#02x(%c) ", (unsigned char)byte, byte);
}*/