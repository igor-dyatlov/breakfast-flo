/*
 * Copyright (C) 2017, Chad Froebel <chadfroebel@gmail.com>
 *		       Jean-Pierre Rasquin <yank555.lu@gmail.com>
 *		       Alex Saiko <solcmdr@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _LINUX_FASTCHG_H
#define _LINUX_FASTCHG_H

#define TAG "[FAST_CHARGE]"

/* Basic defines */
extern unsigned int force_fast_charge;
extern unsigned int usb_charge_level;
extern unsigned int failsafe;

#define USB_LEVELS "500 600 700 800 900 1000"

#define FAST_CHARGE_DISABLED 0
#define FAST_CHARGE_ENABLED 1

#define FAIL_SAFE_DISABLED 0
#define FAIL_SAFE_ENABLED 1

#define USB_CHARGE_500 500
#define USB_CHARGE_600 600
#define USB_CHARGE_700 700
#define USB_CHARGE_800 800
#define USB_CHARGE_900 900
#define USB_CHARGE_1000 1000

#define MIN_CHARGE_LEVEL 100
#define MAX_CHARGE_LEVEL 2000

#endif
