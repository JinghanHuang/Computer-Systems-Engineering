#ifndef _RTC_H
#define _RTC_H
#include "types.h"
#include "file.h"
//RTC data port is on x70
//cmos memory registers are on port x71
#define RTC_PORT     0x70
#define CMOS_PORT    0x71

//need to change the set bit in reg b to 1 inorder to make changes to the
//registers
#define REG_SET    0x80
#define RATE_SET   0xF0

//will be used for later checkpoint
//values for setting up how to read the registers on the rtc for different
//times
#define SECONDS 0x00
#define MINUTES 0x02
#define HOURS   0x04
#define WEEKDAY 0x06
#define DAY_OF_MONTH 0x07
#define MONTH   0x08
#define YEAR    0x09
#define CENTURY 0x32  // not sure if this exists on our hardware

// the register locations
#define REG_A   0x0A
#define REG_B   0x0B
#define REG_C   0x0C

//there are three different kinds of interrupts the rtc can generate
//an alarm interrupt that goes off after a set time
//a periodic interrupt that goes through a cycle and throws and interrupt
//when the cycle ends
// an update interrupt that goes off when a set change occurres like DAY/or month
//changes
#define PERIODIC_INT  0x40
#define ALARM_INT     0x20
#define UPDATE_INT    0x10
#define ALL_INTS      0x70

#define IRQ_LINE 0x08   // should be in irq 8 but which pic is that on?
                        // its on line one of the slave



#define DEFAULT_FOC 2
#define DEFAULT_FOC_VIRTUAL 1024
#define BYTE_SIZE   4
#define VIRTUAL_READ_SPEED   5

//rates defined
#define RATE_1      0xF  //2Hz
#define RATE_2      0xE  //4Hz
#define RATE_3      0xD  //8Hz
#define RATE_4      0xC  //16Hz
#define RATE_5      0xB  //32Hz
#define RATE_6      0xA  //64Hz
#define RATE_7      0x9  //128Hz
#define RATE_8      0x8  //256Hz
#define RATE_9      0x7  //512Hz
#define RATE_10     0x6  //1024Hz

//initializes the rtc. the handler should be initialized before.
//handler is initalized in the idt
//rtc is initialized in the kernel
extern void init_rtc(void);

//for now the interrupt handler doesnt do anything beyond the part one test
void rtc_interrupt_handler(void);

//system calls for the RTC
extern int32_t rtc_open(const uint8_t* fname);
extern int32_t rtc_close(int32_t fd);
extern int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);


//helper function for changing the frequency
int32_t
set_Freq(int frequency);

#endif /* _RTC_H */
