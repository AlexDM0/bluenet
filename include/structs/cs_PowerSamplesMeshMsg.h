/*
 * Author: Bart van Vliet
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: Jun 2, 2016
 * License: LGPLv3+, Apache License, or MIT, your choice
 */

#pragma once

#define POWER_SAMPLE_MESH_NUM_SAMPLES 43
//! 91 bytes in total
struct __attribute__((__packed__)) power_samples_mesh_message_t {
	uint32_t timestamp;
//	uint16_t firstSample;
//	int8_t sampleDiff[POWER_SAMPLE_MESH_NUM_SAMPLES-1];
	uint16_t samples[POWER_SAMPLE_MESH_NUM_SAMPLES];
	uint8_t reserved;
//	struct __attribute__((__packed__)) {
//		int8_t dt1 : 4;
//		int8_t dt2 : 4;
//	} timeDiffs[POWER_SAMPLE_MESH_NUM_SAMPLES / 2];
};
