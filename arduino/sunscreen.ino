/*-------------------------------------------------------------------------------------------
                                            SUNSCREEMINDER
-------------------------------------------------------------------------------------------*/

#include <ePaper.h>
#include <SPI.h>
#include <SD.h>
//#include "GT20L16_drive.h"

#define SCREEN_SIZE 270                     // choose screen size here: 144, 200, 270

#if (SCREEN_SIZE == 144)
#define EPD_SIZE    EPD_1_44

#elif (SCREEN_SIZE == 200)
#define EPD_SIZE    EPD_2_0

#elif (SCREEN_SIZE == 270)
#define EPD_SIZE    EPD_2_7

#else
#error "Unknown EPB size: Change the #define SCREEN_SIZE to a supported value"
#endif

int solx = 0;
int soly = 0;
int UVOUT = A14;
int REF_3V3 = A12;
const int buttonPin = 31;
const int button2Pin = 37;
const int piezoPin = 45;
double intensity;
int numberOfPersons = 1;
char* names[10]={"Mathias", "", "", "", "", "", "", "", "", ""}; //Array with profilenames
//             {age, skintype, sunscreen}
char* info0[3]={"21","3","20"};
char* info1[3]={"","",""};
char* info2[3]={"","",""};
char* info3[3]={"","",""};
char* info4[3]={"","",""};
char* info5[3]={"","",""};
char* info6[3]={"","",""};
char* info7[3]={"","",""};
char* info8[3]={"","",""};
char* info9[3]={"","",""};
long timers[10]={0,0,0,0,0,0,0,0,0,0}; //Array with timers for each profile
int selectionCounter = 0;
int tmpY = 0;
int buttonState = 0;
int lastButtonState = 0;
int button2State = 0;
int lastButton2State = 0;
int uvLevel;
int refLevel;
float outputVoltage;
float uvIntensity;
char uvInt[2];
unsigned long time;
unsigned long startTime;
boolean timeCheck = true;

void setup()
{
  EPAPER.begin(EPD_SIZE);                             // setup epaper, size
  EPAPER.setDirection(DIRRIGHT);                     // set display direction

  eSD.begin(EPD_SIZE);
  
  //-------------------Buttons-------------
  pinMode(buttonPin, INPUT);
  pinMode(button2Pin, INPUT);
  //--------------------End B--------------

  //-------------------UV-SENSOR------------
  Serial.begin(9600);

  pinMode(UVOUT, INPUT);
  pinMode(REF_3V3, INPUT);
  //pinMode(piezoPin,OUTPUT);
  //-------------------END - UV-------------
  startTime = millis();
  updateDisplay();
}

void loop(){
    time = millis();
    buttonState = digitalRead(buttonPin);
    button2State = digitalRead(button2Pin);
    
    //Selects next profile 
    if (buttonState != lastButtonState) {
        if(buttonState == HIGH){
            tone(piezoPin, 2500);
            delay(10);
            noTone(piezoPin);
            tmpY += 16;
            selectionCounter++;
            if (selectionCounter == numberOfPersons){
                tmpY = 0;
                selectionCounter = 0;
            }
            EPAPER.clear_sd();
            drawSun(uvInt, 80, 1);
            drawList(0,120);
            drawIntensity(intensity,0,0);
            EPAPER.display();
        }
    }
    
    
    
    //Sets a timer for currently selected person
    if (button2State != lastButton2State) {
        if(button2State == HIGH){
            tone(piezoPin, 2500);
            delay(10);
            noTone(piezoPin);
            timers[selectionCounter] = time + 60000;
            EPAPER.clear_sd();
            EPAPER.drawString("TIMER SATT",50,124);
            EPAPER.display();
            EPAPER.clear_sd();
            drawSun(uvInt, 80, 1);
            drawList(0,120);
            drawIntensity(intensity,0,0);
            EPAPER.display();
        }
    }
    //Checks every timer and starts the alarm if timer reaches certain point
    for(int i = 0; i < numberOfPersons; i++){
        if(timers[i] != 0){
            if(time >= timers[i]){
                EPAPER.clear_sd();
                EPAPER.drawString("PAA TIDE AA SMOERE ", 15,124);
                EPAPER.drawString(names[i], 65, 138);
                EPAPER.drawString("\"Set\" for ny timer", 15, 154);
                EPAPER.drawString("\"Select\" for aa", 15, 170);
                EPAPER.drawString("avbryte", 15, 184);
                EPAPER.display();
                tone(piezoPin, 1000);
                delay(500);
                noTone(piezoPin);
                tone(piezoPin, 1500);
                delay(500);
                noTone(piezoPin);
                tone(piezoPin, 2000);
                delay(500);
                noTone(piezoPin);
                timers[i] = 0;
            }
        }
    }
//Update the screen and information every minute
    if(time >= startTime + 60000){
        if(timeCheck){
            updateDisplay();
            timeCheck = false;
            startTime = 0;
        }
    }
  delay(100);
}

