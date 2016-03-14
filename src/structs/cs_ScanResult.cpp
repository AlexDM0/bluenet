/**
 * Author: Dominik Egger
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: Nov 4, 2014
 * License: LGPLv3+, Apache, and/or MIT, your choice
 */

//#include <string.h>
//#include <stdio.h>
//
#include "structs/cs_ScanResult.h"
//#include "drivers/cs_Serial.h"
#include "cfg/cs_WhiteList.h"
#include "cfg/cs_BlackList.h"

//! returns the number of elements stored so far
uint8_t ScanResult::getSize() const {
	if (_buffer != NULL) {
		return _buffer->size;
	} else {
		return 0;
	}
}

void ScanResult::clear() {
	memset(_buffer, 0, sizeof(peripheral_device_list_t));
}

//int count = 1;
void ScanResult::update(uint8_t * adrs_ptr, int8_t rssi) {

	if (_buffer == NULL) {
		LOGe("scan result buffer was not assigned!!!");
		return;
	}

//	LOGd("count: %d", count++);
//	char addrs[28];
//	sprintf(addrs, "[%02X %02X %02X %02X %02X %02X]", adrs_ptr[5],
//			adrs_ptr[4], adrs_ptr[3], adrs_ptr[2], adrs_ptr[1],
//			adrs_ptr[0]);
//	LOGd("addrs: %s", addrs);

//	//! Check if the address is in the white list
//	//! If not, return
//	bool whiteListed = false;
//	for (int i=0; i<WHITELIST_LENGTH; i++) {
//		if (memcmp(adrs_ptr, WhiteList + i*BLE_GAP_ADDR_LEN, BLE_GAP_ADDR_LEN) == 0) {
//			whiteListed = true;
//			break;
//		}
//	}
//	if (!whiteListed) {
//		return;
//	}

//	//! Check if the address is in the black list
//	//! If so, return
//	for (int i=0; i<BLACKLIST_LENGTH; i++) {
//		if (memcmp(adrs_ptr, BlackList + i*BLE_GAP_ADDR_LEN, BLE_GAP_ADDR_LEN) == 0) {
//			return;
//		}
//	}


//	bool found = false;
	for (int i = 0; i < getSize(); ++i) {
//		LOGd("_history[%d]: [%02X %02X %02X %02X %02X %02X]", i, _buffer->list[i].addr[5],
//				_buffer->list[i].addr[4], _buffer->list[i].addr[3], _buffer->list[i].addr[2], _buffer->list[i].addr[1],
//				_buffer->list[i].addr[0]);

		if (memcmp(adrs_ptr, _buffer->list[i].addr, BLE_GAP_ADDR_LEN) == 0) {
//			LOGd("found");
			uint16_t occ = _buffer->list[i].occurrences;
			int8_t oldRssi = _buffer->list[i].rssi ;
			_buffer->list[i].rssi = (oldRssi * occ + rssi) / (occ + 1);
			_buffer->list[i].occurrences++;
//			found = true;
			//! TODO: Any reason not to break here?
			return;
		}
	}
//	if (!found) {
		int8_t minRssi = INT8_MAX;
		int8_t idx  = -1;
		if (getSize() >= SR_MAX_NR_DEVICES) {
			//! history full, throw out item with lowest rssi
			for (int i = 0; i < SR_MAX_NR_DEVICES; ++i) {
				if (_buffer->list[i].rssi < minRssi && _buffer->list[i].rssi < rssi) {
					minRssi = _buffer->list[i].rssi;
					idx = i;
				}
			}
//			LOGd("replacing item at idx: %d", idx);
		}
		else {
			idx = _buffer->size++;
		}

		//! If a spot has been found, write it to that position
		if (idx >= 0) {
			LOGi("idx: %d, minRssi: %d", idx, minRssi);
			LOGi("NEW Advertisement from: [%02X %02X %02X %02X %02X %02X], rssi: %d", adrs_ptr[5],
					adrs_ptr[4], adrs_ptr[3], adrs_ptr[2], adrs_ptr[1],
					adrs_ptr[0], rssi);
			memcpy(_buffer->list[idx].addr, adrs_ptr, BLE_GAP_ADDR_LEN);
			_buffer->list[idx].occurrences = 1;
			_buffer->list[idx].rssi = rssi;
		}
//	}
//	else {
////		LOGd("Advertisement from: %s, rssi: %d, occ: %d", addrs, rssi, occ);
//	}

//	BLEutil::printArray<uint8_t>((uint8_t*)_buffer, SR_MAX_NR_DEVICES * SR_SERIALIZED_DEVICE_SIZE);
}

void ScanResult::print() const {
	for (int i = 0; i < getSize(); ++i) {
//		char addrs[28];
//		sprintf(addrs, "[%02X %02X %02X %02X %02X %02X]", _buffer->list[i].addr[5],
//				_buffer->list[i].addr[4], _buffer->list[i].addr[3], _buffer->list[i].addr[2], _buffer->list[i].addr[1],
//				_buffer->list[i].addr[0]);
		LOGi("[%02X %02X %02X %02X %02X %02X]\trssi: %d\tocc: %d", _buffer->list[i].addr[5],
				_buffer->list[i].addr[4], _buffer->list[i].addr[3], _buffer->list[i].addr[2], _buffer->list[i].addr[1],
				_buffer->list[i].addr[0], _buffer->list[i].rssi, _buffer->list[i].occurrences);
	}

}