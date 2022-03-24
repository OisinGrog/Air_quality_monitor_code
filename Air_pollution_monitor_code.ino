// SPDX-FileCopyrightText: 2020 Limor Fried for Adafruit Industries
// SPDX-FileCopyrightText: 2020 Kevin Townsend for Adafruit Industries
//
// SPDX-License-Identifier: MIT

/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

// Librarys used in the code
#include "BluetoothSerial.h"
#include <Adafruit_SSD1306.h>
#include "bsec.h"

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire); // init declaration of the OLED display

Bsec iaqSensor;

BluetoothSerial ESP_BT; // init bluetooth Class:

String output;

void setup() 
{
  Serial.begin(115200);
  ESP_BT.begin("Air Quality Monitor"); // Name of your Bluetooth interface -> will show up on your phone
  
  Serial.println(F("BME680 test")); // prints start message to serial monitor

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) // Address 0x3C for 128x64
  { 
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // OLED display setup
  Serial.println("OLED begun");
  display.display();
  delay(100);
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setRotation(0);

  iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
  output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial.println(output);
  checkIaqSensorStatus(); // Checks the initial status of the BME680 gas sensor
  
  bsec_virtual_sensor_t sensorList[10] = 
  {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };

  iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();
  // Print the header
  output = "Timestamp [ms], raw temperature [°C], pressure [hPa], raw relative humidity [%], gas [Ohm], IAQ, IAQ accuracy, temperature [°C], relative humidity [%], Static IAQ, CO2 equivalent, breath VOC equivalent";
  Serial.println(output); // prints the name of each variable that is printed using output further down in the code
}

void loop() 
{
  display.setCursor(0, 0);
  display.clearDisplay();

  unsigned long time_trigger = millis(); // read timestamp each time the code loops
  
  if (! iaqSensor.run())
  { 
    // If no data is available
    checkIaqSensorStatus(); // Checks the status of the BME680 gas sensor as code loops
    return;
  }
  
// prints a string of the timestamp of each readout followed by all of the readable variables taken from the BME680 gas sensor using BSEC library seperated by commas
  output = String(time_trigger);
  output += ", " + String(iaqSensor.rawTemperature);
  output += ", " + String(iaqSensor.pressure);
  output += ", " + String(iaqSensor.rawHumidity);
  output += ", " + String(iaqSensor.gasResistance);
  output += ", " + String(iaqSensor.iaq);
  output += ", " + String(iaqSensor.iaqAccuracy);
  output += ", " + String(iaqSensor.temperature);
  output += ", " + String(iaqSensor.humidity);
  output += ", " + String(iaqSensor.staticIaq);
  output += ", " + String(iaqSensor.co2Equivalent);
  output += ", " + String(iaqSensor.breathVocEquivalent);
  Serial.println(output); 

  // The following lines of code print the desired values to the serial monitor and the OLED display
  Serial.print("IAQ = "); 
  Serial.print(iaqSensor.staticIaq);
  Serial.println("");
  display.print("IAQ: "); 
  display.print(iaqSensor.staticIaq); 
  display.println("");
  
  Serial.print("Temperature = ");
  Serial.print(iaqSensor.temperature); 
  Serial.println(" *C");
  display.print("Temperature: "); 
  display.print(iaqSensor.temperature); 
  display.println(" *C");
  
  Serial.print("Humidity = "); 
  Serial.print(iaqSensor.humidity); 
  Serial.println(" %");
  display.print("Humidity: "); 
  display.print(iaqSensor.humidity); 
  display.println(" %");
  
  Serial.print("Pressure = "); 
  Serial.print(iaqSensor.pressure / 100.0); 
  Serial.println(" hPa");
  display.print("Pressure: "); 
  display.print(iaqSensor.pressure / 100); 
  display.println(" hPa");
  
  Serial.print("Gas = ");
  Serial.print(iaqSensor.gasResistance);
  Serial.println(" KOhms");
  display.print("Gas: ");
  display.print(iaqSensor.gasResistance);
  display.println(" KOhms");
   
  Serial.print("CO2 equiv = "); 
  Serial.print(iaqSensor.co2Equivalent); 
  Serial.println("");
  display.print("CO2eq: ");
  display.print(iaqSensor.co2Equivalent);
  display.println(" ppm");
  
  Serial.print("Breath VOC = "); 
  Serial.print(iaqSensor.breathVocEquivalent); 
  Serial.println("");
  display.print("bVOC: ");
  display.print(iaqSensor.breathVocEquivalent);
  display.println(" ppm");

  // prints the desired values through bluetooth connection
  ESP_BT.println(iaqSensor.staticIaq);
  ESP_BT.println(iaqSensor.temperature);
  ESP_BT.println(iaqSensor.humidity); 
  ESP_BT.println(iaqSensor.pressure / 100);
  ESP_BT.println(iaqSensor.gasResistance); 
  ESP_BT.println(iaqSensor.co2Equivalent); 
  ESP_BT.println(iaqSensor.breathVocEquivalent);
  
  Serial.println(); // print blank line between readings
  display.display(); // clear display to allow for next values to be displayed
  delay(2000); // two second delay to allow for change in values

  if (ESP_BT.available())
  {
    int incoming = ESP_BT.read(); //Read what we receive and store in "incoming"
    if(incoming == 10) // if restet button pressed on app
    {
    ESP.restart(); // restart the device
    }
  
  }
  
}


// Error function definitions 
// Displays error message when the BME680 gas sensor isnt reading correctly 
void checkIaqSensorStatus(void)
{
  if (iaqSensor.status != BSEC_OK)
  {
    if (iaqSensor.status < BSEC_OK) 
    {
      output = "BSEC error code : " + String(iaqSensor.status);
      Serial.println(output);
      display.setCursor(0, 0);
      display.println(output);
      display.display();
      for (;;)  
      delay(10);
    } 
    
    else 
    {
      output = "BSEC warning code : " + String(iaqSensor.status);
      Serial.println(output);
    }
  }

  if (iaqSensor.bme680Status != BME680_OK)
  {
    if (iaqSensor.bme680Status < BME680_OK)
    {
      output = "BME680 error code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
      display.setCursor(0, 0);
      display.println(output);
      display.display();
      for (;;)  
      delay(10);
    }

    else
    {
      output = "BME680 warning code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
    }
  }
}
