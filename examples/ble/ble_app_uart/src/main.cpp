/*
===============================================================================
 Name        : main.c
 Author      : uCXpresso
 Version     : v1.0.0
 Copyright   : www.ucxpresso.net
 License	 : MIT
 Description : A BLE UAET to Debug Console (Serial I/F) example
===============================================================================
 	 	 	 	 	 	 	 	 History
 ---------+---------+--------------------------------------------+-------------
 DATE     |	VERSION |	DESCRIPTIONS							 |	By
 ---------+---------+--------------------------------------------+-------------
 2014/10/14 v1.0.0	First Edition.									LEO
 ===============================================================================
 */

#include <uCXpresso.h>

#ifdef DEBUG
#include <debug.h>
#include <class/serial.h>
#define DBG dbg_printf
#else
#define DBG(...)
#endif

// TODO: insert other include files here
#include <class/ble/ble_device.h>
#include <class/ble/ble_service.h>
#include <class/pin.h>

#include <class/ble/ble_service_uart.h>
#include <class/timeout.h>

// TODO: insert other definitions and declarations here
#define DEVICE_NAME                          "nano51822"            /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                    "uCXpresso.NRF"        /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                     500                    /**< The advertising interval (in ms). */
#define APP_COMPANY_IDENTIFIER           	 0x004C                 /**< Company identifier for Apple Inc. as per www.bluetooth.org. */

//
// Main Routine
//
int main(void) {
#ifdef DEBUG
	CSerial ser;		// declare a Serial object
	ser.enable();
	CDebug dbg(ser);	// Debug stream use the UART object
	dbg.start();
#endif

	//
	// SoftDevice
	//
//	bleDevice ble(NRF_LFCLKSRC_XTAL_20_PPM);	/**< use external 32.768Khz crystal for SoftDevice */
	bleDevice ble;	// use internal 32.768KHz for SoftDevice
	ble.enable();	// enable BLE SoftDevice stack

	// GAP
	ble.m_gap.settings(DEVICE_NAME);	// set Device Name on GAP
	ble.m_gap.tx_power(BLE_TX_0dBm);

	//
	// Add BLE Service
	//
	bleServiceUART nus(ble);			// declare a BLE "Nordic UART Service" object

	//
	// BLE Advertising
	//
	ble.m_advertising.interval(APP_ADV_INTERVAL);					// set advertising interval
	ble.m_advertising.commpany_identifier(APP_COMPANY_IDENTIFIER);	// add company identifier

	ble.m_advertising.add_uuid_to_complete_list(nus);				// add nus object to the uuid list of advertising

	ble.m_advertising.update();										// update advertising data

	// Start advertising
	ble.m_advertising.start();

	//
	// Your Application setup code here
	//

	CPin led0(18);	// led0 on P0.18
	CPin led1(19);	// led1 on P0.19

	led0.output();	// set led0 as an output pin
	led1.output();	// set led1 as an output pin

	CTimeout	tm;
	uint8_t		ch;

	//
    // Enter main loop.
	//
    while(1) {


    	if ( nus.isAvailable() ) {	// check BLE NUS service
    		led1= LED_ON;

    		if ( nus.readable() ) {
    			ch = nus.read();
    			dbg.putc(ch);
    		}

    		if ( dbg.available() ) {
    			ch = dbg.read();
    			nus.write(ch);		// echo to BLE UART service
    		}

    	} else {
    		led1 = LED_OFF;
    	}

    	//
    	// blink led
    	//
    	if ( tm.isExpired(500) ) {
    		tm.reset();
    		led0 = !led0;	// blink led
    	}
    }
}
