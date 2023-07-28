
#ifndef __GIF_PRINT_STRUCTURE_H__
#define __GIF_PRINT_STRUCTURE_H__

#include "GIFGetStructure.h"

void GIFPrintHeader(struct gifStructure *gs);

void GIFPrintLsd(struct gifStructure *gs);

void GIFPrintGct(struct gifStructure *gs);

void GIFPrintData(struct gifStructure *gs);

void GIFPrintTrailer(struct gifStructure *gs);

/*************************************************
 * This function allows to print choosen sections
 * by the use of a: 
 * 	- Ptr pointing on a functions array
 ************************************************/
void GIFPrintChoosenSections(struct gifStructure *gs, 						\
							void (* fnSections[]) (struct gifStructure *),	\
							int nFns);

void GIFPrintStructure(struct gifStructure *gs);

#endif /* __GIF_PRINT_STRUCTURE_H__ */

