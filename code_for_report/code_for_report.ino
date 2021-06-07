/*
   Author: Chu Shi and Priscilla Pau
   Project: Fog Collector
   Last editted: 6/4/2021 19:30pm
   Included coulomb counting from Christian Islas, replacing battery test
*/

// Sensor Libraries
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <Servo.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <DS1302.h>
#include <MsTimer2.h>
#include <DHT.h>

/* dht22 defination*/
#define DHTPIN 12        // pin 12 for temperature sensor
#define DHTTYPE DHT22   // DHT sensor model 22
DHT dht(DHTPIN, DHTTYPE);

/*sd card*/
File myFile;      // file to write in SD card

/*servos*/
Servo myservo1, myservo2;   //declare servos

/*define pins for led color, buzzer, and relay.(LED and buzzer will not included in the final version */
int blue = 26;
int green = 28;
int red = 30;
int speak = 22;    //not used in varification
int relay = 24;

/*define pins for RTC module*/
DS1302 rtc(10, 8, 7);        // pin 10 for RST, pin 8 for DAT, pin 7 fror CLK

/*variables determined by user*/
int h = 85;      // h is the threshold humidity determined by user. 
                   // h is set to be 85(%) by defult(now 65 only for testing). 85% humidity is determined by
                   // Professor Peter Weiss
float v = 1000;     // v is the threshold visibility determined by user. 
                   // v is set to be 1000(m) by defult. 1000 meter visibility is determined by
                   // Professor Peter Weiss
int buffertime = 1800000;    //set up the buffer time in ms scale.(10000 ms only for testing) 
                           // For example, if you want the buffer time to be 30 seconds, then set buffertime = 30*1000=30000;
int sleeptime = 3;    //set sleep time, sleeptime=1 means sleep 1 minutes;
                      //sleeptime=30 means sleep 30 minutes;
                   
/*Define Pins and Variables*/
#define pinWind (A0)    // pin A0 for windspeed sensor
#define pinWater (A4)   // pin A4 for water level sensor
#define pinOFS (A2)     // pin A2 for miniOFS
#define pinBAT (A3)     //pin A3 battery level
//int batvol = A3;      //Define pin A3 as the pin to detect voltage reading from the battery

/*SENSORS variables*/
int Humidity;       // define humidity reading
int Temperature;   // define temperature reading
                   
int windvalue;     // define windspeed sensor raw value
int windspeed;     // windspeed reading in m/s

int value1;        // define MiniOFS raw value
float voltage;     // voltage reading of MiniOFS
float Distance;    // distance reading of MiniOFS   0-4000m

int waterlevel_val;   // define water level raw reading
int waterlevel;       // actual water level reading in mm  0-40mm

int batvoltag;      //Define voltage reading as variable "batvoltag"
float batvalue;     //define actual bettery level  0-1

unsigned long time1;        //define 2 time variables. Using time2-time1 to calculate the whether 10 second is passed or not
unsigned long time2;

/*counter for low power mode*/
int counter         //this counter is used for low power mode
                      
/*Variable for indicating states*/
volatile byte fogStater = LOW;

/*BATTERY LEVEL variables*/
//from Chris
volatile float SOC = 1.0;
//float Qmax = 0.01; // Battery Amp Hours so 10mAh
float Qmax = 100.0;

/*STATEMACHINE*/
enum Fog {                                        // Declare the states for statemachine
  IDLE, FAN, NOFAN, DATAONLY, CRITICAL, WAIT, TRANSFER    //TRANSFER state will be used when RF is integrate with the state machine
};
Fog state = IDLE;                                 // Initiate state at IDLE

/*watch dog timer for 20 second sleep mode*/
ISR(WDT_vect)    //WDT Interrupt Service Routine
{
  wdt_disable();  // disable watchdog
}

