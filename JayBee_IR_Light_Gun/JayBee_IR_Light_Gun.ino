/*==============================================================================
=
=  Arduino Powered IR Light Gun
=
=  Based on an original code by Samco, rewritten and updated by JayBee.
=
=  Go to https://github.com/JayBee-git
=  and https://github.com/samuelballantyne for code & instructions.
=
=  created September 2019
=  by Jean-Baptiste Bongrand
=
=  This sample code is part of the public domain.
=
===============================================================================*/

#include <HID.h>                // Load libraries
#include <Wire.h>
#include <AbsMouse.h>
#include <Joystick.h>
#include <DFRobotIRPosition.h>
#include <EEPROM.h>


/*-----------------------------
-    Variables declaration    -
-----------------------------*/


//===== User Settings =======//
// You can modify the
// following variables
// to match your setup

int camPin = 5;                                     // Set camera power pin
int recoilPin = 7;                                  // Set recoil command pin

int caliPin = A1;                                   // Set Calibration Pin (change to A4 to use ALT Pin)
int leftPin = A2;                                   // Set Left Mouse Pin
int rightPin = A3;                                  // Set Right Mouse Pin
int middlePin = A6;                                 // Set Middle Mouse Pin (change to a free digital pin if using AtMega 32u4 board)

uint8_t buttonMouse[] = { 0, MOUSE_LEFT, MOUSE_RIGHT, MOUSE_MIDDLE };  // List the corresponding mouse commands in this order: caliPin, leftPin, rightPin, middlePin
int buttonJoy[] = { 0, 0, 1, 2 };                                      // List the corresponding joystick commands
int buttonHold[] = { 2000, 500, 1500, 1500 };                          // Set the hold time to trigger event

bool cursorFilter = false;                          // Enable/Disable filter
float filterRatio = 0.5;                            // Filter ratio: 0.0 ~ 1.0

int caliEEPROM = true;                              // Save calibration in EEPROM

float caliOffset[2] = {0.20,0.25};                  // Set calibration offset Screen ratio (0 is the corner of the screen, 0.5 is the middle)

int recoilHold = 35;                                // Set solenoid hold time
int recoilPause = 65;                               // Set solenoid pause between activations in full auto mode

int timeOut = 5;                                    // Timeout timer in minutes (for sleep mode)

bool axisSwap = false;                              // Axis swap (X,Y => Y,X)
bool axisReverse[2] = { false , false };            // Reverse axis (after swap)

int res_x = 1920;                                   // Put your screen resolution width here
int res_y = 1080;                                   // Put your screen resolution height here

bool joystickMode = false;                          // Set joystick mode


//==== Sketch Variables =====//
// Don't edit them

int xLeft = 0;                                      // Stored calibration points
int yTop = 0;
int xRight = 0;
int yBottom = 0;

int finalX = 0;                                     // Values after tilt correction
int finalY = 0;

int filterX[20];                                    // Previous coordinates for cursor filtering
int filterY[20];

int caliXOffset = 0;                                // Set calibration offset coordinates
int caliYOffset = 0;

int buttonPins[] = { caliPin, leftPin, rightPin, middlePin };   // Regroup the pins in a list
int buttonState[] = { HIGH, HIGH, HIGH, HIGH };                 // Button states
unsigned long buttonTimer[] = { 0, 0, 0, 0 };                   // Timer for button press

bool recoilOn = false;                              // Solenoid recoil current state (leave it off)
bool fullAutoOn = false;                            // Full auto current state (leave it off)
unsigned long  recHoldTimer = 0;                    // Timer for fullauto calculation

bool aimIn = false;                                 // Aiming the screen
unsigned long aimOutTimer = 0;                      // Set timeout timer
bool paused = false;                                // Pause status

DFRobotIRPosition myDFRobotIRPosition;              // Create IR Cam instance
Joystick_ myJoystick;                               // Create joystick instance

bool changingJMode = false;                         // Set joystick mode changing in progress


/*-----------------------------
-    Setup Start function     -
-----------------------------*/

