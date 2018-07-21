/**************************************************************************/
/*!

By AirTracker Project team


@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/
#ifndef DUST_H
#define DUST_H
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif


//
#define SAMPLING_TIME 280
#define DELTA_TIME 40
#define SLEEP_TIME 9680
#define VCC 5.0
#define DELTA_VO  0.03

typedef enum states {ENABLE, DISABLE} state_t;

class DUST {
 
 private:
  uint8_t _pinVOUT;
  uint8_t _pinLED;
  state_t _state;
  uint16_t _voutLevel;
  float _vout;
   
 public:
  void init();
  DUST(uint8_t pinVOUT, uint8_t pinLED);
  void run();
  void test();
  uint16_t getLevel();
  float getVOUT();
  bool isEnable();
  void enable();
  void disable();
  float getDust_mg();
  float getDust_ug();
  float getPPM();
};
#endif


