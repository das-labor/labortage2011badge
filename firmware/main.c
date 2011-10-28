/* Name: main.c
 * Project: hid-custom-rq example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-07
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 * This Revision: $Id: main.c 692 2008-11-07 15:07:40Z cs $
 */

/*
This example should run on most AVRs with only little changes. No special
hardware resources except INT0 are used. You may have to change usbconfig.h for
different I/O pins for USB. Please note that USB D+ must be the INT0 pin, or
at least be connected to INT0 as well.
We assume that an LED is connected to port B bit 0. If you connect it to a
different port or bit, change the macros below:
*/
#define LED_PORT_DDR        DDRB
#define LED_PORT_OUTPUT     PORTB
#define R_BIT            4
#define G_BIT            3
#define B_BIT            1
#define BUTTON_PIN 4

#include <stdint.h>
#include <string.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */
#include "requests.h"       /* The custom request numbers we use */
#include "special_functions.h"

void update_pwm(void);

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

char usbHidReportDescriptor[22] PROGMEM = {    /* USB report descriptor */
    0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
    0xc0                           // END_COLLECTION
};
/* The descriptor above is a dummy only, it silences the drivers. The report
 * it describes consists of one byte of undefined data.
 * We don't transfer our data through HID reports, we use custom requests
 * instead.
 */

union {
	struct {
		uint16_t red;
		uint16_t green;
		uint16_t blue;
	} name;
	uint16_t idx[3];
} color;

#define UNI_BUFFER_SIZE 16

static union {
	uint8_t  w8[UNI_BUFFER_SIZE];
	uint16_t w16[UNI_BUFFER_SIZE/2];
	uint32_t w32[UNI_BUFFER_SIZE/4];
	void*    ptr[UNI_BUFFER_SIZE/sizeof(void*)];
} uni_buffer;

static uint8_t uni_buffer_fill;
static uint8_t current_command;
/* ------------------------------------------------------------------------- */


uint8_t read_button(void){
	uint8_t t,u,v=0;
	t = DDRB;
	u = PORTB;
	DDRB &= ~(1<<BUTTON_PIN);
	PORTB |= 1<<BUTTON_PIN;
	PORTB &= ~(1<<BUTTON_PIN);
	v |= PINB;
	DDRB |= t&(1<<BUTTON_PIN);
	PORTB &= ~(t&(1<<BUTTON_PIN));
	v >>= BUTTON_PIN;
	v &= 1;
	v ^= 1;
	return v;
}

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (usbRequest_t *)data;

    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_VENDOR)
	{
		current_command = rq->bRequest;
    	switch(rq->bRequest)
		{
		case CUSTOM_RQ_SET_RED:
			color.name.red = rq->wValue.bytes[0];
			break;	
		case CUSTOM_RQ_SET_GREEN:
			color.name.green = rq->wValue.bytes[0];
			break;	
		case CUSTOM_RQ_SET_BLUE:
			color.name.blue = rq->wValue.bytes[0];
			break;	
		case CUSTOM_RQ_SET_RGB:
			return USB_NO_MSG;
		case CUSTOM_RQ_GET_RGB:{
			usbMsgLen_t len=6;
			if(len>rq->wLength.word){
				len = rq->wLength.word;
			}
			usbMsgPtr = (uchar*)color.idx;
			return len;
		}
		case CUSTOM_RQ_READ_MEM:
			usbMsgPtr = (uchar*)rq->wValue.word;
			return rq->wLength.word;
		case CUSTOM_RQ_WRITE_MEM:
		case CUSTOM_RQ_EXEC_SPM:
			uni_buffer_fill = 4;
			uni_buffer.w16[0] = rq->wValue.word;
			uni_buffer.w16[1] = rq->wLength.word;
			return USB_NO_MSG;
		case CUSTOM_RQ_READ_FLASH:
			uni_buffer.w16[0] = rq->wValue.word;
			uni_buffer.w16[1] = rq->wLength.word;
			return USB_NO_MSG;
		case CUSTOM_RQ_RESET:
			soft_reset((uint8_t)(rq->wValue.word));
			break;
		case CUSTOM_RQ_READ_BUTTON:
			uni_buffer.w8[0] = read_button();
			usbMsgPtr = uni_buffer.w8;
			return 1;
		}
    }
	else
	{
        /* calls requests USBRQ_HID_GET_REPORT and USBRQ_HID_SET_REPORT are
         * not implemented since we never call them. The operating system
         * won't call them either because our descriptor defines no meaning.
         */
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}

