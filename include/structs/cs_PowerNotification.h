/*
 * Author: Bart van Vliet
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: May 2, 2016
 * License: LGPLv3+, Apache License, or MIT, your choice
 */

#ifndef INCLUDE_STRUCTS_CS_POWERNOTIFICATION_H_
#define INCLUDE_STRUCTS_CS_POWERNOTIFICATION_H_

struct __attribute__((__packed__)) power_notification_t {
	uint8_t length;
	uint16_t values[9];
	uint8_t checksum;
};



#endif /* INCLUDE_STRUCTS_CS_POWERNOTIFICATION_H_ */
