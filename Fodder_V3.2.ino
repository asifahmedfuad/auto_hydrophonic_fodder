/*
  LiquidCrystal Library + Circuit Connection
 The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 */

// include the library code:
#include <DHT.h>
#include <LiquidCrystal.h>
#include "Wire.h"

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//Constants
#define Water 27
#define Auto 25
#define Manual 23
#define DS1307_I2C_ADDRESS 0x68
#define DHTPIN 8     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

//Variables
int sensorLogic=0;
int moistureSensorValue =0;
int lightSensorValue =0;
unsigned char ch_v;
long moisture, light; 
int mCount,mRun;
//FOR DHT Temperature & Humidity
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value 
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
long nextTime;
long currentTimeM,currentTimeS;
long runTime;

//Functions

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

void setup() {
  
  dht.begin();
  Wire.begin();
  Serial.begin(9600);
  
  pinMode(Water,OUTPUT);
  pinMode(Auto,INPUT);
  pinMode(Manual,INPUT);
  digitalWrite(Water,LOW);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 4);
  lcd.clear();
  
  // set the initial time here:
  // DS1307 seconds, minutes, hours, day, date, month, year
  //setDS1307time(55,42,14,7,22,4,17);
  nextTime=1440;
  currentTimeM=0;
  currentTimeS=0;
  runTime=0;
  mCount=0;
  mRun=0;
}

void setDS1307time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS1307
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}


void readDS1307time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year)
{
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 7);
  // request seven bytes of data from DS1307 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}


void displayTime()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS1307
  readDS1307time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  // send it to the serial monitor
  Serial.print(hour, DEC);
  // convert the byte variable to a decimal number when displayed
  Serial.print(":");
  if (minute<10)
  {
    Serial.print("0");
  }
  Serial.print(minute, DEC);
  Serial.print(":");
  if (second<10)
  {
    Serial.print("0");
  }
  Serial.print(second, DEC);
  Serial.print(" ");
  Serial.print(dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(month, DEC);
  Serial.print("/");
  Serial.print(year, DEC);
  Serial.print(" Day of week: ");
  
  switch(dayOfWeek){
  case 1:
    Serial.println("Sunday");
    break;
  case 2:
    Serial.println("Monday");
    break;
  case 3:
    Serial.println("Tuesday");
    break;
  case 4:
    Serial.println("Wednesday");
    break;
  case 5:
    Serial.println("Thursday");
    break;
  case 6:
    Serial.println("Friday");
    break;
  case 7:
    Serial.println("Saturday");
    break;
  }
}