void updateDisplay(){
    uvLevel = averageAnalogRead(UVOUT);
    refLevel = averageAnalogRead(REF_3V3);
    //Use the 3.3V power pin as a reference to get a very accurate output value from sensor
    outputVoltage = 3.3 / refLevel * uvLevel;

    uvIntensity = mapfloat(outputVoltage, 1.20, 5, 0.0, 15.0); //Convert the voltage to a UV intensity level

    intensity = (double) uvIntensity;
    String str;
    str = String(intensity,2);
    str.toCharArray(uvInt,2);

    EPAPER.clear_sd();    // clear sd card data(when use display next time, display is clear)
    drawSun(uvInt, 80, 1);
    drawList(0,120);
    drawIntensity(intensity,0,0);
    EPAPER.display();
    printToSerial();
}
//Draws the intensity of the UV-radiation.
void drawIntensity(int intens, int x, int y){
    EPAPER.drawString("Styrke:", x,y);
    if(intens >= -1 && intens < 3){
        EPAPER.drawString("  LAV", x,y+16);
    } else if(intens >= 3 && intens < 6){
        EPAPER.drawString("MODERAT", x,y+16);
    } else if(intens >= 6 && intens < 8){
        EPAPER.drawString("STERK", x,y+16);
    } else if(intens >= 8 && intens < 11){
        EPAPER.drawString("SVAERT", x,y+16);
        EPAPER.drawString("STERK", x,y+32);
    } else if (intens >= 11) {
        EPAPER.drawString("EKSTREM", x,y+16);
    }
}
//Draws the bell that appears behind a profile name if alarm i active
void drawBell(int xb, int yb){
    EPAPER.drawLine(5+xb,yb,6+xb,yb);
    EPAPER.drawLine(4+xb,yb+1,7+xb,yb+1);
    EPAPER.drawLine(3+xb,yb+2,8+xb,yb+2);
    EPAPER.drawLine(3+xb,yb+3,8+xb,yb+3);
    EPAPER.drawLine(3+xb,yb+4,8+xb,yb+4);
    EPAPER.drawLine(3+xb,yb+5,8+xb,yb+5);
    EPAPER.drawLine(2+xb,yb+6,9+xb,yb+6);
    EPAPER.drawLine(1+xb,yb+7,10+xb,yb+7);
    EPAPER.drawLine(xb,yb+8,11+xb,yb+8);
    EPAPER.drawLine(xb,yb+9,11+xb,yb+9);
    EPAPER.drawLine(1+xb,yb+10,10+xb,yb+10);
    EPAPER.drawLine(4+xb,yb+11,7+xb,yb+11);
}
//Draws the lines used to make the different boxes
void drawList(int x, int y) {
  drawNameList(10 + x, y);
  drawSelection(5 + x,y);
  updateSelection();
  EPAPER.drawLine(0,y,176,y);
  EPAPER.drawLine(88,y,88,264);
}
//Used to calculate the different variables which is then used to calculate the UV-intensity
//Reads same pin 8 times and then finds the average value.
int averageAnalogRead(int pinToRead){
  byte numberOfReadings = 8;
  unsigned int runningValue = 0;

  for (int x = 0 ; x < numberOfReadings ; x++){
    runningValue += analogRead(pinToRead);
  }
  runningValue /= numberOfReadings;
  return (runningValue);
}

