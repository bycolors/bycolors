#include <RunningAverage.h>

/***************************************************************************
  This is a library for the Adafruit AS7262 6-Channel Visible Light Sensor

  This sketch reads the sensor

  Designed specifically to work with the Adafruit AS7262 breakout
  ----> http://www.adafruit.com/products/3779
  
  These sensors use I2C to communicate. The device's I2C address is 0x49
  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!
  
  Written by Dean Miller for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include "Adafruit_AS726x.h"


//create the object
Adafruit_AS726x ams;

//buffer to hold raw values
uint16_t sensorVals[AS726x_NUM_CHANNELS];

//buffer to hold calibrated values (not used by default in this example)
//float calibratedVals[AS726x_NUM_CHANNELS];

bool debug = false;

long lastDetectedTime;
uint16_t vVal;
uint16_t bVal;
uint16_t gVal;
uint16_t yVal;
uint16_t oVal;
uint16_t rVal;
uint8_t  temp;
int      numOfSamples = 0;
int      averageSamples = 1; //waitTime = 3000 : 8 / 1500:3 /1000:1
int      tolerance = 20;

 
bool usingAverage = false;
RunningAverage vAvg(averageSamples);
RunningAverage bAvg(averageSamples);
RunningAverage gAvg(averageSamples);
RunningAverage yAvg(averageSamples);
RunningAverage oAvg(averageSamples);
RunningAverage rAvg(averageSamples);

int avgColors[6]={0,0,0,0,0,0};

//uint16_t jColors[14][6]={
//  {15,9,22,20,23,8},
//  {4,2,6,6,12,6},
//  {4,3,9,7,13,6},
//  {1,2,7,6,12,6},
//  {5,6,18,16,20,7},
//  {4,6,22,19,22,7},
//  {1,3,9,6,8,2},
//  {1,2,4,2,2,0},
//  {14,7,14,10,11,3},
//  {4,1,3,1,2,0},
//  {6,4,11,9,11,3},
//  {1,0,3,2,3,1},
//  {11,6,14,11,12,4},
//  {17,10,26,23,25,8}
//};

// integration time 50, gain 16x
//int jColors[14][6]={
//  
//{17,12,53,27,63,23},
//{21,10,46,20,61,23},
//{9,8,50,22,66,23},
//{9,12,62,31,70,23},
//{18,22,75,48,80,26},
//
//{16,25,90,58,81,23},
//{10,12,50,17,41,5},
//{9,12,54,21,48,9},
//{17,8,42,12,37,4},
//{45,26,62,32,51,12},
//
//{7,4,43,12,41,5},
//{23,17,56,28,55,15},
//{52,35,90,62,82,26},
//{35,24,67,35,56,14},
//};

bool testData = false;
//integration time 50 gain 64x
int jColors[14][6]={
  
{50,43,136,86,177,83},
{62,35,111,62,164,85},
{19,29,130,71,187,95},
{20,40,160,96,202,92},
{55,78,233,174,245,99},

{48,92,272,204,249,93},
{20,34,111,36,68,11},
{17,37,138,58,106,28},
{51,27,93,24,62,9},
{172,100,187,113,134,44},

{12,8,92,20,71,9},
{77,64,159,93,142,53},
{194,133,295,236,259,96},
{119,85,182,112,146,51},
};



nt jColors2[14][6]={
  
{50,43,136,86,177,83},
{62,35,111,62,164,85},
{19,29,130,71,187,95},
{20,40,160,96,202,92},
{55,78,233,174,245,99},

{48,92,272,204,249,93},
{20,34,111,36,68,11},
{17,37,138,58,106,28},
{51,27,93,24,62,9},
{172,100,187,113,134,44},

{12,8,92,20,71,9},
{77,64,159,93,142,53},
{194,133,295,236,259,96},
{119,85,182,112,146,51},
};



int detectThreshold = 5;
bool objectDetected = false;

int sensorState;
int detectedColor;
int waitTime = 1000;
int matchedColor;

void setup() {
  Serial.begin(9600);
  while(!Serial);
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  //begin and make sure we can talk to the sensor
  if(!ams.begin()){
    Serial.println("could not connect to sensor! Please check your wiring.");
    while(1);
  }

  ams.setGain(GAIN_64X);
//  ams.setGain(GAIN_16X);

  // 50:about 1 sec per data
  ams.setIntegrationTime(50);
//  ams.setIntegrationTime(100);

  lastDetectedTime = 0;

  if(usingAverage){
    vAvg.clear();
    bAvg.clear();
    gAvg.clear();
    yAvg.clear();
    oAvg.clear();
    rAvg.clear();
  }
}

void loop() {
  bool requested = false;

  if(Serial.available()>0){
    if(Serial.read()=='a') requested = true;
  }

  readAS7262();
//  printRawVals();

  //when seeing dark, means detected object
  if((objectDetected==false) && (vVal<detectThreshold) && (bVal<detectThreshold) && (gVal<detectThreshold) && (yVal<detectThreshold) && (oVal<detectThreshold) && (rVal<detectThreshold)){
    objectDetected = true;
    ams.drvOn();
    if(debug)Serial.println("detect object!");
  }

  if(objectDetected){
    if((millis()-lastDetectedTime)<waitTime){
        //measuring
        
        numOfSamples++;
//        Serial.print("num of sample");
//        Serial.println(numOfSamples);
        
//        printRawVals();
        if(numOfSamples<=1){
          //ditch the first sample
        }else{
          if(usingAverage){
            vAvg.addValue(vVal);
            bAvg.addValue(bVal);
            gAvg.addValue(gVal);
            yAvg.addValue(yVal);
            oAvg.addValue(oVal);
            rAvg.addValue(rVal);
          }else{
            avgColors[0]=round(vVal/100);
            avgColors[1]=round(bVal/100);
            avgColors[2]=round(gVal/100);
            avgColors[3]=round(yVal/100);
            avgColors[4]=round(oVal/100);
            avgColors[5]=round(rVal/100);
            if(testData) {printForCal();}
          matchedColor = compareColors(avgColors);
                      if (testData){Serial.println(matchedColor);}

          if(debug){Serial.print("matching color: ");
            Serial.println(matchedColor);}
          else{
              if(requested){
                Serial.write(matchedColor);
//                Serial.println(matchedColor);
              }
            }
          }
        }

       if(usingAverage){
        if(numOfSamples>averageSamples){
          avgColors[0]=round(vAvg.getAverage());
          avgColors[1]=round(bAvg.getAverage());
          avgColors[2]=round(gAvg.getAverage());
          avgColors[3]=round(yAvg.getAverage());
          avgColors[4]=round(oAvg.getAverage());
          avgColors[5]=round(rAvg.getAverage());
          
          if(debug){printAvgVals();}

          matchedColor = compareColors(avgColors);
          if(debug){Serial.print("matching color: ");
          Serial.println(matchedColor);}
          else{
              if(requested){
                Serial.write(matchedColor);
                //Serial.println(matchedColor);
              }
          }
        }
       }


        
    }else{
      objectDetected = false;
      ams.drvOff();
      lastDetectedTime = millis();
      numOfSamples = 0;
    }
  }else{
    //no object detected, do nothing;
  }
}

void readAS7262(){
    //read the device temperature
  temp = ams.readTemperature();
  
  //ams.drvOn(); //uncomment this if you want to use the driver LED for readings
  ams.startMeasurement(); //begin a measurement
  
  //wait till data is available
  bool rdy = false;
  while(!rdy){
    delay(5);
    rdy = ams.dataReady();
  }
  //ams.drvOff(); //uncomment this if you want to use the driver LED for readings

  //read the values!
  ams.readRawValues(sensorVals);
  //ams.readCalibratedVals(calibratedVals);

  vVal = sensorVals[AS726x_VIOLET];
  bVal = sensorVals[AS726x_BLUE];
  gVal = sensorVals[AS726x_GREEN];
  yVal = sensorVals[AS726x_YELLOW];
  oVal = sensorVals[AS726x_ORANGE];
  rVal = sensorVals[AS726x_RED];
}

void printForCal(){
  Serial.print("{"); Serial.print(avgColors[0]);
  Serial.print(","); Serial.print(avgColors[1]);
  Serial.print(","); Serial.print(avgColors[2]);
  Serial.print(","); Serial.print(avgColors[3]);
  Serial.print(","); Serial.print(avgColors[4]);
  Serial.print(","); Serial.print(avgColors[5]);
  Serial.println("},");
}

void printAvgVals(){
  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" Violet: "); Serial.print(avgColors[0]);
  Serial.print(" Blue: "); Serial.print(avgColors[1]);
  Serial.print(" Green: "); Serial.print(avgColors[2]);
  Serial.print(" Yellow: "); Serial.print(avgColors[3]);
  Serial.print(" Orange: "); Serial.print(avgColors[4]);
  Serial.print(" Red: "); Serial.println(avgColors[5]);
}

void printRawVals(){
  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" Violet: "); Serial.print(vVal);
  Serial.print(" Blue: "); Serial.print(bVal);
  Serial.print(" Green: "); Serial.print(gVal);
  Serial.print(" Yellow: "); Serial.print(yVal);
  Serial.print(" Orange: "); Serial.print(oVal);
  Serial.print(" Red: "); Serial.println(rVal);
}


int compareColors(int colors[6]){
  int matchColor = 99;
  bool found = false;
  
  for(int j = 0; j<14; j++){
    if(debug){
      Serial.print("check j color no.");
      Serial.print(j);
      Serial.println(" :");
    }
    bool same = true;
    // compare arrays
    for(int i = 0; i < 6; i++){
      if(debug){
        Serial.print("detected color: ");
        Serial.print(colors[i]);
        Serial.print(" j color: ");
        Serial.print(jColors[j][i]);
        Serial.print(" diff:");
        Serial.println(abs(colors[i]-jColors[j][i]));
      }
      if(abs(colors[i]-jColors[j][i])<tolerance||abs(colors[i]-jColors2[j][i])<tolerance){
        same = true;
        if(debug)Serial.print(" true ");
      }else{
        same = false;
        if(debug)Serial.print(colors[i]);
        if(debug)Serial.println(" false, next ");
        break;
      }
    }

    // report or move on
    if(same){
      found = true;
      matchColor = j+1;
      return matchColor;
    }else{
      found = false;
    }
  }

  return matchColor;


}