uchar usbFunctionWrite(uchar *data, uchar len)
{
	switch(current_command){
	case CUSTOM_RQ_SET_RGB:
		if(len!=6){
			return 1;
		}
		memcpy(color.idx, data, 6);
		return 1;
	case CUSTOM_RQ_WRITE_MEM:
		memcpy(uni_buffer.ptr[0], data, len);
		uni_buffer.w16[0] += len;
		return !(uni_buffer.w16[1] -= len);
	case CUSTOM_RQ_EXEC_SPM:
		if(uni_buffer_fill<8){
			uint8_t l = 8-uni_buffer_fill;
			if(len<l){
				len = l;
			}
			memcpy(&(uni_buffer.w8[uni_buffer_fill]), data, len);
			uni_buffer_fill += len;
			return 0;
		}
		uni_buffer.w16[1] -= len;
		if(uni_buffer.w16[1]>8){
			memcpy(uni_buffer.ptr[0], data, len);
			uni_buffer.w16[0] += len;
			return 0;
		}else{
			memcpy(&(uni_buffer.w8[uni_buffer_fill]), data, len);
			exec_spm(uni_buffer.w16[2], uni_buffer.w16[3], uni_buffer.ptr[0], data, len);
			return 1;
		}
	default:
		return 1;
	}
	return 0;
}
uchar usbFunctionRead(uchar *data, uchar len){
	uchar ret=len;
	switch(current_command){
	case CUSTOM_RQ_READ_FLASH:
		while(len--){
			*data++ = pgm_read_byte((uni_buffer.w16[0])++);
		}
		return ret;
	default:
		break;
	}
	return 0;
}

static void calibrateOscillator(void)
{
uchar       step = 128;
uchar       trialValue = 0, optimumValue;
int         x, optimumDev, targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);
 
    /* do a binary search: */
    do{
        OSCCAL = trialValue + step;
        x = usbMeasureFrameLength();    // proportional to current real frequency
        if(x < targetValue)             // frequency still too low
            trialValue += step;
        step >>= 1;
    }while(step > 0);
    /* We have a precision of +/- 1 for optimum OSCCAL here */
    /* now do a neighborhood search for optimum value */
    optimumValue = trialValue;
    optimumDev = x; // this is certainly far away from optimum
    for(OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++){
        x = usbMeasureFrameLength() - targetValue;
        if(x < 0)
            x = -x;
        if(x < optimumDev){
            optimumDev = x;
            optimumValue = OSCCAL;
        }
    }
    OSCCAL = optimumValue;
}
 

void usbEventResetReady(void)
{
    cli();  // usbMeasureFrameLength() counts CPU cycles, so disable interrupts.
    calibrateOscillator();
    sei();
// we never read the value from eeprom so this causes only degradation of eeprom
//    eeprom_write_byte(0, OSCCAL);   // store the calibrated value in EEPROM
}

/* ------------------------------------------------------------------------- */

int main(void)
{
	uchar   i;

    wdt_enable(WDTO_1S);
    /* Even if you don't use the watchdog, turn it off here. On newer devices,
     * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
     */
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */

    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    LED_PORT_DDR |= _BV(R_BIT) | _BV(G_BIT) | _BV(B_BIT);   /* make the LED bit an output */
	
    sei();

    for(;;){                /* main event loop */
		update_pwm();
		
        wdt_reset();
        usbPoll();
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
