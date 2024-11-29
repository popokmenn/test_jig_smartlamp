/* 
	LTR308-ALS library example sketch


This sketch shows how to use the LTR308 library to read the Lite-On LTR-308ALS light sensor.

Hardware connections:

3V3 to 3.3V
GND to GND

(WARNING: do not connect 3V3 to 5V or the sensor will be damaged!)

You will also need to connect the I2C pins (SCL and SDA) to your Arduino.
The pins are different on different Arduinos:

                    SDA    SCL
Any Arduino        "SDA"  "SCL"
Uno, Pro            A4     A5
Mega2560, Due       20     21
Leonardo            2      3
ESP8266		    5      4
ESP32               21     22
You can connect the INT (interrupt) pin  but it is not required for basic operation.
*/

// Your sketch must #include this library, and the Wire library
// (Wire is a standard library included with Arduino):

#include <LTR308.h>
#include <Wire.h>

// Create an LTR308 object, here called "light":

LTR308 light;

// Global variables:
   
// If gain = 0, device is set to 1X gain
// If gain = 1, device is set to 3X gain (default)
// If gain = 2, device is set to 6X gain
// If gain = 3, device is set to 9X gain
// If gain = 4, device is set to 18X gain
unsigned char gain = 0;     // Gain setting, values = 0-4 

//------------------------------------------------------
// If integrationTime = 0, integrationTime will be 400ms / 20 bits result
// If integrationTime = 1, integrationTime will be 200ms / 19 bits result 
// If integrationTime = 2, integrationTime will be 100ms / 18 bits result (default)
// If integrationTime = 3, integrationTime will be 50ms / 17 bits result 
// If integrationTime = 4, integrationTime will be 25ms / 16 bits result 
unsigned char integrationTime = 0;  // Integration ("shutter") time, values 0 - 4

//------------------------------------------------------
// If measurementRate = 0, measurementRate will be 25ms
// If measurementRate = 1, measurementRate will be 50ms
// If measurementRate = 2, measurementRate will be 100ms (default)
// If measurementRate = 3, measurementRate will be 500ms
// If measurementRate = 5, measurementRate will be 1000ms
// If measurementRate = 6, measurementRate will be 2000ms
// If measurementRate = 7, measurementRate will be 2000ms
unsigned char measurementRate = 3;  // Interval between DATA_REGISTERS update, values 0 - 7, except 4

//------------------------------------------------------
// Chip ID - should be 0xB1 for all LTR-308
unsigned char ID;

//------------------------------------------------------
// Main Control Register
unsigned char control;

void setup() {
  // Initialize the Serial port:
  
  Serial.begin(9600);
  Serial.println();
  Serial.println("LTR-308ALS example sketch");

  // Initialize the LTR308 library
  // 100ms 	initial startup time required
  delay(100);

  // You can pass nothing to light.begin() for the default I2C address (0x53)
  light.begin();

  // Get factory ID from sensor:
  // (Just for fun, you don't need to do this to operate the sensor)

  if (light.getPartID(ID)) {
    Serial.print("Got Sensor Part ID: 0X");
    Serial.print(ID, HEX);
    Serial.println();
  }
  // Most library commands will return true if communications was successful,
  // and false if there was a problem. You can ignore this returned value,
  // or check whether a command worked correctly and retrieve an error code:
  else {
    byte error = light.getError();
    printError(error);
  }
  
  // To start taking measurements, power up the sensor
  
  if (light.setPowerUp()) {
    Serial.print("Powering up...");
    Serial.println();
  }
  else {
    byte error = light.getError();
    printError(error);
  }

  // Allow for a slight delay in power-up sequence (typ. 5ms from the datasheet)
  delay(10);

  if (light.getPower(control)) {
    Serial.print("Control byte is: 0X");
    Serial.print(control, HEX);
    Serial.println();
  }
  else {
    byte error = light.getError();
    printError(error);
  }
  // The light sensor has a default integration time of 100ms,
  // and a default gain of low (3X).
  
  // If you would like to change either of these, you can
  // do so using the setGain() and setMeasurementRate() command.
  
  Serial.println("Setting Gain...");
  
  if (light.setGain(gain)) {
    light.getGain(gain);
    
    Serial.print("Gain Set to 0X");
    Serial.print(gain, HEX);
    Serial.println();
  }
  else {
    byte error = light.getError();
    printError(error);
  }

  Serial.println("Set timing...");
  if (light.setMeasurementRate(integrationTime, measurementRate)) {
    light.getMeasurementRate(integrationTime, measurementRate);
    
    Serial.print("Timing Set to ");
    Serial.print(integrationTime, HEX);
    Serial.println();

    Serial.print("Meas Rate Set to ");
    Serial.print(measurementRate, HEX);
    Serial.println();
  }
  else {
    byte error = light.getError();
    printError(error);
  }
 
}

void loop() {
  // Wait between measurements before retrieving the result
  // You can also configure the sensor to issue an interrupt 
  // when measurements are complete)
  
  // This sketch uses the LTR308's built-in integration timer.
  
  int ms = 1000;
  
  delay(ms);
  
  // Once integration is complete, we'll retrieve the data.
  
  // Retrieve the data from the device:

  unsigned long rawData;
  
  if (light.getData(rawData)) {
    
    Serial.print("Raw Data: ");
    Serial.println(rawData);
  
    // To calculate lux, pass all your settings and readings
    // to the getLux() function.
    
    // The getLux() function will return 1 if the calculation
    // was successful, or 0 if the sensor was
    // saturated (too much light). If this happens, you can
    // reduce the integration time and/or gain.
  
    double lux;    // Resulting lux value
    boolean good;  // True if sensor is not saturated
    
    // Perform lux calculation:

    good = light.getLux(gain, integrationTime, rawData, lux);
    
    // Print out the results:
	
    Serial.print("Lux: ");
    Serial.print(lux);
    if (good) Serial.println(" (valid data)"); 
    else Serial.println(" (BAD)");
  }
  else {
    byte error = light.getError();
    printError(error);
  }
}

void printError(byte error) {
  // If there's an I2C error, this function will
  // print out an explanation.

  Serial.print("I2C error: ");
  Serial.print(error,DEC);
  Serial.print(", ");
  
  switch(error) {
    case 0:
      Serial.println("success");
      break;
    case 1:
      Serial.println("data too long for transmit buffer");
      break;
    case 2:
      Serial.println("received NACK on address (disconnected?)");
      break;
    case 3:
      Serial.println("received NACK on data");
      break;
    case 4:
      Serial.println("other error");
      break;
    default:
      Serial.println("unknown error");
  }
}
