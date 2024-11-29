#include <WiFi.h>
#include <LTR308.h>
#include <defVar.h>
#include <Arduino.h>
#include <Wire.h>

#define SHARP_SCK 13  // Define the clock pin
#define SHARP_MOSI 14 // Define the data pin
#define SHARP_SS 15   // Define the chip select pin
#define BLACK 0

#define SHARP_WIDTH 400
#define SHARP_HEIGHT 240
uint16_t res1;
int yc0 = 50;
int x0 = 10;
int x1 = 220;
int x2 = 240;
int margin_y = 20;
int delaymb = 500;

void setupDAC();
void setupTempSensor();
void setupLightSensor();
void setupEnergySensor();
void printError(byte error);
void frameDisplay(String sensorName, int x);
void writeLCD();

String dacReadPassed = "Initiating";
String dacWritePassed = "Initiating";
String tempPassed = "Initiating";
String energyPassed = "Initiating";
String lightPassed = "Initiating";

void setup()
{
  Serial.begin(115200);
  SerialAT.begin(BAUDRATE, SERIAL_8N1, AT_RX_PIN, AT_TX_PIN);
  delay(10);

  DEBUGPRINT("Firmware Version :");
  DEBUGPRINTLN(firmware_version);
  DEBUGPRINTLN("Start");
  DEBUGPRINTLN("");
  DEBUGPRINTLN("");
  pinMode(EM_LED_PIN, INPUT);
  pinMode(EM_CF_PIN, INPUT_PULLDOWN);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PWRKEY_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(PWRKEY_PIN, LOW);
  digitalWrite(RESET_PIN, LOW);

  // initialize DHT
  pinMode(10, INPUT_PULLUP);
  delay(2000);

  // Set DAC Pins
  Wire.setPins(13, 14);
  // Set Light Sensor Pins
  Wire1.setPins(21, 22);

  pinMode(17, INPUT);
  pinMode(21, INPUT);
  pinMode(BACKLIGHT, OUTPUT);
  digitalWrite(BACKLIGHT, HIGH);

  display.begin();
  display.setRotation(1);
  display.fillScreen(BACKGROUND_COLOR);
  display.setCursor(40, 50);
  delay(2000);
}

void loop()
{
  setupDAC();
  setupTempSensor();
  writeLCD();
  setupLightSensor();
  setupEnergySensor();
}

void setupDAC()
{
  DEBUGPRINTLN("");
  DEBUGPRINTLN("============ DAC READ =============");
  bool setupPassed = true;
  bool dacWrite = false;

  if (dacZeroTen.begin() != 0)
  {
    DEBUGPRINTLN("DAC setup failed.");
    setupPassed = false;
    dacReadPassed = "NOT PASSED";
    dacWritePassed = "NOT PASSED";
    delay(1000);
  }
  else
  {
    DEBUGPRINTLN("DAC setup passed.");
    dacReadPassed = "PASSED";
  }
  if (setupPassed)
  {
    DEBUGPRINTLN("");
    DEBUGPRINTLN("============ DAC WRITE =============");
    qc_dimming = 1;
    dacZeroTen.setDACOutRange(dacZeroTen.eOutputRange10V);
    digitalWrite(RELAY_PIN, HIGH);
    dacZeroTen.setDACOutVoltage(5000, 1);
    dacWritePassed = "PASSED";
    DEBUGPRINTLN("DAC write passed.");
    dacWrite = true;
  }
}

void setupTempSensor()
{
  DEBUGPRINTLN("");
  DEBUGPRINTLN("============ Temperature Sensor =============");
  dht.begin();

  delay(3000);
  temp = dht.readTemperature();

  bool tempPassed = !isnan(temp);
  if (tempPassed)
  {
    DEBUGPRINTLN("Temperature sensor setup passed.");
    tempPassed = "PASSED";
  }
  else
  {
    DEBUGPRINTLN("Temperature sensor setup failed.");
    tempPassed = "NOT PASSED";
  }
}

void setupEnergySensor()
{
  DEBUGPRINTLN("");
  DEBUGPRINTLN("============ Energy Sensor =============");
  DEBUGPRINTLN(em_bl0940.getState());
  delay(1000);

  // Uncomment the following lines to read and print energy values
  em_bl0940.readValues();
  voltage = em_bl0940.getVoltage();
  current = em_bl0940.getCurrent();
  activePower = em_bl0940.getActivePower();
  em_bl0940.readValuesPhAngle();
  apparentPower = voltage * current;
  PowerFactor = activePower / apparentPower;

  // Print the energy value
  float energy = em_bl0940.getEnergy(em_bl0940.getCF_CNT(), calibrated_blink_second);
  DEBUGPRINTLN("Energy (kWh):");
  DEBUGPRINTLN(energy);

  DEBUGPRINTLN("Voltage :");
  DEBUGPRINTLN(voltage);
  DEBUGPRINTLN("Current :");
  DEBUGPRINTLN(current);
  DEBUGPRINTLN("Active Power :");
  DEBUGPRINTLN(activePower);
  DEBUGPRINTLN("Apparent Power :");
  DEBUGPRINTLN(apparentPower);

  if (PowerFactor > 1)
  {
    PowerFactor = 1;
  }
  if (activePower == 0 || apparentPower == 0)
  {
    PowerFactor = 0.01;
  }
  if (PowerFactor < 0.01)
  {
    PowerFactor = 0.01;
  }

  if (em_bl0940.getState() == 0)
  {
    DEBUGPRINTLN("Energy sensor setup passed.");
    energyPassed = "PASSED";
  }
  else
  {
    DEBUGPRINTLN("Energy sensor setup failed.");
    energyPassed = "NOT PASSED";
  }
}

