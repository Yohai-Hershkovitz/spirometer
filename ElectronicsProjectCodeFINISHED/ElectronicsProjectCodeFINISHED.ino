//written for board Arduino Mega 2560, with microcontroller ATmega2560
#include <Servo.h>
#include <LiquidCrystal.h> //doesn't come initially, must be downloaded?
#include <Keypad.h> //doesn't come initially, must be downloaded 
LiquidCrystal lcd(7, 6, 5, 4, 3, 2); // initialize the lcd library with the numbers of the interface pins.
Servo myservo; //creating an object from the class of servo.
int sensorPin=0;
int servoPin=13;
int speakerPin=9;
int redPin=10;
int greenPin=11;
int bluePin=12;
int angleVal=0; //the servo motor angle value
int toneVal=450; //the loudspeaker tone value
float analogRead0; //used when the initial analog read is unknown
const float oneMeasureTime=20;
int measuresCount=0;
float measuresTime=0;
double Read=0;
//float zRead=0;//temporary read, variable used for testing
float pressureSum=0;
float lungsVC=0; //should be around 4.8/3.1 liters for men/wemen
float maxPressure=0;
//making the kepad:
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = { // Define the Keymap
  {'A','B','C','D'},
  {'3','6','9','#'},
  {'2','5','8','0'},
  {'1','4','7','*'}
};
byte rowPins[ROWS] = {50, 48, 46, 44}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {36, 38, 40, 42}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS ); // Create the Keypad
char Key2=0; //used to detect pressing a key
  //for testing, and for delaying the wait for a pressure applience
 float analogRead1=1000; //minimal Read0
 float analogRead2=0; //max Read0
 
 int fakesCounter=0; //counts the negative unwanted readings from the sensor

void setup() { // put your setup code here, to run once:
//  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps. used only in testing the system functionality.
  lcd.begin(16, 2);    // set up the LCD's number of columns and rows.
  analogReference(INTERNAL1V1);
  pinMode(sensorPin,INPUT);
  pinMode(speakerPin,OUTPUT);
  myservo.attach(servoPin);
  pinMode(redPin,OUTPUT);
  pinMode(greenPin,OUTPUT);
  pinMode(bluePin,OUTPUT);
  setColor(0,0,0); //off
  keypad.addEventListener(keypadEvent); //add an event listener for this keypad. printing pressed keys.
  for(int i=0;i<50;i++){ //update- increased amount of samplings from 20 to 50
    Read=analogRead(sensorPin);
//    Serial.println(Read);
    analogRead0+=Read;
    //for testing, and for delaying the wait for a pressure applience
    if(analogRead1>Read) //for testing functionality of the system
      analogRead1=Read;
    if(analogRead2<Read)
      analogRead2=Read;
     
    delay(20);
  }
  analogRead0=(analogRead0/50)-11; // the average read volt from pressure
  /*
  //testing values of the 0 pressure variables
  lcd.print(analogRead1); //min pressure 0
  lcd.print(" ");
  lcd.print(analogRead0); //average pressure 0
  lcd.setCursor(0,1);
  lcd.print(" ");
  lcd.print(analogRead2); //max pressure 0
//  Serial.println(analogRead1);
//  Serial.println(analogRead2);
  delay(4000);
  //end of testing 
  lcd.clear();
  */
}
 
