/*
	Copyright (C) 2013 CurlyMo

	This file is part of pilight.

	pilight is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later
	version.

	pilight is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with pilight. If not, see	<http://www.gnu.org/licenses/>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../../core/pilight.h"
#include "../../core/common.h"
#include "../../core/dso.h"
#include "../../core/log.h"
#include "../protocol.h"
#include "../../core/binary.h"
#include "../../core/gc.h"
#include "arctech_remote.h"

#define LEARN_REPEATS			40
#define NORMAL_REPEATS		10
#define PULSE_MULTIPLIER	4
#define MIN_PULSE_LENGTH	274
#define MAX_PULSE_LENGTH	320
#define AVG_PULSE_LENGTH	300
#define RAW_LENGTH				132

static int validate(void) {
	if(arctech_remote->rawlen == RAW_LENGTH) {
		if(arctech_remote->raw[arctech_remote->rawlen-1] >= (MIN_PULSE_LENGTH*PULSE_DIV) &&
		   arctech_remote->raw[arctech_remote->rawlen-1] <= (MAX_PULSE_LENGTH*PULSE_DIV) &&
			 arctech_remote->raw[1] >= AVG_PULSE_LENGTH*(PULSE_MULTIPLIER*1.5)) {
			return 0;
		}
	}

	return -1;
}

static void createMessage(int id, int all, int unit, int state) {
	arctech_remote->message = json_mkobject();
	json_append_member(arctech_remote->message, "id", json_mknumber(id, 0));
	if(all == 1) {
		json_append_member(arctech_remote->message, "all", json_mknumber(all, 0));
		json_append_member(arctech_remote->message, "unit", json_mknumber(0, 0));
	} else {
		json_append_member(arctech_remote->message, "all", json_mknumber(0, 0));
		json_append_member(arctech_remote->message, "unit", json_mknumber(unit, 0));
	}
	if(state == 1) {
		json_append_member(arctech_remote->message, "button", json_mkstring("on"));
	} else {
		json_append_member(arctech_remote->message, "button", json_mkstring("off"));
	}
	json_append_member(arctech_remote->message, "event", json_mknumber(1, 0));
}

static void parseCode(void) {
	int binary[RAW_LENGTH/4], x = 0, i = 0;

	for(x=0;x<arctech_remote->rawlen;x+=4) {
		if(arctech_remote->raw[x+3] > (int)((double)AVG_PULSE_LENGTH*((double)PULSE_MULTIPLIER/2))) {
			binary[i++] = 1;
		} else {
			binary[i++] = 0;
		}
	}

	int id = binToDecRev(binary, 0, 25);
	int all = binary[26];
	int unit = binToDecRev(binary, 28, 31);
	int state = binary[27];

	createMessage(id, all, unit, state);
}

#if !defined(MODULE) && !defined(_WIN32)
__attribute__((weak))
#endif
void arctechRemoteInit(void) {
	protocol_register(&arctech_remote);
	protocol_set_id(arctech_remote, "arctech_remote");
	protocol_device_add(arctech_remote, "kaku_remote", "KlikAanKlikUit Remotes");
	protocol_device_add(arctech_remote, "dio_remote", "D-IO Remotes");
	protocol_device_add(arctech_remote, "nexa_remote", "Nexa Remotes");
	protocol_device_add(arctech_remote, "coco_remote", "CoCo Technologies Remotes");
	protocol_device_add(arctech_remote, "intertechno_remote", "Intertechno Remotes");
	arctech_remote->devtype = REMOTE;
	arctech_remote->hwtype = RF433;
	arctech_remote->txrpt = NORMAL_REPEATS;
	arctech_remote->minrawlen = RAW_LENGTH;
	arctech_remote->maxrawlen = RAW_LENGTH;
	arctech_remote->maxgaplen = MAX_PULSE_LENGTH*PULSE_DIV;
	arctech_remote->mingaplen = MIN_PULSE_LENGTH*PULSE_DIV;

	options_add(&arctech_remote->options, 'i', "id", OPTION_HAS_VALUE, DEVICES_ID, JSON_NUMBER, NULL, "^([0-9]{1,7}|[1-5][0-9]{7}|6([0-6][0-9]{6}|7(0[0-9]{5}|10([0-7][0-9]{3}|8([0-7][0-9]{2}|8([0-5][0-9]|6[0-3]))))))$");
	options_add(&arctech_remote->options, 'a', "all", OPTION_HAS_VALUE, DEVICES_ID, JSON_NUMBER, NULL, "^(0|1)$");
	options_add(&arctech_remote->options, 'u', "unit", OPTION_HAS_VALUE, DEVICES_ID, JSON_NUMBER, NULL, "^([0-9]{1}|[1][0-5])$");
	options_add(&arctech_remote->options, 'b', "button", OPTION_HAS_VALUE, DEVICES_ID, JSON_STRING, NULL, "^(on|off)$");
	options_add(&arctech_remote->options, 'e', "event", OPTION_HAS_VALUE, DEVICES_VALUE, JSON_NUMBER, NULL, "^(0|1)$");

	arctech_remote->parseCode=&parseCode;
	arctech_remote->validate=&validate;
}

#if defined(MODULE) && !defined(_WIN32)
void compatibility(struct module_t *module) {
	module->name = "arctech_remote";
	module->version = "3.2";
	module->reqversion = "6.0";
	module->reqcommit = "84";
}

void init(void) {
	arctechRemoteInit();
}
#endif
