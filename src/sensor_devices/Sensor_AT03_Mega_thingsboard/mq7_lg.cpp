  
#include <Arduino.h>
#include "MQ7_LG.h"


MQ7_LG::MQ7_LG(uint8_t VRLPin, int8_t DOutPin)
    : _VRLPin(VRLPin), _DOutPin(DOutPin) {
	_VRLPin = VRLPin;
	_DOutPin = DOutPin;
	 if (_DOutPin != -1) {
		pinMode(_DOutPin, INPUT);
		_DOutIsConnected = true;
	}else
		_DOutIsConnected = false;
}

void MQ7_LG::init() {
  
   _state = ENABLE; 
  _prevTime = millis();
}

bool MQ7_LG::isEnable()
{
   return _state==ENABLE;
}

void MQ7_LG::enable()
{
  _state = ENABLE;
}

void MQ7_LG::disable()
{
  _state = ENABLE;
}

float MQ7_LG::getVRL() {
  float v; 
  v = analogRead(_VRLPin);
  v = map(v, 0, 1023, 0, (int)(VCC_HEATER * 1000)); // Map integer value to Voltage value
  return v / 1000.0; 
}
  
int MQ7_LG::getDOut() {
  if (_DOutIsConnected)
    return digitalRead(_DOutPin);
  else
    return -1;
}

float MQ7_LG::getPPM_CO_f() {
  if(_state==DISABLE) return -1.0;
  float v; 
  v = analogRead(_VRLPin);
  v = VCC_HEATER / 1023.0 * v;
  //v = map(v, 0, 1023, 0, (int)(VCC_HEATER * 1000)); // Map integer value to Voltage value
  //v = v/1000.0;
  return CONSTANT_B * pow((float)RL * (VCC_HEATER / v - 1.0)/(float)R0, COEF_A);
}

int MQ7_LG::getPPM_CO() {
  if(_state==DISABLE) return -1;
  
  return (int)CONSTANT_B * pow((float)RL * (VCC_HEATER / getVRL() - 1.0) /(float)R0 , COEF_A);    //(CONSTANT_B * pow((((float)RL / R0) * ((VCC_HEATER / getVRL()) - 1)), COEF_A));
}