void setup() {

  pinMode (camPin, OUTPUT);                         // Set the camera pin to output
  digitalWrite (camPin, HIGH);                      // Set the camera pin to ON
  pinMode (recoilPin, OUTPUT);                      // Set the recoil pin to output
  digitalWrite (recoilPin, LOW);                    // Set the recoil pin to OFF
  delay(500);
  
  Serial.begin(9600);                     // For debugging (make sure your serial monitor has the same baud rate)           

  for (int i = 0; i < (sizeof(buttonPins)/sizeof(buttonPins[0])); i++) pinMode(buttonPins[i], INPUT_PULLUP);  // Set pin modes   

  myDFRobotIRPosition.begin();            // Start IR Camera

  if (joystickMode) {                               // If joystick mode
    myJoystick.setXAxisRange(0, res_x);               // initialize joystick range
    myJoystick.setYAxisRange(0, res_y);
    myJoystick.begin();                               // Begin joystick emulation
  } else {                                          // Else if mouse mode
    AbsMouse.init(res_x, res_y);                      // Initialize mouse
    AbsMouse.move((res_x / 2), (res_y / 2));          // Set mouse position to centre of the screen
  }
  
  delay(500);

  if (caliEEPROM){                                // If option to save/load to EEPROM
    EEPROM.get(0, xLeft);                           // load calibration points from EEPROM
    EEPROM.get(5, yTop); 
    EEPROM.get(10, xRight);
    EEPROM.get(15, yBottom);    
    if ( xLeft == -1 && yTop == -1 && xRight == -1 && yBottom == -1 ) xLeft = yTop = xRight = yBottom = 0; // If no EEPROM value found, put them to 0
  }

  for ( int i = 0; i < (sizeof(filterX) / sizeof(filterX[0])); i++ ){     // Initialize the coordinate lists for filtering
    filterX[i] = 0;
    filterY[i] = 0;
  }
  
  caliXOffset = res_x * caliOffset[0];       // Calibration coordinates calculation
  caliYOffset = res_y * caliOffset[1];
}


/*-----------------------------
-     Main Loop Function      -
-----------------------------*/

void loop() {               // Main loop
  
  if ( ! paused ) {               // If the gun is not in sleep mode
    
    setRecoilState();             // Set the recoil current state (ON or OFF)
    getPosition();                // Get the Position for IR Cam
    
    if ( aimIn ) {            // If aiming inside the screen
      
      aimOutTimer = millis();     // Record last used time (for sleeping timeout)
      
      // Calculate the relative position with the offset. I did my own float map function since the normal map function is integer and not very precise
      int MoveXAxis = floatMap (finalX, xLeft, xRight, (res_x * caliOffset[0]), (res_x - (res_x * caliOffset[0])));        
      int MoveYAxis = floatMap (finalY, yTop, yBottom, (res_y * caliOffset[1]), (res_y - (res_y * caliOffset[1])));
      int conMoveXAxis = constrain (MoveXAxis, 0, res_x);
      int conMoveYAxis = constrain (MoveYAxis, 0, res_y);

      moveTo(conMoveXAxis, conMoveYAxis);  // Move the cursor/joystick to the corresponding point
      
    } else {                  // Else if in pause
      
      unsigned long curTime = millis();    // Get current execution time
      
      if ( ( curTime - aimOutTimer ) > ( timeOut * 60000 ) ){   // If the time passed since the last action is bigger than the timeout
        paused = true;        // pause the Gun
        //digitalWrite (camPin, LOW); 
        Serial.println("Pause on");
      }
      
    }
  }
  readButtons();

}


/*------------------------------------------------*/
/*-------------- CUSTOM FUNCTIONS ----------------*/
/*------------------------------------------------*/

