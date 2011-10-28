/* Name: set-led.c
 * Project: hid-custom-rq example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-10
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 * This Revision: $Id: set-led.c 692 2008-11-07 15:07:40Z cs $
 */

/*
General Description:
This is the host-side driver for the custom-class example device. It searches
the USB for the LEDControl device and sends the requests understood by this
device.
This program must be linked with libusb on Unix and libusb-win32 on Windows.
See http://libusb.sourceforge.net/ or http://libusb-win32.sourceforge.net/
respectively.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <usb.h>        /* this is libusb */
#include <arpa/inet.h>
#include "hexdump.h"
#include "opendevice.h" /* common code moved to separate module */

#include "../firmware/requests.h"   /* custom request numbers */
#include "../firmware/usbconfig.h"  /* device's VID/PID and names */

int safety_question_override=0;
int pad=-1;

char* fname;
usb_dev_handle *handle = NULL;

void set_rgb(char* color){
	uint16_t buffer[3] = {0, 0, 0};
	sscanf(color, "%hi:%hi:%hi", &(buffer[0]), &(buffer[1]), &(buffer[2]));
	buffer[0] &= (1<<10)-1;
	buffer[1] &= (1<<10)-1;
	buffer[2] &= (1<<10)-1;
	usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, CUSTOM_RQ_SET_RGB, 0, 0, (char*)buffer, 6, 5000);
}

void get_rgb(char* param){
	uint16_t buffer[3];
	int cnt;
	cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, CUSTOM_RQ_GET_RGB, 0, 0, (char*)buffer, 6, 5000);
	if(cnt!=6){
		fprintf(stderr, "ERROR: received %d bytes from device while expecting %d bytes\n", cnt, 6);
		exit(1);
	}
	printf("red:   %3.3hX\ngreen: %3.3hX\nblue:  %3.3hX\n", buffer[0], buffer[1], buffer[2]);
}

void read_mem(char* param){
	int length=0;
	uint8_t *buffer, *addr;
	int cnt;
	FILE* f=NULL;
	if(fname){
		f = fopen(fname, "wb");
		if(!f){
			fprintf(stderr, "ERROR: could not open %s for writing\n", fname);
			exit(1);
		}
	}
	sscanf(param, "%i:%i", (int*)&addr, &length);
	if(length<=0){
		return;
	}
	buffer = malloc(length);
	if(!buffer){
		if(f)
			fclose(f);
		fprintf(stderr, "ERROR: out of memory\n");
		exit(1);
	}
	cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, CUSTOM_RQ_READ_MEM, (int)((unsigned)addr), 0, (char*)buffer, length, 5000);
	if(cnt!=length){
		if(f)
			fclose(f);
		fprintf(stderr, "ERROR: received %d bytes from device while expecting %d bytes\n", cnt, length);
		exit(1);
	}
	if(f){
		cnt = fwrite(buffer, 1, length, f);
		fclose(f);
		if(cnt!=length){
			fprintf(stderr, "ERROR: could write only %d bytes out of %d bytes\n", cnt, length);
			exit(1);
		}

	}else{
		hexdump_block(stdout, buffer, addr, length, 8);
	}
}

void write_mem(char* param){
	int length;
	uint8_t *addr, *buffer, *data=NULL;
	int cnt=0;
	FILE* f=NULL;

	if(fname){
		f = fopen(fname, "rb");
		if(!f){
			fprintf(stderr, "ERROR: could not open %s for writing\n", fname);
			exit(1);
		}
	}
	sscanf(param, "%i:%i:%n", (int*)&addr, &length, &cnt);
	data += cnt;
	if(length<=0){
		return;
	}
	buffer = malloc(length);
	if(!buffer){
		if(f)
			fclose(f);
		fprintf(stderr, "ERROR: out of memory\n");
		exit(1);
	}
	memset(buffer, (uint8_t)pad, length);
	if(!data && !f && length==0){
		fprintf(stderr, "ERROR: no data to write\n");
		exit(1);
	}
	if(f){
		cnt = fread(buffer, 1, length, f);
		fclose(f);
		if(cnt!=length && pad==-1){
			fprintf(stderr, "Warning: could ony read %d bytes from file; will only write these bytes", cnt);
		}
	}else if(data){
		char xbuffer[3]= {0, 0, 0};
		uint8_t fill=0;
		unsigned idx=0;
		while(*data && idx<length){
			while(*data && !isxdigit(*data)){
				++data;
			}
			xbuffer[fill++] = *data;
			if(fill==2){
				uint8_t t;
				t = strtoul(xbuffer, NULL, 16);
				buffer[idx++] = t;
				fill = 0;
			}
		}

	}
	cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, CUSTOM_RQ_WRITE_MEM, (int)addr, 0, (char*)buffer, length, 5000);
	if(cnt!=length){
		fprintf(stderr, "ERROR: device accepted ony %d bytes out of %d\n", cnt, length);
		exit(1);
	}

}

