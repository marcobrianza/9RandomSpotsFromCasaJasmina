
//
// set true for casa Jasmina False for remote friends
//boolean const CASA=true; String Name="9ColorSpots"; 
boolean const CASA=false; String Name="9ColorSpotsFriend";


// LED data-----------------------------------------
byte colorStart=B11100000;
byte Brightness=31; //0..31 Global Brightness
byte globalBrightness=colorStart+Brightness; 

struct LED {
  byte r;
  byte g;
  byte b;
};

const int SPOTS=9;
const int N_LEDS=4; const int LEDS_SIDE=14;


LED spots[SPOTS];
int colori[SPOTS];

int PANEL_LEDS=LEDS_SIDE*LEDS_SIDE;

int pixels[SPOTS][4]={
{ 15,  16,  39,  40},
{95, 96, 99, 100},
{155, 156, 179, 180},
{ 20,  21,  34,  35},
{90, 91, 104, 105},
{160, 161, 174, 175},
{ 25,  26,  29,  30 },
{85, 86, 109, 110},
{165, 166, 169, 170},
};

int LED_CLOCK_PIN = 9;
int LED_DATA_PIN = 8;

// geiger counter data -----------------------------
int GEIGER_SIG_PIN = 2; 
int signCount=0;  //Counter for Radiation Pulse
int sON=0;//Lock flag for Radiation Pulse

long MAX_C=16;
long MAX_P=9;

long MAX_CI=MAX_C*MAX_C*MAX_C;
long MAX_NUM=MAX_CI*MAX_P;

unsigned long loops;

// spaceBrew---------------------------
#include <Bridge.h>
#include <SpacebrewYun.h>

// create a variable of type SpacebrewYun and initialize it with the constructor
SpacebrewYun sb = SpacebrewYun(Name, "from casa Jasmina");

void setup() {

  
  Serial.begin(115200);
  pinMode(LED_CLOCK_PIN, OUTPUT);
  pinMode(LED_DATA_PIN, OUTPUT); 
  
 sendColor(PANEL_LEDS,0,0,0);
 
 // start-up the bridge
Bridge.begin();
sb.verbose(false); // configure the spacebrew object to print status messages to serial

 
 if (CASA){
 Serial.println("Casa mode");  
 //sb.addPublish("spot", "string"); // configure the spacebrew publisher and subscriber
 sb.addPublish("allSpots", "string"); // configure the spacebrew publisher and 
 pinMode(GEIGER_SIG_PIN,INPUT_PULLUP);
 }
 else{
   Serial.println("Friend mode");
   sb.addSubscribe("allSpots", "string");
   	// register the string message handler method 
   sb.onStringMessage(handleString);
 }
 
 sb.connect("net.marcobrianza.it"); // connect to cloud spacebrew server
delay (1000);
sb.monitor();

}


void loop() {
 
  if (CASA){
    long rn=checkRadiation();
    if (rn>0) {
     rnd2color(rn);    
     drawDots();
     sendSpots(); 
    }
 ;
  }
  else{
    delay(100);
    sb.monitor();
  }
   
}

//---- Random number generation ------------------------------

 long checkRadiation(){
   loops++;
   long r=-1;
   
  // Raw data of Radiation Pulse: Not-detected -> High, Detected -> Low
  byte sign = digitalRead(GEIGER_SIG_PIN);

  //Radiation Pulse normally keeps low for about 100[usec]
  if(sign==0 && sON==0)
    {//Deactivate Radiation Pulse counting for a while
      sON = 1;
  
      Serial.print("Seconds=");       Serial.print(millis()/1000);

      signCount++;
      Serial.print(" Count=");   Serial.print(signCount);
      long g=loops/MAX_NUM;
      Serial.print(" Turns=");       Serial.print(g);
     Serial.println();     
          
      r=loops%MAX_NUM;
      Serial.print(" Random=");    Serial.print(r);
      
    }
  else if(sign==1 && sON==1){
    sON = 0;
  }
  return(r); 

}



//--------- Space Brew------------



void sendSpots(){
   if ( sb.connected() ){

    //send all color codes
    String cs="";
    for (int i=0; i<SPOTS; i++){
     cs=cs+String(colori[i])+",";

    }
     Serial.println("sending allSpots: "+cs);
     sb.send("allSpots", cs); 
     
   }

   
}

