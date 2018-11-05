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
int      averageSamples = 4;
int      tolerance = 10;
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

int jColors[14][6]={
  {162,97,235,208,240,85},
  {49,28,71,61,130,68},
  {47,36,92,81,143,70},
  {15,22,69,60,126,65},
  {54,64,188,168,215,84},
  {42,68,221,194,229,79},
  {17,32,93,70,86,30},
  {17,23,43,22,25,6},
  {147,81,148,113,116,39},
  {43,19,31,17,23,6},
  {66,42,112,95,113,37},
  {13,10,34,24,39,13},
  {108,63,140,113,122,39},
  {181,112,267,238,256,87}
};

int detectThreshold = 5;
bool objectDetected = false;

int sensorState;
int detectedColor;

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

  ams.setGain(1);
//  ams.setIntegrationTime(8);
  lastDetectedTime = 0;

  vAvg.clear();
  bAvg.clear();
  gAvg.clear();
  yAvg.clear();
  oAvg.clear();
  rAvg.clear();

}

void loop() {

  readAS7262();
  
  //when seeing dark, means detected object
  if((objectDetected==false) && (vVal<detectThreshold) && (bVal<detectThreshold) && (gVal<detectThreshold) && (yVal<detectThreshold) && (oVal<detectThreshold) && (rVal<detectThreshold)){
    objectDetected = true;
    ams.drvOn();
    if(debug)Serial.println("detect object!");
  }

  if(objectDetected){
    if((millis()-lastDetectedTime)<2000){
        //measuring
        
        numOfSamples++;
        //Serial.println(numOfSamples);
        
        printRawVals();
        if(numOfSamples<=1){
          //ditch the first sample
        }else{
          vAvg.addValue(vVal);
          bAvg.addValue(bVal);
          gAvg.addValue(gVal);
          yAvg.addValue(yVal);
          oAvg.addValue(oVal);
          rAvg.addValue(rVal);
        }
        
        if(numOfSamples>averageSamples){
          avgColors[0]=round(vAvg.getAverage()/10);
          avgColors[1]=round(bAvg.getAverage()/10);
          avgColors[2]=round(gAvg.getAverage()/10);
          avgColors[3]=round(yAvg.getAverage()/10);
          avgColors[4]=round(oAvg.getAverage()/10);
          avgColors[5]=round(rAvg.getAverage()/10);
          
          if(debug){printAvgVals();}

          int matchedColor = compareColors(avgColors);
          Serial.print("matching color: ");
          Serial.println(matchedColor);
          

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
      if(abs(colors[i]-jColors[j][i])<tolerance){
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
