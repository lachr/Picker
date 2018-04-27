#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include <RampStepper.h>

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

enum jobs {
  JOB_TEST,
  JOB_INIT,
  JOB_IDLE,
  JOB_PICKUP,
  JOB_DROP_L,
  JOB_DROP_R
};

int moduleJob = JOB_INIT;

// Initialise Variables -------------------------
const int stepsPerRevolution = 64;
const int steps = 2048;
RampStepper motor(stepsPerRevolution, 9, 11, 10, 12);

int photoDiode = 6;
int sensorRead = A1;
// END Initialize Variables ---------------------

void setup() {
  // while (!Serial); { // required for Flora & Micro
  //   delay(500);
  // }

  pinMode(photoDiode,OUTPUT);  
  digitalWrite(photoDiode,HIGH);   

  Serial.begin(9600);
  initBLE();
  ble.verbose(false);  // debug info is a little annoying after this point!
  /* Wait for connection 
  while (! ble.isConnected()) {
      delay(500);
  } */
}


void loop() {


  //rampMotor.setSpeed(850);
  if(moduleJob == JOB_TEST)
  { 
    Serial.println(analogRead(sensorRead));
    delay(100);
  }

  if(moduleJob == JOB_INIT)
  { 
    motor.setSpeed(150);
    motor.setRamp(false);
    while(!isInterrupted())
    {
      motor.step(2);
    }
    while(isInterrupted())
    {
      motor.step(2);
    }
    motor.resetPosition();
    delay(500);
    motor.setRamp(true);
    motor.toStep(1024, false);

    moduleJob = JOB_IDLE;
  }

  if(moduleJob == JOB_IDLE)
  { 
    listenBLE();
  }

  if(moduleJob == JOB_PICKUP)
  { 
    motor.setSpeed(150);
    motor.setRamp(true);
    motor.toStep(1563, true);
    delay(10);
    motor.toStep(0, true);
    delay(500);
    if(isInterrupted()){
      sendBLE("pickupSuccess(1)");
    } else {
      sendBLE("pickupSuccess(0)");
    }
    delay(500);
    moduleJob = JOB_IDLE;
  }

  if(moduleJob == JOB_DROP_L)
  { 
    motor.setSpeed(850);
    motor.setRamp(true, false);
    motor.toStep(1536, false);
    motor.setSpeed(300);
    motor.setRamp(false, true);
    motor.toStep(0, true);
    delay(500);
    if(isInterrupted()){
      sendBLE("dropSuccess(0)");
    } else {
      sendBLE("dropSuccess(1)");
    }
    delay(500);
    motor.toStep(1024, false);
    moduleJob = JOB_IDLE;
  }

  if(moduleJob == JOB_DROP_R)
  { 
    motor.setSpeed(850);
    motor.setRamp(true, false);
    motor.toStep(512, true);
    motor.setSpeed(300);
    motor.setRamp(false, true);
    motor.toStep(0, false);
    delay(500);
    if(isInterrupted()){
      sendBLE("dropSuccess(0)");
    } else {
      sendBLE("dropSuccess(1)");
    }
    delay(500);
    motor.toStep(1024, true);
    moduleJob = JOB_IDLE;
  }

  delay(10);
}



bool isInterrupted() {
  return (analogRead(sensorRead) >= 500);
}



// Handle API commands
void handleApiCommands(String command) {
  Serial.println(command);
  if (command == "init();") {
    moduleJob = JOB_INIT;
  }
  if (command == "pickup();") {
    moduleJob = JOB_PICKUP;
  }
  if (command == "drop(1);") {
    moduleJob = JOB_DROP_R;
  }
  if (command == "drop(0);") {
    moduleJob = JOB_DROP_L;
  }
}

// Listen to incomminc commands from Bluetooth
void listenBLE() {
  ble.println("AT+BLEUARTRX");
  ble.readline();
  if (strcmp(ble.buffer, "OK") == 0) {
    // no data
    return;
  }
  handleApiCommands(ble.buffer);
}

// Send message over Bluetooth
void sendBLE(String msg) {
  ble.print("AT+BLEUARTTX=");
  ble.println(msg);

  // check response stastus
  if (! ble.waitForOK() ) {
    Serial.println(F("Failed to send?"));
  }
}

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

void initBLE() {
  Serial.println(F("Adafruit Bluefruit Command Mode Example"));
  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();
  ble.println("AT+GAPDEVNAME=ferrari");

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println(F("Then Enter characters to send to Bluefruit"));
  Serial.println();
}