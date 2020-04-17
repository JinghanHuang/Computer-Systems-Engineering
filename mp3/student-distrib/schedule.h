#ifndef SCHEDULING_H
#define SCHEDULING_H

#include "types.h"
#include "system_call.h"

#define VAL_1 		0x36
#define PIT_PORT_1 	0x43
#define PIT_PORT_2	0x40
#define SET_40HZ   	29829 /* = 1193182 / 40 */
#define PIT_IRQ 	0
#define PIT_MASK	0xFF
#define PIT_BIT_MOVE 8
#define WAIT_TIME   5
extern volatile uint8_t cur_term_sch;
extern volatile uint8_t next_term_sch;

/* initialize pit */
void pit_init(void);

/* pit interrupt handler */
void pit_interrupt_handler(void);
/* get the index of next pcb*/
uint32_t get_next(void);

#endif