void setupLightSensor()
{
  DEBUGPRINTLN("");
  DEBUGPRINTLN("============ Light Sensor =============");
  light.begin();
  bool lightPass = true; // Assume success initially

  if (light.getPartID(ID))
  {
    DEBUGPRINT("Got Sensor Part ID: 0X");
    DEBUGPRINT(ID);
    DEBUGPRINTLN();
  }
  else
  {
    byte error = light.getError();
    printError(error);
    lightPass = false; // Mark as failed if this step fails
  }

  // Power up the sensor
  if (light.setPowerUp())
  {
    DEBUGPRINT("Powering up...");
    DEBUGPRINTLN();
  }
  else
  {
    byte error = light.getError();
    printError(error);
    lightPass = false; // Mark as failed if this step fails
  }

  // Set gain and measurement rate
  if (light.setGain(gain) && light.setMeasurementRate(integrationTime, measurementRate))
  {
    unsigned long rawData;
    if (light.getData(rawData))
    {
      double luxValue;
      if (light.getLux(gain, integrationTime, rawData, luxValue))
      {
        DEBUGPRINT("Lux Value: ");
        DEBUGPRINTLN(luxValue);
      }
      else
      {
        DEBUGPRINTLN("Failed to calculate lux value.");
        lightPass = false;
      }
    }
    else
    {
      DEBUGPRINTLN("Failed to get raw data.");
      lightPass = false;
    }
  }
  else
  {
    DEBUGPRINTLN("Failed to set gain or measurement rate.");
    lightPass = false;
  }

  // Update the global status
  if (lightPass)
  {
    DEBUGPRINTLN("Light sensor setup passed.");
    lightPassed = "PASSED";
  }
  else
  {
    DEBUGPRINTLN("Light sensor setup failed.");
    lightPassed = "NOT PASSED";
  }
}

void printError(byte error)
{
  DEBUGPRINTLN("");
  DEBUGPRINT("I2C error: ");
  switch (error)
  {
  case 0:
    DEBUGPRINTLN("success");
    break;
  case 1:
    DEBUGPRINTLN("data too long for transmit buffer");
    break;
  case 2:
    DEBUGPRINTLN("received NACK on address (disconnected?)");
    break;
  case 3:
    DEBUGPRINTLN("received NACK on data");
    break;
  case 4:
    DEBUGPRINTLN("other error");
    break;
  default:
    DEBUGPRINTLN("unknown error");
  }
  DEBUGPRINTLN("");
}

void frameDisplay(String sensorName, int x)
{
  // size 128 x 160 tft 1.8 inch
  display.fillScreen(BACKGROUND_COLOR);
  display.fillRoundRect(8, 8, 145, 25, 3, YELLOW); // yellow BGR
  display.setCursor(x, 12);
  display.setTextColor(BLACK);
  display.setTextSize(2);
  display.print(sensorName);

  display.setCursor(10, 40);
  display.setTextSize(1);
  display.setTextColor(TEXT_COLOR);
  display.println("DAC READ:");

  display.setCursor(10, 55);
  display.setTextSize(1);
  display.setTextColor(TEXT_COLOR);
  display.println("DAC WRITE:");

  display.setCursor(10, 70);
  display.setTextSize(1);
  display.setTextColor(TEXT_COLOR);
  display.println("LIGHT SENSOR:");

  display.setCursor(10, 85);
  display.setTextSize(1);
  display.setTextColor(TEXT_COLOR);
  display.println("ENERGY SENSOR:");
}

void writeLCD()
{
  display.fillScreen(BLACK);
  frameDisplay("TESTJIG ID", 10);

  display.setCursor(95, 40);
  display.setTextColor(dacReadPassed == "PASSED" ? HIGHLIGHT_COLOR : RED, BLACK);
  display.println(dacReadPassed);

  display.setCursor(95, 55);
  display.setTextColor(dacWritePassed == "PASSED" ? HIGHLIGHT_COLOR : RED, BLACK);
  display.println(dacWritePassed);

  display.setCursor(95, 70);
  display.setTextColor(lightPassed == "PASSED" ? HIGHLIGHT_COLOR : RED, BLACK);
  display.println(lightPassed);

  display.setCursor(95, 85);
  display.setTextColor(energyPassed == "PASSED" ? HIGHLIGHT_COLOR : RED, BLACK);
  display.println(energyPassed);
}
