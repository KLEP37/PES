#include "HCMotor.h"
#include "Button.h"
#include <LiquidCrystal_I2C.h>
//const int rs = A0, en = A1, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal_I2C lcd(0x27, 16, 2);


#define MOTOR_PIN_1 7
#define MOTOR_PIN_2 8
#define keyStart 4
#define keyNext 5
#define keyUp 2
#define keyDown 3
//maxPumps je 2 jde dat i vice
//pozor kdyby bylo vice nez 4 je potreba stene predefinovat
//i MAXMOTORS z knihovny HCMotor.h
#define maxPumps 2
#define dutyMax 8
#define dutyMin 1

/* Create an instance of the library */
HCMotor HCMotor;
Button kStart(keyStart);
Button kNext(keyNext);
Button kUp(keyUp);
Button kDown(keyDown);

//If the pumps are turned on
bool running = false;
//Selected pump on the display
int myPump = 0;
//The selected speeds on the display
int pumpSpeed[maxPumps];

int originalPumpSpeed[maxPumps];

//Testmode for modifying ratios
bool testMode = false;

//Fills the tubes first
bool startFillMode = true;

unsigned long fillModeStopTime = 15000;

//Used for stopping pumps
unsigned long lastPumpTime = 0;

//the cycles since last reset
int cycles[] = {1,1};

//The time at which the pumps were turned on, in miliseconds
unsigned long startTime = 0;

//Sets the pumps on the lowest setting (?)
void clearPumps() {
 for (int i = 0; i<maxPumps; i++) pumpSpeed[i] = dutyMin;
}

//Selects the next pump on the display
void nextPump() {
  myPump++;
  if (myPump >= maxPumps) myPump = 0;
}

//Changes the speed of the selected pump (input as direction)
void pumpMove(int kam) {
  if (kam>0) pumpSpeed[myPump]++;
  else pumpSpeed[myPump]--;
  if (pumpSpeed[myPump] > dutyMax) pumpSpeed[myPump] = 0;
  else if (pumpSpeed[myPump] < 0) pumpSpeed[myPump] = dutyMax;
  else if (pumpSpeed[myPump] == 1) pumpSpeed[myPump] = dutyMin;
  else if (pumpSpeed[myPump] == dutyMin-1) pumpSpeed[myPump] = 0;
}

//Turns a pump to certain speeds
void runPump(int pump, bool on) {
  int cycle;
  if (pumpSpeed[pump] == 0 || !on)
  {
    cycle = 0;
    Serial.println(cycle);
  }
  else
  {
    if (pump == 0)
    {
      switch (pumpSpeed[0]) {
      case 1:
        cycle = 30;
      break;
      case 2:
        cycle = 34;
      break;
      case 3:
        cycle = 33;
      break;
      case 4:
        cycle = 33;
      break;
      case 5:
        cycle = 33;
      break;
      case 6:
        cycle = 33;
      break;
      case 7:
        cycle = 33;
      break;
      case 8:
        cycle = 37;
      break;
      default:
        cycle = 0;
      break;
      }
      Serial.println(cycle);
    }
    else
    {
      switch (pumpSpeed[1]) {
      case 1:
        cycle = 30;
      break;
      case 2:
        cycle = 35;
      break;
      case 3:
        cycle = 35;
      break;
      case 4:
        cycle = 35;
      break;
      case 5:
        cycle = 35;
      break;
      case 6:
        cycle = 35;
      break;
      case 7:
        cycle = 35;
      break;
      case 8:
        cycle = 38;
      break;
      default:
        cycle = 0;
      break;
      }
      Serial.println(cycle);
    }
  }
  if (running) HCMotor.OnTime(pump, (int)round(cycle));
  else HCMotor.OnTime(pump, 0);
}