//The Arduino Map function but for floats
//From: http://forum.arduino.cc/index.php?topic=3922.0
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
//Writes the profile namelist. If a profile is set it calls the "drawBell" method.
void drawNameList(int x, int y) {
    for(int i = 0; i < numberOfPersons; i++){
        EPAPER.drawString(names[i], 0 + x , 0 + y);
        if(timers[i] != 0){
            drawBell(x + 65, y +2);
        }
        y += 16;
    }
}
//Method to check what profile is currently selected, then sends the info to "drawInfo".
void updateSelection(){
    switch (selectionCounter){
        case 0: drawInfo(info0[0],info0[1],info0[2]); break;
        case 1: drawInfo(info1[0],info1[1],info1[2]); break;
        case 2: drawInfo(info2[0],info2[1],info2[2]); break;
        case 3: drawInfo(info3[0],info3[1],info3[2]); break;
        case 4: drawInfo(info4[0],info4[1],info4[2]); break;
        case 5: drawInfo(info5[0],info5[1],info5[2]); break;
        case 6: drawInfo(info6[0],info6[1],info6[2]); break;
        case 7: drawInfo(info7[0],info7[1],info7[2]); break;
        case 8: drawInfo(info8[0],info8[1],info8[2]); break;
        case 9: drawInfo(info9[0],info9[1],info9[2]); break;
        default: drawInfo(" ", " ", " "); break;
    }
    
}
//Writes the info of the currently selected profile.
void drawInfo(char* age, char* skinType, char* spf){
    EPAPER.drawString("Alder: ", 90,120);
    EPAPER.drawString(age,90,136);
    EPAPER.drawString("Hudtype: ", 90, 152);
    EPAPER.drawString(skinType, 90, 168);
    EPAPER.drawString("Solfaktor: ", 90, 184);
    EPAPER.drawString(spf, 90, 200);
    if(timers[selectionCounter] != 0){
        EPAPER.drawString("TIMER PAA", 90, 216);
    } else{
        EPAPER.drawString("TIMER AV", 90, 216);
    }
}
//Draws a circle in front of the currently selected profile
void drawSelection(int x, int y){
        EPAPER.fillCircle(0 + x,0 + y + 7 + tmpY,3);
}
//Draws the sun which is used to illustrate the UV-index
void drawSun(char* uv, int x, int y) {
  EPAPER.drawCircle(48 + x, 48 + y, 30);
  EPAPER.drawString("UVi", 37 + x, 32 + y);
  EPAPER.drawString(/*(char*)uv*/"4", 44 + x, 48 + y);
  EPAPER.drawLine(48 + x, 0 + y, 48 + x, 12 + y);
  EPAPER.drawLine(48 + x, 84 + y, 48 + x, 96 + y);
  EPAPER.drawLine(0 + x, 47 + y, 12 + x, 47 + y);
  EPAPER.drawLine(81 + x, 47 + y, 94 + x, 47 + y);
  EPAPER.drawLine(16 + x, 12 + y, 25 + x, 20 + y);
  EPAPER.drawLine(81 + x, 12 + y, 72 + x, 20 + y);
  EPAPER.drawLine(81 + x, 82 + y, 72 + x, 74 + y);
  EPAPER.drawLine(16 + x, 82 + y, 25 + x, 74 + y);
}
//Prints accurate values to serialmonitor.
void printToSerial(){
      Serial.print("output: ");
      Serial.print(refLevel);
      Serial.print("ML8511 output: ");
      Serial.print(uvLevel);
      Serial.print(" / ML8511 voltage: ");
      Serial.print(outputVoltage);
      Serial.print(" / UV Intensity (mW/cm^2): ");
      Serial.print(uvIntensity);
      Serial.print(" INTENSITY: ");
      Serial.print(uvInt);
      Serial.println();
}
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