void WatchdogEnable(const byte interval)    //It is nominally 128 kHz, but the actual frequency will can be off by +/- 10%
//depending on the temperature, VCC,
{
  MCUSR = 0;                          // clear register
  WDTCSR |= 0b00011000;              // enable WDCE 5th from right(allow changes) WDCE need to be set before you can change the WDE
  //or prescaler bits (WDP3:0)
  //WDE 4th from right(disable reset)
  WDTCSR =  0b01000000 | interval;    // set WDIE 7th from right(interrupt enable) WDE(enable rest), and 8 seconds delay
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  sleep_mode();            // now goes to Sleep and waits for the interrupt
}

/*Helper Functions*/
/////by Chris Islas
void SOC_estimation() {
  float average_current = 0;
  float ms = 0.000833333;   // this is a 3000 miliseconds in the unit of one hour

  // This for loop takes a moving average of the measure current for stability
  for (int i = 0; i < 1000; i++) {
    average_current = average_current + (14.2981 * (analogRead(A5) * (5.0 / 1023.0)) - 36.1166) / 1000;
  }

  average_current = average_current + 0.31;  // This is to take care of the offset at 0A that was observed during testing

  // current = average_current;  // Here is to save the current value

  SOC = SOC + (((average_current * ms) / Qmax));  // This is the calculation of the new state of charge
  Serial.println(SOC);

}

void battery_test()   //Check battery level by read the voltage value.(this is only for defence verification.
{
  batvoltag =  analogRead(pinBAT);    //Read bettery level, it will read
  //a value ranges from 0 to 1023
  batvalue = batvoltag * (5.0 / 1024.0); //Use the formula to covert 0-1023 scale to 0-5 scale
  Serial.print("battery level: ");    //print out battery level reading
  Serial.print( batvalue );
  Serial.print(" ");
}