//Display the pump states on the display
void printPumps() {
  String text = "";
  
  if (pumpSpeed[0] != 0)
  {
    if (myPump == 0)
      text += ">0 1/" + String (pumpSpeed[0]) + " (" + String (20/pumpSpeed[0]) + ")";
    else
      text += " 0 1/" + String (pumpSpeed[0]) + " (" + String (20/pumpSpeed[0]) + ")";
  }
  else
  {
    if (myPump == 0)
      text += ">0 OFF ";
    else
      text += " 0 OFF ";
  }
  
  text += "Cas:"; 
  
  while (text.length() < 16) text += " ";
  text = text.substring(0,16);
  lcd.setCursor(0,0);
  lcd.print(text);
  //Serial.println(text);
  
  text = "";
  
  if (pumpSpeed[1] != 0)
  {
    if (myPump == 1)
      text += ">1 1/" + String (pumpSpeed[1]) + " (" + String (20/pumpSpeed[1]) + ")";
    else
      text += " 1 1/" + String (pumpSpeed[1]) + " (" + String (20/pumpSpeed[1]) + ")";
  }
  else
  {
    if (myPump == 1)
      text += ">1 OFF ";
    else
      text += " 1 OFF ";
  }

  float cas = (float)(millis() - startTime)/60000;
  text += String (cas, 1) + "m"; 
  
  while (text.length() < 16) text += " ";
  text = text.substring(0,16);
  lcd.setCursor(0,1);
  lcd.print(text);
  //Serial.println(text);
}

//I have no idea whats going on here
void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.print("Test Displeje");
  /* Initialise the library */
  HCMotor.Init();

  /* Attach motor 0 to digital pin 7. The first parameter specifies the 
     motor number, the second is the motor type, and the third is the 
     digital pin that will control the motor */
  HCMotor.attach(0, DCMOTOR, MOTOR_PIN_1);
  HCMotor.attach(1, DCMOTOR, MOTOR_PIN_2);
  /* Set the duty cycle of the PWM signal in 100uS increments. 
     Here 100 x 100uS = 1mS duty cycle. */
  HCMotor.DutyCycle(0, 100); 
  HCMotor.DutyCycle(1, 100);  
  // Min duty cycle is 25%, otherwise there will be problems.
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW); //Motory jsou vypnuty tak vypnu i diodu
  clearPumps();
}

/*Testy pumpy Grothen G328
 */

//Basicly a loop for everything
void loop() {
  if (kStart.isPressed()) {
    originalPumpSpeed[0] = pumpSpeed[0];
    originalPumpSpeed[1] = pumpSpeed[1];

    running = !running;    
    if (running) startTime = millis();    
    printPumps();    
    digitalWrite(13, running);
    runPump(0, true);
    runPump(1, true);
   }
  if (kNext.isPressed()) {
    nextPump();
    printPumps();
  }
  if (kUp.isPressed()) {
     pumpMove(1);
     printPumps();    
   }
  if (kDown.isPressed()) {
    pumpMove(-1);
    printPumps();    
   }

  // stops after one minute
  if (testMode)
  {
    if ((millis() - startTime) > 60000){
      running = false;
      runPump(0,false);
      runPump(1,false);
    }
  }

   if(running && millis() > lastPumpTime + 100)
   {
      lastPumpTime = millis();

      if(millis() - startTime < fillModeStopTime)
      {
        if(originalPumpSpeed[0] == 0)
        {
          pumpSpeed[0] = 0;
        }
        else
        {
          pumpSpeed[0] = 1;
        }

        if(originalPumpSpeed[1] == 0)
        {
          pumpSpeed[1] = 0;
        }
        else
        {
          pumpSpeed[1] = 1;
        }
        
        runPump(0, true);
        runPump(1, true);

        pumpSpeed[0] = originalPumpSpeed[0];
        pumpSpeed[1] = originalPumpSpeed[1];
      }
      else
      {
        for(int i = 0; i < 2; i++)
        {
          if (cycles[i] == 1)
          {
            runPump(i, true);
          }
          else
          {
            runPump(i, false);
          }
          cycles[i]++;
          if(cycles[i] > pumpSpeed[i])
          {
            cycles[i] = 1;
          }
        }
      }
   }
}