/*
 MSGEQ7-MOSFET Demo app
 Look for the breakout board on www.whizoo.com and eBay
 
 This code runs on an Arduino Duemilanove, but will run on other Arduino models.  It will blink the LEDs in time to the music.  It
 Auto-adapt to volume changes so soft songs are as responsive as loud songs.
 
 Arduino Connections:
 - GND to GND on MSGEQ7-MOSFET breakout board
 - 5V to VDD on MSGEQ7-MOSFET breakout board
 - A0 to OUT on MSGEQ7-MOSFET breakout board
 - D7 to STROBE on MSGEQ7-MOSFET breakout board
 - D8 to RESET on MSGEQ7-MOSFET breakout board
 - D9 to S1 on MSGEQ7-MOSFET breakout board (this pin should support PWM). This will control the red LED.
 - D10 to S2 on MSGEQ7-MOSFET breakout board (this pin should support PWM). This will control the green LED.
 - D11 to S3 on MSGEQ7-MOSFET breakout board (this pin should support PWM). This will control the blue LED.

RGB LED and Power connections:
- 12V (2A or more) power supply to INPUT on MSGEQ7-MOSFET breakout board (wire both '+' and '-')
- RGB LED string to OUTPUT on MSGEQ7-MOSFET breakout board (wire '+', then R to '1', G to '2' and B to '3')

*/

// Hardware-specific defines (define which pin has what function)
#define MSGEQ7_STROBE_PIN      7
#define MSGEQ7_RESET_PIN       8
#define MSGEQ7_ANALOG_PIN      A0
#define RED_LED                9
#define GREEN_LED              10
#define BLUE_LED               11

// The MSGEQ7 divides the audio into 7 bands (but this example doesn't use the highest frequency band)
#define NUM_FREQUENCY_BANDS    7

// Global variables
int volumeInBand[NUM_FREQUENCY_BANDS];
int minimumVolume, maximumVolume;


void setup() {
  // Set the LED pins as outputs
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  
  // Set up the MSGEQ7 IC
  pinMode(MSGEQ7_ANALOG_PIN, INPUT);
  pinMode(MSGEQ7_STROBE_PIN, OUTPUT);
  pinMode(MSGEQ7_RESET_PIN, OUTPUT);
  digitalWrite(MSGEQ7_RESET_PIN, LOW);
  digitalWrite(MSGEQ7_STROBE_PIN, HIGH);

  // Initialize the minimum and maximum volume levels
  minimumVolume = 1023;
  maximumVolume = 0;
}


// This loop executes around 100 times per second
void loop() {
  static unsigned long nextLoopTime = 50; 
  int cutoffVolume;
  
  // Toggle the RESET pin of the MSGEQ7 to start reading from the lowest frequency band
  digitalWrite(MSGEQ7_RESET_PIN, HIGH);
  digitalWrite(MSGEQ7_RESET_PIN, LOW);
  
  // Read the volume in every frequency band from the MSGEQ7
  for (int i=0; i<NUM_FREQUENCY_BANDS; i++) {
    digitalWrite(MSGEQ7_STROBE_PIN, LOW);
    delayMicroseconds(30); // Allow the output to settle
    volumeInBand[i] = analogRead(MSGEQ7_ANALOG_PIN);
    autoAdjustMinMax(volumeInBand[i]);
    digitalWrite(MSGEQ7_STROBE_PIN, HIGH);
  }
  
  // A volume greater than 35% should turn the LED on
  cutoffVolume = map(35, 0, 100, minimumVolume, maximumVolume);
  
  // Turn the RED LED on if the volume in the lower frequency bands is high enough
  if (volumeInBand[0] > cutoffVolume || volumeInBand[1] > cutoffVolume)
    analogWrite(RED_LED, map(max(volumeInBand[0], volumeInBand[1]), cutoffVolume, maximumVolume, 0, 255));
  else
    analogWrite(RED_LED, 0);
    
  // Turn the GREEN LED on if the volume in the middle frequency bands is high enough
  if (volumeInBand[2] > cutoffVolume || volumeInBand[3] > cutoffVolume)
    analogWrite(GREEN_LED, map(max(volumeInBand[2], volumeInBand[3]), cutoffVolume, maximumVolume, 0, 255));
  else
    analogWrite(GREEN_LED, 0);
 
  // Turn the BLUE LED on if the volume in the lower frequency bands is high enough
  if (volumeInBand[4] > cutoffVolume || volumeInBand[5] > cutoffVolume)
    analogWrite(BLUE_LED, map(max(volumeInBand[4], volumeInBand[5]), cutoffVolume, maximumVolume, 0, 255));
  else
    analogWrite(BLUE_LED, 0);
   
  // Execute this loop 20 times per second (every 50ms)
  if (millis() < nextLoopTime)
    delay(nextLoopTime - millis());
  nextLoopTime += 50;
}


// Automatically adjust the high and low volume.  This routine is called 20 times per
// second for each band, or 20 * 7 bands = 140 times per second.
void autoAdjustMinMax(int volume) {
  static int increaseMin = 0;
  static int decreaseMax = 0;

  // Adjust the minimum volume level
  if (volume < minimumVolume)
    minimumVolume = volume;
  else {
    // Move the minimum volume level upwards (1 per second)
    if (++increaseMin > 140) {
      increaseMin = 0;
      minimumVolume++;
    }
  }
  
  // Adjust the maximum volume level
  if (volume > maximumVolume)
    maximumVolume = volume;
  else {
    // Move the maximum volume level downwards (5 per second)
    if (++decreaseMax > 28) {
      decreaseMax = 0;
      // Keep a reasonable level of separation between max and min
      if (maximumVolume - minimumVolume > 100)
        maximumVolume--;
    }
  }
}


