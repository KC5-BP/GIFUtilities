/*..--------------------------------------------------------------------------.
../ .------------------------------------------------------------------------. \
./´/
|x| __--""'\
|x|  ,__  -'""`;
|x| /   \  /"'  \
|x|   __// \-"-_/
|x| ´"   \  |           > Module: gifInfoDisplayer
|x| \     |  \  _.-"',
|x| "^,-´\/\  '" ,--. \
|x|  \|\| | | , /    | |
|x|     '`'\|._ |   / /
|x|         '\),/  / |          > Creation: July 2023
|x|           |/.-"_/           > By: KC5-BP
|x| .__---+-_/'|--"
|x|         _| |_--,            > Description :
|x|        ',/ |   /                > Functions to fully extract informations
|x|        /|| |  /                 > of GIF Picture or Animation.
|x|     |\| |/ |- |
|x| .-,-/ | /  '/-"
|x| -\|/-/\/ ^,'|
|x| _-     |  |/
|x|  .  --"  /
|x| /--__,.-/
.\`\_________________________________________________________________________/´/
..`___________________________________________________________________________´
===============================================================================>
============================================================================= */

#ifndef __GIFINFODISPLAYER_H__
#define __GIFINFODISPLAYER_H__

#include <stdio.h>
#include "GIFReadContent.h"

#define LIGHT_CYAN "\033[1;36m"
#define ORANGE     "\033[0;33m"
#define GREEN      "\033[0;32m"
#define RED        "\033[0;31m"
#define NC         "\033[0m"

#define FMT_BYTE   "%02x(%c) "

#define GIF_PIC_EXT_STRING      "Graphic Control Extension"
#define GIF_ANIM_EXT_STRING     "Application Extension"

/**********************************************
 *********************************************/
void gifPrintSignature(char *fmt, struct gifContent *gf);

/**********************************************
 *********************************************/
void gifPrintLogicalScreenDescriptor(struct gifContent *gf, int printGct);

/**********************************************
 *********************************************/
void gifPrintGce(struct gifContent *gf);

/**********************************************
 *********************************************/
void gifPrintGcePicture(struct gifContent *gf);

/**********************************************
 *********************************************/
void gifPrintGceAnimation(struct gifContent *gf);

/**********************************************
 *********************************************/
void gifPrintImgDescr(struct gifContent *gf);

#endif /* __GIFINFODISPLAYER_H__ */

