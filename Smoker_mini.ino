  #include <FWB.h>
  #include <SoftwareSerial.h>
  #include <serLCD.h>
  #include <Servo.h>
  #include <SD.h>
 
 //v1     intial build
 //v1.1   fixed fan indicator
 //v1.2   fixed fan timing
 //v1.3   changed servo handle based on % of max/min
  
# define Btn1pin 2 //the pin where your button is connected
# define Btn2pin 3 //the pin where your button is connected
  
//constants
//digital pins
const int giLCDPin = 5;
const int giLEDPin =  6;
const int giFanRelayPin = 7;
const int giFanOnState = LOW; //HIGH=NC relay, LOW=NO relay
const int giServoPin = 8;

//Analog Pins
const int giPitPin = 0;
const int giMeatPin = 1;
//other constants
const long day = 86400000; // 86400000 milliseconds in a day
const long hour = 3600000; // 3600000 milliseconds in an hour
const long minute = 60000; // 60000 milliseconds in a minute
const long second =  1000; // 1000 milliseconds in a second

//Variables

int giMode = 0;
int giItem = 0;
int giItems[4] = {195,220,165,1};
String gsItems[4] = {"Lo Temp","Hi Temp","Done Temp","Stop when done"};
String gaServoModes[3] {"Min","Auto","max"};
int giServoMode = 1;
int giLEDBlinkRate = 100;
long glLEDBlinkTimer =0;
boolean gbLEDState = LOW;
int giLCDRefresh = 500;
long glLCDTimer = 0;
int giServoMin = 10; //servo min position. Servo min position is the "Closed" position
int giServoMax = 120; //servo max position.
int giServoValue = giServoMax;
int giServoRefresh = 10000; //time between servo adjustments 
long glServoTimer = 0; //acumulated servo time

long glFanRunTime = 1; 
long glFanRunTimer = 0; 
long glFanRunLimit = 10000; //max time for 1 "run". this should be 45000 for 45 seconds
long glFanRunInterval = 120000; //Min time between runs. this should be 120000 for 2 minutes
String gaFanModes[3] = {"Off","Auto","Stoke"};
int giFanMode = 1;

boolean gbSetup = 0;
boolean gbDone = false;
boolean gbAlarm = false;

int giPitTemp;
int giMeatTemp;

byte gbDegree[8] = {B01100,B10010,B10010,B01100,B00000,B00000,B00000,B00000};
byte gbFork[8] =   {B10101,B10101,B10101,B01110,B00100,B00100,B00100,B00100};
byte gbCold[8] =   {B00100,B00100,B00100,B00100,B10101,B10101,B01110,B00100};
byte gbHot[8] =    {B00100,B01110,B10101,B10101,B00100,B00100,B00100,B00100};
                     
//Classes
  FWB btn1;
  FWB btn2;
  serLCD lcd(giLCDPin);
  Servo myservo;
  
   void OnClick(int pin)
 {
   //what button was clicked?
    if (pin == Btn1pin){
      if (gbSetup){
          //setup is true, increase current value
          giItems[giItem]++;
      }else{
       //not in setup, incrament mode 
        if (giMode < 2){
          giMode++;
        }else{
          giMode = 0;
        }
      }
     }else if (pin == Btn2pin){
      if (gbSetup){
          //setup is true, decrease current value
          giItems[giItem]--;
      }else{
       //not in setup, decrease mode 
        if (giMode >0){
          giMode--;
        }else{
          giMode = 2;
        }
      }
     }
}

void Setup(int pin)
  {
    gbSetup = !gbSetup;
  }
void Reset(int pin)
  {
    gbAlarm = false;
    gbDone = false;
  }

void Btn1DblClick(int pin)
  {
    if (gbSetup){
      if (giItem <3){
      giItem++;
      }else{
      giItem = 0;
    }
    }
  }
  
void Btn2DblClick(int pin)
  {
    if (gbSetup){
      if (giItem >0){
      giItem--;
      }else{
      giItem = 3;
    }
    }else{
      gbDone = true;
    }
  }

void CheckLED(){
if(gbAlarm || gbDone){
  //set flashrate (alarm overrides done)
  if (gbAlarm){
    giLEDBlinkRate = 75;
  }else{
    giLEDBlinkRate = 1000;
  }
   if(millis() - glLEDBlinkTimer > giLEDBlinkRate) {
      // save the last time you blinked the LED 
      glLEDBlinkTimer = millis();   
      gbLEDState = !gbLEDState;
      // set the LED with the ledState of the variable:
    }
  }else{
      gbLEDState = false;
  }
  digitalWrite(giLEDPin, gbLEDState);
}
int thermister_temp(int aval) {
	double R, T;

	// These were calculated from the thermister data sheet
	//	A = 2.3067434E-4;
	//	B = 2.3696596E-4;
	//	C = 1.2636414E-7;
	//
	// This is the value of the other half of the voltage divider
	//	Rknown = 22200;

	// Do the log once so as not to do it 4 times in the equation
	//	R = log(((1024/(double)aval)-1)*(double)22200);
	R = log((1 / ((1024 / (double) aval) - 1)) * (double) 22200);
	//lcd.print("A="); lcd.print(aval); lcd.print(" R="); lcd.print(R);
	// Compute degrees C
	T = (1 / ((2.3067434E-4) + (2.3696596E-4) * R + (1.2636414E-7) * R * R * R)) - 273.25;
	// return degrees F
	return ((int) ((T * 9.0) / 5.0 + 32.0));
}
void DrawLCD()
{
  if (millis() - glLCDTimer > giLCDRefresh){
    glLCDTimer = millis();
    lcd.clear();
    //MAIN display logic
    
      switch (giMode){
        case 0:
          lcd.selectLine(1);
          lcd.print("Pit :");
          lcd.print(String(giPitTemp));
          lcd.printCustomChar(1); //degree symbol
          lcd.selectLine(2);
          lcd.print("Meat:");
          lcd.print(String(giMeatTemp));
          lcd.printCustomChar(1); //degree symbol
          
          if (gbDone){
          lcd.setCursor(1, 16);
          lcd.printCustomChar(2); //fork
          }else if (gbAlarm){
            lcd.setCursor(1, 16);
            if (giPitTemp>giItems[1]){
              lcd.printCustomChar(3);
            }else if (giPitTemp<giItems[0]){
              lcd.printCustomChar(4);          
            }
          }else if (digitalRead(giFanRelayPin) == giFanOnState){
            lcd.setCursor(1, 16);
            lcd.print("*");
          }
           break;
         case 1:
           lcd.selectLine(1);
           lcd.print("Lo/Hi/Done tmp");
           lcd.selectLine(2);
           lcd.print(String(giItems[0]));
           lcd.print("/");
           lcd.print(String(giItems[1]));
           lcd.print("/");
           lcd.print(String(giItems[2]));
           break;
	case 2:
           lcd.selectLine(1);
           lcd.print("Servo:" + string(gaServoModes[giServoMode]));
	  //servo position
          lcd.selectLine(2);
          lcd.print(String(giServoValue));
           break;
	case 3:
           lcd.selectLine(1);
           lcd.print("Fan:" + string(gaFanModes[giFanMode]));
           lcd.selectLine(2);
           lcd.print(String(giFanMode));
           break;
      }
    }

}

