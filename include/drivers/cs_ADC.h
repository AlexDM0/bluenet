/*
 * Author: Bart van Vliet
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: 6 Nov., 2014
 * License: LGPLv3+, Apache License, or MIT, your choice
 */
#pragma once

//#include <stdint.h>
//
//#include "cs_RTC.h"
//#include "structs/cs_CurrentCurve.h"
#include "structs/cs_PowerCurve.h"
#include "structs/buffer/cs_CircularBuffer.h"

//#include "common/cs_Types.h"

/** Analog-Digital conversion.
 *
 * The ADC is a hardware peripheral that can be initialized for a particular pin. Subsequently, the analog signal
 * is converted into a digital value. The resolution of the conversion is limited (default 10 bits). The conversion
 * is used to get the current curve.
 */
class ADC {

public:
	//! Use static variant of singleton, no dynamic memory allocation
	static ADC& getInstance() {
		static ADC instance;
		return instance;
	}

	/** Initialize ADC.
	 * @param pin The pin to use as input (AIN<pin>).
	 *
	 * The init function must called once before operating the AD converter.
	 * Call it after you start the SoftDevice.
	 */
	uint32_t init(uint8_t pins[], uint8_t size);

	/** Start the ADC sampling
	 *
	 * Will add samples to the current curve if it's assigned.
	 */
	void start();

	/** Stop the ADC sampling
	 */
	void stop();

	//! Each tick we have time to dispatch events e.g.
	void tick();

//	void setCurrentCurve(CurrentCurve<uint16_t>* curve) { _currentCurve = curve; }
	void setPowerCurve(PowerCurve<uint16_t>* curve) { _powerCurve = curve; }

	void setBuffer(CircularBuffer<uint16_t>* buffer){ _buffer = buffer; }

	CircularBuffer<uint16_t>* getBuffer() { return _buffer; }

//	/** Set threshold to start writing samples to buffer.
//	 *
//	 * Default threshold is DEFAULT_RECORDING_THRESHOLD
//	 */
//	inline void setThreshold(uint8_t threshold) { _threshold = threshold; }

	//! Function to be called from interrupt, do not do much there!
	void update(uint32_t value);

	//! Function to set the input pin, this can be done after each sample
	uint32_t config(uint8_t pin);

private:
	/** Constructor
	 */
//	ADC(): _currentCurve(NULL) {}
	ADC(): _powerCurve(NULL), _buffer(NULL) {}

	//! This class is singleton, deny implementation
	ADC(ADC const&);
	//! This class is singleton, deny implementation
	void operator=(ADC const &);

	uint8_t _pins[MAX_ADC_PINS];
	uint8_t _numPins;
	uint8_t _lastPinNum;
	uint32_t _lastSampleTime;
	uint16_t _sampleNum;
	CircularBuffer<uint16_t>* _buffer;
//	uint16_t _lastResult;
//	uint8_t _threshold;
//	CurrentCurve<uint16_t>* _currentCurve;
	PowerCurve<uint16_t>* _powerCurve;
};
