// Ketose detector by Jens Clarholm (jenslabs.com)
// LCD crystal library by David A. Mellis, Limor Fried, Tom Igoe


/*
LCD Pins specification
 RS: Pin 2
 EN: Pin 3
 D4: Pin 4
 D5: Pin 5
 D6: Pin 6
 D7: Pin 7
 */

// Include lcd library
#include <LiquidCrystal.h>
#include <dht11.h>
#include <math.h>
//////////////////////////////


// Define inputs/output pins ///
//DHT11 sensor
dht11 DHT11;
#define DHT11PIN 10

//LCD
LiquidCrystal lcd(2,3,4,5,6,7);

//TGS882 sensor
int gasValuePin = 0;

//Buttons
int resetMaxSwitchPin = 13;
int resetSensorSwitchPin = 12;
int toggleModeSwitchPin = 11;
//////////////////////////////

//Variable definitions /////
int GlobalMaxValue;
int GlobalMinValue;
int temperatureScaledValue;

//senstor stability check
int newCalibrationVal;
int calibrationHigh;
int calibrationLow;
float warmupHumidity;
float warmupTemperature;
///////////////////////////

int currentMode=1;



//Initial values for variables
//Starting state for buttons
boolean resetMaxSwitchLastButton = LOW;
boolean resetMaxSwitchCurrentButton = LOW;
boolean resetSensorSwitchLastButton = LOW;
boolean resetSensorSwitchCurrentButton = LOW;
boolean toggleModeLastButton = LOW;
boolean toggleModeCurrentButton = LOW;

// Read gas variables
int tempRead1 = 0;
int tempRead2 = 0;
int tempRead3 = 0;
float value1 = 0;
float value2 = 0;
float value3 = 0;

// General Var
float R0 = 4500;
int lastPPM = 0;

double currentHumidity;
double currentTemperature;
double scalingFactor;
//////////////////////////////


void setup() {
  
  //Initiate LCD
  lcd.begin(16,2);
  
  //Define pinmode
  pinMode(gasValuePin,INPUT);
  pinMode(resetMaxSwitchPin, INPUT);
  pinMode(resetSensorSwitchPin, INPUT);
  pinMode(toggleModeSwitchPin, INPUT);

  //Write welcome message
  printToRow1("Hi I'm Ketosense");
  printToRow2("Time to warm up.");
  delay(2000);

  //Initiate DHT11 temperature and humidity sensor and verify it.
  clearLcd();
  printToRow1("Check DHT sensor: ");
  int chk = DHT11.read(DHT11PIN);
  switch (chk)
  {
  case 0: 
    printToRow2("Ok. ");
    break;
  case -1: 
    printToRow2("Checksum error"); 
    break;
  case -2: 
    printToRow2("Time out error"); 
    break;
  default: 
    printToRow2("Unknown error"); 
    break;
  }
  delay(2000);


//Warmup, check that sensor is stabile.
  while (checkIfSensorIsStabile() == false){
    checkIfSensorIsStabile();
  }
  
  clearLcd();
  printToRow1("Warmup finished");
  delay(1000);
  clearLcd();
  printToRow1("Blow into mouth-");
  printToRow2("piece to start."); 
}






void loop() {
  
  //read three times from gas sensor with 5ms between each reading
  readsensor();
  //Check if all readings are the same which indictae a stabile behaviour and if the value is higher or lower update global max and min variables.
  updateNewMaxOrMinWithTempHumidity(tempRead1, tempRead1, tempRead1);
  //print result to display if current value is different to previous value and update the highest value.
  if (acetoneResistanceToPPMf(toResistance(temperatureScaledValue)) != lastPPM){
    lastPPM = acetoneResistanceToPPMf(toResistance(temperatureScaledValue));
    updateScreen();
  }

  //read buttons

  //read resetMaxMinSwitch
  resetMaxSwitchCurrentButton = debounce(resetMaxSwitchLastButton, resetMaxSwitchPin);
  if (resetMaxSwitchLastButton == LOW && resetMaxSwitchCurrentButton == HIGH){
/*    GlobalMaxValue=0;
    GlobalMinValue=0;
    updateScreen();
*/
   clearLcd();
   printToRow1("Result displayed");
   printToRow2("as mmol/l."); 
  }
  resetMaxSwitchLastButton = resetMaxSwitchCurrentButton;


  //read resetSensorSwitch
  resetSensorSwitchCurrentButton = debounce(resetSensorSwitchLastButton, resetSensorSwitchPin);
  if (resetSensorSwitchLastButton == LOW && resetSensorSwitchCurrentButton == HIGH){ 
    while (checkIfSensorIsStabile() == false){
      checkIfSensorIsStabile();
    }
    clearLcd();
    printToRow1("Reset finished");
    delay(1000);
    clearLcd();
    GlobalMaxValue=0;
    GlobalMinValue=0;
    printToRow1("Blow into mouth-");
    printToRow2("piece to start."); 
  }
  resetSensorSwitchLastButton = resetSensorSwitchCurrentButton;


toggleModeCurrentButton = debounce(toggleModeLastButton, toggleModeSwitchPin);
  if (toggleModeLastButton == LOW && toggleModeCurrentButton == HIGH){
   if (currentMode == 1){
   currentMode=2;
   clearLcd();
   printToRow1("Result displayed");
   printToRow2("as mmol/l."); 
   delay(1000);
   updateScreen();
     }
   else if (currentMode == 2){
   currentMode=1;
   clearLcd();
   printToRow1("Result displayed");
   printToRow2("as PPM.");
    delay(1000);
    updateScreen();
     }
 }
  toggleModeLastButton = toggleModeCurrentButton;


}
//////////////////////////
/// Methods start here ///
//////////////////////////