/*SD card*/
void recordData() {
    myFile = SD.open("data.txt", FILE_WRITE);
    if (myFile) {
      Serial.print(" recording data");
      Serial.print(" ");
      myFile.print(rtc.getDateStr(FORMAT_LONG, FORMAT_LITTLEENDIAN));    //record day/month/year
      myFile.print("   ");
      myFile.print(  rtc.getTimeStr());                                  //record hours/minutes/seconds
      myFile.print("-->");
      myFile.print("     Humidity(%)=");                 
      myFile.print(Humidity);                                            //record humidity
      myFile.print("     Temperature(C)=");
      myFile.print(Temperature);                                         //record temperature
      myFile.print("     Visibility(m)=");
      myFile.print(Distance);                                            //record visibility
      myFile.print("     windspeed(m/s)=");
      myFile.print(windspeed);                                           //record wind speed
      myFile.print("     waterlevel(mm)=");
      myFile.print(waterlevel);                                          //record water level
      myFile.print("     battery level(%)=");
      myFile.print(batvalue);                                            //record battery level
      myFile.print("     fogstate=");
      if (fogStater == LOW) {
        myFile.println("OFF");                                           //record OFF if not collecting fog
      }
      else if (fogStater == HIGH) {
        myFile.println("ON");                                            //record ON if collecting fog
      }
      myFile.close(); // close the file
    }
    // if the file didn't open, print an error:
    else {
      Serial.println("error opening data.txt");
    }
  }
  
  void getdatetime_andsensordata() {
    /*assign value for fog indicator base on state*/
    if (state == IDLE || state == DATAONLY || state == CRITICAL || state == WAIT) {   

      fogStater = LOW;                                                                 
    }
    else if (state == FAN || state == NOFAN ) {

      fogStater = HIGH;                                                              //fogStater only go to high when system is collecting fog
    }
    /*print the date and time from RTC*/
    Serial.print(rtc.getDateStr(FORMAT_LONG, FORMAT_LITTLEENDIAN));    //get time from DS1302
    Serial.print("    ");
    Serial.print(rtc.getTimeStr());

    /*temperature and humidity*/
    Humidity = dht.readHumidity() ;       // define humidity reading
    Temperature = dht.readTemperature();   // define temperature reading
    Serial.println(" ");
    Serial.print(F("Humidity: "));
    Serial.print(Humidity);
    Serial.print(F("%  Temperature: "));
    Serial.print(Temperature);
    Serial.println(F("°C "));

    /*windspeed*/
    windvalue = analogRead(pinWind);                   // define wind speed reading
    windspeed = map(windvalue, 0, 1023, 0, 30);
    Serial.print("windspeed is: ");
    Serial.print( windspeed );
    Serial.println( "m/s" );

    /*miniOFS*/
    value1 = analogRead(pinOFS);                     // define MiniOFS raw value
    voltage = value1 * (5.0 / 1023.0);         // converting raw value to voltage reading
    Distance = voltage * 500 ;                 // converting voltage to distance reading
    Serial.print("Visibility range(m): ");
    Serial.println(Distance);

    /*Water Level*/
    waterlevel_val = analogRead(pinWater);             // define water level reading
    waterlevel = map(waterlevel_val, 0, 1023, 0, 40);
    Serial.print("water level is: ");
    Serial.print( waterlevel );
    Serial.println( "mm" );

    /*battery test*/
    batvoltag =  analogRead(pinBAT);    //Read bettery level, it will read
    //a value ranges from 0 to 1023
    batvalue = batvoltag * (5.0 / 1024.0); //Use the formula to covert 0-1023 scale to 0-5 scale
    Serial.print("battery level: ");
    Serial.print( batvalue );

    //SOC_estimation();    //uncomment this when we have current sensor
    recordData();    //record above data into sd card
    Serial.println(" ");
    Serial.println(" ");
    Serial.println(" ");
  }

  /*Code Setup*/
  void setup() {
    Serial.begin(9600);
    // Set the clock to run-mode, and disable the write protection
    //rtc.halt(false);
    //rtc.writeProtect(false);
    //rtc.setTime(21, 06, 30);     // Set the time to hours/minutes/secconds format
    //rtc.setDate(30, 5, 2021);   // Set the date to day/month/year format
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
    Serial.println("Fog Collector Project");
    Serial.print("Initializing SD card...");     //print message
    if (!SD.begin(53)) {
      Serial.println("Initialization failed!");  //print error message
      while (1);
    }
    Serial.println("Initialization done.");      //print message
    dht.begin();                                 //start reading dht22 sensor
    //Serial.print("batvalue=");
    //Serial.println(batvalue);
    pinMode(A0, INPUT);       // initialize anemomoter input
    //pinMode(A1, INPUT);       // initialize Anemometer input
    pinMode(A2, INPUT);       // initialize MINIOFS input
    pinMode(A3, INPUT);       // initialize bettery level input
    pinMode(A4, INPUT);       // initialize waterlevel input
    getdatetime_andsensordata();

    /*set up servos*/
    myservo1.attach(9);        // initialize front door servo
    myservo2.attach(6);       // initialize back door servo
    myservo1.write(0);        //write front door servo to 0 degree
    myservo2.write(180);      //write back door servo to 180 degree

    /*set up LED, relay, and buzzer */
    pinMode(blue, OUTPUT);
    pinMode(green, OUTPUT);
    pinMode(red, OUTPUT);
    pinMode(speak, OUTPUT);
    pinMode(relay, OUTPUT);
    digitalWrite(blue, HIGH);
    digitalWrite(green, HIGH);
    digitalWrite(red, HIGH);
    digitalWrite(speak, HIGH);
    digitalWrite(relay, HIGH);

    MsTimer2::set(3000, getdatetime_andsensordata);     // set read sensor data trigger every 3 seconds//MsTimer2 "hardcode" 1 ms on timer 2
    MsTimer2::start();    // enable timer interrupt
  }

  // Main Code
  void loop() {
    /*state machine*/
    switch (state) {
      case IDLE:
        if (batvalue > 0.2) {                                                // check battery level; only check for fog event if theres enough battery
          if (Humidity > h && Distance < v && waterlevel < 20) {         // condition for fog event happen and water jar not full
            if (windspeed <= 6.5) {
              time1 = millis();                                              //Returns the number of milliseconds passed since program begin. 
              Serial.println("NORMAL TO WAIT");
              Serial.println("start 10 seconds count down");
              state = WAIT;                                                  //go to FAN if detect fog event and low wind speed
            }
            else if (windspeed > 6.5) {
              time1 = millis();
              Serial.println("NORMAL TO WAIT");
              Serial.println("start 10 seconds count down");
              state = WAIT;                                                  //go to NOFAN if detect fog event and high wind speed
            }
          }
          else {
            digitalWrite(red, HIGH);
            digitalWrite(blue, HIGH);
            digitalWrite(green, LOW);
            state = IDLE;                                                    //stay in IDLE if battery more than 40% and no fog event happening
          }
        }
        else if (batvalue <= 0.2) {
          Serial.println("NORMAL TO DATAONLY");
          state = DATAONLY;                                                  //go to data only state when battery is less than 40%
        }
        break;

      case WAIT:
            if (batvalue > 0.2){                                           //check bettery level
              if (Humidity > h && Distance < v && waterlevel < 20){    //if fog event still happening
              time2 = millis();                                            //get time2, use time2 - time1 to determine whether 10 minutes pass or not
                if(time2 - time1 >=buffertime && windspeed <=6.5){              //if 10 seconds passed and fog event still happening with a wind speed less than 6.5m/s
                  Serial.println("30 minutes passed");
                  Serial.println("");
                  Serial.println("");
                  Serial.println("WAIT TO FAN");
                  state = FAN;                                             //go to FAN mode
                }
                else if(time2 - time1 >=buffertime && windspeed >6.5){         //if 10 seconds passed and fog event still happening with a wind speed greater or equal than 6.5m/s
                  Serial.println("30 minutes passed");
                  Serial.println("");
                  Serial.println("");
                  Serial.println("WAIT TO NOFAN");
                  state = NOFAN;                                          //go to NOFAN mode
                } 
              }
              else if(Humidity <= h || Distance > v || waterlevel >= 20){  //if fog event disappear in the 10 seconds, go to NORMAL mode
                Serial.println("30 mintues waiting not passed");
                Serial.println("");
                Serial.println("");
                Serial.println("WAIT TO NORMAL");
                state = IDLE;
              }
            }
            else if (batvalue <= 0.2) {                                   //go to data only state when battery is less than 20%
            Serial.println("30 mintues waiting not passed");
            Serial.println("");
            Serial.println("");
            Serial.println("WAIT TO DATAONLY");
            state = DATAONLY;
            }
            break;  
                      
      case FAN:
        if (batvalue > 0.2) {
          if (windspeed <= 6.5) {
            if (Humidity > h && Distance < v && waterlevel < 20) {              // check fog event still happening and water jar not full    
              myservo1.write(90);                                                   // open front door
              myservo2.write(90);                                                   // open back door
              digitalWrite(relay, LOW);                                             // fan on
              digitalWrite(red, HIGH);                                              //turn on red green light only
              digitalWrite(blue, HIGH);
              digitalWrite(green, LOW);
              state = FAN;                                                          // make sure we stay in FAN state
            }
            else if (Humidity <= h || Distance > v || waterlevel >= 20) {        // if fog event no longer valid or water jar full
              myservo1.write(0);                                                    // front door close
              myservo2.write(180);                                                  // back door close
              digitalWrite(relay, HIGH);                                            // fan off
              Serial.println("FAN TO NORMAL");
              state = IDLE;                                                         // go back to NORMAL state
            }
          }

          else if (windspeed > 6.5) {                                              // if wind speed increase and pass the threshold value, go to NO FAN state
            Serial.println("FAN TO NOFAN");
            state = NOFAN;
          }
        }
        else if (batvalue <= 0.2) {                                                // go to data only state when battery is less than 40%
          Serial.println("FAN TO DATAONLY");
          state = DATAONLY;
        }
        break;

      case NOFAN:
        if (batvalue > 0.2) {
          if (windspeed > 6.5) {                                                    // only stay in state if windspeed high
            if (Humidity > h && Distance < v && waterlevel < 20) {              // check fog event still happening and water jar not full
              myservo1.write(90);                                                   // open front door
              myservo2.write(90);                                                   // open back door
              digitalWrite(relay, HIGH);                                            // fan off
              digitalWrite(red, HIGH);                                              //green led on only
              digitalWrite(blue, HIGH);
              digitalWrite(green, LOW);
              state = NOFAN;                                                        // stay in NO FAN state
            }
            else if (Humidity <= h || Distance > v || waterlevel >= 20) {        // if fog event no longer valid or water jar full
              myservo1.write(0);                                                    // front door close
              myservo2.write(180);                                                  // back door close
              digitalWrite(relay, HIGH);                                            // fan off
              Serial.println("NOFAN TO NORMAL");
              state = IDLE;                                                         // go back to NORMAL state
            }
          }
          else if (windspeed <= 6.5) {                                              // if wind speed is low then threshold value, switch to FAN state
            Serial.println("NOFAN TO FAN");
            state = FAN;
          }
        }
        else if (batvalue <= 0.2) {                                                // go to data only state when battery is less than 20%
          Serial.println("NOFAN TO DATAONLY");
          state = DATAONLY;
        }
        break;

      case DATAONLY:
        if (batvalue <= 0.15) {                                                     //if battery level low than 20%, go to critical(low power mode)
          Serial.println("DATAONLY TO CRITICAL");
          state = CRITICAL;
        }
        else if (batvalue > 0.2) {                                                 //go to normal mode if battery level greater than 20%
          Serial.println("DATAONLT TO NORMAL");
          digitalWrite(red, HIGH);
          digitalWrite(blue, HIGH);
          digitalWrite(green, LOW);
          state = IDLE;
        }
        else if (batvalue > 0.15 && batvalue <= 0.2) {                            //stay in DATAONLY mode and turn on blue led
          digitalWrite(red, HIGH);
          digitalWrite(blue, LOW);                                                //turn on blue LED only
          digitalWrite(green, HIGH);
          myservo1.write(0);                                                      // front door close
          myservo2.write(180);                                                    // back door close
          digitalWrite(relay, HIGH);                                              // fan off
          state = DATAONLY;
        }
        break;

      case CRITICAL:
        if (batvalue > 0.15) {                                                    //go to DATAONLY if battery level greater than 15%
          Serial.println("CRITICAL TO DATAONLY");
          MsTimer2::start();                                                     //restart interrupt in case it is cloesd in the low power mode
          state = DATAONLY;
        }
        else if (batvalue <= 0.15) {
          digitalWrite(red, LOW);                                                //turn on red led only
          digitalWrite(green, HIGH);
          digitalWrite(blue, HIGH);
          MsTimer2::stop();                                                      //stop 3 seconds interrupt when arduino go to sleep mode
          Serial.print("Arduino go to sleep for ");
          Serial.print(sleeptime*4);
          Serial.println(" seconds");
          delay(250);                                                            //half second to let text print out before go to sleep mode
          sleepmode();                                                           //sleep X minutes determined by user
          delay(250);                                                            //wait half second for arduino to wake up and read battery level
          state = CRITICAL; 
        }
        break;
    }
  }
  void sleepmode(){                                                             //sleep X minutes determined by user
    counter = 0;
    while(counter < sleeptime){
       WatchdogEnable (0b100001);  // 8 seconds
       WatchdogEnable (0b100001);  // 8 seconds
       WatchdogEnable (0b100001);  // 8 seconds
       WatchdogEnable (0b100001);  // 8 seconds
       WatchdogEnable (0b100001);  // 8 seconds
       WatchdogEnable (0b100001);  // 8 seconds
       WatchdogEnable (0b100001);  // 8 seconds
       WatchdogEnable (0b100000);  // 4 seconds                                total 60 seconds
       counter=counter+1;                                                      //go to next round and check counter and sleep time
    }
    Serial.print(sleeptime);
    Serial.println( "minutes has passed");
    batvoltag =  analogRead(pinBAT);                                       //Read bettery level, when arduino wake up
    batvalue = batvoltag * (5.0 / 1024.0);
  }
