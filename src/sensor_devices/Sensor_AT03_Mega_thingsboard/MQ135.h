/**************************************************************************/
/*!
@file     MQ135.h
@author   G.Krocker (Mad Frog Labs)
@license  GNU GPLv3

Modified by AirTracker Project team

First version of an Arduino Library for the MQ135 gas sensor
TODO: Review the correction factor calculation. This currently relies on
the datasheet but the information there seems to be wrong.

@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/
#ifndef MQ135_H
#define MQ135_H
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

/// The load resistance on the board
#define RLOAD 10000
/// Calibration resistance at atmospheric CO2 level
#define RZERO_CO2 800000
#define RZERO_NH3 50000



/// Parameters for calculating ppm of CO2 from sensor resistance
#define PARA 106.3566243    // 116.6020682
#define PARB 2.794635235   //2.769034857

#define PARA_NH3 103.1686777    
#define PARB_NH3 2.50604601   

#define VRL_MAX 0.92340367
#define RS_MIN 50000

/// Parameters to model temperature and humidity dependence
#define CORA 0.00035
#define CORB 0.02718
#define CORC 1.39538
#define CORD 0.0018

/// Atmospheric CO2 level for calibration purposes
#define ATMOCO2  408.35
#define ATMOCO2_INDOOR  500
#define VCC 5.0
#define ADC_MIN 0
#define  ADC_MAX 1023
#define HEAT_TIME 30000



class MQ135 {
 typedef enum states {INACTIVE=0, ACTIVE, READY} state_t; 
 private:
  uint8_t _pinVRL;
  uint8_t _pinDout;
  state_t _state;
  uint64_t _prevTime;
  
 public:
  void init();
  MQ135(uint8_t pinVRL, uint8_t pinDout);
  float getCorrectionFactor(float t, float h);
  int getDOut();
  float getResistance();
  float getVRL();
  float getCorrectedResistance(float t, float h);
  int getPPM_CO2();
  int getPPM_NH3();
  float getCorrectedPPM(float t, float h);
  float getRZeroIndoor();
  float getRZero();
  float getCorrectedRZero(float t, float h);
};
#endif