/*-----------------------------
-    Get Position Function    -
-----------------------------*/
void getPosition() {        // Get tilt adjusted position from IR positioning camera

  myDFRobotIRPosition.requestPosition();

  if (myDFRobotIRPosition.available()) {  // If the camera is available
    
    int positionX[4];               // RAW Sensor Values
    int positionY[4];  

    int oneY = 0;                   // Re-mapped so left sensor is always read first
    int oneX = 0;
    int twoY = 0;
    int twoX = 0;
    
    for (int i = 0; i < 4; i++) {           // Get the values from sensor and swap/reverse it if needed
      if(axisSwap) {
        if ( axisReverse[0] ) positionX[i] = map (myDFRobotIRPosition.readY(i), 0, 1023, 1023, 0);
        else positionX[i] = myDFRobotIRPosition.readY(i);
        if ( axisReverse[1] ) positionY[i] = map (myDFRobotIRPosition.readX(i), 0, 1023, 1023, 0);
        else positionY[i] = myDFRobotIRPosition.readX(i);
      } else {
        if ( axisReverse[0] ) positionX[i] = map (myDFRobotIRPosition.readX(i), 0, 1023, 1023, 0);
        else positionX[i] = myDFRobotIRPosition.readX(i);
        if ( axisReverse[1] ) positionY[i] = map (myDFRobotIRPosition.readY(i), 0, 1023, 1023, 0);
        else positionY[i] = myDFRobotIRPosition.readY(i);
      }
    }

    // Always keep the left point on the left
    if ( positionX[0] > positionX[1] ) {
      oneY = positionY[0];
      oneX = positionX[0];
      twoY = positionY[1];
      twoX = positionX[1];
    } else if ( positionX[0] < positionX[1] ) {
      oneY = positionY[1];
      oneX = positionX[1];
      twoY = positionY[0];
      twoX = positionX[0];
    } else {
      oneY = 1023;
      oneX = 0;
      twoY = 1023;
      twoX = 0;
    }

    // If less than 2 points detected
    if ( (positionX[0] == 1023 && positionY[0] == 1023) || (positionX[1] == 1023 && positionY[1] == 1023) ) {      // If less than 2 points detected, freeze the position
      
      aimIn = false;    // Indicated the aiming is offscreen
      
    } else {       // Else calculate the new position 
      
      finalX = (int)(512.0 + cos(atan2(twoY - oneY, twoX - oneX) * -1.0) * (((oneX - twoX) / 2.0 + twoX) - 512.0) - sin(atan2(twoY - oneY, twoX - oneX) * -1.0) * (((oneY - twoY) / 2.0 + twoY) - 384.0) + 0.5);
      finalY = (int)(384.0 + sin(atan2(twoY - oneY, twoX - oneX) * -1.0) * (((oneX - twoX) / 2.0 + twoX) - 512.0) + cos(atan2(twoY - oneY, twoX - oneX) * -1.0) * (((oneY - twoY) / 2.0 + twoY) - 384.0) + 0.5);
      aimIn = true;     // Indicated the aiming is onscreen
      
    }

    // If cursor filtering is activated
    if ( cursorFilter ){
      
      // Reoder the arrays
      for ( int i = (sizeof(filterX) / sizeof(filterX[0]) - 1); i > 0; i-- ){
        filterX[i]= filterX[i-1];
        filterY[i]= filterY[i-1];
      }
      
      // Add last position to the arrays
      filterX[0]= finalX;
      filterY[0]= finalY;

      // initialize needed variables
      float totalCoef = 1;
      float curCoef = 1;
      float totalX = 0;
      float totalY = 0;

      // Calculate the non linear averaging filter
      for ( int i = 0; i < (sizeof(filterX) / sizeof(filterX[0])); i++ ){
        if ( filterX[i] != 0 && filterY[i] != 0 ) {
          totalX += filterX[i] * curCoef;
          totalY += filterY[i] * curCoef;
          totalCoef += curCoef;
        }
        curCoef *= filterRatio;
      }
      finalX = totalX / totalCoef;
      finalY = totalY / totalCoef;
      
    }
    
    /*Serial.print("RAW: ");
    Serial.print(finalX);
    Serial.print(", ");
    Serial.print(finalY);
    Serial.print("     Calibration: ");
    Serial.print(xLeft);
    Serial.print(", ");
    Serial.print(yTop);
    Serial.print(", ");
    Serial.print(xRight);
    Serial.print(", ");
    Serial.println(yBottom);*/
    
  } else {
    Serial.println("Device not available!");
  }
}

