
/*
 Reads encoder position and distance sensors 
 to capature data about grass biomass via grass sward density measurements.
 Code for Arduino Leanardo with SD shield and sensor shield V4
 
 
 Written by John Carter 
 
 Data is collected (in 20, 'n_pos' angle bins) while encoder swich is pressed
 Direction (which position) does not really matter, just binning up readings to make sure a representative sample
 as could record endlessly in just one direction giving a false impression of sward (biomass) density.
 
 When button is released data is written to SD, and serial (potentially calibration calculations done) 
 Code needs lots of clean up and further testing 
 Needs some leds / buzzer to show state for later on
 (1) Sampling but not enough samples (flashing green)
 (2) Sampling in current position is adequate (buzz?) (solid green)
 (3) Data is stored / not currently measuring but site is inadequately sampled (flashing orange)
 (4) Site sampling running mean is stable and > 10 sample sites (orange + gereen solid)
 (5) Site data completed written to file (double click ??) ready for next paddock/treatment
 
 Sensors so far: 
 Encoder with switch for position KY-040 (20 positions)
 Distance sensors (4 planned) E18-D80NK-N Adjustible Infrared Sensor  
 
 
 */
// include the SD library:
#include <SPI.h>
#include <SD.h>
// set up variables using the SD utility library functions:
//Sd2Card card;
//SdVolume volume;
//SdFile root;
File OutFile;
// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
const int chipSelect = 4;    


// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  Serial.begin(9600); // Open serial monitor 
  delay(500);
  Serial.println("connected");  
  delay(500);
  pinMode(SS, OUTPUT);
  
  Serial.println("\nInitializing SD card...");
  delay(500);
  
  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  SD.begin(4);
  delay(1500);
  Serial.println("SD card initialised");  
  delay(2500);  
 
  pinMode(led, OUTPUT);  
  pinMode(2, INPUT);         //encoder channel 1 SW
  pinMode(3, INPUT);         //encoder channel 2 DT
  pinMode(7, INPUT_PULLUP);  //encoder integrated swich SW
}

// main area for declaring variables and initialising
 int dist1;         // stores state of analog read low = nearby, high values further than threshold sort of binary
 int dist2;
 int recording;      // still acquiring data to fill rotations at this point
 int p1;             //  encoder position indicator for change
 int p2;             //  encoder position indicator direction change
 int last_p1 = LOW;  //  encoder last position indicator
 int encoder_pos;    // which on of the n_pos positions am i in 
 int done;           // 
 int placements = 0; // How many locations have been measured (n > =30 for good stats)
 int adequate = 0;   // stores how mych of 360 degree circle samoled (n_pos positions)
 int n_pos = 19;     // number of encoder posotions -1 
 float pcnt_hits;    // percentage of "hits" / sample
 bool  debug;        // turn on debugging to serial line to PC
 
 // arrays to hold count of positions sampled and hits at each of 4 heights 
 int counts[] =  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 int height1[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
 int height2[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
 int height3[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
 int height4[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
 
// the MAIN loop routine runs over and over again forever! at this stage:

void loop() {
  recording = digitalRead(7); // on/off
  
  if (recording == 0){
     digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
     Serial.print("Recording = ");
     Serial.println(recording);
     done = 0; // for storing or uploading to pc
     last_p1 = p1;
     p1   = digitalRead(2);
     p2   = digitalRead(3);
     
//////////////////// DIAGNOSTIC /////////////////////     
  Serial.print(" P1 = ");
  Serial.print (p1);
  Serial.print(" P1_Last ");
  Serial.print (last_p1);
  Serial.print (" P2 = ");
  Serial.print (p2);
  Serial.println(" ");
  delay(10);               // wait for a second UNITS are miliseconds
  
//////////////////// DIAGNOSTIC /////////////////////     
    
     delay(2);
     if((last_p1 == LOW) && (p1 == HIGH)){
       if (p2 == LOW) {
         encoder_pos --;
         if(encoder_pos < 0) {
           encoder_pos = n_pos;
         } // loop counter around so is always in range 1-20 (0-19 actually)
      } else { // not backwards but forwards rotation
         encoder_pos ++; 
         if(encoder_pos > n_pos) {
           encoder_pos = 0;
         } // loop counter around so is always in range 1-20 (0-19 actually)
      }
      }  //  a change has happened in encoder position
     Serial.print("Encoder Position = "); 
     Serial.println(encoder_pos);
     dist1 = analogRead(0);
     dist2 = analogRead(1);
//     dist1 = 55;
     Serial.print ("Distance Sensor 1  = ");
     Serial.println( dist1);
     if( dist1 > 50) {
        height1[encoder_pos] = height1[encoder_pos] + 1; 
     }
     if( dist2 > 50) {
        height2[encoder_pos] = height2[encoder_pos] + 1; 
     }
     
     counts[encoder_pos] = counts[encoder_pos] + 1;
     
     for (int n=0; n <= n_pos;n++){  // printout data count and hits for 0-19 positions
       if(counts[n] > 1) {
         adequate = adequate + 1;
       }
     }
    
    if(adequate > n_pos + 1) {
//   BUZZ OR CHANGE A LED HERE   // adequately sampled all bins/segments/encoder_positions have data
     Serial.print("POINT SAMPLE ADEQUATE  ");
    } 
     
    }  // end of loop for resording switch is on
    
  if (recording == 1 && done == 0)
    {digitalWrite(led, LOW);   // turn the LED off  
     placements = placements + 1;    
     Serial.print("ENDED COLLECTING POINT DATA PLACEMENT = ");
     Serial.println(placements);
    
     OutFile = SD.open("Grass1.txt", FILE_WRITE);
     Serial.print("opened data file:  ");
     Serial.println(OutFile);  
     OutFile.print(placements);
     OutFile.println(" th Placement"); 
     delay(1000);
     
     
     
     for (int n=0; n <= n_pos;n++){  // printout data count and hits for 0-19 positions
        Serial.print("POSN ");     
        Serial.print(n);
        Serial.print(" KNTS ");
        Serial.print(counts[n]);
        Serial.print(" HITS  ");         
        Serial.print(height1[n]); 
        Serial.print("  ");          
        Serial.println(height2[n]);  
        pcnt_hits = 0.0;
        if(counts[n] > 0) {
          pcnt_hits = height1[n] / counts[n];
        }
        OutFile.println(height2[n]); 
        
        counts[n]  = 0;
        height1[n] = 0;
        height2[n] = 0;        
     } //end loop through data for N encoder positions   
        OutFile.close();     
        Serial.println("File closed ENDED RETURN OF DATA  ");  
        delay(5000);    
        if (placements == 20) {
          Serial.println("Done 20 Collections ");  
          delay (20000);
        }  
      
        
        done = 1;        
     } // case not recording and output done
    
   }   // END OF LOOP STRUCTURE

