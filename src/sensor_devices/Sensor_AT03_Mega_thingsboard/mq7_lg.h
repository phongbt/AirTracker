
/*
  MQ7_LG.h - Library for MQ7 Mono-carbon dioxide Gas Sensor v1.3 hardware.
  Modified from code of Laurent Gibergues, January 21, 2015 
  Copyright  AirTracker Project
  Released into the public domain.
 
*/


#ifndef MQ7_LG_h
#define MQ7_LG_h
//#include <inttypes.h>

#define RL  10000              // Ohms   10KOhm
#define R0  2000               // Ohms    2KOhm
#define HEAT_TIME_MQ7 30000              
#define VCC_HEATER 5.1       // Volts
#define COEF_A -1.611975          // See MQ7 documentation :
#define CONSTANT_B 514.184887        // log(ppmCO) = COEF_A * log(Rs/R0) + log(CONSTANT_B)

class MQ7_LG {
public :
  MQ7_LG(uint8_t VRLPin, int8_t DOutPin = -1);
  void init();
  float getVRL();
  int getDOut();
  int getPPM_CO();
  float getPPM_CO_f();
  bool isEnable();
  void enable();
  void disable();
  typedef enum states {ENABLE=0, DISABLE} state_t;  
private :
  uint8_t _VRLPin;
  int8_t _DOutPin;

  bool _DOutIsConnected;
  state_t _state;
  unsigned long _prevTime;

  //functions
 
  // Specifics data
 
};

#endif
