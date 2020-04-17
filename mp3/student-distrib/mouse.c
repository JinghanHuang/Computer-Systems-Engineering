/* mouse.c - Functions to interact with the mouse
   Reference: https://wiki.osdev.org/Mouse_Input
 */

#include "mouse.h"
#include "i8259.h"
#include "lib.h"
#include "terminal.h"
#include "multi_terminal.h"


int32_t cycle;
int8_t pack[3];
int mouse_x =0;
int mouse_y = 0;

// bit 1 (value=2) of port 0x64 == 0 -> outb(0x60/0x64)
// bit 0 (value=1) of port 0x64 == 1 -> inb(0x60)
void mouse_wait(unsigned char value) {
    int timecounter = 1000;
    if (value == 2) {
        // waiting for bit 1 (value=2) of port 0x64 to become clear. 
        while (timecounter--) {
            if ((inb(0x64) & value) == 0)
                return;
        }
    }
    else {
        // bytes cannot be read from port 0x60 until bit 0 (value=1) 
        // of port 0x64 is set
        while (timecounter--) {
            if ((inb(0x64) & value) == 1)
                return;
        }
    }
}
// write to port 0x60
// Sending a command or data byte to the mouse (to port 0x60)
// must be preceded by sending a 0xD4 byte to port 0x64
// (with appropriate waits on port 0x64, bit 1, before sending each output byte).
// Note: this 0xD4 byte does not generate any ACK, 
//       from either the keyboard or mouse.
void mouse_write (unsigned char ch) {
    // appropriate waits
    mouse_wait(OUT_WAIT);
    // sending a 0xD4 byte to port 0x64
    outb(0xD4, MOUSE_ADDR_PORT);
    // appropriate waits
    mouse_wait(OUT_WAIT);
    // Sending a command or data byte to the mouse (to port 0x60)
    outb(ch, MOUSE_DATA_PORT);
}

/* void mouse_init(void)
 * Inputs:       void
 * Return Value: void
 * Function: Initialize the mouse */
void mouse_init(void) {
    unsigned char statusByte;
    enable_irq(MOUSE_IRQ);    // enable the Interrupt ReQuest for mouse

    // Aux Input Enable Command
    mouse_wait(OUT_WAIT);
    outb(0xA8, MOUSE_ADDR_PORT);

    // Get Compaq Status Byte
    mouse_wait(OUT_WAIT);
    outb(0x20, MOUSE_ADDR_PORT);

    // set bit number 1 (value=2, Enable IRQ12)
    mouse_wait(IN_WAIT);
    statusByte = (inb(MOUSE_DATA_PORT) | OUT_WAIT);
    // clear bit number 5 (value=0x20, Disable Mouse Clock)
    statusByte &= 0xDF;

    // send command byte 0x60 ("Set Compaq Status") to port 0x64
    mouse_wait(OUT_WAIT);
    outb(0x60, MOUSE_ADDR_PORT);

    // followed by the modified Status byte to port 0x60
    mouse_wait(OUT_WAIT);
    outb(statusByte, MOUSE_DATA_PORT);

    // Set Defaults
    // Disables streaming, sets the packet rate to 100 per second, and resolution to 4 pixels per mm.
    mouse_write(0xF6);
    // Enable Packet Streaming
    // The mouse starts sending automatic packets when the mouse moves or is clicked.
    mouse_write(0xF4);
    cycle = 1;
}

/* void mouse_handler()
 * Inputs:  uint32_t irq_num = the Interrupt ReQuest number which
 *                                the EOI signal will be sent to
 * Return Value: void
 * Function: Handle mouse interrupt) */
void mouse_interrupt_handler(void) {
    send_eoi(MOUSE_IRQ);
    if(((pack[0]>>6) == 0 ) && ((pack[0]&0x08) == 0))
        cycle = 0;
    pack[cycle] = inb(0x60);
    cycle++;

    if (cycle == 3) {
        cycle = 0;
        char x = pack[1];
        char y = pack[2];
        if ((pack[0] & 0x80) || (pack[0] & 0x40)){
          x = 0;
          y = 0;
        }

        old_attr(mouse_x, mouse_y);
        if(mouse_x + x/10 >= 0 && mouse_x + x/10 < NUM_COLS)
            mouse_x += x/10;
        if(mouse_y - y/10 >= 0 && mouse_y - y/10 < NUM_ROWS)
            mouse_y -= y/10;
        new_attr(mouse_x, mouse_y);
	}
}
