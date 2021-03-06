int NUM_X=3;
int NUM_Y=3;
int  NUM_SPOT=NUM_X*NUM_Y;
int[] spots = new int[NUM_SPOT];

color BG=color(0, 0, 0);

int U=100; // scal unit

import spacebrew.*;
Spacebrew sb;
String server="spacebrew.marcobrianza.it";
String name="9ColorSpotsFriendP5";
String description ="Reading Colors";

String colorString="";
void setup() {
  size(U*NUM_X*2, U*NUM_Y*2);

  colorMode(RGB, 255);
  smooth();
  noStroke();
  ellipseMode(CENTER);

  frameRate(1);

  sb = new Spacebrew( this );
  sb.addSubscribe( "allSpots", "string" );
  sb.connect(server, name, description );  
  background(BG);
}

void draw() {
} 

void onStringMessage( String name, String value ) {
  println("got string message " + name + " : " + value);
  if (name.equals("allSpots")) {
    str2colori(value);
    drawSpots();
  }
}

// fills the color index array from the spacebrew string message
void str2colori(String colori) {
  String   [] col=split(colori, ',');
  for (int i=0; i< NUM_SPOT; i++) {
    spots[i]=int(col[i]);
  }
}

// transforms the color index in color RGB values assigning 4 bits per each color
color ci2c(int ci) {
  int r, g, b;
  r=ci & 0x000F;
  g=ci>>4 & 0x000F;
  b=ci>>8 & 0x000F;  
  return color(r*16, g*16, b*16);
}

// draws the color dots, from left to right, top to bottom
void drawSpots() {
  background(BG);
  for (int i=0; i<NUM_SPOT; i++) {
    int x=(i % NUM_X);
    int y=int(i/NUM_Y);
    int cx=x*U*2 + U;
    int cy=y*U*2+U;
    //println( x,y);
    fill(ci2c(spots[i]));
    ellipse(cx, cy, U, U);
  }
}
