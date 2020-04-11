 /*
Compteur Training Board
25.04.2018, updated 11.04.2020
*/

#include <EEPROM.h>
#include <TimerFreeTone.h>

/* UNO
#define inputA 6 // Bleu
#define inputB 7 // Jaune
#define inputButton 13  // Orange
#define outputLed 14
#define outputBuzz 19 // Vert
#define LATCH_PIN 3 // Jaune
#define CLOCK_PIN 4 // Marron
#define DATA_PIN 2 // Gris
*/

/* ATTiny 84 */
#define inputA 2 // Bleu
#define inputB 1 // Jaune
#define inputButton 3  // Orange
#define outputLed 4
#define outputBuzz 5
#define LATCH_PIN 6 // Jaune
#define CLOCK_PIN 7 // Marron
#define DATA_PIN 8 // Gris


byte displayCount = 0;
byte displaySec;
byte counter = 0;
byte pause = 0;
byte repCount;
byte tens;
byte ones;
int aState;
int aLastState;
unsigned long chronoRun;
boolean halfCounterUp = LOW ;
boolean halfCounterDown = LOW;
boolean counterSet = LOW;
boolean pauseSet = LOW;
boolean chronoRunning = LOW;
boolean pauseRunning = LOW;

void setup() {

  // initialize serial for arduino only
  // Serial.begin(9600);

  // INPUTS
  pinMode(inputButton, INPUT);
  pinMode (inputA,INPUT);
  pinMode (inputB,INPUT);

  // OUTPUT
  pinMode(outputLed, OUTPUT);
  pinMode(outputBuzz, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  // Reads the initial state of the outputA
  aLastState = digitalRead(inputA);

  // flash Led:
  digitalWrite(outputLed, HIGH);
  setValue(B00000000, B00000000, B00000000);
  TimerFreeTone(outputBuzz, 523,180); //C5
  delay(20);
  TimerFreeTone(outputBuzz, 1046,180); //C6
  delay(200);
  digitalWrite(outputLed, LOW);

  // Read Eprom at address 0 and prepare counter
  counter = EEPROM.read(0);
  pause = EEPROM.read(1);
  setDisplay(counter);
  displayCount = counter;

}

void loop() {

      //!!!!!!!!!!!!!!!!!!!!
      //!! Button Pressed !!
      //!!!!!!!!!!!!!!!!!!!!
      if (digitalRead(inputButton) == HIGH) {
          if (chronoRunning == LOW && pauseRunning == LOW){
             if (counterSet == LOW){
                  counterSet = HIGH;
                  TimerFreeTone(outputBuzz, 440,200); //A4
                  counter = displayCount;
                  if (EEPROM.read(0) != counter) {EEPROM.write(0, counter);}
                  setDisplay(pause);
                  displayCount = pause;
             } else if (pauseSet == LOW){
                  pauseSet = HIGH;
                  TimerFreeTone(outputBuzz, 880,200); //A5
                  pause = displayCount;
                  if (EEPROM.read(1) != pause) {EEPROM.write(1, pause);}
                  repCount = 1;
                  chronoRunning = HIGH;
                  delay(1000);
                  displaySec = counter;
                  countDownAndStart();
             }
          } else if (millis() - chronoRun >= 200){
             chronoRunning = LOW;
             pauseRunning = LOW;
             counterSet = LOW;
             pauseSet = LOW;
             setDisplay(counter);
             digitalWrite(outputLed, HIGH);
             TimerFreeTone(outputBuzz, 440,200); //A4
             delay(150);
          }
      } else {
      // turn LED off:
      digitalWrite(outputLed, LOW);
      }


      //!!!!!!!!!!!!!!!!!
      //!! Use Encoder !!
      //!!!!!!!!!!!!!!!!!
      if (chronoRunning == LOW && pauseRunning == LOW){
         aState = digitalRead(inputA); // Reads the "current" state of the outputA
         // If the previous and the current state of the outputA are different, that means a Pulse has occured
         if (aState != aLastState){
           // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
           if (digitalRead(inputB) != aState) {
                if (halfCounterUp == HIGH){
                   if (displayCount == 99) {
                   displayCount = 0;
                   } else {
                   displayCount ++; }
                   halfCounterUp = LOW;
                   setDisplay(displayCount);
                } else {
                   halfCounterUp = HIGH;
                }
           } else {
                if (halfCounterDown == HIGH){
                   if (displayCount == 0){
                    displayCount = 99;
                   } else {
                   displayCount --; }
                   halfCounterDown = LOW;
                   setDisplay(displayCount);
                } else {
                   halfCounterDown = HIGH;
                }
           }
         }
         aLastState = aState; // Updates the previous state of the outputA with the current state
      }


      //!!!!!!!!!!!!!!!!!!!!
      //!! Chrono Running !!
      //!!!!!!!!!!!!!!!!!!!!
      if (chronoRunning == HIGH && pauseRunning == LOW){
          digitalWrite(outputLed, HIGH);
          if (millis() - chronoRun >= 1000)
              {
              if (displaySec > 1)
                  {
                  displaySec --;
                  }
              else
                  {
                  for (int i=0; i <= 3; i++){
                      TimerFreeTone(outputBuzz, 523,50); //C5
                      digitalWrite(outputLed, HIGH);
                      delay(50);
                      digitalWrite(outputLed, LOW);
                      delay(50);
                      }
                  chronoRunning = LOW;
                  pauseRunning = HIGH;
                  displaySec = pause;
                  }
              chronoRun = millis();
              setDisplay(displaySec);
              }
          }
       else if (chronoRunning == LOW && pauseRunning == HIGH){
          if (millis() - chronoRun >= 1000)
              {
              if (displaySec > 1)
                  {
                  displaySec --;
                  }
              else
                  {
                  for (int i=0; i <= 3; i++){
                      TimerFreeTone(outputBuzz, 1046,50); //C6
                      digitalWrite(outputLed, HIGH);
                      delay(50);
                      digitalWrite(outputLed, LOW);
                      delay(50);
                      }
                  chronoRunning = HIGH;
                  pauseRunning = LOW;
                  repCount ++;
                  displaySec = counter;
                  }
              chronoRun = millis();
              setDisplay(displaySec);
              }
        }
}



//-----------------------------------
void setDisplay(int counterDisplay)
{
// Serial.print(hundreds);
tens = ((counterDisplay/10)%10);
// Serial.print(tens);
ones = (counterDisplay%10);
// Serial.println(ones);
if (chronoRunning == HIGH || pauseRunning == HIGH){
  setValue(convertToLed(repCount) - B00010000, convertToLed(tens), convertToLed(ones));
  }
else {
  setValue(B11111111, convertToLed(tens), convertToLed(ones));
  }
}


//-----------------------------------
void setValue(int a, int b, int c)
{
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, a);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, b);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, c);
  digitalWrite(LATCH_PIN, HIGH);
}


