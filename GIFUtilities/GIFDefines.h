
#ifndef __GIF_DEFINES_H__
#define __GIF_DEFINES_H__

#define EN_DBG 1

/**********************************************
 * GIF Extension code / tag
 * to identify the file type
 *********************************************/
typedef enum {
	GIF_PIC_EXT_CODE=0xF9, GIF_ANIM_EXT_CODE=0xFF
} gifExtCode;

#define N_DIM_BYTES			 2

/* *** HEADER *** */
#define GIF_HEADER_SIZE		 6
/* ************** */

/* *** LOGICAL SCREEN DESCRIPTOR (LSD) *** */
#define GIF_LSD_SIZE		 7
#define GIF_LSD_OFFSET_WIDTH 		 	 0
#define GIF_LSD_OFFSET_HEIGHT 		 	 2
#define GIF_LSD_OFFSET_GCT_INFOS	 	 4
#define GIF_LSD_OFFSET_BACKGROUND	 	 5
#define GIF_LSD_OFFSET_TRANSPARENT	 	 6
/* *************************************** */

/* *** GLOBAL COLOR TABLE (GCT) *** */
#define CT_ENTRY_SIZE					 3

#define CT_TRANSPARENCY_BIT	 0
#define CT_BITFIELD_TRANSPARENCY	(1 << CT_TRANSPARENCY_BIT)

#define CT_BITFIELD_PAL_BITS       0x07

#define CT_INTERLACED_BIT	 6
#define CT_BITFIELD_INTERLACED		(1 << CT_INTERLACED_BIT)

#define CT_PRESENCE_BIT		 7
#define CT_BITFIELD_CT_PRESENCE		(1 << CT_PRESENCE_BIT)
/* ******************************** */

/* ************* GIF COMPOSITION ************* */
/* **  GLOBAL CONTROL EXTENSION (ANIMATION) ** */
#define GIF_GCE_ANIM_SIZE	19
#define GIF_GCE_ANIM_APPLI_NAME_SIZE		11

#define GIF_GCE_ANIM_OFFSET_START_SYMBOLE	 0
#define GIF_GCE_ANIM_OFFSET_EXT_CODE 		 1
#define GIF_GCE_ANIM_OFFSET_APPLI_NAME_SIZE	 2
#define GIF_GCE_ANIM_OFFSET_APPLI_NAME 		 3
#define GIF_GCE_ANIM_OFFSET_SUB_BLOCK_SIZE	14
#define GIF_GCE_ANIM_OFFSET_CURRENT_INDEX	15
#define GIF_GCE_ANIM_OFFSET_REPETITIONS		16
#define GIF_GCE_ANIM_OFFSET_END_SYMBOLE		18

/* **    GLOBAL CONTROL EXTENSION (FRAME)   ** */
#define GIF_GCE_PIC_SIZE	 8

#define GIF_GCE_PIC_OFFSET_START_SYMBOLE	 0
#define GIF_GCE_PIC_OFFSET_EXT_CODE 		 1
#define GIF_GCE_PIC_OFFSET_BLOCK_SIZE		 2
#define GIF_GCE_PIC_OFFSET_TRANSPARENT		 3
#define GIF_GCE_PIC_OFFSET_FRAME_DELAY		 4
#define GIF_GCE_PIC_OFFSET_TRANSP_NBR		 6
#define GIF_GCE_PIC_OFFSET_END_SYMBOLE		 7

/* **            IMAGE DESCRIPTION          ** */
#define GIF_IMG_DESCR_SIZE	10

#define GIF_IMG_DESCR_OFFSET_START_SYMBOLE	 0
#define GIF_IMG_DESCR_OFFSET_NORTH_POS		 1
#define GIF_IMG_DESCR_OFFSET_WEST_POS		 3
#define GIF_IMG_DESCR_OFFSET_WIDTH			 5
#define GIF_IMG_DESCR_OFFSET_HEIGHT			 7
#define GIF_IMG_DESCR_OFFSET_LCT			 9
/* ******************************************* */

#endif /* __GIF_DEFINES_H__ */

