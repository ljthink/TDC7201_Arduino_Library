#include <TDC7201.h>
#include <Arduino.h>
#include <inttypes.h>

// Connections
//  TDC7200         AdaFruit Feather M0 pin #
//  OSC Enable      0
//  TDC7201 Enable  1
//  INT1            5
//  TRIG1           6
//  DTG_TRIG        9 -- Combined trigger signal?
//  MSP_START       10 -- Combined mode start signal?
//  TRIG2           11
//  CSB2            12
//  CSB1            14
//  INT2            21

#include <TDC7201.h>

constexpr uint8_t PIN_EVM_OSC_ENABLE{0};      // EVM Oscillator enable
constexpr uint8_t PIN_TDC7201_ENABLE{1};      // TDC7201 device enable
constexpr uint8_t PIN_TDC7201_TIMER1_INT{5};    // TDC7201 clock 1 interrupt
constexpr uint8_t PIN_TDC7201_TIMER1_TRIG{6};   // TDC7201 clock 1 trigger
constexpr uint8_t PIN_EVM_DTG_TRIG{9};        // EVM Combined timer trigger
constexpr uint8_t PIN_EVM_MSP_START{10};      // EVM Combined mode start
constexpr uint8_t PIN_TDC7201_TIMER2_TRIG{11};    // TDC7201 clock 2 trigger
constexpr uint8_t PIN_TDC7201_CSB2{12};       // TDC7201 Chip select 2
constexpr uint8_t PIN_TDC7201_CSB1{14};       // TDC7201 Chip select 1
constexpr uint8_t PIN_TDC7201_TIMER2_INT{21};   // TDC7201 clock 1 interrupt

constexpr uint8_t PIN_M0_LED{13}; // MCU built in LED


static  TDC7201 TDC7201(PIN_TDC7201_ENABLE, PIN_TDC7201_CSB1, PIN_TDC7201_CSB2, PIN_TDC7201_TIMER1_TRIG,
              PIN_TDC7201_TIMER2_TRIG, PIN_TDC7201_TIMER1_INT, PIN_TDC7201_TIMER2_INT, 8000000);
              
void printRegisters(int timer=PIN_TDC7201_CSB1, int startReg=0, int stopReg=23) {
  
  for(int reg{startReg}; reg < stopReg+1; reg++){
    if(reg < 10){
      Serial.print("Register "); Serial.print(reg);Serial.print(": ");
      Serial.println(TDC7201.spiReadReg8(timer, reg), BIN);
      //delay(100);
    }
    else{ 
      Serial.print("Register "); Serial.print(reg);Serial.print(": ");
      Serial.println(TDC7201.spiReadReg24(timer, reg), BIN);
      //delay(100);
    }
  }
}

volatile boolean flag1;
volatile boolean flag2;

void timerTrig1ISR()
{
  flag1 = true;
}

void timerTrig2ISR()
{
  flag2 = true;
}

void setup() {
  int calibrationPeriods{40};
  int averageCycles{16};
  int numberOfStops{2};
  int measurementMode{1};
  uint64_t stopMaskPs{300};
  delay(5000);
  Serial.begin(9600);
 
  if(TDC7201.begin())
    Serial.println("TDC7201 Initialization Success");
  else
    Serial.println("TDC7201 Initialization Fail");
  
  if(TDC7201.setupMeasurement(PIN_TDC7201_CSB1, calibrationPeriods, averageCycles, numberOfStops, measurementMode))
    Serial.println("TDC7201 Setup success");
  else
    Serial.println("TDC7201 Setup fail");
  
  TDC7201.setupStopMask(PIN_TDC7201_CSB1, stopMaskPs);
  printRegisters(PIN_TDC7201_CSB1, 0, 23);
  
  pinMode(PIN_TDC7201_TIMER1_TRIG, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_TDC7201_TIMER1_TRIG), timerTrig1ISR, RISING); 
  
  pinMode(PIN_TDC7201_TIMER1_INT, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_TDC7201_TIMER1_INT), timerTrig2ISR, RISING); 

  TDC7201.startMeasurement(PIN_TDC7201_CSB1);
  
  double normLSB{static_cast<int>(TDC7201.m_normLSB)};
  Serial.print("normLSB: "); Serial.println(normLSB, 0);
  //pinMode(PIN_M0_LED, OUTPUT);

  
}

void loop() {
 if (flag1){
  digitalWrite(PIN_M0_LED, flag1);
  delay(500);
  Serial.println("Timer 1 start triggered");
  flag1 = false;
  digitalWrite(PIN_M0_LED, flag1);
  //delay(1000);
  
 }
 if (flag2){
  Serial.println("Timer 1 stop triggered");
  flag2 = false;
  printRegisters(PIN_TDC7201_CSB1, 0, 23);
 }
 
}
