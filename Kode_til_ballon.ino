/* -- Libraries der skal inkluderes -- */
#include <SD.h>                 //SD-kort
#include <SPI.h>                //SD-kort
#include <Adafruit_ADS1015.h>   //ADC
#include <Wire.h>               //Barometer og ADC
#include "SparkFunMPL3115A2.h"  //Barometer
/* Følgende to (tre) libraries skal tilføjes under Library Manager:
    - SparkFun MPL3115A2 Altitude and Pressure Sensor Breakout
    - Adafruit ADS1X15
    - SdFat     (Kun nødvendig vis SD-kort microcontroller melder fejl)
*/

/* -- Globale variable og funktioner -- */
MPL3115A2 myPressure;           //Barometer
float pressure;                 //Barometer
float temperature;              //Barometer
File datafil;                   //SD-kort
Adafruit_ADS1115 ads1115;       //ADC
int16_t voltage;                //ADC
int outputPin1 = 7;             //Relæ mellem poler
int outputPin2 = 6;             //Relæ fra første pol
int outputPin3 = 5;             //Relæ fra anden pol
const int measSize = 50;
unsigned long seriesTimes[measSize];
float seriesVoltages[measSize];
float myTemp, myPres;
long count = 0;
int n=0;
unsigned long periods[]={1000,2000,3000,4000,5000,10000,20000};
 
void takeMeasurement(int prePoints, int postPoints, float chargeTime, unsigned long localTimes[], float localVoltages[], float* pressure, float* temperature){

  digitalWrite(outputPin2, LOW);                  //Kortslutning af kondensator
  digitalWrite(outputPin3, LOW);                  //Kortslutning af kondensator
  digitalWrite(outputPin1, HIGH);                 //Kortslutning af kondensator
  delay(10);                                      //Tiden der kortsluttes i
  
  digitalWrite(outputPin1, LOW);                  //Opladning af kondensator
  delay(chargeTime);                              //Tiden der oplades i
  *pressure = myPressure.readPressure();
  *temperature = myPressure.readTemp();

  for(int i = 0; i < prePoints+postPoints; i++){

    if(i >= measSize) break;
    
    if( i == prePoints-1)
    {
        digitalWrite(outputPin2, HIGH);                 //Opfangning af spænding
        digitalWrite(outputPin3, HIGH);                 //Opfangning af spænding
    }
    localVoltages[i] = ads1115.readADC_Differential_2_3(); //Aflæsning af spænding
    localTimes[i] = millis();
    
  }
  
  digitalWrite(outputPin1, LOW);
  digitalWrite(outputPin2, LOW);                        // Åbner alle relæer
  digitalWrite(outputPin3, LOW);
}

void setup() {

  Serial.begin(9600);
  

/* -- Opsætning af signal til relæer -- */
  pinMode(outputPin1, OUTPUT);
  pinMode(outputPin2, OUTPUT);
  pinMode(outputPin3, OUTPUT);
  digitalWrite(outputPin1, LOW);
  digitalWrite(outputPin2, LOW);                  //Kortslutning af kondensator
  digitalWrite(outputPin3, LOW);                  //Kortslutning af kondensator

/* -- Opsætning af ADC microcontroller -- */
  Serial.println(F("Differential spændingsaflæsning opsat på pin AIN2 (P) og AIN3 (N)"));
  Serial.println(F("ADC Range: +/- 6.144V (1 bit = 188uV)"));
  ads1115.begin();
  //ads1115.setGain(GAIN_TWOTHIRDS);      //Mulighed for justering af forstærkning - ikke nødvendigt

/* -- Opsætning af SD-kort microcontroller -- */
  Serial.println(F("Initialiserer SD-kort..."));
  if(!SD.begin(10)) {
    Serial.println(F("Initialisering af SD-kort mislykkedes!"));
    delay(1000);
    
  } 
  
  Serial.println(F("Initialisering af SD-kort gennemført"));
  Serial.println(F("Genererer datafil..."));
  datafil=SD.open("datafil.txt", FILE_WRITE);
  datafil.println("Ny måling");
  datafil.close();
  Serial.println(F("Ny måling noteret"));
  //Serial.println();

/* -- Opsætning af barometer microcontroller -- */
  Wire.begin();        // Join i2c bus
  Serial.println(F("Barometer initialiseres"));
  myPressure.begin();
  Serial.println(F("Initialisering af barometer gennemført"));
  Serial.println();

  myPressure.setModeBarometer(); //Måler tryk fra 20 til 120 kpa
  myPressure.setOversampleRate(7); // Beskriver støj fra hver målling og tiden til at fange hver målling. Anbelfafer en rate på 7
  myPressure.enableEventFlags(); // Aktiverer alle tre tryk- og temp hændelsesflag 
}

void loop() {
  
  Serial.println(F("Data opsamles..."));

  //Test af takeMeasurement
  takeMeasurement(0,50,periods[n],seriesTimes, seriesVoltages, &myPres, &myTemp);
  count+= 1;
  for(int i = 0; i < measSize; i++){
    Serial.println(F("Følgende data skrives til fil (tid(ms), spænding(V), tryk(Pa), temperatur(C)):"));
    Serial.print(count);
    Serial.print(F(","));
    Serial.print(seriesTimes[i]);
    Serial.print(F(","));
    Serial.print(seriesVoltages[i]);
    Serial.print(F(","));
    Serial.print(myPres);
    Serial.print(F(","));
    Serial.println(myTemp);
    Serial.println();

      if(SD.exists("datafil.txt")) {      //Tjekker om filen eksisterer
                                          //Serial.println(F("Datafil fundet!"));

      datafil=SD.open("datafil.txt", FILE_WRITE);
        if(datafil) {                     //Tjekker om filen er tilgængelig
          datafil.print(count);
          datafil.print(F(","));
          datafil.print(seriesTimes[i]);
          datafil.print(",");
          datafil.print(seriesVoltages[i]);
          datafil.print(",");
          datafil.print(myPres);
          datafil.print(",");
          datafil.println(myTemp);
          datafil.close();
        }
        else {
          Serial.println(F("Fejl opstod: Fil kan ikke redigeres"));
        }
      }
      else {
        Serial.println(F("Fejl opstod: Fil blev ikke fundet!"));
      }
  }
   if (n<6) {
    n++; }
  else {
  n=0; }
   Serial.println();
  delay(1);
}
