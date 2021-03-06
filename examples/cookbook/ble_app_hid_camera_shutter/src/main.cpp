/*
===============================================================================
 Name        : ble_app_hid_camera_shutter
 Author      : uCXpresso
 Version     : v1.0.1
 Description : BLE HID keyboard Test
===============================================================================
 	 	 	 	 	 	 	 	 History
 ---------+---------+--------------------------------------------+-------------
 DATE     |	VERSION |	DESCRIPTIONS							 |	By
 ---------+---------+--------------------------------------------+-------------
 2014/12/1 v1.0.0	First Edition.									Jason
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
//#include <class/ble/ble_device_manager.h>
#include <class/ble/ble_device.h>
#include <class/ble/ble_service.h>
#include <class/ble/ble_conn_params.h>
#include <class/ble/ble_device_manager.h>
#include <class/ble/ble_service_bat.h>
#include <class/ble/ble_service_hid.h>
#include <class/adc.h>
#include <class/power.h>
#include <class/button.h>
#include <class/thread.h>

// TODO: insert other definitions and declarations here
#define DEVICE_NAME                      "nanoShutter"			/**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                "uCXpresso.NRF"		/**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                 2000					/**< The advertising interval (in ms). */

//
// Board LED define
//
//#define BOARD_PCA10001
#define BOARD_LILYPAD
#include <config/board.h>

//
// key input task
//
void keyTask(CThread *p_thread, xHandle p_params) {
	CButton btn0(BUTTON_PIN_0);	// set BUTTON_PIN_0 as input pin with weakup sense.
	CPin led(LED_PIN_1);
	led.output();

	while( p_thread->isAlive() ) {

		//
		// wait for BLE connected.
		// The wait member will return the true immediately when in connected status.
		// The task will be blocked when BLE in disconnected status.
		//
		gpBLE->wait();

    	//
    	// HID KB test
    	//
    	if ( gpKB->isAvailable() ) {

    		//
    		// button 0 (Remote Camera Shutter)
    		//
    		switch( btn0.isPressed() ) {
    		case BTN_PRESSED:
    			gpKB->send(MM_KEY_VOL_UP);	// Camera shutter can be defined to Volume Up or Volume Down by Smart-Phone
    			led = LED_ON;
    			break;

    		case BTN_RELEASED:
    			gpKB->send(MM_KEY_RELEASE);	// release the pressed key
    			led = LED_OFF;
     			break;

    		case BTN_NOTHING:
    			break;
    		}
    		sleep(100);
    	}
	}
}

//
// Main Routine
//
int main(void) {
#ifdef DEBUG
	CSerial ser;		// declare a UART object
	ser.enable();
	CDebug dbg(ser);	// Debug stream use the UART object
	dbg.start();
#endif
	//
	// SoftDevice
	//
	bleDevice ble;
	ble.enable("Shutter", "1234");	// enable BLE SoftDevice task

	// Pin to clear all center list
	CPin btn(17);
	btn.input();

	// Device Manager for bond device.
	bleDeviceManager man(ble, (btn==LOW ? true : false));
	man.connect_directed_mode();	// enable "connect directed" mode to fast connected. (BT 4.1 spec.)

	// GAP
	ble.m_gap.settings( DEVICE_NAME,
			 	 	 	20,		// min. connection interval in millisecond
						100,	// max. connection interval in millisecond
						12);	// slave latency (save power)

	ble.m_gap.tx_power(BLE_TX_0dBm);

	//
	// HID Keyboard Service
	//
	bleServiceKB kbs(ble);

	//
	// Battery Level Service
	//
	bleServiceBattery bat(ble);

	//
	// Optional: Add "Connection Parameters Update" negotiation.
	//
	bleConnParams conn(ble);

	//
	// BLE Advertising
	//
	ble.m_advertising.add_uuid_to_complete_list(bat);				// add bat object to the uuid list of advertising
	ble.m_advertising.add_uuid_to_complete_list(kbs);				// add kbs object to the uuid list of advertising

	// Optional: add appearance to indicate who you are in advertisement.
	ble.m_advertising.appearance(BLE_APPEARANCE_HID_KEYBOARD);
	ble.m_advertising.update();										// update advertisement data

	// Start advertising
	ble.m_advertising.interval(APP_ADV_INTERVAL);					// set advertising interval
	ble.m_advertising.timeout(30);									// Enter system power off mode when adv. timeout. (disable system off if timeout=0)
	ble.m_advertising.start();

	//
	// Optional: Enable tickless technology
	//
#ifndef DEBUG
	CPowerSave::tickless(true);
#endif

	//
	// Key Test Task
	//
	CThread t(keyTask);
	t.start("kb", 76);		// start the keyTask

	//
	// Analog
	//
	CAdc::init();
	CAdc::source(VDD_1_3);	// to detect the VDD voltage
	CAdc::enable();

	//
	// LED
	//
	CPin led(LED_PIN_0);
	led.output();

	CTimeout tmBAT, tmLED;
	uint16_t value;
	float 	 percentage;

	//
    // Enter main loop.
	//
    while(1) {

    	//
    	// BLE Battery Service
    	//
    	if ( bat.isAvailable() ) {
    		if ( tmBAT.isExpired(1000) ) {
    			tmBAT.reset();
    			if ( CAdc::read(value) ) {
    				percentage = (value / 1024.0f) * 100;
    				bat.send(percentage);
    			}
    		}
    	}

    	//
    	// blink led
    	//
    	led = LED_ON;
    	sleep(10);
    	led = LED_OFF;
    	if ( ble.isConnected() ) {
    		sleep(290);
    	} else {
    		sleep(1990);
    	}

    	// Negotiate the "connection parameters update" in main-loop
    	conn.negotiate();
    }
}
