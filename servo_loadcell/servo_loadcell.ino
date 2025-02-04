#include <HX711_ADC.h>
#include <Servo.h>  // Include the Servo library

//pins:
const int HX711_dout = 4; //mcu > HX711 dout pin
const int HX711_sck = 5;  //mcu > HX711 sck pin
const int servoPin = 9;    // Pin for the servo

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);
Servo myServo;  // Create a Servo object

const int calVal_eepromAdress = 0;
unsigned long t = 0;

void setup() {
  Serial.begin(57600); delay(10);
  Serial.println();
  Serial.println("Starting...");

  LoadCell.begin();
  float calibrationValue = 696.0; // Calibration value for the load cell
  LoadCell.setCalFactor(calibrationValue); // Set calibration value

  unsigned long stabilizingtime = 2000; // Allow some time for stabilization
  boolean _tare = true; // Set this to false if you don't want tare
  LoadCell.start(stabilizingtime, _tare);

  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    Serial.println("Startup is complete");
  }

  myServo.attach(servoPin);  // Attach the servo on the specified pin
  myServo.write(90);  // Set servo to 90 degrees initially
  //Serial.println("Servo at 90 degrees");
}

void loop() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; // Increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float weight = LoadCell.getData();
      Serial.print("Load_cell output val: ");
      Serial.println(weight);
      
      // Check if the weight is 1000g or more
      if (weight >= 100.0) 
      {
        myServo.write(0);  // Move servo to 0 degrees
        //Serial.println("Servo moved to 0 degrees");
      }
      else{
        myServo.write(90); 
      }
      
      newDataReady = 0;
      t = millis();
    }
  }

  // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  // check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
}