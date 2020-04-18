// Pin definition
#define PIN_TEMP A0                         // Set temperature sensor pin
// button pins
#define PIN_CALI A1                         // Set Calibration Pin
#define PIN_TRIGGER A2                      // Set Trigger Pin

// if the board is leonardo (pro micro)
#if defined(ARDUINO_AVR_LEONARDO) 
  // Pin definition
  #define PIN_RECOIL 7                        // Set recoil command pin
  #define PIN_RUMBLE 5                        // Set rumble command pin
  
  #define PIN_A A3                            // Set A button (reload) Pin
  #define PIN_B A6                            // Set B button (alt) Pin
  #define PIN_SELECT A8                       // Set Select Pin
  // Dpad pins (only activated in GCON2 mode)
  #define PIN_UP 15                           // Set Dpad Up Pin
  #define PIN_DOWN 14                         // Set Dpad Down Pin
  #define PIN_LEFT 16                         // Set Dpad Left Pin
  #define PIN_RIGHT A10                       // Set Dpad Right Pin
  
  #define PIN_PEDAL_D A7                        // Set Pedal Pin for dpad mode
  #define PIN_START_D A9                        // Set Start Pin for dpad mode
  #define PIN_PEDAL_L 15                        // Set Pedal Pin for led mode
  #define PIN_START_L 14                        // Set Start Pin for led mode
  
  #define PIN_LED_R A7                         // Red LED Pin *resistor 270o
  #define PIN_LED_G A9                         // Green LED Pin *resistor 470o
  #define PIN_LED_B A10                        // Blue LED Pin *resistor 220o

// else if the board is micro (beetle)
#elif defined(ARDUINO_AVR_MICRO)
  // Pin definition
  #define PIN_RECOIL 16                        // Set recoil command pin
  #define PIN_RUMBLE 11                        // Set rumble command pin
  
  #define PIN_A 14                            // Set A button (reload) Pin
  #define PIN_B 15                            // Set B button (alt) Pin
  
  #define PIN_LED_R A8                          // Red LED Pin *resistor 270o
  #define PIN_LED_G A10                         // Green LED Pin *resistor 470o
  #define PIN_LED_B A11                         // Blue LED Pin *resistor 220o
  
#endif

// led mode: false for dpad, true for rgb led
const bool ledMode = false;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600); 

  // pins initialization
  pinMode(PIN_CALI, INPUT_PULLUP);
  pinMode(PIN_TRIGGER, INPUT_PULLUP);
  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);

  // if the board is leonardo (pro micro)
  #if defined(ARDUINO_AVR_LEONARDO) 
  
  pinMode(PIN_SELECT, INPUT_PULLUP);
  if ( !ledMode ){
    pinMode(PIN_PEDAL_D, INPUT_PULLUP);
    pinMode(PIN_START_D, INPUT_PULLUP);
    pinMode(PIN_UP, INPUT_PULLUP);
    pinMode(PIN_DOWN, INPUT_PULLUP);
    pinMode(PIN_LEFT, INPUT_PULLUP);
    pinMode(PIN_RIGHT, INPUT_PULLUP);
  } else {
    pinMode(PIN_PEDAL_L, INPUT_PULLUP);
    pinMode(PIN_START_L, INPUT_PULLUP);   
  }
  
  #endif
  
}

void loop() {
  int reading = analogRead(PIN_TEMP);

  float voltage = reading * 0.004882814;

  float temperatureC = (voltage - 0.5) * 100 ;

  if ( !digitalRead(PIN_CALI) ) Serial.print( " Calibration pressed. " );
  if ( !digitalRead(PIN_TRIGGER) ) Serial.print( " Trigger pressed. " );
  if ( !digitalRead(PIN_A) ) Serial.print( " A pressed. " );
  if ( !digitalRead(PIN_B) ) Serial.print( " B pressed. " );
  
  #if defined(ARDUINO_AVR_LEONARDO) 
  
  if ( !digitalRead(PIN_SELECT) ) Serial.print( " Select pressed. " );
  
  if ( !ledMode ){
    if ( !digitalRead(PIN_PEDAL_D) ) Serial.print( " Pedal pressed. " );
    if ( !digitalRead(PIN_START_D) ) Serial.print( " Start pressed. " );
    if ( !digitalRead(PIN_UP) ) Serial.print( " Up pressed. " );
    if ( !digitalRead(PIN_DOWN) ) Serial.print( " Down pressed. " );
    if ( !digitalRead(PIN_LEFT) ) Serial.print( " Left oressed. " );
    if ( !digitalRead(PIN_RIGHT) ) Serial.print( " Right pressed. " );

  } else {
    if ( !digitalRead(PIN_PEDAL_L) ) Serial.print( " Pedal pressed. " );
    if ( !digitalRead(PIN_START_L) ) Serial.print( " Start pressed. " );
  }
  
  #endif
  
  Serial.print( "Temperature: " );
  Serial.print( temperatureC );
  Serial.println( " °C" );
  delay(500);
}