//-----------------------------------
int convertToLed(int n)
{
int output;
  if (n == 0)
      {
      output = B00011000;
      }
  else if (n == 1)
      {
      output = B11011110;
      }
  else if (n == 2)
      {
      output = B00110100;
      }
  else if (n == 3)
      {
      output = B10010100;
      }
  else if (n == 4)
      {
      output = B11010010;
      }
  else if (n == 5)
      {
      output = B10010001;
      }
  else if (n == 6)
      {
      output = B00010001;
      }
  else if (n == 7)
      {
      output = B11011100;
      }
  else if (n == 8)
      {
      output = B00010000;
      }
  else if (n == 9)
      {
      output = B10010000;
      }
return output;
}


//------------------------
void countDownAndStart()
{

      digitalWrite(outputLed, HIGH);
      setValue(B10111111, B10111111, B10111111);
      TimerFreeTone(outputBuzz, 523,150); //C5
      delay(100);
      setValue(B11111111, B11111111, B11111111);
      digitalWrite(outputLed, LOW);
      delay(750);

      digitalWrite(outputLed, HIGH);
      setValue(B10110111, B10110111, B10110111);
      TimerFreeTone(outputBuzz, 523,150); //C5
      delay(100);
      setValue(B11111111, B11111111, B11111111);
      digitalWrite(outputLed, LOW);
      delay(750);

      digitalWrite(outputLed, HIGH);
      setValue(B10110101, B10110101, B10110101);
      TimerFreeTone(outputBuzz, 523,150); //C5
      delay(100);
      setValue(B11111111, B11111111, B11111111);
      digitalWrite(outputLed, LOW);
      delay(750);

      chronoRun = millis();
      displaySec = counter +1;
      // Serial.print("Chrono: ");
      // Serial.println(chronoRun);
      setValue(B00010000, B00010000, B00010000);
      TimerFreeTone(outputBuzz, 1046,200); //C6
      delay(300);

}