/*-----------------------------
-   Button Reading Function   -
-----------------------------*/
void readButtons() {

  // For each buttons
  for (int i = 0; i < (sizeof(buttonPins)/sizeof(buttonPins[0])); i++) {   
    int curState = digitalRead(buttonPins[i]);              // check the state   
    if (buttonState[i] != curState) {                 // If state changed     
      if ( ! paused ) {                                 // if not in sleep mode       
        buttonState[i] = curState;                        // save the last state      
        if (curState == LOW) {                              // If the button is pressed          
          buttonTimer[i] = millis();                          // Start the button timer
          pressButton(i);                                     // Run button press function     
        } else {      
          releaseButton(i);                                 // Else run button release function
        }
      } else {                                          // if in sleep mode, wake it up
        paused = false;
        /*digitalWrite (camPin, HIGH); 
        delay(500);
        myDFRobotIRPosition.begin();                    // Start IR Camera
        delay(500);
        aimOutTimer = millis();*/
        Serial.println("Pause off");      
      }
    } else if (curState == LOW) holdButton(i);          // Else run the button pressed function
  }          
  
}

/*-----------------------------
-     Aim Moving Function     -
-----------------------------*/
void moveTo(int X, int Y) {
  if (joystickMode) {                     // If in joyustick mode
    myJoystick.setXAxis(X);               // Set the joystick axises
    myJoystick.setYAxis(Y);
  } else {                                // Else
    AbsMouse.move(X, Y);                  // Set cursor coordinates
  }
}

/*-----------------------------
-    Button Press Function    -
-----------------------------*/
void pressButton(int buttonID){
  if ( buttonPins[buttonID] != caliPin ) {              // If not calibration button
    if (joystickMode) {                                   // If in joy mode
      myJoystick.pressButton(buttonJoy[buttonID]);          // Press joy button
    } else AbsMouse.press(buttonMouse[buttonID]);           // Else press mouse button
    if ( buttonPins[buttonID] == leftPin ){             // If left button pressed (Gun trigger)
      startRecoil();                                      // Start the recoil
    }
  }
}

/*-----------------------------
-   Button Release Function   -
-----------------------------*/
void releaseButton(int buttonID){
  if ( buttonPins[buttonID] != caliPin ) {              // If not calibration button
    if (joystickMode) {                                   // If in joy mode
      myJoystick.releaseButton(buttonJoy[buttonID]);        // Release joy button
    } else AbsMouse.release(buttonMouse[buttonID]);         // Else press mouse button
    if ( buttonPins[buttonID] == leftPin ) stopFullAuto(); // If left button pressed (Gun trigger) stop full auto mode
  } else {                                              // Else
    unsigned long curTime = millis();                     // Get current time
    if ( (curTime - buttonTimer[buttonID]) < buttonHold[buttonID] ) {   // If time passed < button hold timing
      startCalibration();                                               // Start calibration
    } else {                                              // Else
      changingJMode = false;                                // Tell the app it's not changing mode
    }
  }
}

/*-----------------------------
-     Button Hold Function    -
-----------------------------*/
void holdButton(int buttonID){
  unsigned long curTime = millis();                     // Get current time
  if ( (curTime - buttonTimer[buttonID]) > buttonHold[buttonID] ) {  // If time passed > button hold timing
    switch (buttonID) {          
      case 0:                                              // If calibration button hold
        if ( ! changingJMode ) {                             // if the mode is not changing already
          changingJMode = true;                               // change joystick mode
          changeJoyMode();
        }
        break;
      case 1:                                              // If left button (Gun trigger) hold
          startFullAuto();                                   // Start full auto mode
        break;
      default:
        // nothing
        break;
    }
  }
}

/*-----------------------------
-    Start Recoil Function    -
-----------------------------*/
void startRecoil(){
  recHoldTimer = millis();          // Reset solenoid hold timer
  if (aimIn) {                      // If aiming inside the screen
    digitalWrite (recoilPin, HIGH);   // Trigger the recoil ON
    recoilOn = true;                  // Tell the app it's ON
  } 
}

