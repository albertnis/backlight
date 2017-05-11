//Lib
#include <IRremote.h>
#include <LiquidCrystal.h>

//LCD
LiquidCrystal lcd(8, 12, 3, 4, 6, 7);
byte on = 0b11111;
byte off = 0b00000;
byte musLevels[8][8] = {
  {off,off,off,off,off,off,off, on},
  {off,off,off,off,off,off, on, on},
  {off,off,off,off,off, on, on, on},
  {off,off,off,off, on, on, on, on},
  {off,off,off, on, on, on, on, on},
  {off,off, on, on, on, on, on, on},
  {off, on, on, on, on, on, on, on},
  {on, on, on, on, on, on, on, on}
};

//Color sensor
int S3 = 4;
int S2 = 3;
int tscIn = 2;
unsigned int pulseWidth;
int rStrength;
int gStrength;
int bStrength;
int cStrength;
int rStrengthPrev;
int gStrengthPrev;
int bStrengthPrev;
int cStrengthPrev;
long int lastColorDetect = 0;
int detectDelay = 1000;

//IR
int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results;
int pressed = 0;
int received = 0;

//System
int mode = 1; //0 - Off; 1 - Colour; 2 - Music; 3 - Strobe; 4 - Detector
int oldMode = 1;
int colourChanged = 0;
const String modeNames[5] = {"Off ", "Colour", "Music", "Strobe", "Colour detect"};
int brightness = 7; //0-7 inclusive
double bFac = brightness/7;
int curR = 0;
int curG = 0;
int curB = 0;
int aimR = 0;
int aimG = 0;
int aimB = 0;
int fromR = 0;
int fromB = 0;
int fromG = 0;

//Crossfade
long int lastStart = 0;
int fadeTime = 500;

//Strobe
const int strobeCount = 5;
const int strobeTimes[strobeCount] = {200, 100, 70, 40, 20};
int strobeInd = 4;
int gated = 0; //1 if temporarily suppressed;
long int lastChange = 0;

//Colour pins
int rpin = 9;
int bpin = 5;
int gpin = 10;

//Spectral Analysis
int strobeOut = A1;
int resetOut = A2;
int signalIn = A0;
int spectrum[7];
int cutoffs[7] = {150, 150, 140, 130, 120, 150, 150};

//Colour functions
void crossFade() {
  bFac = ((double)brightness)/((double)7);
  if (millis() - lastStart <= fadeTime) {
    curR = map(millis() - lastStart, 0, fadeTime, fromR, bFac*aimR);
    curG = map(millis() - lastStart, 0, fadeTime, fromG, bFac*aimG);
    curB = map(millis() - lastStart, 0, fadeTime, fromB, bFac*aimB);
  }
  else {
    curR = aimR*bFac;
    curG = aimG*bFac;
    curB = aimB*bFac;
  }
  
}

void outRGB(int r, int g, int b) {
  analogWrite(rpin, r);
  analogWrite(gpin, g);
  analogWrite(bpin, b);
}

void setRGBComp(int r, int g, int b) {
  lastStart = millis();
  fromR = curR;
  fromG = curG;
  fromB = curB;
  aimR = r;
  aimG = g;
  aimB = b;
}

void displayStat() {
  lcd.setCursor(0, 0);
  lcd.print(modeNames[mode]);
  int pos = modeNames[mode].length();
  if (mode == 3) {
    String space = " ";
    String disp = space + strobeTimes[strobeInd] + "ms";
    lcd.print(disp);
    pos = pos + disp.length();
    while (pos < 16) {
      lcd.print(" ");
      pos++;
    }
  }
  while (pos < 16) {
      lcd.print(" ");
      pos++;
    }
  lcd.setCursor(0, 1);
  if (mode == 1 || mode == 3 || mode == 4) {
    String bracket = "(";
    String disp = bracket + aimR + "," + aimG + "," + aimB + ")";
    int pos = disp.length();
    lcd.print(disp);
    while (pos < 15) {
      lcd.print(" ");
      pos++;
    }
    lcd.write((uint8_t)brightness);
  }
}

