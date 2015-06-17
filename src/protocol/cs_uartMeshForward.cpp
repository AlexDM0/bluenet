/**
 * Author: Alex de Mulder
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: 5 Jun., 2015
 * License: LGPLv3+, Apache License, or MIT, your choice
 */

#include <stdint.h>
#include "drivers/cs_Serial.h"
#include "protocol/cs_uartMeshForward.h"

// Each tick we have time to dispatch events e.g.
void UartBuffer::tick() {};

void UartBuffer::update(uint8_t value) {
	if (value == 0) {
		this->null_found = true;
	}
	else if (value == 1) {
		this->one_found = true;
	}

	if (value == 2) {
		length = 0;
		uart_buffer_head = 0;
		this->null_found = false;
		this->one_found = false;
	}
	else {
		uart_buffer[uart_buffer_head] = value;
		length++;
		uart_buffer_head = (uart_buffer_head + 1) % (MAX_MESH_MESSAGE_LEN+1);
	}
}

bool UartBuffer::bufferLoaded() {
	if (this->one_found == true) {
		write("CRC:");
		uart_buffer[length-1] = 0;
		this->one_found = false;
		write((char*)uart_buffer);
		write("\n");
		return false;
	}

	if (length >= MAX_MESH_MESSAGE_LEN || this->null_found == true) {
		LOGi("length on send: %i", length);
		length = 0;
		this->null_found = false;
		uart_buffer_head = 0;
		return true;
	}
	return false;
}

void UartBuffer::clear() {
	for (uint8_t i = 0; i < MAX_MESH_MESSAGE_LEN; i++) {
		uart_buffer[i] = 0;
	}
}


extern "C" void UART0_IRQHandler(void)
{
	UartBuffer & buff = UartBuffer::getInstance();
	buff.update(read_uart());
}