void loop() 
{
  displayTime(); // display the real-time clock data on the Serial Monitor
  
  //Read Humidity & Temperature from DHT22 & Moisture and Light
  hum = dht.readHumidity();   temp= dht.readTemperature();
  moistureSensorValue = analogRead(A0); lightSensorValue = analogRead(A1);
  
  moisture= (long) moistureSensorValue * 100; moisture= moisture / 1023;
  
  // set the cursor to (0,0):
  lcd.setCursor(0, 0);
  lcd.print("M:");  lcd.print(moisture);    lcd.print('%');
  
  light= (long) lightSensorValue * 100;
  light= light / 1023;
  lcd.setCursor(6, 0);
  lcd.print("L:");    lcd.print(light);     lcd.print('%');

  lcd.setCursor(12, 0);
  lcd.print("C:");    lcd.print(mCount); 

  lcd.setCursor(0,1);  
  lcd.print("H:");     lcd.print(hum);
  
  lcd.setCursor(9,1);
  lcd.print("T:"); lcd.print(temp);
  
  delay(20); 
  
  if((moisture>99) or (temp>30) or (hum<20))   {sensorLogic=1;}
  else {sensorLogic=0;}


  // retrieve data from DS1307
  readDS1307time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  lcd.setCursor(0,3);
  lcd.print(hour, DEC);
  // convert the byte variable to a decimal number when displayed
  lcd.print(":");
  if (minute<10)
  {
    lcd.print("0");
  }
  lcd.print(minute, DEC);
  lcd.print(":");
  if (second<10)
  {
    lcd.print("0");
  }
  lcd.print(second, DEC);
  currentTimeM=((hour*60)+minute);
  currentTimeS=((minute*60)+second);
  

  // Conditions
  
  if(digitalRead(Auto)==HIGH)
  {
    if(digitalRead(Manual)==HIGH)
    {
      Serial.println("MANUAL MODE");
      lcd.setCursor(0,2);  lcd.print("Manual Starting");
      Serial.println(currentTimeM);
      nextTime=currentTimeM+120;
      Serial.println(nextTime);
      
      if(nextTime>1440) {nextTime=nextTime-1440;}
      else  {nextTime=nextTime;}
      
      runTime=currentTimeS+120; //Second
      if(runTime>3600) {runTime=runTime-3600;}
      else  {runTime=runTime;}
      Serial.println(runTime);
      mRun=0;
      while((currentTimeS <= runTime)) //and (moisture> 45))
      {
        digitalWrite(Water, HIGH) ;
        //Read Humidity & Temperature from DHT22 & Moisture and Light
        hum = dht.readHumidity();   temp= dht.readTemperature();
        moistureSensorValue = analogRead(A0); lightSensorValue = analogRead(A1);
        
        moisture= (long) moistureSensorValue * 100; moisture= moisture / 1023;
        
        // set the cursor to (0,0):
        lcd.setCursor(0, 0);
        lcd.print("M:");  lcd.print(moisture);    lcd.print('%');
        
        light= (long) lightSensorValue * 100;
        light= light / 1023;
        lcd.setCursor(6, 0);
        lcd.print("L:");    lcd.print(light);     lcd.print('%');
        lcd.setCursor(12, 0);
        lcd.print("C:");    lcd.print(mCount);
        lcd.setCursor(0,1);  
        lcd.print("H:");     lcd.print(hum);
        
        lcd.setCursor(9,1);
        lcd.print("T:"); lcd.print(temp);
        lcd.setCursor(0,2);
        lcd.print("Manual: Watering"); 
         
        readDS1307time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
        currentTimeS=((minute*60)+second);
        lcd.setCursor(0,3);
        lcd.print(hour, DEC);
        // convert the byte variable to a decimal number when displayed
        lcd.print(":");
        if (minute<10)
        {
          lcd.print("0");
        }
        lcd.print(minute, DEC);
        lcd.print(":");
        if (second<10)
        {
          lcd.print("0");
        }
        lcd.print(second, DEC);
        
        delay(100);
        lcd.clear();
        mRun=1;       
      }
      if(mRun==1){mCount=mCount+1;} else 
      {mCount=mCount;
      lcd.setCursor(0,2);
      lcd.print("Sufficient Water"); }
      digitalWrite(Water, LOW) ; 
      runTime=0;
      mRun=0;
      delay(1000);
      lcd.clear();  
    }
    else 
    {
      Serial.println("Auto Mode");
      lcd.setCursor(0,2);  
      lcd.print("Auto Mode");
      
      
      if((sensorLogic==1) or (currentTimeM==nextTime))
      {
        Serial.println("Sensor or Time");
        Serial.println(currentTimeM);
        nextTime=currentTimeM+120;
        Serial.println(nextTime);
        if(nextTime>1440)
        {
          nextTime=nextTime-1440;
        }
        else
        {
          nextTime=nextTime;
        }
        runTime=currentTimeS+120;
        if(runTime>3600) {runTime=runTime-3600;}
        else  {runTime=runTime;}
        Serial.println(runTime);
        mRun=0;
        while((currentTimeS <= runTime))// and (moisture>45))
        {
          digitalWrite(Water, HIGH) ;
          //Read Humidity & Temperature from DHT22 & Moisture and Light
          hum = dht.readHumidity();   temp= dht.readTemperature();
          moistureSensorValue = analogRead(A0); lightSensorValue = analogRead(A1);
          
          moisture= (long) moistureSensorValue * 100; moisture= moisture / 1023;
          
          // set the cursor to (0,0):
          lcd.setCursor(0, 0);
          lcd.print("M:");  lcd.print(moisture);    lcd.print('%');
          
          light= (long) lightSensorValue * 100;
          light= light / 1023;
          lcd.setCursor(6, 0);
          lcd.print("L:");    lcd.print(light);     lcd.print('%');
          lcd.setCursor(12, 0);
          lcd.print("C:");    lcd.print(mCount);
          lcd.setCursor(0,1);  
          lcd.print("H:");     lcd.print(hum);
          
          lcd.setCursor(9,1);
          lcd.print("T:"); lcd.print(temp);
          lcd.setCursor(0,2);
          lcd.print("Auto: Watering"); 
          readDS1307time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
          currentTimeS=((minute*60)+second);
          lcd.setCursor(0,3);
          lcd.print(hour, DEC);
          // convert the byte variable to a decimal number when displayed
          lcd.print(":");
          if (minute<10)
          {
            lcd.print("0");
          }
          lcd.print(minute, DEC);
          lcd.print(":");
          if (second<10)
          {
            lcd.print("0");
          }
          lcd.print(second, DEC);
          delay(100);
          lcd.clear();
          mRun=1;
        }
        if(mRun==1){mCount=mCount+1;} 
        else 
        {mCount=mCount; 
        lcd.setCursor(0,2);
        lcd.print("Sufficient Water");}
        digitalWrite(Water, LOW) ; 
        runTime=0;
        mRun=0;
        delay(1000);
        lcd.clear();
      }
      else
      {
        if(nextTime==1440)
        {
          digitalWrite(Water, LOW);
          nextTime=currentTimeM+120;
          if(nextTime>1440)
          {
            nextTime=nextTime-1440;
          }
          else
          {
            nextTime=nextTime;
          }
          runTime=0;
          Serial.println("Auto Mode: First Time Initialized");
          Serial.println(nextTime);          
        }
        
      }
    delay(100);
      
    }
  }
  else
  {
    digitalWrite(Water, LOW); 
  }
  lcd.clear();
}

