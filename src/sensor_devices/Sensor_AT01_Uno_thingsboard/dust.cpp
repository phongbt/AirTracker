/**************************************************************************/
/*!
@file     Dust.cpp
@author   Air Tracker Team
@license  GNU GPLv3

@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/

#include "dust.h"

DUST::DUST(uint8_t pinVOUT, uint8_t pinLED)
{
	_pinVOUT = pinVOUT;
	_pinLED = pinLED;
	_state = DISABLE;
}

void DUST::init()
{
	_state = ENABLE;
	pinMode(_pinLED,OUTPUT);
	_voutLevel = 0;
  _vout = 0.0;
}

void DUST::run()
{
   digitalWrite(_pinLED,LOW); // power on the LED
   delayMicroseconds(SAMPLING_TIME);
   _voutLevel = analogRead(_pinVOUT); // read the dust value
   delayMicroseconds(DELTA_TIME);
   digitalWrite(_pinLED,HIGH); // turn the LED off
   delayMicroseconds(SLEEP_TIME);
   _vout =  _voutLevel* VCC / 1024.;
   Serial.println(_voutLevel);
   Serial.println(_vout - DELTA_VO);
}

void DUST::test()
{
  // Test LED PIN
  digitalWrite(_pinLED,LOW); // power on the LED
  delay(500);
  Serial.println(digitalRead(12));
  delay(5000);
  digitalWrite(_pinLED,HIGH); // power on the LED
  delay(500);
  Serial.println(digitalRead(12));
  delay(5000);
  digitalWrite(_pinLED,LOW); // power on the LED
  delay(500);
  Serial.println(digitalRead(12));
  delay(5000);
  digitalWrite(_pinLED,HIGH); // power on the LED
  delay(500);
  Serial.println(digitalRead(12));
}

bool DUST::isEnable()
{
   return _state==ENABLE;
}

void DUST::enable()
{
  _state = ENABLE;
}

void DUST::disable()
{
  _state = DISABLE;
}

uint16_t DUST::getLevel()
{
	return _voutLevel;
}
float DUST::getVOUT()
{
	return _vout - DELTA_VO ;
}

float DUST::getDust_mg()
{
	float mg = 0.170 * (_vout - DELTA_VO) - 0.1;
	if(mg < 0.) mg = 0.;
	return mg;
}

float DUST::getDust_ug()
{
	float ug = (0.170 * (_vout - DELTA_VO) - 0.1) * 1000;
	if(ug < 0.) ug = 0.;
	return ug;
}
float DUST::getPPM()
{
	float ppm;
	ppm = (_vout - DELTA_VO - 0.0256) * 120000;
  if(ppm<0) ppm = 0;
  return ppm;
}


