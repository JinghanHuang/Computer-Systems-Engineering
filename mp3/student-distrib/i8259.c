/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/*
 * This contains the irq mask for both 8259A irq controllers, copied from kernel/i8259.c
 */
static unsigned int cached_irq_mask = 0xffff;

#define __byte(x,y) 	(((unsigned char *)&(y))[x])
#define cached_21	(__byte(0,cached_irq_mask))
#define cached_A1	(__byte(1,cached_irq_mask))

/* void i8259_init(void)
 * Inputs:  	 void
 * Return Value: void
 * Function: Initialize the 8259 PIC */
void i8259_init(void) {	
	outb(MASK_ALL, MASTER_8259_DATA_PORT);		/* mask all of 8259A-1 */
	outb(MASK_ALL, SLAVE_8259_DATA_PORT);		/* mask all of 8259A-2 */
	// Refer to 8259A_PIC_Datasheet.pdf/INITIALIZATION COMMAND WORDS & https://wiki.osdev.org/8259_PIC & kernel/i8259.c
	// ICW1 starts the initalization sequence, selects 8259A-1/2 init
	// ICW4 needed in this case
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW1, SLAVE_8259_PORT);
	// ICW2 sets the master PIC's vector offset to 0x20 and the slave's to 0x28
	outb(ICW2_MASTER, MASTER_8259_DATA_PORT);	// 8259A-1 IR0-7 mapped to 0x20-0x27
	outb(ICW2_SLAVE, SLAVE_8259_DATA_PORT);		// 8259A-2 IR0-7 mapped to 0x28-0x2f
	// ICW3 sets the slave(s) in the master mode and indentify the slave in the slave mode
	outb(ICW3_MASTER, MASTER_8259_DATA_PORT);	// master mode: 8259A-1 (the master) has a slave on IR2
	outb(ICW3_SLAVE, SLAVE_8259_DATA_PORT);		// slave mode: 8259A-2 is a slave on master's IR2
	// ICW4 sets the 8259A for 8086 system operation
	outb(ICW4, MASTER_8259_DATA_PORT);			/* master expects normal EOI */
	outb(ICW4, SLAVE_8259_DATA_PORT);			/* (slave's support for AEOI in flat mode
				    								is to be investigated) */
	outb(cached_21, MASTER_8259_DATA_PORT);		/* restore master IRQ mask */
	outb(cached_A1, SLAVE_8259_DATA_PORT);		/* restore slave IRQ mask */
	enable_irq(ICW3_SLAVE);
}

/* void enable_irq(uint32_t irq_num)
 * Inputs:  uint32_t irq_num = the Interrupt ReQuest number which
 * 							   the EOI signal will be sent to
 * Return Value: void
 * Function: Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
	unsigned int mask;
	// check if the irq number is valid
	if((irq_num < IRQ_LOWBOUND) || (irq_num > IRQ_HIGHBOUND)) return;
	// create mask for enabling
	if(irq_num >= ONLY_MASTER_PIC)
		mask = ~(1 << (irq_num - ONLY_MASTER_PIC));	// slave's mask
	else
		mask = ~(1 << irq_num);						// master's mask
	
	// check it is whose irq
	if (irq_num >= ONLY_MASTER_PIC){
		slave_mask &= mask;
		outb(slave_mask, SLAVE_8259_DATA_PORT);
	}
	else{
		master_mask &= mask;
		outb(master_mask, MASTER_8259_DATA_PORT);
	}
}

/* void disable_irq(uint32_t irq_num)
 * Inputs:  uint32_t irq_num = the Interrupt ReQuest number which
 * 							   the EOI signal will be sent to
 * Return Value: void
 * Function: Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
	unsigned int mask;
	// check if the irq number is valid
	if((irq_num < IRQ_LOWBOUND) || (irq_num > IRQ_HIGHBOUND)) return;
	// create mask for disabling
	if(irq_num >= ONLY_MASTER_PIC)
		mask = 1 << (irq_num - ONLY_MASTER_PIC);	// slave's mask
	else
		mask = 1 << irq_num;						// master's mask

	// check it is whose irq
	if (irq_num >= ONLY_MASTER_PIC){
		slave_mask &= mask;
		outb(slave_mask, SLAVE_8259_DATA_PORT);
	}
	else{
		master_mask &= mask;
		outb(master_mask, MASTER_8259_DATA_PORT);
	}
}

/* void send_eoi(uint32_t irq_num)
 * Inputs:  uint32_t irq_num = the Interrupt ReQuest number which
 * 							   the EOI signal will be sent to
 * Return Value: void
 * Function: Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
	// If the IRQ came from the Master PIC, it is sufficient to issue this command only to the Master PIC; 
	// however if the IRQ came from the Slave PIC, it is necessary to issue the command to both PIC chips.
	// Reference: https://wiki.osdev.org/8259_PIC
	// note: it is an specific EOI command, first four bits of OCW2 are 0011
	if (irq_num >= ONLY_MASTER_PIC)
	{
		// the IRQ came from the Slave PIC
		outb(EOI | (irq_num - ONLY_MASTER_PIC), SLAVE_8259_PORT);
		outb(EOI | ICW3_SLAVE, MASTER_8259_PORT);
	}
	else{
		// the IRQ came from the Master PIC
		outb(EOI | irq_num, MASTER_8259_PORT);
	}
	
}