void handleString (String route, String value) {
	// print the message that was received
	Serial.print("From ");
	Serial.print(route);
	Serial.print(", received msg: ");
	Serial.println(value);
Serial.println();



if (route=="allSpots"){
  int num;
  String temp;
  int j=0;
  
 Serial.println("new allSpot");
 for(int i=0; i<value.length();i++){
  char c=value[i];
  if (c==',')
  {
    num=temp.toInt();
    //Serial.println(num);
    addSpot(j,num);
    temp="";
    j++;
  }
  else{
    temp=temp+c;
  }
 }
 
 drawDots();
}


}
//-------------- Color Stuff --------------------


void rnd2color(unsigned long r){
  
  int P=r>>12; // number of the spot
  unsigned int C=r & 0x0FFF; //color index
   addSpot(P,C);
}


void addSpot(int p, unsigned int c){
  colori[p]=c;
  
  byte R=c    & B00001111; // red 
  byte G=c>>4 & B00001111; //green
  byte B=c>>8 & B00001111;  //blue

  
  spots[p].r=cal8(R);
  spots[p].g=cal8(G);
  spots[p].b=cal8(B);    

//     long CC=r/MAX_P;
//     byte PP=r/MAX_CI;
// 
//     byte RR=CC%MAX_C;
//     byte G=(CC/MAX_C)%MAX_C;
//     byte B=(CC/MAX_C*2)%MAX_C;

      Serial.print(" C=");  Serial.print(c);
      Serial.print(" P=");  Serial.print(p);
      Serial.print(" R=");  Serial.print(R);
      Serial.print(" G=");  Serial.print(G);
      Serial.print(" B=");  Serial.print(B);
      Serial.println();  

}



void drawDots(){
  int n=0;
  int l=0;
  
  APA102_Start();
  
  for (int i=0; i<PANEL_LEDS; i++){
   int f=-1;
   for (int s=0; s<SPOTS; s++){
     for( n=0; n<N_LEDS; n++){
       if(i==pixels[s][n]){
         f=l;
         //Serial.println(f);
         APA102_Color(globalBrightness,spots[s].b,spots[s].g,spots[s].r); 
          break;
         }
    }
   }
    
   if (f<0) 
       APA102_Color(globalBrightness,0,0,0);
   }
   APA102_Stop(PANEL_LEDS);
}

// calibrates to the color to a better human scale
unsigned int cal8(unsigned int rawVal) {

  float gammaL=3;
  float outRangeL=255;
  float inRange=MAX_C;
  float l; 
  float lC;

  //Serial.println("Calibrate");
  l=rawVal;  
  lC=pow((l/inRange), gammaL)*outRangeL;
  return((lC));
  

}    

//----------------------- APA102 LEDFunctions

// sets all the LEDs to the same color 
void sendColor(int l, byte r, byte g, byte b){
 APA102_Start(); 
  for (int i=0; i<l; i++){
   APA102_Color(globalBrightness,r,g,b);
  }

APA102_Stop(l);

}

// start frame
void APA102_Start(){  
  shiftOut(LED_DATA_PIN, LED_CLOCK_PIN, MSBFIRST, 0);
  shiftOut(LED_DATA_PIN, LED_CLOCK_PIN, MSBFIRST, 0);
  shiftOut(LED_DATA_PIN, LED_CLOCK_PIN, MSBFIRST, 0);
  shiftOut(LED_DATA_PIN, LED_CLOCK_PIN, MSBFIRST, 0);  
  
}

//data color frame
void APA102_Color(byte gb, byte r, byte g, byte b){   
    shiftOut(LED_DATA_PIN, LED_CLOCK_PIN, MSBFIRST, gb);
    shiftOut(LED_DATA_PIN, LED_CLOCK_PIN, MSBFIRST, r);
    shiftOut(LED_DATA_PIN, LED_CLOCK_PIN, MSBFIRST, g);
    shiftOut(LED_DATA_PIN, LED_CLOCK_PIN, MSBFIRST, b);      
   
}

//stop frames
void APA102_Stop(int n){
for(int e=0; e<n; e+=16){
    shiftOut(LED_DATA_PIN, LED_CLOCK_PIN, MSBFIRST, B11111111);
  }

}



