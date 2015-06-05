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
	uart_buffer[uart_buffer_head] = value;
	length++;
	uart_buffer_head = (uart_buffer_head + 1) % UART_BUFFER_SIZE;
}

bool UartBuffer::bufferLoaded() {
	if (length >= PACKET_SIZE) {
		length = 0;
		uart_buffer_head = 0;
		return true;
	}
	return false;
}


extern "C" void UART0_IRQHandler(void)
{
	UartBuffer & buff = UartBuffer::getInstance();
	buff.update(read_uart());
}
