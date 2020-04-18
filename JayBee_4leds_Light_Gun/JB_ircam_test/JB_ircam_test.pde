/*
    This is the accompanying Processing testing code for the 4 led lightgun system
    Coded by Jean-Baptiste Bongrand, April. 2020
*/


//===========================//
//===== User Settings =======//
//===========================//

// You can modify the
// following variables
// to match your setup



boolean fullscreen = true;       // true: enable fullscreen, false: disable fullscreen
boolean showFixedPoints = false;  // true: show the tilt corrected points behind the normal ones, false: hide them

// !! The following settings are optionals, change only if really needed !!
int comPort = 0;      // Your COM port number. Leave it to 0 for auto detection, change it to your port number for manual selection
int screenX = 1024;   // Horizontal resolution of the test window. Default: 1024
int screenY = 768;    // Vertical resolution of the test window. Default: 768



//===========================//
//===========================//

// DON'T MODIFY ANYTHING 
// AFTER THIS POINT !!

import processing.serial.*;

int lf = 10;    // Linefeed in ASCII
String myString = null;
Serial myPort;  // The serial port

float scale = 1.0;
float[] screenCoord = new float[4];

// declare variables to hold coordinates for four points, and initialize them with the value of 1023
int[] px = new int[10];
int[] py = new int[10];
int[] pid = new int[10];
int[] pSize = new int[10];
int[] sortP = new int[10];
int[] butState = new int[11];
String[] butLabels = { "CALI", "TRIGGER", "A", "B", "SELECT", "START", "PEDAL", "UP", "RIGHT", "DOWN", "LEFT" };
 
// declare variables to hold color for the points
/*color[] p1color = { color( 0, 0, 0 ) , color( 255, 0, 0 ) , color( 0, 255, 0 ) , color( 0, 0, 255 ) , color( 255, 0, 255 ) , color( 255, 255, 255 ) };
color[] p2color = { color( 120, 120, 120 ) , color( 255, 120, 120 ) , color( 120, 255, 120 ) , color( 120, 120, 255 ) , color( 255, 120, 255 ) };*/
color[] p1color = { color( 0, 0, 0 ) , color( 255, 0, 255 ) , color( 0, 255, 0 ) , color( 255, 0, 0 ) , color( 0, 0, 255 ) , color( 255, 255, 255 ) };
color[] p2color = { color( 120, 120, 120 ) , color( 255, 120, 255 ) , color( 120, 255, 120 ) , color( 255, 120, 120 ) , color( 120, 120, 255 ) };

void settings() {
  if (fullscreen) fullScreen();
  else size(screenX,screenY);
}
 
