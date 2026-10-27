/* CardText.h - Calc-U-59 text cards ("Calc-U-59 Card 1.0") into a 246-byte bank; see CardText.cp. */

#ifndef CARDTEXT_H
#define CARDTEXT_H

#include "TI59Core.h"

/* the buffer starts with the text card magic */
short CardTextIs(const char* buf, long n);

/* parse into bank[246]; 1 on success, 0 if the text is malformed */
short CardTextParse(const char* buf, long n, u8* bank);

#endif