void CheckServo(){
  float gfStep = (giServoMax-giServoMin)/5;
  
    if (giPitTemp < giItems[1] - 5){
      giServoValue = giServoMax;
    }else if (giPitTemp == giItems[1] - 5){
      giServoValue = giServoMin + gfStep * 5;
    }else if (giPitTemp == giItems[1] - 4){
      giServoValue = giServoMin + gfStep * 4;
    }else if (giPitTemp == giItems[1] - 3){
      giServoValue = giServoMin + gfStep * 3;
    }else if (giPitTemp == giItems[1] - 2){
      giServoValue = giServoMin + gfStep * 2;
    }else if (giPitTemp == giItems[1] - 1){
      giServoValue = giServoMin + gfStep;
    }else if (giPitTemp >= giItems[1]){
        //CurrentTemp reached HI TEMP shut it down.
        giServoValue = giServoMin;
    }
    if (gbDone){giServoValue = 10;}
}

void MoveServo(int moveto){
  myservo.attach(giServoPin);
  if (moveto != myservo.read()){
    myservo.write(moveto);
    delay(1000); //required delay to ensure the servo moves.
  }
  myservo.detach();
}

void CheckFan(){
    glFanRunTime = glFanRunTime + (millis()- glFanRunTimer);
    glFanRunTimer = millis();

if ((giPitTemp < giItems[0]) && (!gbDone)){
  if ( glFanRunTime < glFanRunLimit){
	  //runtime is less than limit, turn fan on.
      digitalWrite(giFanRelayPin, giFanOnState);
    }else{
      //the fan has run long enough, stop it.
      digitalWrite(giFanRelayPin, !giFanOnState);
    }
    if (glFanRunTime > glFanRunInterval){
      //the run interval has exceeded, and it's still cold, reset timers
      glFanRunTime = 0;
    }
  }else{ //current temp is above Low Limit
    if ( glFanRunTime > glFanRunLimit){
		//to prevent fan bounce, force fan to run for full g_lFanRunLimit
		digitalWrite(giFanRelayPin, !giFanOnState);
	}
  }
}

 void setup()
 {
    lcd.clear();
    lcd.home();
    lcd.selectLine(1);
    lcd.createChar(1, gbDegree);
    lcd.createChar(2, gbFork);
    lcd.createChar(3, gbHot);
    lcd.createChar(4, gbCold);
    lcd.print("Light the fire..");
    
    
    pinMode(giLEDPin,OUTPUT);
    pinMode(giFanRelayPin, OUTPUT);
    
    btn1.Configure(Btn1pin);
    btn2.Configure(Btn2pin);
    btn1.OnClick = OnClick;
    btn1.OnDblClick = Btn1DblClick;
    btn1.OnLongPress = Setup;
    btn2.OnClick = OnClick;
    btn2.OnDblClick = Btn2DblClick;
    btn2.OnVLongPress = Reset;
    delay(1000);  
 }

//*********************************************** MAIN LOOP **********************************************
 void loop()
 {
   //read temps
    giPitTemp = thermister_temp(analogRead(giPitPin));
    giMeatTemp = thermister_temp(analogRead(giMeatPin));
    if (giMeatTemp <0) {giMeatTemp = 0;}


if ((giItems[3] = 1 ) && (giMeatTemp >= giItems[2])){
  gbDone = true;
}

gbAlarm = false;
if ((giPitTemp > giItems[1]) || (giPitTemp < giItems[0]) && (!gbDone)){
          
          gbAlarm = true;
}else{
          gbAlarm = false;
}

    //Servo Logic
    //dont adjust the servo more than ServoRefresh
    if (millis()-glServoTimer > giServoRefresh){
      glServoTimer= millis();
      CheckServo();
      MoveServo(giServoValue);
    }
    
    //Fan Logic
    CheckFan();
    
    // button Logic
    btn1.CheckBP();
    btn2.CheckBP();
    
    //Display Logic
    DrawLCD();
    //LED logic
    CheckLED();
 }
 
 //********************************************** END MAIN LOOP ******************************************