void fetchSensorData() {
    rStrengthPrev = rStrength;
    gStrengthPrev = gStrength;
    bStrengthPrev = bStrength;
    cStrengthPrev = cStrength;
    //RED
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    pulseWidth = pulseIn(tscIn, LOW);
    rStrength = pulseWidth/400. - 1;
    rStrength = 255 - rStrength;
    
    //GREEN
    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);
    pulseWidth = pulseIn(tscIn, LOW);
    gStrength = pulseWidth/400. - 1;
    gStrength = 255 - gStrength;
    
    //BLUE
    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH);
    pulseWidth = pulseIn(tscIn, LOW);
    bStrength = pulseWidth/400. - 1;
    bStrength = 255 - bStrength;
    
    //CLEAR
    digitalWrite(S2, HIGH);
    digitalWrite(S3, LOW);
    pulseWidth = pulseIn(tscIn, LOW);
    cStrength = pulseWidth/400. - 1;
    cStrength = 255 - cStrength;
    
    Serial.print(rStrength);
    Serial.print(",");
    Serial.print(gStrength);
    Serial.print(",");
    Serial.print(bStrength);
    Serial.print(" -> ");
    
    /*if (rStrength > gStrength && gStrength > bStrength) { rStrength=255; gStrength=gStrength/2; bStrength = 0; }
    else if (rStrength > bStrength && bStrength > gStrength) { rStrength=255; bStrength=bStrength/2; gStrength = 0; }
    else if (gStrength > rStrength && rStrength > bStrength) { gStrength=255; rStrength=rStrength/2; bStrength = 0; }
    else if (gStrength > bStrength && bStrength > rStrength) { gStrength=255; bStrength=bStrength/2; rStrength = 0; }
    else if (bStrength > rStrength && rStrength > gStrength) { bStrength=255; rStrength=rStrength/2; gStrength = 0; }
    else if (bStrength > gStrength && gStrength > rStrength) { bStrength=255; gStrength=gStrength/2; rStrength = 0; }
    else if (bStrength == gStrength && rStrength > bStrength) { rStrength=255; gStrength=gStrength/4; rStrength = rStrength/4; }
    else if (bStrength == rStrength && gStrength > bStrength) { gStrength=255; bStrength=bStrength/4; rStrength = rStrength/4; }
    else if (rStrength == gStrength && bStrength > rStrength) { bStrength=255; rStrength=rStrength/4; gStrength = gStrength/4; }
    else if (bStrength == gStrength && rStrength < bStrength) { bStrength=255; gStrength=255; rStrength = 0; }
    else if (bStrength == rStrength && gStrength < bStrength) { bStrength=255; rStrength=255; gStrength = 0; }
    else if (rStrength == gStrength && bStrength < rStrength) { gStrength=255; rStrength=255; bStrength = 0; }
    else { gStrength=255; rStrength=255; bStrength = 255; }*/
    
    int maxStrength = maxi(rStrength, gStrength, bStrength);
    int minStrength = mini(rStrength, gStrength, bStrength);
    int span = maxStrength - minStrength;
    int cols[3] = {rStrength, gStrength, bStrength};
    int maxInd = 0;
    int minInd = 1;
    int midInd = -1;
    
    for (int n = 0; n < 3; n++) {
      if (cols[n] == maxStrength) { maxInd = n; cols[n] = 255; }
      else if (cols[n] == minStrength) { 
        minInd = n;
        if (span < 12) {
          cols[n] = 24-2*span;
        }
       else {
          cols[n] = 0;
       } 
      }
      else { midInd = n; cols[n] = map(cols[n], minStrength, maxStrength, 0, 255); }
    }
    
    rStrength = cols[0];
    gStrength = cols[1];
    bStrength = cols[2];
    
    if (gStrength < 256) {
      gStrength = gStrength * 0.3;
    }
    if (bStrength < 256) {
      bStrength = bStrength * 0.7;
    }
    
    Serial.print(rStrength);
    Serial.print(",");
    Serial.print(gStrength);
    Serial.print(",");
    Serial.print(bStrength);
    Serial.print(" Pulse ");
    Serial.print(pulseWidth);
    Serial.print("ms | Intensity ");
    Serial.print(cStrength);
    Serial.println("");
    
    if (cStrength < 210) {
      rStrength = rStrengthPrev;
      gStrength = gStrengthPrev;
      bStrength = bStrengthPrev;
    }
}

int maxi(int a, int b, int c)
{
     int m = a;
     (m < b) && (m = b); //these are not conditional statements.
     (m < c) && (m = c); //these are just boolean expressions.
     return m;
}

int mini(int a, int b, int c)
{
     int m = a;
     (m > b) && (m = b); //these are not conditional statements.
     (m > c) && (m = c); //these are just boolean expressions.
     return m;
}

void setup() {
  Serial.begin(9600);
  pinMode(signalIn, INPUT);
  pinMode(strobeOut, OUTPUT);
  pinMode(resetOut, OUTPUT);
  
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(tscIn, INPUT);
  
  pinMode(rpin, OUTPUT);
  pinMode(gpin, OUTPUT);
  pinMode(bpin, OUTPUT);
  
  digitalWrite(resetOut, LOW);
  digitalWrite(strobeOut, HIGH);
  
  irrecv.enableIRIn(); // Start the receiver
  
  Serial.println("Setup completed");
  
  for (int n = 0; n < 8; n++) {
    lcd.createChar(n, musLevels[n]);
  }
  
  lcd.begin(16, 2);
  lcd.print("Welcome");
  
  setRGBComp(255,255,255);
}

