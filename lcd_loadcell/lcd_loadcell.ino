
      
#include <HX711_ADC.h>
#include <Servo.h>  
#include <LiquidCrystal.h>  // Include the LiquidCrystal library

// pins:
const int HX711_dout = 4; // mcu > HX711 dout pin
const int HX711_sck = 5;  // mcu > HX711 sck pin
const int servoPin = 9;    // Pin for the servo

// LCD pin configuration (without I2C)
const int rs = 7, en = 8, d4 = 10, d5 = 11, d6 = 12, d7 = 13; // Adjust pin numbers as needed

// HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);
Servo myServo;  // Create a Servo object

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);  // Initialize the LCD

const int calVal_eepromAdress = 0;
unsigned long t = 0;

void setup() {
  Serial.begin(57600); delay(10);
  Serial.println();
  Serial.println("Starting...");

  // Initialize LCD
  lcd.begin(16, 2);  // Set up the LCD's number of columns and rows
  lcd.print("Initializing...");

  LoadCell.begin();
  float calibrationValue = 696.0; // Calibration value for the load cell
  LoadCell.setCalFactor(calibrationValue); // Set calibration value

  unsigned long stabilizingtime = 2000; // Allow some time for stabilization
  boolean _tare = true; // Set this to false if you don't want tare
  LoadCell.start(stabilizingtime, _tare);

  if (LoadCell.getTareTimeoutFlag()) {
    lcd.clear();
    lcd.print("HX711 Timeout");
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  } else {
    lcd.clear();
    lcd.print("Startup done");
    Serial.println("Startup is complete");
  }

  myServo.attach(servoPin);  // Attach the servo on the specified pin
  myServo.write(90);  // Set servo to 90 degrees initially
  Serial.println("Servo at 90 degrees");

  delay(2000);  // Display "Startup done" for 2 seconds before clearing the screen
  lcd.clear();
}

void loop() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; // Increase value to slow down serial print activity

  // Check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // Get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float weight = LoadCell.getData();
      Serial.print("Load_cell output val: ");
      Serial.println(weight);
      
      // Clear the LCD before printing new values
      lcd.clear();
      lcd.setCursor(0, 0);  // Set cursor to the first row, first column
      lcd.print("Weight: ");
      lcd.print(weight);    // Display weight value
      lcd.print("g");

      // Check if the weight is 1000g or more
      if (weight >= 100.0) {
        myServo.write(0);  // Move servo to 0 degrees
        Serial.println("Servo moved to 0 degrees");

        lcd.setCursor(0, 1);  // Set cursor to the second row
        lcd.print("Servo at 0 deg");
      } else {
        myServo.write(90);  // Keep the servo at 90 degrees otherwise
        lcd.setCursor(0, 1);  // Set cursor to the second row
        lcd.print("Servo at 90 deg");
      }

      // Add delay for LCD to display the values properly
      delay(600);  // Delay for 1 second (1000 ms)

      newDataReady = 0;
      t = millis();
    }
  }

  // Receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  // Check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
}