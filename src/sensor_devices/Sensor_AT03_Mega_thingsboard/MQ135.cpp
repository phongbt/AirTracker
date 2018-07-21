/**************************************************************************/
/*!
@file     MQ135.cpp
@author   G.Krocker (Mad Frog Labs)
@license  GNU GPLv3

First version of an Arduino Library for the MQ135 gas sensor
TODO: Review the correction factor calculation. This currently relies on
the datasheet but the information there seems to be wrong.

@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/

#include "MQ135.h"

/**************************************************************************/
/*!
@brief  Default constructor

@param[in] pin  The analog input pin for the readout of the sensor
*/
/**************************************************************************/

MQ135::MQ135(uint8_t pinVRL, uint8_t pinDout) {
  _pinVRL = pinVRL;
  _pinDout = pinDout;
  _state = READY;
  _prevTime = millis();
}

void MQ135::init()
{  
    if (_state == ACTIVE) {
		unsigned long time = millis();
		if (time - _prevTime >= HEAT_TIME) {
		_state = READY;
		}
	}
}

int MQ135::getDOut() {
  if (_pinDout!=-1)
    return digitalRead(_pinDout);
  else
    return -1;
}

/**************************************************************************/
/*!
@brief  Get the correction factor to correct for temperature and humidity

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The calculated correction factor
*/
/**************************************************************************/
float MQ135::getCorrectionFactor(float t, float h) {
  return CORA * t * t - CORB * t + CORC - (h-33.)*CORD;
}

/**************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the measurement value

@return The sensor resistance in kOhm
*/
/**************************************************************************/
float MQ135::getResistance() {
  if(_state!=READY) return -1.0;
  int val = analogRead(_pinVRL);
  return ((1023./(float)val)- 1.)*RLOAD;
}

float MQ135::getVRL() {
  
  if(_state!=READY) return -1.;
  int val = analogRead(_pinVRL);
  return  (float)val/1023. * VCC;  
}  

/**************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the measurement value corrected
        for temp/hum

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The corrected sensor resistance kOhm
*/
/**************************************************************************/
float MQ135::getCorrectedResistance(float t, float h) {
  return getResistance()/getCorrectionFactor(t, h);
}

/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air)

@return The ppm of CO2 in the air
*/
/**************************************************************************/
int MQ135::getPPM_CO2() {
  if(_state!=READY) return -1;
  float RS = getResistance();
  if(RS<RS_MIN)
    RS = RS_MIN;
  return (int)PARA * pow((RZERO_CO2/RS), PARB);
}

/*
@return The ppm of NH3 in the air
*/
/**************************************************************************/
int MQ135::getPPM_NH3() {
  if(_state!=READY) return -1;
  float RS = getResistance();
  return (int)PARA_NH3 * pow((RZERO_NH3/RS), PARB_NH3);
}

/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air), corrected
        for temp/hum

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The ppm of CO2 in the air
*/
/**************************************************************************/
float MQ135::getCorrectedPPM(float t, float h) {
  return PARA * pow((RZERO_CO2/getCorrectedResistance(t, h)), PARB);
}

/**************************************************************************/
/*!
@brief  Get the resistance RZero of the sensor for calibration purposes

@return The sensor resistance RZero in kOhm
*/
/**************************************************************************/
float MQ135::getRZero() {
  return getResistance() / pow((PARA/ATMOCO2), (1./PARB));
}

float MQ135::getRZeroIndoor() {
  return getResistance() / pow((PARA/ATMOCO2_INDOOR), (1./PARB));
}

/**************************************************************************/
/*!
@brief  Get the corrected resistance RZero of the sensor for calibration
        purposes

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The corrected sensor resistance RZero in kOhm
*/
/**************************************************************************/
float MQ135::getCorrectedRZero(float t, float h) {
  return getCorrectedResistance(t, h) * pow((PARA/ATMOCO2), (1./PARB));
}