void loop() {
  oldMode = mode;
  
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    pressed = results.value;
    received = 1;
    irrecv.resume(); // Receive the next value
  }
  else {
    received = 0;
  }
  
  if (received) {
    switch (pressed)
    {
      //White
      case 16764975:
        colourChanged = 1;
        setRGBComp(255,255,255);
        break;
      //Red column
      case 16748655:
        colourChanged = 1;
        setRGBComp(255,0,0);
        break;
      case 16756815:
        colourChanged = 1;
        setRGBComp(255,10,0);
        break;
      case 16754775:
        colourChanged = 1;
        setRGBComp(255,25,0);
        break;
      case 16750695:
        colourChanged = 1;
        setRGBComp(255,40,0);
        break;
      case 16746615:
        colourChanged = 1;
        setRGBComp(255,60,0);
        break;
        
      //Green column
      case 16716015:
        colourChanged = 1;
        setRGBComp(0,255,0);
        break;
      case 16724175:
        colourChanged = 1;
        setRGBComp(255,255,0);
        break;
      case 16722135:
        colourChanged = 1;
        setRGBComp(120,255,255);
        break;
      case 16718055:
        colourChanged = 1;
        setRGBComp(80,200,255);
        break;
      case 16713975:
        colourChanged = 1;
        setRGBComp(50,140,255);
        break;
        
      //Blue column
      case 16732335:
        colourChanged = 1;
        setRGBComp(0,0,255);
        break;
      case 16740495:
        colourChanged = 1;
        setRGBComp(70,140,255);
        break;
      case 16738455:
        colourChanged = 1;
        setRGBComp(150,40,255);
        break;
      case 16734375:
        colourChanged = 1;
        setRGBComp(255,0,255);
        break;
      case 16730295:
        colourChanged = 1;
        setRGBComp(255,60,255);
        break;
        
      //Mode buttons
      case 16773135: //FLASH -> music mode
        mode = 2;
        break;
      case 16771095: //STROBE
        mode = 3;
        if (oldMode == 3) {
          strobeInd = strobeInd + 1;
          if (strobeInd >= strobeCount) {
            strobeInd = 0;
          }
        }
        break;
      case 16767015: //FADE
        mode = 4;
        break;
      case 16762935: //SMOOTH
        mode = 1;
        break;
        
      //Dimmers
      case 16752735: //Brighter
        brightness = brightness + 1;
        if (brightness > 7) {
          brightness = 7;
        }
        break;
      case 16720095: //Dimmer
        brightness = brightness - 1;
        if (brightness < 0) {
          brightness = 0;
        }
        break;
        
      //Power
      case 16769055: //ON
        mode = 1;
        break;
      case 16736415: //OFF
        mode = 1;
        colourChanged = 1;
        setRGBComp(0, 0, 0);
        break;
    }
  }
  
  if (mode == 1) {
    gated = 0;
    crossFade();
    outRGB(curR, curG, curB);
  }
  else if (mode == 2) {
    digitalWrite(resetOut, HIGH);
    digitalWrite(resetOut, LOW);
    
    lcd.setCursor(0,1);
    lcd.write(" ");
    for (int i = 0; i < 7; i++) {
      digitalWrite(strobeOut, LOW);
      delayMicroseconds(30);
      spectrum[i] = map(analogRead(signalIn),cutoffs[i],1024,0,255);
      if (spectrum[i] < 0) {
        spectrum[i] = 0;
      }
      
      int level = spectrum[i]/32;
      lcd.setCursor(2*i+1,1);
      lcd.write((uint8_t)level);
      lcd.setCursor(2*i+2,1);
      lcd.write((uint8_t)level);
      
      Serial.print(spectrum[i]);
      Serial.print(" ");
      
      digitalWrite(strobeOut, HIGH);
    }
    lcd.write(" ");
    Serial.println();
    
    outRGB(spectrum[0], spectrum[5], spectrum[1]);
  }
  else if (mode == 3) {
    crossFade();
    if (millis() - lastChange > strobeTimes[strobeInd]) {
      if (gated == 1) {
        outRGB(curR, curG, curB);
        gated = 0;
      }
      else {
        outRGB(0, 0, 0);
        gated = 1; 
      }
      lastChange = millis();
    }
  }
  else if (mode == 4) {
    if (millis() - lastColorDetect > detectDelay) {
      lastColorDetect = millis();
      fetchSensorData();
      setRGBComp(rStrength,gStrength,bStrength);
    }
    crossFade();
    outRGB(curR, curG, curB);
  }
  
  displayStat();
  
  if (received == 1) {
    //irrecv.resume(); // Receive the next value
  }
}