//Reads sensor 3 times with 5ms delay between reads and store read values in tempRead1, 2 and 3
float ppmToMmol(int PPM){
 float ppmInmmol = (((float) PPM / 1000) / 58.08);
 ppmInmmol = ppmInmmol * 1000;
 return ppmInmmol;
 }

void readsensor(){
  tempRead1 = analogRead(0);
  delay(5);
  tempRead2 = analogRead(0);
  delay(5);
  tempRead3 = analogRead(0);
}

//Update screen with result
void updateScreen(){
  clearLcd();
///// DEBUG start
    printToRow1("H:");
    printIntToCurrentCursorPossition((int)currentHumidity);
    printStringToCurrentCursorPossition(" T:");
    printIntToCurrentCursorPossition((int)currentTemperature);
   // printStringToCurrentCursorPossition(" S:");
   // printFloatToCurrentCursorPossition((float)scalingFactor);
    printStringToCurrentCursorPossition(" R:");
    printIntToCurrentCursorPossition((int)GlobalMaxValue);
///// Debug end

  printToRow2("Now:    Max:    ");
  if (currentMode == 2){
        //Result in mmol/l
        lcd.setCursor(4,1);
        printFloatToCurrentCursorPossition(ppmToMmol(acetoneResistanceToPPMf(toResistance(temperatureScaledValue))));
        lcd.setCursor(12,1);
        printFloatToCurrentCursorPossition(ppmToMmol(acetoneResistanceToPPMf(toResistance(GlobalMaxValue))));
  
  }
  else if (currentMode == 1){
    //result in PPM
    lcd.setCursor(4,1);
  printIntToCurrentCursorPossition(acetoneResistanceToPPMf(toResistance(temperatureScaledValue)));
  lcd.setCursor(12,1);
  printIntToCurrentCursorPossition(acetoneResistanceToPPMf(toResistance(GlobalMaxValue)));
  }
}

//calculate the gas concentration relative to the resistance
int acetoneResistanceToPPMf(float resistance){
  double tempResistance = (double)resistance;
  double PPM; 
  if (tempResistance > 50000){
  double PPM = 0;
  }
  else {
  double logPPM = (log10(tempResistance/R0)*-2.6)+2.7;
   PPM = pow(10, logPPM);
  }
  return (int)PPM;
}



//debounce function
boolean debounce(boolean last, int pin )
{
  boolean current = digitalRead(pin);
  if (last != current)
  {
    delay(5);
    current = digitalRead(pin);
  }
  return current;
}

//temperature sensor function, values has been hardcoded to humidity = 60 and temperature = 28 to speed up the measuring.
int tempHumidityCompensation(int value){
    int chk = DHT11.read(DHT11PIN);
    delay(300);
    //currentHumidity = ((double)DHT11.humidity);
    //Hardcoded after realizing that the temperature and humidity were beahaving stabilly.
    currentHumidity = 60;
    //currentTemperature = ((double)DHT11.temperature);
    currentTemperature = 28;
    //function derrived from regression analysis of the graph in the datasheet
    scalingFactor = (((currentTemperature * -0.02573)+1.898)+((currentHumidity*-0.011)+0.3966));
    //debug
    //clearLcd();
    //printToRow1("Scalefactor:");
    //printFloatToCurrentCursorPossition((float)scalingFactor);
    delay(1000);
    //debugstop*/
    double scaledValue = value * scalingFactor;
    return (int)scaledValue;
    
}
      
