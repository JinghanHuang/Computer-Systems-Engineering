/* mouse.h - Defines used in interactions with the mouse
 */

#ifndef _MOUSE_H
#define _MOUSE_H

#include "types.h"
#include "lib.h"

#define MOUSE_IRQ			12
#define MOUSE_ADDR_PORT		0x64
#define MOUSE_DATA_PORT		0x60
#define OUT_WAIT			2
#define IN_WAIT				1

/* Initialize the mouse input */
void mouse_init(void);
/* Handle mouse interrupt */
void mouse_interrupt_handler(void);
// Waiting to Send Bytes to Port 0x60 and 0x64
void mouse_wait(unsigned char value);
// write to port 0x60
void mouse_write (unsigned char ch);

#endif /* _MOUSE_H */
