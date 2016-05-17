/**
 * Author: Bart van Vliet
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: 4 Nov., 2014
 * License: LGPLv3+, Apache, and/or MIT, your choice
 */

#include "drivers/cs_ADC.h"

//#include "nrf.h"
//
//
//#if(NRF51_USE_SOFTDEVICE == 1)
//#include "nrf_sdm.h"
//#endif
//
#include "cfg/cs_Boards.h"
#include "drivers/cs_Serial.h"
#include "util/cs_BleError.h"
//
//#include "cs_nRF51822.h"

#include "drivers/cs_RTC.h"

//! Check the section 31 "Analog to Digital Converter (ADC)" in the nRF51 Series Reference Manual.
uint32_t ADC::init(uint8_t pins[], uint8_t size) {
	uint32_t err_code;
	assert(size <= MAX_ADC_PINS && size > 0, "Too many or few pins");
	for (uint8_t i=0; i<size; i++) {
		_pins[i] = pins[i];
	}
	_numPins = size;

	CircularBuffer<uint16_t>* buffer = new CircularBuffer<uint16_t>(400, true);
	ADC::getInstance().setBuffer(buffer);

	LOGd("Configure ADC on pin %u", _pins[0]);
	err_code = config(_pins[0]);
	APP_ERROR_CHECK(err_code);

	NRF_ADC->EVENTS_END  = 0;    //! Stop any running conversions.
	NRF_ADC->ENABLE     = ADC_ENABLE_ENABLE_Enabled; //! Pin will be configured as analog input

	NRF_ADC->INTENSET   = ADC_INTENSET_END_Msk; //! Interrupt adc

	//! Enable ADC interrupt
#if(NRF51_USE_SOFTDEVICE == 1)
	err_code = sd_nvic_ClearPendingIRQ(ADC_IRQn);
	APP_ERROR_CHECK(err_code);
	err_code = sd_nvic_SetPriority(ADC_IRQn, NRF_APP_PRIORITY_LOW);
	APP_ERROR_CHECK(err_code);
	err_code = sd_nvic_EnableIRQ(ADC_IRQn);
	APP_ERROR_CHECK(err_code);
#else
	NVIC_ClearPendingIRQ(ADC_IRQn);
	NVIC_SetPriority(ADC_IRQn, NRF_APP_PRIORITY_LOW);
	NVIC_EnableIRQ(ADC_IRQn);
#endif

	assert(_buffer != NULL, "buffer not initialized");


	//! Configure timer
	NRF_TIMER1->TASKS_CLEAR = 1;
	NRF_TIMER1->BITMODE =    (TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos); //! Counter is 16bit
	NRF_TIMER1->MODE =       (TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos);
	NRF_TIMER1->PRESCALER =  (4 << TIMER_PRESCALER_PRESCALER_Pos); //! 16MHz / 2^4 = 1Mhz, 1us period

//	NRF_TIMER1->TASKS_CLEAR = 1;
//	NRF_TIMER1->BITMODE =    (TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos); //! Counter is bit
//	NRF_TIMER1->MODE =       (TIMER_MODE_MODE_Counter << TIMER_MODE_MODE_Pos);

	//! Configure timer events
	NRF_TIMER1->CC[0] = 10000; // 10ms compare value
//	NRF_TIMER1->CC[0] = 20000; // 20ms compare value
//	NRF_TIMER1->CC[0] = 160000; // 10ms compare value
//	NRF_TIMER1->CC[1] = 1;

	//! Shortcut clear timer at compare0 event
	NRF_TIMER1->SHORTS = (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos);

	//! Configure ADC task on PPI Channel 7
//	NRF_PPI->CH[7].EEP = (uint32_t)&NRF_TIMER1->EVENTS_COMPARE[0];
//	NRF_PPI->CH[7].TEP = (uint32_t)&NRF_ADC->TASKS_START;
	sd_ppi_channel_assign(7, &NRF_TIMER1->EVENTS_COMPARE[0], &NRF_ADC->TASKS_START);
	//	NRF_PPI->CHENSET = (1UL << 7);
	sd_ppi_channel_enable_set(1UL << 7);

//	sd_ppi_channel_assign(6, &NRF_TIMER1->EVENTS_COMPARE[1], &NRF_TIMER1->TASKS_COUNT);
//	sd_ppi_channel_enable_set(1UL << 6);

//	NVIC_SetPriority(PWM_IRQn, NRF_APP_PRIORITY_LOW);
//	NVIC_EnableIRQ(PWM_IRQn);
//	sd_nvic_SetPriority(TIMER1_IRQHandler, NRF_APP_PRIORITY_LOW);
//	sd_nvic_EnableIRQ(TIMER1_IRQHandler);

	_lastSampleTime = 0;
	_sampleNum = 0;
	return 0;
}

