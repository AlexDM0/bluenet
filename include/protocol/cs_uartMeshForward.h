/**
 * Author: Alex de Mulder
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: 6 Nov., 2014
 * License: LGPLv3+, Apache License, or MIT, your choice
 */
#pragma once


#include "cs_MeshMessageTypes.h"
//#include <stdint.h>
//
class UartBuffer {

public:
	// Use static variant of singleton, no dynamic memory allocation
	static UartBuffer& getInstance() {
		static UartBuffer instance;
		return instance;
	}

	// Each tick we have time to dispatch events e.g.
	void tick();

	uint8_t uart_buffer_head = 0;
	uint8_t uart_buffer[MAX_MESH_MESSAGE_LEN + 1];
	uint8_t length = 0;
	bool null_found = false;
	bool one_found = false;

	// Function to be called from interrupt, do not do much there!
	void update(uint8_t value);

	bool bufferLoaded();

	void clear();
private:
	// constructor is hidden from the user
	UartBuffer() {}

	// This class is singleton, deny implementation
	UartBuffer(UartBuffer const&);
	// This class is singleton, deny implementation
	void operator=(UartBuffer const &);
};
