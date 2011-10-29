/* Name: requests.h
 * Project: custom-class, a basic USB example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-09
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 * This Revision: $Id: requests.h 692 2008-11-07 15:07:40Z cs $
 */

/* This header is shared between the firmware and the host software. It
 * defines the USB request numbers (and optionally data types) used to
 * communicate between the host and the device.
 */

#ifndef __REQUESTS_H_INCLUDED__
#define __REQUESTS_H_INCLUDED__

#define CUSTOM_RQ_SET_RED      0x03
#define CUSTOM_RQ_SET_GREEN    0x04
#define CUSTOM_RQ_SET_BLUE     0x05
#define CUSTOM_RQ_SET_RGB      0x06
#define CUSTOM_RQ_GET_RGB      0x07
#define CUSTOM_RQ_FADE_RGB     0x08

#define CUSTOM_RQ_READ_MEM     0x10
#define CUSTOM_RQ_WRITE_MEM    0x11
#define CUSTOM_RQ_READ_FLASH   0x12
#define CUSTOM_RQ_EXEC_SPM     0x13
#define CUSTOM_RQ_RESET        0x14
#define CUSTOM_RQ_READ_BUTTON  0x15
#define CUSTOM_RQ_READ_TMPSENS 0x16





#endif /* __REQUESTS_H_INCLUDED__ */
