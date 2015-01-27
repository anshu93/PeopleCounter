#include <NewPing.h>
#define timeOut 3;

//----------------------------PING SENSOR------------------------------

#define TRIGGER_PIN 10

#define ECHO_PIN 9

#define MAX_DISTANCE 10000

#define weakThreshold 80 //120 for inside

#define TALK_TIME 40


//------------------------VARIABLES FOR SENSOR--------------------

//PARAMS
int peopleCount = 0;     //number of people seen
int floorCount = 0;      //number of times the floor is seen
int floorThresh = 5;     //threshold for number of times needing to see floor before lowering flag.
int flagMatrix[2][1];
int timerMatrix[2][1];

int num_in = 0;       //Number of people who have come in
int num_in_VR = 0;
int num_out = 0;      // Number of people who have gone out
int counter = TALK_TIME;     // Space sending signals by these many seconds

//center
NewPing sonar2(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
boolean FlagRaised = false; 

void setup() {
  //----------------------SETUP PING------------------------
  /*
  Matrix looks like
  outside
  [D][E][F]
  [A][B][C]
  inside
  */
   //set up serial
   Serial.begin(9600);
   for(int i = 0; i < 3; i++){
     Serial.println("Y");
   }
   
   // Hardware timer is used to kill values stored in the matrix
   
   noInterrupts();           // disable all interrupts
   TCCR1A = 0;
   TCCR1B = 0;

   TCNT1 = 34286;            // preload timer 65536-16MHz/256/2Hz
   TCCR1B |= (1 << CS12);    // 256 prescaler 
   TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
   interrupts();
   
   //clear flagMatrix -- clean start
   flagMatrix[0][0] = 0;
   timerMatrix[0][0] = 0;
   flagMatrix[1][0] = 0;
   timerMatrix[1][0] = 0;
 }
 
void clearMatrices(){
   flagMatrix[0][0] = 0;
   timerMatrix[0][0] = 0;
   flagMatrix[1][0] = 0;
   timerMatrix[1][0] = 0;
   FlagRaised = false;    // Tell inside sensor he can detect again
   Serial.println("M");    // Tell outside sensor he can detect again
 }
 
 ISR(TIMER1_OVF_vect)        // interrupt service routine that wraps a user defined function supplied by attachInterrupt
{
  decrementTimer();
  if(counter > 0){
    counter--;
    if(counter == 0){
      Serial.println("Counter is 0...");
      while(num_out > 0){ 
        Serial.println("<");
        num_out--;
      }
      if(num_in_VR > 0){
        num_in_VR--;
        if(num_in > 0){
          num_in_VR++;
          num_in--;
          Serial.println(">");
          counter = TALK_TIME;
        }
      }  
    }
  }
}


void serialEvent(){
 while(Serial.available()>0){
       
       char SOP = Serial.read();
       
       if(SOP == 'X'){
         //Serial.println("Got an X");
         Detect();
       }
       
       if(SOP == 'E'){
         Serial.println("Received an E");
         checkAround(1,0);                //checkAround(int row, int col, int strength)
       }
       
       if(SOP == '/'){
         for(int i = 0; i < 100; i++){
           unsigned int x = sonar2.ping_cm();
           delay(20);
           Serial.println(x);
         }
       }

       
//------------------[DEBUG]-------------------
       if(SOP == 'B'){
         Serial.println("Received a B");
         checkAround(0,0);                //checkAround(int row, int col, int strength)
       }
   } 
}


void pairOff(int row, int col){
  Serial.println(" in Pair Off" );
  // set floorCount to zero in a pairOff.
  floorCount = 0;
  // Assuming row 0 is side in room, then when we get here we should be incrementing peopleCount
  // if row == 0. If row == 1, we decrement. In all cases, squash (opp row, col) val and timer.
  // Conditionally do the same to the (opp row, middleCol)
  if (row == 0 ){
     peopleCount++;    // Increment no. of people
     if(num_in_VR == 0){
       Serial.println(">");
       num_in_VR++;
       counter = TALK_TIME; 
     }
     else if(num_in_VR > 0){
       num_in++;  
     }
     //Serial.println(">"); // Let the wifi shield know that the number of people has gone up
//     num_in++;
//     counter = TALK_TIME;
  }
  if (row == 1 ){
     peopleCount--; 
     if(num_in_VR > 0){
       num_out++;
     }else{
       Serial.println("<");  
     }
//     counter = TALK_TIME;
//     Serial.println("<");  // Let the wifi shield know that the number of people has gone down
  }
  clearMatrices();
  Serial.print("PeopleCount: ");
  Serial.println(peopleCount);
  delay(200);
//  digitalWrite(13,LOW);
//  digitalWrite(6,LOW);
  
  //printMatrix();
  return; 
}

void checkAround(int row, int col){
 /* 0,0 = A ; 1,0 = D ...
  outside 
  [D][E][F]
  [A][B][C]
  inside 
  int to access opposite row. Specific to 2 row system
*/
 int oppRow = (row + 1) % 2; 
   
   // If inserting strong, first check if opposite row is strong. If so, pair off. 
   if (flagMatrix[oppRow][0] == 2){  // Check if middle is strong
     pairOff(row,col);
     return;
   }
 
 // Actually insert the value into the matrix  
 
 flagMatrix[row][col] = 2;           // Instert into matrix because it didn't hit any pair off conditions
 timerMatrix[row][col]= timeOut;              // setting timer for kill
 printMatrix(); 
 return;
}

// a helpful debugging tool to print out our flagMatrix and timerMatrix
void printMatrix(){
  Serial.print("[");
  Serial.print(flagMatrix[1][0]);
  Serial.println("]");
  Serial.print("[");
  Serial.print(flagMatrix[0][0]);
  Serial.println("]");
}

void printTimerMatrix(){
  Serial.println("Timer: ");
  Serial.print("[");
  Serial.print(timerMatrix[1][0]);
  Serial.println("]");
  Serial.print("[");
  Serial.print(timerMatrix[0][0]);
  Serial.println("]");
}

// iterates through our timerMatrix (down columns then across rows) checking to see if a timer needs to decrement. 
// if a timer hits zero the value is squashed, the FlagRaised boolean is reset, and the user is notified. 
void decrementTimer(){
 for (int i = 0; i < 2; i++){
     if(timerMatrix[i][0] != 0){
       timerMatrix[i][0]--;
       if(timerMatrix[i][0] == 0){
         flagMatrix[i][0] = 0;
         if (i == 0){
            FlagRaised = false; 
         }       
         if (i == 1){
            Serial.println("?");
         }       
         Serial.println("Killed value");
       }
     }
 }  
 //printTimerMatrix();
 TCNT1 = 34286;            // preload timer 65536-16MHz/256/2Hz
}

void Detect(){

   //---------------------------------------- CENTER ---------------------------------------------------
   
   unsigned int reading = sonar2.ping_cm(); 
   //delay(100);
   //Serial.println(reading);
   
   // if you see the floor increment up;
   if(reading > 200){
	floorCount++;
   }
   
   if(reading < weakThreshold && FlagRaised != true){    // If seen enough indications to throw a flag    
     Serial.println("B");                                                          // And if the floor has been seen in between readings
     Serial.println(reading);
     checkAround(0,0);                // Try inserting an A
     FlagRaised = true;               // Prevent from sending multiple flags for the same person
   }
   
   if(reading > weakThreshold && floorCount > floorThresh){
      FlagRaised = false; 
      floorCount = 0;
   }
   
   Serial.println("Y");
   return;
}

void loop(){  
}




