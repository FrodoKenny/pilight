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
#include "arctech_remote_old.h"

#define PULSE_MULTIPLIER	3
#define MIN_PULSE_LENGTH	310
#define MAX_PULSE_LENGTH	405
#define AVG_PULSE_LENGTH	335
#define RAW_LENGTH				50

static int validate(void) {
	if(arctech_remote_old->rawlen == RAW_LENGTH) {
		if(arctech_remote_old->raw[arctech_remote_old->rawlen-1] >= (MIN_PULSE_LENGTH*PULSE_DIV) &&
		   arctech_remote_old->raw[arctech_remote_old->rawlen-1] <= (MAX_PULSE_LENGTH*PULSE_DIV)) {
			return 0;
		}
	}

	return -1;
}

static void createMessage(int id, int all, int unit, int state) {
	arctech_remote_old->message = json_mkobject();
	json_append_member(arctech_remote_old->message, "id", json_mknumber(id, 0));
	if(all == 1) {
		json_append_member(arctech_remote_old->message, "all", json_mknumber(all, 0));
		json_append_member(arctech_remote_old->message, "unit", json_mknumber(0, 0));
	} else {
		json_append_member(arctech_remote_old->message, "all", json_mknumber(0, 0));
		json_append_member(arctech_remote_old->message, "unit", json_mknumber(unit, 0));
	}
	if(state == 1) {
		json_append_member(arctech_remote_old->message, "button", json_mkstring("on"));
	} else {
		json_append_member(arctech_remote_old->message, "button", json_mkstring("off"));
	}
	json_append_member(arctech_remote_old->message, "event", json_mknumber(1, 0));
}

static void parseCode(void) {
	int binary[RAW_LENGTH/4], x = 0, i = 0;
	int len = (int)((double)AVG_PULSE_LENGTH*((double)PULSE_MULTIPLIER/2));

	for(x=0;x<arctech_remote_old->rawlen-2;x+=4) {
		if(arctech_remote_old->raw[x+3] < len) {
			if(arctech_remote_old->raw[x+2] < len) {
				binary[i++] = -1;
			} else {
				binary[i++] = 1;
			}
		} else {
			binary[i++] = 0;
		}
	}

	int id = binToDec(binary, 0, 3);
	int all = 0;
	int unit = -1;
	if(binary[4] == -1 && binary[5] == -1 && binary[6] == -1 && binary[7] == -1) {
		all = 1;
	} else {
		unit = binToDec(binary, 4, 7);
	}
	int state = binary[11];
	createMessage(id, all, unit, state);
}

#if !defined(MODULE) && !defined(_WIN32)
__attribute__((weak))
#endif
void arctechRemoteOldInit(void) {

	protocol_register(&arctech_remote_old);
	protocol_set_id(arctech_remote_old, "arctech_remote_old");
	protocol_device_add(arctech_remote_old, "kaku_remote_old", "Old KlikAanKlikUit Remotes");
	protocol_device_add(arctech_remote_old, "cogex", "Cogex Remotes");
	protocol_device_add(arctech_remote_old, "intertechno_old", "Old Intertechno Remotes");
	protocol_device_add(arctech_remote_old, "byebyestandby", "Bye Bye Standby Remotes");
	protocol_device_add(arctech_remote_old, "duwi", "Düwi Terminal Remotes");
	protocol_device_add(arctech_remote_old, "eurodomest", "Eurodomest Remotes");
	arctech_remote_old->devtype = REMOTE;
	arctech_remote_old->hwtype = RF433;
	arctech_remote_old->minrawlen = RAW_LENGTH;
	arctech_remote_old->maxrawlen = RAW_LENGTH;
	arctech_remote_old->maxgaplen = MAX_PULSE_LENGTH*PULSE_DIV;
	arctech_remote_old->mingaplen = MIN_PULSE_LENGTH*PULSE_DIV;

	options_add(&arctech_remote_old->options, 'i', "id", OPTION_HAS_VALUE, DEVICES_ID, JSON_NUMBER, NULL, "^([0-9]{1}|[1][0-5])$");
	options_add(&arctech_remote_old->options, 'a', "all", OPTION_HAS_VALUE, DEVICES_ID, JSON_NUMBER, NULL, "^(0|1)$");
	options_add(&arctech_remote_old->options, 'u', "unit", OPTION_HAS_VALUE, DEVICES_ID, JSON_NUMBER, NULL, "^([0-9]{1}|[1][0-5])$");
	options_add(&arctech_remote_old->options, 'b', "button", OPTION_HAS_VALUE, DEVICES_ID, JSON_STRING, NULL, "^(on|off)$");
	options_add(&arctech_remote_old->options, 'e', "event", OPTION_HAS_VALUE, DEVICES_VALUE, JSON_NUMBER, NULL, "^(0|1)$");

	arctech_remote_old->parseCode=&parseCode;
	arctech_remote_old->validate=&validate;
}

#if defined(MODULE) && !defined(_WIN32)
void compatibility(struct module_t *module) {
	module->name = "arctech_remote_old";
	module->version = "2.5";
	module->reqversion = "6.0";
	module->reqcommit = "84";
}

void init(void) {
	arctechRemoteOldInit();
}
#endif