//check if we have new max or min after temperature and humidity scaling has been done. 
void updateNewMaxOrMinWithTempHumidity(int value1, int value2, int value3){
  if (value1 == value2 && value1 == value3){
    temperatureScaledValue = tempHumidityCompensation(value1);
    
    if (GlobalMaxValue==0){
      GlobalMaxValue = temperatureScaledValue;
    }
    if (GlobalMinValue==0){
      GlobalMinValue = temperatureScaledValue;
    }
    if (temperatureScaledValue < GlobalMinValue){
      GlobalMinValue = temperatureScaledValue;
    }
    if (temperatureScaledValue > GlobalMaxValue){
      GlobalMaxValue = temperatureScaledValue;
    }
  }
}
//check if we have new max or min without temperature and humidity scaling. 
void updateNewMaxOrMin(int value1, int value2, int value3){
  if (value1 == value2 && value1 == value3){
    if (GlobalMaxValue==0){
      GlobalMaxValue = value1;
    }
    if (GlobalMinValue==0){
      GlobalMinValue = value1;
    }
    if (value1 < GlobalMinValue){
      GlobalMinValue = value1;
    }
    if (value1 > GlobalMaxValue){
      GlobalMaxValue = value1;
    }
  }
}

//Convert the 1-1023 voltage value from gas sensor analog read to a resistance, 9800 is the value of the other resistor in the voltage divide.
float toResistance(int reading){
  float resistance = ((5/toVoltage(reading) - 1) * 9800);
  return resistance;
}



//Convert the 1-1023 value from analog read to a voltage.
float toVoltage(int reading){
  //constant derived from 5/1023 = 0.0048875
  float voltageConversionConstant = 0.0048875;
  float voltageRead = reading * voltageConversionConstant;
  return voltageRead;
}

//Clear all text on both lines of LCD
void clearLcd(){
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("                ");
}

//Clears one row of LCD and prints the text sent to the method starting on possition 0 on row 1
void printToRow1(String text){
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,0);
  lcd.print(text);
}

//Clears one row of LCD and prints the text sent to the method starting on possition 0 on row 2
void printToRow2(String text)
{
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print(text);
}

//Prints what ever string is sent to it to current cursor possition
void printStringToCurrentCursorPossition(String text)
{
  lcd.print(text);
}

//Prints what ever intiger is sent to it to current cursor possition
void printIntToCurrentCursorPossition(int text)
{
  lcd.print(text);
}

//Prints what ever float is sent to it to current cursor possition
void printFloatToCurrentCursorPossition(float text)
{
  lcd.print(text);
}

boolean checkIfSensorIsStabile()
{
  //read samples for 10 seconds
  for (int i=0; i <= 20 ; i++){

    //read Acetone Sensor
    newCalibrationVal = analogRead(0);
    //Read DHT humidity
    DHT11.read(DHT11PIN);
    delay(300);
    warmupHumidity = ((float)DHT11.humidity);
    warmupTemperature = ((float)DHT11.temperature);

    //set first sample as baseline
    if (i==0){
      calibrationLow = newCalibrationVal;
      calibrationHigh = newCalibrationVal;
    }

    //Determine if last sensor reading is higher then previous high
    if (newCalibrationVal > calibrationHigh){
      calibrationHigh = newCalibrationVal;
    }

    //Determine if sensor reading is lower then previous high
    if (newCalibrationVal < calibrationLow){
      calibrationLow = newCalibrationVal;
    }   

    // Print current max and min to lcd
    clearLcd();
    printToRow1("H:");
    printIntToCurrentCursorPossition(warmupHumidity);
    printStringToCurrentCursorPossition(" T:");
    printIntToCurrentCursorPossition(warmupTemperature);
    printStringToCurrentCursorPossition(" i:");
    printIntToCurrentCursorPossition(i);
    printToRow2("Max:");
    printIntToCurrentCursorPossition(calibrationHigh);
    printStringToCurrentCursorPossition(" Min:");
    printIntToCurrentCursorPossition(calibrationLow);
    delay(1000);
    //(printToRow1("Calibrating.");

    if (calibrationHigh - calibrationLow > 5){
      return false;
    }
  }

  // Check if resistance has not changed more then 5 steps during 10 sec = unstabile sensor
  if ((calibrationHigh - calibrationLow) <= 3 || calibrationHigh <= 135){
    // calculateR0(calibrationHigh);
    return true;
    
  }
  
  else return false;

}