void read_flash(char* param){
	int length=0;
	uint8_t *buffer, *addr;
	int cnt;
	FILE* f=NULL;
	if(fname){
		f = fopen(fname, "wb");
		if(!f){
			fprintf(stderr, "ERROR: could not open %s for writing\n", fname);
			exit(1);
		}
	}
	sscanf(param, "%i:%i", (int*)&addr, &length);
	if(length<=0){
		return;
	}
	buffer = malloc(length);
	if(!buffer){
		if(f)
			fclose(f);
		fprintf(stderr, "ERROR: out of memory\n");
		exit(1);
	}
	cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, CUSTOM_RQ_READ_FLASH, (int)addr, 0, (char*)buffer, length, 5000);
	if(cnt!=length){
		if(f)
			fclose(f);
		fprintf(stderr, "ERROR: received %d bytes from device while expecting %d bytes\n", cnt, length);
		exit(1);
	}
	if(f){
		cnt = fwrite(buffer, 1, length, f);
		fclose(f);
		if(cnt!=length){
			fprintf(stderr, "ERROR: could write only %d bytes out of %d bytes\n", cnt, length);
			exit(1);
		}

	}else{
		hexdump_block(stdout, buffer, addr, length, 8);
	}
}

void soft_reset(char* param){
	unsigned delay=0;
	if(param){
		sscanf(param, "%i", &delay);
	}
	delay &= 0xf;
	usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, CUSTOM_RQ_RESET, (int)delay, 0, NULL, 0, 5000);
}

static struct option long_options[] =
             {
               /* These options set a flag. */
               {"i-am-sure", no_argument,       &safety_question_override, 1},
               {"pad", optional_argument,       0, 'p'},
               /* These options don't set a flag.
                  We distinguish them by their indices. */
               {"set-rgb",    required_argument, 0, 's'},
               {"get-rgb",          no_argument, 0, 'g'},
               {"read-mem",   required_argument, 0, 'r'},
               {"write-mem",  required_argument, 0, 'w'},
               {"read-flash", required_argument, 0, 'z'},
               {"exec-spm",   required_argument, 0, 'x'},
               {"read-adc",   required_argument, 0, 'a'},
               {"reset",      optional_argument, 0, 'q'},
               {"file",       required_argument, 0, 'f'},
               {0, 0, 0, 0}
             };

static void usage(char *name)
{
	char *usage_str =
	"usage:\n"
    "    %s [<option>] <command> <parameter string>\n"
    "  where <option> is one of the following:\n"
	"    -p --pad [<pad value>] ............................ pad writing data with <pad value> (default 0) to specified length\n"
	"    -f --file <name> .................................. use file <name> for reading or writing data\n"
	"    --i-am-sure ....................................... do not ask safety question\n"
	"  <command> is one of the following\n"
	"    -s --set-rgb <red>:<green>:<blue> ................. set color\n"
	"    -g --get-rgb ...................................... read color from device and print\n"
	"    -r --read-mem <addr>:<length> ..................... read RAM\n"
	"    -w --write-mem <addr>:<length>[:data] ............. write RAM\n"
	"    -z --read-flash <addr>:<length> ................... read flash\n"
	"    -x --exec-spm <addr>:<length>:<Z>:<R0R1>[:data] ... write RAM, set Z pointer, set r0:r1 and execute SPM\n"
	"    -a --read-adc <adc> ............................... read ADC\n"
	"    -q --reset ........................................ reset the controller"
	;
	fprintf(stderr, usage_str, name);
}


int main(int argc, char **argv)
{
  const unsigned char rawVid[2] = {USB_CFG_VENDOR_ID}; 
  const unsigned char rawPid[2] = {USB_CFG_DEVICE_ID};
  char vendor[] = {USB_CFG_VENDOR_NAME, 0};
  char product[] = {USB_CFG_DEVICE_NAME, 0};
  int  vid, pid;
  int  c, option_index;
  void(*action_fn)(char*) = NULL;
  char* main_arg = NULL;

  usb_init();
  if(argc < 2){   /* we need at least one argument */
    usage(argv[0]);
    exit(1);
  }
  /* compute VID/PID from usbconfig.h so that there is a central source of information */
    vid = rawVid[1] * 256 + rawVid[0];
    pid = rawPid[1] * 256 + rawPid[0];
    /* The following function is in opendevice.c: */
    if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0){
        fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);
        exit(1);
    }

    for(;;){
    	c = getopt_long(argc, argv, "s:gr:z:w:x:a:f:p::q::",
                long_options, &option_index);
    	if(c == -1){
    		break;
    	}

    	if(action_fn && strchr("sgrzwxaq", c)){
    		/* action given while already having an action */
    		usage(argv[0]);
    		exit(1);
    	}

    	if(strchr("sgrzwxaq", c)){
    		main_arg = optarg;
    	}

    	switch(c){
    	case 's': action_fn = set_rgb; break;
    	case 'g': action_fn = get_rgb; break;
    	case 'r': action_fn = read_mem; break;
    	case 'z': action_fn = read_flash; break;
    	case 'w': action_fn = write_mem; break;
    	case 'q': action_fn = soft_reset; break;
    	case 'f': fname = optarg; break;
    	case 'p': pad = 0; if(optarg) pad=strtoul(optarg, NULL, 0); break;
    	case 'x':
    	case 'a':
    	default:
    		break;
    	}
    }

    if(!action_fn){
    	usage(argv[0]);
    	fprintf(stderr, "Error: no action specified\n");
    	return 1;
    }else{
    	action_fn(main_arg);
        usb_close(handle);
    	return 0;
    }

}
