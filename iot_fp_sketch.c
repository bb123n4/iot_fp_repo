#include <AlertNodeLib.h>
#include <Servo.h>

// TONES  ==========================================
// Start by defining the relationship between 
//       note, period, &  frequency. 
#define  c     3830    // 261 Hz 
#define  d     3400    // 294 Hz 
#define  e     3038    // 329 Hz 
#define  f     2864    // 349 Hz 
#define  g     2550    // 392 Hz 
#define  a     2272    // 440 Hz 
#define  b     2028    // 493 Hz 
#define  C     1912    // 523 Hz 
// Define a special note, 'R', to represent a rest
#define  R     0

int speakerOut = 6; // speaker pin must support pwm
int motorPin = 9; // motor pin must support pwm

int tempPin = 3;  // pin for receiving data from temperature pin
float temp; 
const float tempFireThreshold = 100.0; // threshold used to determine the reaction of the guard during a fire alert
const float tempThreshold = 25.0;  // threshold for cloths advisor

const String myNodeName = "jinbo's node";

int photocellPin = 0;      
int photocellReading;     // the analog reading from the sensor divider
int rubyLED = 11;          // connect ruby like LED
int rubyLEDbrightness; 

int high_LED_limit = 300;
int low_LED_limit = 200;   // Hard coded constants to make the LED display better

int pos = 0;    
int startPos = 30; // start position of the motor

// LEDs for fire alarm
int led1 = 13;
int led2 = 12;

Servo myservo;
AlertNode myNode;

