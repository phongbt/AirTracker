/*
	S8 - CO2 sensor library
	Written by AirTracker team
	July 2018
*/

#ifndef _S8_H_
#define	_S8_H_

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include <SoftwareSerial.h>

class S8 {
private:
  uint8_t pinRX = 3;
  uint8_t pinTX = 2;
  unsigned int ppmCO2;
  int valMultiplier = 1;
    
  int sendRequest(byte packet[],int);
  int getResponse(byte packet[], int);
  unsigned int getValue(byte packet[]);
public:
  void init();
  unsigned int readCO2();
  int background_calibrate();
};
#endif
