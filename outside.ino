#include <NewPing.h>

/*
From perspective inside the room, the thresholds are as follows:
outside
[120][100][80]
[130][120][140]
inside
*/


// center SENSOR

#define TRIGGER_PIN 10

#define ECHO_PIN 9

#define MAX_DISTANCE 10000

#define weakThreshold 40 //95 for inside

#define strongThreshold 130

// CENTER

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
boolean FlagRaised = false;
int floorCount = 0;
int floorThresh = 5;


void setup() {
   Serial.begin(9600);
   
   noInterrupts();           // disable all interrupts
   TCCR1A = 0;
   TCCR1B = 0;

   TCNT1 = 34286;            // preload timer 65536-16MHz/256/2Hz
   TCCR1B |= (1 << CS12);    // 256 prescaler 
   TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
   interrupts();
} 

ISR(TIMER1_OVF_vect){
  //Serial.println("Timedout....kickstarting other side");
  TCNT1 = 34286;            // preload timer
  Serial.println("X");
}

void Detect(){
 
 //------------------------------------------------CENTRE------------------------------------

   unsigned int reading = sonar.ping_cm();
   //delay(200);
   //Serial.println(avg2);
   if (reading > 200){
	floorCount++;
   }
   if (reading < strongThreshold && reading > weakThreshold && FlagRaised != true && floorCount > floorThresh){
     Serial.println("E"); 
	 floorCount = 0;
     FlagRaised = true;    // Raise boolean indicating flag has been sent
   }
   if (reading > strongThreshold || reading < weakThreshold){
	  FlagRaised = false;
   }
   
   Serial.println("X");  // Indicate that the other Arduino can begin pinging
   TCNT1 = 34286;            // preload timer
   return;
}

void loop(){
}

// -----------------Serial event--------------------------------

void serialEvent(){
   while(Serial.available()>0){  // If anything is in on the serial buffer read it and process
     char SOP = Serial.read();
     if(SOP == 'Y'){        // Flag from the other sensor!
       Detect();      // Start pinging 
     }
     if(SOP == 'M'){
       FlagRaised = false;
     }
     
     if(SOP == 'T'){
       for(int i = 0; i < 100; i++){
         unsigned int x = sonar.ping_cm();
         delay(20);
         Serial.println(x);
       }
     }
   }
}   
