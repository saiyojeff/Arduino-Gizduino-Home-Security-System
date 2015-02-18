/*
ARDUINO HOME SECURITY SYSTEM
Based on the work of Jun Peng
Modified by Jeffrey James Valerio
Kimberly Joy San Juan
Geraldine Hinahon
Nico Macatangay
Domingo Marasigan
Irish Cupo
Marlett Alvarez

of Cavite State University Rosario Campus in the Philippines
*/

#include <LiquidCrystal.h>
#include <Password.h>
#include <Keypad.h> 
#include <Servo.h> 


//Servo
Servo myservo;        // create servo object to control a servo           
int pos = 90;         // variable to store the servo position 
int passwd_pos = 11;  // the postition of the password input


//Password
Password password = Password( "4321" );

const byte ROWS = 4; // Four rows
const byte COLS = 3; // Three columns

char keys[ROWS][COLS] = { // Define the Keymap
  {
    '1','2','3'      }
  ,
  {
    '4','5','6'      }
  ,
  {
    '7','8','9'      }
  ,
  {
    '*','0','#'      }
};

byte rowPins[ROWS] = {
  20, 22, 24, 26};     //connect to the row pinouts of the keypad
byte colPins[COLS] = {
  28, 30, 32};     //connect to the column pinouts of the keypad

// Create the Keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // Assign arduino pins to LCD display module


int RGBPin = 31;

//constants for LEDs, inputs and outputs
//int blueLED = 36;
int greenLED = 14;
int redLED = 15;
int pirPin1 = 39;

int reedPin1 = 9;
int reedPin2 = 10;
int speakerPin = 13; 


int lights = 17; 

int alarmStatus = 0;
int zone = 0;
int alarmActive = 0;

void setup(){
  Serial.begin(9600);
  lcd.begin(16, 2);
  
  myservo.attach(7);  // attaches the servo on pin 0 to the servo object 

  displayCodeEntryScreen();

  //Police LED Lights
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  //setup and turn off both LEDs
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  

  pinMode(lights, OUTPUT);  //12V Blue LED lighting 

 
  pinMode(pirPin1, INPUT);  //Bedroom 2
  pinMode(reedPin1, INPUT); //Front door
  pinMode(reedPin2, INPUT); //Back door

  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  digitalWrite(speakerPin, LOW);
 

  digitalWrite(lights, LOW); // 12V Blue LED lighting 


  keypad.addEventListener(keypadEvent); //add an event listener for this keypad
  myservo.write(pos);
}

void loop(){

  
  //Serial.println(digitalRead(reedPin1));
  //delay(1000);
  //Serial.println(digitalRead(reedPin2));
  //delay(1000);
  //Serial.println(digitalRead(pirPin1)); 
  //delay(1000);
  keypad.getKey();
  
  if (alarmActive == 1){ 
    if (digitalRead(pirPin1) == HIGH)
    {
      zone = 3;
      alarmTriggered();
    }
    if (digitalRead(reedPin1) == LOW)  //LOW when switch is CLOSED
    {
      zone = 1;
      alarmTriggered();
    }
    if (digitalRead(reedPin2) == LOW)   //LOW when switch is CLOSED
    {
      zone = 2;
      alarmTriggered();
    }
    
   }
}

/////////////////////////  Functions  /////////////////////////////////
//take care of some special events
void keypadEvent(KeypadEvent eKey){
  switch (keypad.getState()){
  case PRESSED:
    if (passwd_pos - 11 >= 5) { 
     lcd.clear();
      password.reset(); 
      passwd_pos = 11;
      displayCodeEntryScreen();
    }
    lcd.setCursor((passwd_pos++),0);
    switch (eKey){
    case '#':                 //# is to validate password 
      passwd_pos  = 11;
      checkPassword(); 
      break;
    case '*':                 //* is to reset password attempt
    lcd.clear();
      password.reset(); 
      passwd_pos = 11;
      displayCodeEntryScreen();
   // TODO: clear the screen output 
      break;
    default: 
      password.append(eKey);
      lcd.print("*");
    }
  }
}

void alarmTriggered(){
  int expected_pos;
  int incr;
  digitalWrite(speakerPin, HIGH);
  digitalWrite(lights, HIGH);
  digitalWrite(RGBPin, HIGH);
  digitalWrite(redPin, HIGH);
  
//
  password.reset();
  alarmStatus = 1;
  //alarmActive = 0;
  lcd.clear();
  displayCodeEntryScreen();
   keypad.getKey();
  //lcd.setCursor(0,0);
  //lcd.print("TRIGGERED");
  lcd.setCursor(0,1);
  if (zone == 1)
  { 
    lcd.print("Front Door Open");
    expected_pos = 65;
    delay(1000);
  }
   if(zone == 3){
    expected_pos = 40;
    lcd.print("Motion in Bedroom 1 ");
    delay(1000);
  }
  else if(zone == 2){
    expected_pos = 10;
    lcd.print("Backdoor Open");
    delay(1000);
  }
   
   if (expected_pos > pos) {
     incr = 1;
   } else {
     incr = -1;
   }
   
   for (pos = pos; pos != expected_pos; pos += incr) {
    myservo.write(pos);                  // tell servo to go to position in variable 'pos' 
    delay(5);                            // waits 5ms for the servo to reach the position 
   }
   
  
  {
     digitalWrite(RGBPin, HIGH);
  }
}

                                                     

void checkPassword(){                  // To check if PIN is corrected, if not, retry!
  if (password.evaluate())
  {  
    if(alarmActive == 0 && alarmStatus == 0)
    {
      activate();
    } 
    else if( alarmActive == 1 || alarmStatus == 1) {
      deactivate();
    }
  } 
  else {
    invalidCode();
  }
}  

void invalidCode()    // display meaasge when a invalid is entered
{
  password.reset();
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("INVALID CODE!");
  lcd.setCursor(0,0);
  lcd.print("TRY AGAIN!");
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, HIGH);
  delay(2000);
  digitalWrite(redLED, LOW);
  delay(1000);
  displayCodeEntryScreen();
}

void activate()      // Activate the system if correct PIN entered and display message on the screen
{
  if((digitalRead(reedPin1) == HIGH) && (digitalRead(reedPin2) == HIGH)){
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    //digitalWrite(2, HIGH);
    lcd.setCursor(0,1);
    lcd.print("SYSTEM ACTIVE!"); 
    alarmActive = 1;
    password.reset();
    delay(2000);
  }
  else{
    deactivate();   // if PIN not corrected, run "deactivate" loop
  }
}

void deactivate()
{
  //digitalWrite(camera, LOW);
  alarmStatus = 0;
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("DEACTIVATED!");
  digitalWrite(speakerPin, LOW);
  alarmActive = 0;
  password.reset();
  delay(5000);
  digitalWrite(lights, HIGH);
  //digitalWrite(relay3, HIGH);


  displayCodeEntryScreen();
}

void displayCodeEntryScreen()    // Dispalying start screen for users to enter PIN
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enter PIN:");
}







