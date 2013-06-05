// 
//    FILE: dht11.h
// VERSION: 0.3.2
// PURPOSE: DHT11 Temperature & Humidity Sensor library for Arduino
// LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)
//
// DATASHEET: http://www.micro4you.com/files/sensor/DHT11.pdf
//
//     URL: http://arduino.cc/playground/Main/DHT11Lib
//
// HISTORY:
// George Hadjikyriacou - Original version
// see dht.cpp file
// *** Terry King: Changed include Arduino.h for 1.0x  
// include core Wiring API and now Arduino
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
  #else
  #include "WProgram.h"
  #endif

#ifndef dht11_h
#define dht11_h

#define DHT11LIB_VERSION "0.3.2"

class dht11
{
public:
    int read(int pin);
	int humidity;
	int temperature;
};
#endif
//
// END OF FILE
//