void setup() {
  
  // List all the available serial ports
  println(Serial.list());
  
  // get the COM port name
  String portNbr = "" ;
  if ( comPort == 0 ){
    portNbr = Serial.list()[0];
  } else {
    portNbr = "COM" + str(comPort);
  }
  
  // Open the port you are using at the rate you want:
  myPort = new Serial(this, portNbr, 9600);
  myPort.clear();
  
  for (int i = 0; i < 9; i++) {
    px[i] = 1023;
    py[i] = 1023;
    pid[i] = -1;
  }
  for (int i = 0; i < 11; i++) {
    butState[i] = 0;
  }
  
  if (fullscreen){
    screenX = width;
    screenY = height;
  }
  
  float screenRatio = float(screenX) / float(screenY);
  
  screenCoord[0] = 0;
  screenCoord[1] = screenX;
  screenCoord[2] = 0;
  screenCoord[3] = screenY;  
  
  scale = float(screenY) / 768;
  
  float sensorRatio = 1.333333 ;
  if ( screenRatio > sensorRatio ){
    screenCoord[0] = float(screenX) / 2 - ( float(screenY) * sensorRatio ) / 2 ;
    screenCoord[1] = float(screenX) / 2 + ( float(screenY) * sensorRatio ) / 2 ;
  } else if ( screenRatio < sensorRatio ) {
    screenCoord[2] = float(screenY) / 2 - ( float(screenX) / sensorRatio ) / 2 ;
    screenCoord[3] = float(screenY) / 2 + ( float(screenX) / sensorRatio ) / 2 ;
    scale = float(screenX) / 1024;
  } 
  
  textAlign(CENTER);
  
  frameRate(60);
  
  myPort.write(84);
}


 
void draw() {
  // preparation stage
  while( myPort.available() > 0 ) {
      myString = myPort.readStringUntil(lf);
     if( myString != null ) {
       convertmyStringToData();
     } 
  }
  
  for (int i = 0; i < 10; i++)sortP[i] = 1023;
  
  for (int i = 0 ; i < 5 ; i++) {
    if ( pid[i] >= 0 ) {
      sortP[ pid[i] *  2 ] = px[i];
      sortP[ pid[i] * 2 + 1 ] = py[i];
    } else {
      sortP[8] = 1023;
      sortP[9] = 1023;
    }
  }
  
  // drawing stage   
  background(0); // repaint the whole drawing area with dark grey color (77,77,77), making the whole window clear and clean
  
  rectMode(CORNERS);
  fill( 77 );
  rect(screenCoord[0], screenCoord[2], screenCoord[1], screenCoord[3]);
  //rect(300, 500, 1200, 800);
  
  for (int i = 0 ; i < 4 ; i++) {
    int j = i + 1 ;
    if ( j == 4 ) j = 0 ;
    stroke(0);
    drawLine( sortP[ i *  2 ], sortP[ i *  2 + 1 ], sortP[ j *  2 ], sortP[ j *  2 + 1 ] );
    stroke(255);
    drawLine( sortP[ i *  2 ], sortP[ i *  2 + 1 ], sortP[8], sortP[9] );
    stroke(0);
  }
  
  // immediately draw the circles after clearing, we've done the time-consuming preparation beforehand in convertmyStringToCoordinates() so this will give us minimal lag ( hopefully no flickering ).
  for (int i = 8; i >= 0 ; i--) {
    if ( i < 5 ) drawCircle( px[i], py[i], p1color[pid[i] + 1], pSize[i] );
    else if (showFixedPoints) drawCircle( px[i], py[i], p2color[pid[i - 5] + 1], pSize[i - 5] );
  }
  
    
  for (int i = 0; i < 11 ; i++) {
    if ( butState[i] > 0 ) fill( color( 0, 150, 0 ) );
    else fill( color( 150, 0, 0 ));
    textSize(height / 35);
    text( butLabels[i] , i * (height / 8.5) + screenCoord[0] + 50 , height - 40 ); 
  }
  
}  

void drawLine( int xval1, int yval1,  int xval2, int yval2 ){
  if( xval1 != 1023 && yval1 != 1023 && xval2 != 1023 && yval2 != 1023 ){
    line(xval1, yval1, xval2, yval2);
  }
}
    
void drawCircle( int xval, int yval, color c, int s ){ 
  if( xval != 1023 && yval != 1023 ){ // only draw when both x and y is not 1023. When x=1023 and y=1023, the point is NOT detected by the IR camera, i.e. out of range
    ellipseMode(RADIUS);  // Set ellipseMode to RADIUS
    fill( c );  // Set the fill color
    ellipse(xval, yval, (s + 1)  * 5 * scale , (s + 1) * 5 * scale); //draws an ellipse
  }
}



void convertmyStringToData() {
  println(myString); // display the incoming string

  // the next line does many things - it creates an array of integers named output, splits the string into 8 pieces of text, using comma as the delimiter, converts each of the 8 pieces of text into numbers and store them into the array in a sequential manner.  
  int[] output = int (split(trim(myString), ',')); 
  int sLength = output.length;
  if ( sLength == 41 ){
    
    
    // now we need to copy the values from the array into global variables p1x..p4y, and make them available outside of this procedure.
    // because we need to access them at the drawing stage, later in the draw() cycle
    for (int i = 0; i < 5; i++) {
      int outId = 6 * i;
      px[i] = int( map( output[outId] , 0, 1024, screenCoord[0], screenCoord[1] ) );
      py[i] = int( map( output[outId + 1] , 0, 768, screenCoord[3], screenCoord[2] )  );
      px[i + 5] = int( map( output[outId + 2] , 0, 1024, screenCoord[0], screenCoord[1] )  );
      py[i + 5] = int( map( output[outId + 3] , 0, 768, screenCoord[3], screenCoord[2] )  );
      pid[i] = output[outId + 4];
      pSize[i] = output[outId + 5];
    }
    
    for (int i = 0; i < 11 ; i++) butState[i] = output[i + 30];
  }
}