/*-----------------------------
-  Set Recoil State Function  -
-----------------------------*/
void setRecoilState(){\
  if ( recoilOn ){                          // If recoil on
    unsigned long curTime = millis();         // Get current time
    if ( (curTime - recHoldTimer) >= recoilHold ) {  // If time passed >= recoil hold timing
      digitalWrite (recoilPin, LOW);                  // Set the recoil OFF
      recoilOn = false;                               // Tell the app it's OFF
    }
  } else {                                  // Else
    if (fullAutoOn){                          // If in fullauto mode
      unsigned long curTime = millis();         // Get current time
      if ( (curTime - recHoldTimer) >= (recoilHold + recoilPause) ) { // If time passed >= recoil hold + pause timing
        startRecoil();                          // Start the recoil
      }
    }
  }
}

/*-----------------------------
-   Recoil Fullauto Functions  -
-----------------------------*/
void startFullAuto(){
  fullAutoOn = true;
}

void stopFullAuto(){
  fullAutoOn = false;
}

/*-----------------------------
-   Joystick Mode Function    -
-----------------------------*/
void changeJoyMode(){
  joystickMode = ! joystickMode;
  if (joystickMode) {
    myJoystick.setXAxisRange(0, res_x);
    myJoystick.setYAxisRange(0, res_y);
    myJoystick.begin();
  } else {
    myJoystick.setXAxis(res_x/2);
    myJoystick.setYAxis(res_y/2);
    myJoystick.end();
    AbsMouse.init(res_x, res_y); 
    AbsMouse.move((res_x / 2), (res_y / 2));
  }
}

/*-----------------------------
-    Calibration Function     -
-----------------------------*/
void startCalibration(){
  int caliStep = 1;                                         // Set the calibration step
  
  while ( caliStep != 4 ){                                  // While in calibration
    getPosition();
    int pressedButton = getCaliButton();
    
    switch (caliStep) {      
             
      case 1:                                              // If step 1
        moveTo((res_x / 2), (res_y / 2));
        if (pressedButton == 2){
          caliStep++;
        } else if (pressedButton == 1) caliStep = 4;
        break;
        
      case 2:                                              // If step 2
        moveTo(caliXOffset, caliYOffset);
        if (pressedButton == 2){
          if (aimIn) {
            xLeft = finalX;
            yTop = finalY;
      
            if (caliEEPROM){
              EEPROM.put(0, xLeft);               // save calibration points from EEPROM
              EEPROM.put(5, yTop);   
            }
          } else Serial.println("Aim out of range!");
          caliStep++;
        } else if (pressedButton == 1) caliStep = 4;
        break;
        
      case 3:                                              // If step 3
        moveTo((res_x - caliXOffset), (res_y - caliYOffset));
        if (pressedButton == 2){
          if (aimIn) {
            xRight = finalX;
            yBottom = finalY;
      
            if (caliEEPROM){
              EEPROM.put(10, xRight);             // save calibration points from EEPROM
              EEPROM.put(15, yBottom);   
            }
            delay(100);
          } else Serial.println("Aim out of range!");
          caliStep++;
        } else if (pressedButton == 1) caliStep = 4;
        break;
        
      default:
        caliStep = 4;
        break;
        
    }
  }
}

/*-----------------------------
-  Get Calibration Button Fn  -
-----------------------------*/
int getCaliButton(){
  int pressedButton = 0;
  for (int i = 0; i < 2; i++) {
    int curState = digitalRead(buttonPins[i]);
    if (buttonState[i] != curState) {
      if (curState == LOW) {
        pressedButton = i + 1 ;
        delay(75);
      } 
      delay(50);
      buttonState[i] = curState;
    }
  }
  return pressedButton;
}    

/*-----------------------------
-     Float Map Function      -
-----------------------------*/
int floatMap ( float srcValue, float srcMin, float srcMax, float tgtMin, float tgtMax ){
  int tgtValue = (int) ( ( ( srcValue - srcMin ) / (srcMax - srcMin) ) * ( tgtMax - tgtMin ) + tgtMin + 0.5 );
  return tgtValue;
}