void setup() {
  myservo.attach(motorPin); // must support pwm
  myservo.write(startPos); 
  pinMode(photocellPin,INPUT);
  pinMode(speakerOut, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  
  Serial.begin(9600);
  Serial.print("\n\n*** Starting AlertNodeBasic demo: ");
  Serial.println(myNodeName);
  
  myNode.setDebug(false);
  myNode.begin(myNodeName);
}


// MELODY and TIMING  =======================================
//  melody[] is an array of notes, accompanied by beats[], 
//  which sets each note's relative length (higher #, longer note) 

// Following are 3 pieces of melody composed to fit different situations
int melody_ht[] = {  g,  b,  g,  b,  g, b, g, b}; // stands for melody being played when high temperature is detected during fire
int beats_ht[]  = { 32,32,32,32,32,32,32,32 };  
int melody_lt[] = {  g,  g,  g,  g,  g, g, g, g}; // stands for melody being played when low temperature is detected during fire
int beats_lt[]  = { 32,32,32,32,32,32,32,32 };  
int melody_zb[] = {  a,  a,  C,  C,  a, a, e, e}; // stands for melody being played when zombie is coming
int beats_zb[]  = { 32,32,32,32,32,32,32,32 };  

int MAX_COUNT = sizeof(melody_ht) / 2; // Melody length, for looping.

// Set overall tempo
long tempo = 10000;
// Set length of pause between notes
int pause = 1000;
// Loop variable to increase Rest length
int rest_count = 100; //<-BLETCHEROUS HACK; See NOTES

// Initialize core variables
int tone_ = 0;
int beat = 0;
long duration  = 0;

void playTone() {
  long elapsed_time = 0;
  if (tone_ > 0) { // if this isn't a Rest beat, while the tone has 
    //  played less long than 'duration', pulse speaker HIGH and LOW
    while (elapsed_time < duration) {

      digitalWrite(speakerOut,HIGH);
      delayMicroseconds(tone_ / 2);

      // DOWN
      digitalWrite(speakerOut, LOW);
      delayMicroseconds(tone_ / 2);

      // Keep track of how long we pulsed
      elapsed_time += (tone_);
    } 
  }
  else { // Rest beat; loop times delay
    for (int j = 0; j < rest_count; j++) { // See NOTE on rest_count
      delayMicroseconds(duration);  
    }                                
  }                                 
}

void respond_to_alerts(int alert) {
  if (alert != AlertNode::NO_ALERT) {
    Serial.println("alert received"); //  for debug purpose
    if (alert == AlertNode::FIRE) {
        digitalWrite(led1, LOW);
        digitalWrite(led2, LOW);

       // two condition when received a fire alert
       // when temperature is relatively low 
       if (temp < tempFireThreshold) {
        
         for (int i=0; i<MAX_COUNT; i++) {
            tone_ = melody_lt[i];
            beat = beats_lt[i];
            duration = beat * tempo; // Set up timing
            playTone(); 
            // A pause between notes...
            delayMicroseconds(pause);
            int l1 = digitalRead(led1);
  
            if (l1 == HIGH) {
              digitalWrite(led1, LOW);
            } else {
              digitalWrite(led1, HIGH);
            }
         } 
       } else {  // when temperature is relatively high
        digitalWrite(led1, HIGH);
        for (int i=0; i<MAX_COUNT; i++) {
            tone_ = melody_ht[i];
            beat = beats_ht[i];
            duration = beat * tempo; // Set up timing
            playTone(); 
            // A pause between notes...
            delayMicroseconds(pause);
            int l1 = digitalRead(led1);
            int l2 = digitalRead(led2);
            if (l1 == HIGH) {
              digitalWrite(led1, LOW);
            } else {
              digitalWrite(led1, HIGH);
            }
            if (l2 == LOW) {
              digitalWrite(led2, HIGH);
            } else {
              digitalWrite(led2, LOW);
            }
         } 
       }
      }   else if (alert == AlertNode::ZOMBIE) {
       for (int i=0; i<MAX_COUNT; i++) {
            tone_ = melody_zb[i];
            beat = beats_zb[i];
            duration = beat * tempo; // Set up timing
            playTone(); 
            // A pause between notes...
            delayMicroseconds(pause);
            int l1 = digitalRead(rubyLED);
  
            if (l1 == HIGH) {
              digitalWrite(rubyLED, LOW);
            } else {
              digitalWrite(rubyLED, HIGH);
            }
         } 
       } else if (alert == AlertNode::GAS) { // res
       for (pos = startPos; pos <= 180; pos += 5) { // goes from startPos degrees to 180 degrees
                                                    // in steps of 5 degrees
         myservo.write(pos);               // tell servo to go to position in variable 'pos'
         delay(100);                       // waits 100ms for the servo to reach the position
       }
         delay(7000);      
         myservo.write(startPos);          // after several seconds, the motor will return to its initial position
       }
  // reset all the LEDs after an alert is triggered
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(rubyLED, LOW);
 }
}

void light_energy_saver(int photocellReading) {
  photocellReading = 1023 - photocellReading;
  if (photocellReading > high_LED_limit) {
    photocellReading = high_LED_limit;
   }
   
  if (photocellReading < low_LED_limit) {
    photocellReading = low_LED_limit;
   }
  rubyLEDbrightness = map(photocellReading, low_LED_limit, high_LED_limit, 30, 255);
  if (rubyLEDbrightness > 255 ) {
    rubyLEDbrightness = 255;
  } else if (rubyLEDbrightness < 0) {
    rubyLEDbrightness = 0;
  }
  analogWrite(rubyLED, rubyLEDbrightness);
 }

void cloths_advisor(float temp) {
  temp = temp / 5.0;
  
  if (temp < tempThreshold) {
      digitalWrite(led1, LOW);
      digitalWrite(led2, HIGH);
   } else {
       digitalWrite(led2, LOW);
      digitalWrite(led1, HIGH);
   }
}

void loop() {
  // this part deal with the cloths advisor
  temp = analogRead(tempPin); 
  cloths_advisor(temp);
  
  // this part deal with the light energy saver logic
  photocellReading = analogRead(photocellPin);  
  light_energy_saver(photocellReading);

  // deal with different incoming alerts
  int alert = myNode.alertReceived();
  respond_to_alerts(alert);
}