/** Configure the AD converter.
 *
 *   - set the resolution to 10 bits
 *   - set the prescaler for the input voltage (the input, not the input supply)
 *   - use the internal VGB reference (not the external one, so no need to use its multiplexer either)
 *   - do not set the prescaler for the reference voltage, this means voltage is expected between 0 and 1.2V (VGB)
 * The prescaler for input is set to 1/3. This means that the AIN input can be from 0 to 3.6V.
 */
uint32_t ADC::config(uint8_t pinNum) {
	NRF_ADC->CONFIG     =
			(ADC_CONFIG_RES_10bit                            << ADC_CONFIG_RES_Pos)     |
#if(HARDWARE_BOARD==CROWNSTONE)
			(ADC_CONFIG_INPSEL_AnalogInputNoPrescaling       << ADC_CONFIG_INPSEL_Pos)  |
#else
			(ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos)  |
#endif
			(ADC_CONFIG_REFSEL_VBG                           << ADC_CONFIG_REFSEL_Pos)  |
			(ADC_CONFIG_EXTREFSEL_None                       << ADC_CONFIG_EXTREFSEL_Pos);

	assert(_pins[pinNum] < 8, "no such pin");
	NRF_ADC->CONFIG |= ADC_CONFIG_PSEL_AnalogInput0 << (_pins[pinNum]+ADC_CONFIG_PSEL_Pos);
	_lastPinNum = pinNum;
	return 0;
}

void ADC::stop() {
	NRF_TIMER1->TASKS_STOP = 1;
	NRF_ADC->TASKS_STOP = 1;
}

void ADC::start() {
	NRF_ADC->EVENTS_END  = 0;
//	NRF_ADC->TASKS_START = 1;
	NRF_TIMER1->TASKS_START = 1;
}

void ADC::update(uint32_t value) {
//	//! Subsample
//	if (_currentCurve != NULL && (_sampleNum++%2)) {
////		if (_clock != NULL) {
//			_currentCurve->add(value, RTC::getCount());
////		} else {
////			_currentCurve->add(value);
////		}
//	}

//	_buffer->push(value);
//	_sampleNum++;
	uint32_t time = RTC::getCount();
//	if (_sampleNum >= 10000) {
//	if (RTC::ticksToMs(RTC::difference(time, _lastSampleTime)) >= 1000) {
	uint32_t diff = RTC::difference(time, _lastSampleTime);
//	NRF_TIMER1->TASKS_CAPTURE[1] = 1;
//	uint16_t timerVal = NRF_TIMER1->CC[1];
//	NRF_TIMER1->TASKS_CLEAR = 1;
//	if (diff >= RTC::msToTicks(10)) {
//		LOGi("1000ms = %u ticks", RTC::msToTicks(1000));
//		LOGe("ADC: %u %u", _sampleNum, value);
		_buffer->push(value);
//		_buffer->push(diff);
//		_buffer->push(_sampleNum++);
//		_buffer->push(timerVal);
		_lastSampleTime = time;
//		_sampleNum = 0;
//	}

	config((_lastPinNum+1) % _numPins);
	if (_lastPinNum > 0) {
		//! next sample
		NRF_ADC->TASKS_START = 1;
	}
}

void ADC::tick() {
//	dispatch();
}

/** The interrupt handler for an ADC data ready event.
 */
extern "C" void ADC_IRQHandler(void) {
	uint32_t adc_value;

	//! clear data-ready event
	NRF_ADC->EVENTS_END     = 0;

	//! write value to buffer
	adc_value = NRF_ADC->RESULT;

	ADC &adc = ADC::getInstance();
	adc.update(adc_value);

//	if (adc_result->full()) {
//		NRF_ADC->TASKS_STOP = 1;
//	       	return;
//	}

	//! Use the STOP task to save current. Workaround for PAN_028 rev1.5 anomaly 1.
	//NRF_ADC->TASKS_STOP = 1;

//	//! next sample
//	NRF_ADC->TASKS_START = 1;

}


