/*
 * tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

/* file variables */
static spinlock_t button_lock;
static unsigned long button_state;  
static unsigned long led_state;
static bool command_complete; //used for ACK

/* the data of 0-9, A-F to be printed on the tux controller */
const static unsigned char led_content[16] = {0xE7, 0x06, 0xCB, 
    0x8F, 0x2E, 0xAD, 0xED, 0x86, 0xEF, 0xAF, 0xEE, 0x6D, 0xE1, 0x4F, 0xE9, 0xE8};

/* functions used for tuxctl_ioctl */
static int tuxctl_ioctl_tux_init(struct tty_struct* tty);
static int tuxctl_ioctl_tux_buttons(unsigned long arg);
static int tuxctl_ioctl_tux_set_led(struct tty_struct* tty, unsigned long arg);

/* define used costants */
#define LED_MASK       0x000F  // check the low 16 bit of arg and 4 bit one by one
#define ARG_MASK 	   0x000F // check the low 4 bit of third and fourth byte of arg
#define LED_STATE_MASK 0x0001  // check the state of each led
#define LED_BUF_SIZE 6 	    // we need to put 4 bytes to set led
#define LED_NUMBER   4 	    // the number of LEDs
#define LED_BIT      4 	    // the number of bits of LED value

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in
 * tuxctl-ld.c. It calls this function, so all warnings there apply
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet) {
    unsigned a, b, c;
    unsigned long flags;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

    /*printk("packet : %x %x %x\n", a, b, c); */

	// check the opcode
    switch (a) {
		// the command is complete
        case MTCP_ACK:
            command_complete = true;
            return;
		// update button state
        case MTCP_BIOC_EVENT:
            spin_lock_irqsave(&button_lock, flags);
			/*
			 *     Packet format:
			 *        Byte 0 - MTCP_BIOC_EVENT
			 *        byte 1  +-7-----4-+-3-+-2-+-1-+---0---+
			 *            | 1 X X X | C | B | A | START |
			 *            +---------+---+---+---+-------+
			 *        byte 2  +-7-----4-+---3---+--2---+--1---+-0--+
			 *            | 1 X X X | right | down | left | up |
			 *            +---------+-------+------+------+----+
			 */
			// we need to let packet b and c  be the form of | right | left | down | up | c | b | a | start
			// flip b and c, so we can get active high value
			// we use 0x09 to get bit 3 and 0 of packet c, and left shift 4 bits
			// use 0x02 to get bit 1 of packet c, and left shift 5 bits
			// use 0x04 to get bit 2 of packet c, and left shift 3 bits
			// use 0x0F to get low 4 bits of packet b
			// After combining them, we can get what we want
            button_state = ((~c & 0x09) << 4) | ((~c & 0x02) << 5) | ((~c & 0x04) << 3) | (~b & 0x0F);
            spin_unlock_irqrestore(&button_lock, flags);
            return;
		// reset tux controller
        case MTCP_RESET:
            tuxctl_ioctl(tty, NULL, TUX_INIT, 0);
            tuxctl_ioctl(tty, NULL, TUX_SET_LED, led_state);
            return;
        default:
            return;
    }
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/

int tuxctl_ioctl(struct tty_struct* tty, struct file* file,
                 unsigned cmd, unsigned long arg) {
    switch (cmd) {
		// initiailize tux controller
        case TUX_INIT:
            return tuxctl_ioctl_tux_init(tty);
		// put button information to the argument
        case TUX_BUTTONS:
            return tuxctl_ioctl_tux_buttons(arg);
		// set led
        case TUX_SET_LED:
            return tuxctl_ioctl_tux_set_led(tty, arg);
        default:
            return -EINVAL;
    }
}

/*
 * tuxctl_ioctl_tux_init
 *   DESCRIPTION: Initialize tux controller
 *   INPUTS: tty -- tty_struct
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: initialize file variables
 */
static int tuxctl_ioctl_tux_init(struct tty_struct* tty) {

	// put initial opcode to tux controller
    unsigned char buf;
	buf = MTCP_BIOC_ON;
    tuxctl_ldisc_put(tty, &buf, 1);
	buf = MTCP_LED_USR;
	tuxctl_ldisc_put(tty, &buf, 1);

	// initialize file variables
    button_state = 0;
    led_state = 0;
    command_complete = false;
	button_lock = SPIN_LOCK_UNLOCKED;

    return 0;
}
  
/*
 * tuxctl_ioctl_tux_buttons
 *   DESCRIPTION: put button state to the pointer arg
 *   INPUTS: arg -- a pointer will store button state 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 -- copy success
 *				   -EINVAL -- copy fail
 *   SIDE EFFECTS: copy button state to the pointer arg
 */
static int tuxctl_ioctl_tux_buttons(unsigned long arg) {
    unsigned long flags;
    int check;

    spin_lock_irqsave(&button_lock, flags);
	// copy button state to arg
    check = copy_to_user((unsigned long *)arg, (unsigned long *)&(button_state), sizeof(unsigned long));
    
	// if copy success
    if(check == 0){
		spin_unlock_irqrestore(&button_lock, flags);
        return 0;
    }
	// if copy fail
    else{
		spin_unlock_irqrestore(&button_lock, flags);
        return -EINVAL;
    }
}

/*
 * tuxctl_ioctl_tux_set_led
 *   DESCRIPTION: Set the User-set LED display values
 *   INPUTS: tty -- tty_struct
 *           arg -- the led set value
 *   OUTPUTS: none
 *   RETURN VALUE: 0 
 *   SIDE EFFECTS: put opcode to tux controller
 */
static int tuxctl_ioctl_tux_set_led(struct tty_struct* tty, unsigned long arg) {
    
	// declare variables
    int i, LED_start_index;
	unsigned int mask;
	unsigned char buf[LED_BUF_SIZE];
    uint8_t led_should_turn_on;
    uint8_t decimal_point;
    
	// check if the last command is complete
	if(command_complete == false){
		return 0;
	}
	command_complete = false;
	
	// store led state
	led_state = arg;
	
	// get LED states and decimal point states
    mask = ARG_MASK;
	// shift arg 16 bits to know which LED’s should be turned on
    led_should_turn_on = (arg >> 16) & mask; 
	// shift arg 24 bits to know whether the corresponding decimal points should be turned on
    decimal_point = (arg >> 24) & mask;	
	
	// put opcode to buf
	buf[0] = MTCP_LED_SET;
    buf[1] = 0xF; // 0xF means we assume all the LEDs should be turned on

	LED_start_index = 2; // we start to store the value of LED from index 2 of buf
	
	// get the digit of each LED
    mask = LED_MASK;
    for(i = 0; i < LED_NUMBER; i++){
        buf[i + LED_start_index] = led_content[(arg & mask) >> (LED_BIT*i)];
        mask = mask << LED_BIT;
    }
		
	// get the open state and decimal points state of each LED
    mask = LED_STATE_MASK;
    for(i = 0; i < LED_NUMBER; i++){
		// if the LED should be turned on
        if(led_should_turn_on & mask){
            if(decimal_point & mask){
                buf[i + LED_start_index] |= 0x10; // 00010000 means that the decimal points should be turned on
            }
        }
		// if the LED should not be turned on
        else{
            buf[i + LED_start_index] = 0x00; // 00000000 means let no segments be turned on
        }
        mask = mask << 1;
    }

	// put opcode to the buf
    tuxctl_ldisc_put(tty, buf, LED_BUF_SIZE);
    return 0;
}



