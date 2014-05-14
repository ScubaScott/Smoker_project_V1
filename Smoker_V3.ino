  #include <FWB.h>
  #include <SoftwareSerial.h>
  #include <serLCD.h>
  #include <Servo.h>
  #include <SD.h>
  
# define Btn1pin 2 //the pin where your button is connected
# define Btn2pin 3 //the pin where your button is connected

  
  
//constants
//digital pins
//  Button 1  = pin 2
//  Button 2  = pin 3
// ** SD Card CS - pin 4
const int giLCDPin = 5;
const int giLEDPin =  6;
const int giFanRelayPin = 7;
const int giServoPin = 8;
// ** SD Card output 10
// ** SD Card MOSI - pin 11
// ** SD Card MISO - pin 12
// ** SD Card CLK - pin 13

//Analog Pins
const int giPitPin = 0;
const int giMeatPin = 1;
//other constants
const long day = 86400000; // 86400000 milliseconds in a day
const long hour = 3600000; // 3600000 milliseconds in an hour
const long minute = 60000; // 60000 milliseconds in a minute
const long second =  1000; // 1000 milliseconds in a second
//SD
const int chipSelect = 4;

//Variables

int giMode = 0;
int giItem = 0;
int giItems[4] = {75,85,100,0};
String gsItems[4] = {"Lo Temp","Hi Temp","Done Temp","Stop when done"};
int giLEDBlinkRate = 100;
long glLEDBlinkTimer =0;
boolean gbLEDState = LOW;
int giLCDRefresh = 500;
long glLCDTimer = 0;
int giServoValue = 100;
int giServoRefresh = 10000;
long glServoTimer = 0;

long glFanRunTime = 1;
long glFanRunTimer = 0;
long glFanRunLimit = 4500; //this should be 45000 for 45 seconds
long glFanRunInterval = 12000; //this should be 120000 for 2 minutes



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
    if (gbSetup){
        lcd.selectLine(1);
        lcd.print(gsItems[giItem]);
        lcd.selectLine(2);
        lcd.print(String(giItems[giItem]));
    }else{
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
           break;
         case 1:
           lcd.selectLine(1);
           lcd.print("Ck Tm:");
           break;
         case 2:
           lcd.selectLine(1);
           lcd.print("Lo/Hi/Done tmp");
           lcd.selectLine(2);
           lcd.print(String(giItems[0]));
           lcd.print("/");
           lcd.print(String(giItems[1]));
           lcd.print("/");
           lcd.print(String(giItems[2]));
           break;
      }
    }
  }
    if (gbSetup){
       lcd.setCursor(2, 16);
       lcd.print("S");
    }
    if (gbAlarm){
      lcd.setCursor(1, 16);
      if (giPitTemp>giItems[1]){
        lcd.printCustomChar(3);
      }else if (giPitTemp<giItems[0]){
        lcd.printCustomChar(4);          
      }
    }
    if (gbDone){
        lcd.setCursor(1, 16);
        lcd.printCustomChar(2); //fork
    }
      //Debug display
        //lcd.setCursor(1, 14);
        //lcd.print(String(giServoValue));
}

void CheckServo(){
  //dont adjust the servo more than ServoRefresh
  
    if (giPitTemp < giItems[1] - 5){
      giServoValue = 100;
    }else if (giPitTemp == giItems[1] - 5){
      giServoValue = 85;
    }else if (giPitTemp == giItems[1] - 4){
      giServoValue = 70;
    }else if (giPitTemp == giItems[1] - 3){
      giServoValue = 55;
    }else if (giPitTemp == giItems[1] - 2){
      giServoValue = 40;
    }else if (giPitTemp == giItems[1] - 1){
      giServoValue = 25;
    }else if (giPitTemp >= giItems[1]){
        //CurrentTemp reached HI TEMP shut it down.
        giServoValue = 10;
    }
    if (gbDone){giServoValue = 10;}
}

void MoveServo(int moveto){
  myservo.attach(giServoPin);
  if (moveto != myservo.read()){
    myservo.write(moveto);
    delay(200); //required delay to ensure the servo moves.
  }
  myservo.detach();
}

void CheckFan(){
    glFanRunTime = glFanRunTime + (millis()- glFanRunTimer);
glFanRunTimer = millis();

if ((giPitTemp < giItems[0]) && (!gbDone)){
  if ( glFanRunTime < glFanRunLimit){
	  //runtime is less than limit, turn fan on.
      digitalWrite(giFanRelayPin, HIGH);
    }else{
      //the fan has run long enough, stop it.
      digitalWrite(giFanRelayPin, LOW);
    }
    if (glFanRunTime > glFanRunInterval){
      //the run interval has exceeded, and it's still cold, reset timers
      glFanRunTime = 0;
    }
  }else{ //current temp is above Low Limit
    if ( glFanRunTime > glFanRunLimit){
		//to prevent fan bounce, force fan to run for full g_lFanRunLimit
		digitalWrite(giFanRelayPin, LOW);
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