void loop() {  // put your main code here, to run repeatedly:
  //user interface script start
  lcd.print("Welcome to the");
  lcd.setCursor(0,1);
  lcd.print("Spirometer!");
  delay(2000);
  lcd.clear();
  lcd.print("for better");
  lcd.setCursor(0,1);
  lcd.print("experience,");
  delay(2000);
  lcd.clear();
  lcd.print("Please look at");
  lcd.setCursor(0,1);
  lcd.print("the instructions");
  delay(2500);
  lcd.clear();
//script for manual- test or train
  lcd.print("For PFT - 1");
  lcd.setCursor(0,1);
  lcd.print("For excercise- 2");
  delay(2000);
  lcd.clear();  
  do{
     Key2=keypad.getKey();
  }
  while(Key2==0);
  do{  
    switch (Key2){
      case '1': measuresCount=0;
                pressureSum=0;
                maxPressure=0;
                fakesCounter=0; //resets number of fake readings in PFT
                //PFT script
                delay(1500); 
                lcd.clear();
                lcd.print("get ready for");
                lcd.setCursor(0,1);
                lcd.print("the PFT");
                for(int j=0;j<3;j++){
                  blinkRed();
                }
                lcd.clear();
                lcd.print("Ready,");
                setColor(255,0,0); //red
                tone(speakerPin,toneVal,400);
                delay(1200);
                lcd.clear();
                lcd.print("set,");
                setColor(0,0,255); //blue
                tone(speakerPin,toneVal,350);
                delay(1200);
                lcd.clear();
                lcd.print("GO!");
                setColor(0, 255, 0); // green
                tone(speakerPin,toneVal,300);
                do{  //waiting for human pressure to be applied
                  Read=analogRead(sensorPin);      //is this line needed, or could it stay blank?
              //    Serial.println(Read);
                } while(Read<=analogRead2+3); //analogRead0 had been increased due to sensitivity of the sensor
                do{  //starting the PFT
                    Read=toneVal=angleVal=analogRead(sensorPin);//samples the output voltage of the sensor
                    //zRead=Read; //for testing
                    angleVal=toneVal=map(toneVal,0,1023,0,3071); //val final value is the frequency of the note heard. target: hight is about 600 Hz. max=1024*3-1. //0 pressure turns into 450, avarage max pressure gets ~600. 
                    angleVal=600-angleVal;//making value of 450 a ~0, and 600 a ~150 degrees. must make arrow background picture for showing strain in training according to these angles.
                    myservo.write(angleVal);
                    tone(speakerPin,toneVal);
                    if(Read>analogRead0){
                      fakesCounter=0; 
                      measuresCount++;
                      Read=Read-analogRead0; //reducing standard pressure (instead of 1 atmosphere in previous versions)
                      Read=map(Read,0,1023,0,700000); //mapping to pascal units
                      Read=Read/1000; // mapping to kPa units. 
                      pressureSum+=Read; 
                      if (maxPressure<Read)
                       maxPressure=Read;
                    }
                    else{
                      fakesCounter++; //detected turbolences with sensor which caused too early of a halt. counting the first 16 fakes and disableing them.
                    }
                    /*
                    //checking functionality:
                       Serial.println(measuresCount); //learnd that functions take time to execute
                       Serial.print("angleVal=  ");
                       Serial.print(angleVal);
                       Serial.print("  toneVal=  ");
                       Serial.print(toneVal);
                       Serial.print("  Read=");
                       Serial.print(Read,4);
                       Serial.print("   analogRead(sensorPin)=");
                       Serial.print(zRead);
                       Serial.print("   analogRead0=");
                       Serial.print(analogRead0);
                       Serial.print("   pressureSum=");
                       Serial.println(pressureSum);
                     */
                    delay(oneMeasureTime);
                  }  while(fakesCounter<16); // condition to stop measuring: when readings are under the 0 for a steady amount of times
                noTone(speakerPin);
             //   Serial.println(measuresCount);
                measuresTime=measuresCount*oneMeasureTime/1000;//MT=MC*time per measure in seconds
             //   Serial.print("    measuresTime=");
             //  Serial.print(measuresTime);
                pressureSum=pressureSum/measuresCount; //giving pressureSum the value of the avarage pressure applied in the test
                lungsVC=measuresTime*pressureSum*0.25; //V=t*(average P)*A. last formula was WRONG. area matters, multiplied by a quarter to consider pipes slice areas
             //   Serial.print("    lungsVC=");
             //   Serial.print(lungsVC);
                setColor(0,0,0);//off
                lcd.clear();
                lcd.print("VC(L):");
                lcd.print(lungsVC);
                lcd.setCursor(0,1);
                lcd.print("Pmax(kPa):");
                lcd.print(maxPressure,3);
                delay(5000);
        break;
      case '2': //training script
                delay(1500); 
                lcd.clear();
                lcd.print("get ready for");
                lcd.setCursor(0,1);
                lcd.print("the exercise");
                for(int j=0;j<3;j++){
                  blinkRed();
                }
                lcd.clear();
                lcd.print("Ready,");
                setColor(255,0,0); //red
                tone(speakerPin,toneVal,400);
                delay(1200);
                lcd.clear();
                lcd.print("set,");
                setColor(0,0,255); //blue
                tone(speakerPin,toneVal,350);
                delay(1200);
                lcd.clear();
                lcd.print("TRAIN!"); //starts the exercise when the user starts inhaling 
                setColor(0,255,0); //green
                tone(speakerPin,toneVal,100);
                delay(500);
                lcd.clear();   
                setColor(0,0,0); //off
                do{  //waiting for human pressure to be applied
                  Read=analogRead(sensorPin);      //is this line needed, or could it stay blank?
                } while(Read>=analogRead0);
                for(int i=0;i<6;i++){ //starting the exercise
                  for(int j=6-i;j>0;j--){ //blinks green for 6,5,4,3,2,1 times
                    lcd.clear();
                    lcd.print(j);
                    blinkGreen();
                  }
                  for(int j=6-i;j<12;j++){ //blinks red for 6,7,8,9,10,11 times
                    lcd.clear();
                    lcd.print(12-j);
                    blinkRed();
                  }
                }
                lcd.clear();
                lcd.print("excellent!");
        break;
      default: delay(1500);
               lcd.clear();
               lcd.print("error! not a"); 
               lcd.setCursor(0,1);
               lcd.print("valid entry");
               delay(2000);
               lcd.clear();
        break;             
      }
       lcd.clear();
       lcd.print("if you want");
       lcd.setCursor(0,1);
       lcd.print("another action,");
       delay(2000);
       lcd.clear();
       lcd.print("press the right");
       lcd.setCursor(0,1);
       lcd.print("key");
       delay(2000);
       lcd.clear();
       do{
          Key2=keypad.getKey();
       }
       while(Key2==0);
       delay(1000);
       lcd.clear();
       if(Key2!='1'&&Key2!='2'){
        lcd.print("error! not a"); 
        lcd.setCursor(0,1);
        lcd.print("valid entry");
        delay(2000);
        lcd.clear();
        lcd.print("restarting the");
        lcd.setCursor(0,1);
        lcd.print("spirometer...");
        delay(3000);
        lcd.clear();
        delay(1000);
       }
    } 
    while(Key2=='1'||Key2=='2');
  //no need for body measurements to compare, the doc knows it all.
}
void keypadEvent(KeypadEvent Key){
  if(keypad.getState()==PRESSED){  //I didn't use the switch function because I have only 1 case, but if needed- it can be expanded.
   lcd.print(Key); 
  }  
}
int setColor(int red, int green, int blue){
  red=255-red;
  green = 255 - green;
  blue = 255 - blue;
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}
void blinkRed(){
  setColor(255,0,0); //red
  delay(200);
  setColor(0,0,0); //off
  delay(800);
}
void blinkGreen(){
  setColor(0,255,0); //green
  delay(200);
  setColor(0,0,0); //off
  delay(800);
}
