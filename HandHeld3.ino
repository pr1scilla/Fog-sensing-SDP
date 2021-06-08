//(Fog Sensing Team)

//Libraries needed


#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <nRF24L01.h>
#include <RF24.h>
#include <SD.h>
#include <DigitalIO.h>
#include <printf.h>
#include <RF24_config.h>


//Initate the radio pin for the CE and CSN of the hardware pin of the actual transciever
RF24 radio (33, 34);

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

//How we will enter the SD file.

File myFile;



//the variable used to keep track of the time while debouneing



//the number of the push button (we are using interrupts for the buttons)
//the interrupts buttons for the mega are ( 2,3,21,20,19,18);

const int buttonPin = 2;
const int buttonPin2 = 3;
const int buttonPin3 = 19;
//const int buttonPin4 = 29;
const int ledPin =  13;      // the number of the LED pin

int i = 0;

//initalizing the states of the button to be off. Interrupt will be activating it to be high
volatile byte buttonState = LOW;           //the variable for reading the pushbutton status
volatile byte buttonState2 = LOW;
volatile byte buttonState3 = LOW;    //this button will be acting as the signal connection to transciever for now
//volatile byte buttonState4 = 0;



// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 1000;    // the debounce time; increase if the output flickers

//the address which are going to be used for the transciever
const byte address [6] = "00001"; //for sending
const byte address2 [6] = "00002"; //for recieving


//counter for the recycling the buttons

int buton1 = 0;
int buton2 = 0;
int buton3 = 0;


//Strings for recieveing data
char recieve[] = "Damn Famn this shit is crazy " ;//what we are sending
char text[32] = ""; //where we recieve a string
char hands[32] = "FSG";
char getdata[32] = "";
char datos[32] = "data";
char sdcr[32] = "sdcr";
char rec[] = "";
unsigned long starts;
unsigned long check = 1000;




//the strings which will be storing the information from the SD card.
//the maximum the LCD screen can display is 20 characters.
char dateStampE [20];
char timeStampE [20];
char fogStateE [20];
char batlevelE [20];
char tempE [20];
char visE[20];
char humidE[20];
char waterE[20];
char windE [20];

//wtf

float humidL;
float tempL;
float visL ;
float windL ;
float waterL;
float batL ;
float fogL ;
float endL ;

char fogIn [4] = "";



//the different states for the state machine ( its bound to change)
enum fogState {
  HANDSHAKE, DISPLAYER, TRANSFER, READER, ENGLISH, SUCCESS, SECONDL, ERRORS, ERRORS2, TRANSFER2, SUCCESS2
} ;

//Another set of state for once youre displaying the language
enum SenInfo {
  INFO1, INFO2, INFO3
} ;

//set of states for interpreting the transciever
enum DataManagement {
  SEND, RECIEVE
};

//Make the states available and set them to the very beginning
fogState states = HANDSHAKE;
SenInfo sensorENG = INFO1;
SenInfo sensor2L = INFO1;
DataManagement handing = SEND;
DataManagement dataSending = SEND;
DataManagement transferD = SEND;




// this helper function is for printing a string stating that the handshake was unsuccessfull
void handShakeFailure() {
  Serial.println("There was an error with the handshake.Press button 1 to retry");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print ("There is an error");
  lcd.setCursor(0, 1);
  lcd.print ("with the primary ");
  lcd.setCursor(0, 2);
  lcd.print ("connection. Press");
  lcd.setCursor(0, 3);
  lcd.print ("button 1 to retry. ");

}
void handShakeSuccess() {
  Serial.println("There was no error with the handshake.Press button 2 for english 3 for spanish ");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print ("The handshake was");
  lcd.setCursor(0, 1);
  lcd.print ("successful.Press ");
  lcd.setCursor(0, 2);
  lcd.print ("button 1 to ");
  lcd.setCursor(0, 3);
  lcd.print ("request data. ");

}
//This will read the data and store them into printable strings
void sdReader() {





  /*
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }


    Serial.print("Initializing SD card...");
    //  lcd.print ("Initializing SD card...");

    if (!SD.begin(53)) {
      Serial.println("initialization failed!");
      while (1);
    }
    Serial.println("initialization done.");


    // re-open the file for reading:
    myFile = SD.open("HHD.txt");
    if (myFile) {
      Serial.println("HHD.txt:");

      // read from the file until there's nothing else in it:
      myFile.read(dateStampE, 10);
      myFile.seek(13);
      myFile.read(timeStampE, 8);
      myFile.seek(41);
      myFile.read(humidE, 2);
      myFile.seek(63);
      myFile.read(tempE, 3);
      myFile.seek(74);
      myFile.read(visE, 7);
      myFile.seek (100);
      myFile.read(windE, 2);

      myFile.seek(121);
      myFile.read(waterE, 2);

      myFile.seek(145);
      myFile.read(batlevelE, 4);

      myFile.seek(163);
      myFile.read(fogStateE, 3);



      // close the file:
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
  */
}

void successDataScreen() {

  // Print a message to the LCD.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Successful Transfer");
  lcd.setCursor(0, 1);
  lcd.print("Press Button 2 to ");
  lcd.setCursor(0, 2);
  lcd.print("display sensor");
  lcd.setCursor(2, 3);
  lcd.print("information!");
  delay(1000);

}

void DataScreen() {

  // Print a message to the LCD.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("We are collecing");
  lcd.setCursor(0, 1);
  lcd.print("data.Give it a ");
  lcd.setCursor(0, 2);
  lcd.print("minute or two. ");
  lcd.setCursor(2, 3);
  lcd.print("");
  delay(1000);

}

//temporary functions to print sensor information.
//used in order to demonstrate the cycling of buttons.


//This will display the date, time, and state of the fog chamber, and battery level
void Info1 () {
  lcd.clear();

  if (states == ENGLISH) {

    lcd.setCursor(0, 0);
    lcd.print("Date:");
    lcd.print("N/A");
    lcd.setCursor(0, 1);
    lcd.print("Time:");
    lcd.print("N/A 2");
    lcd.setCursor(0, 2);
    lcd.print("Fog Status:");
    lcd.print(fogIn);
    lcd.setCursor(0, 3);
    lcd.print("Battery Level:");
    lcd.print( batL);

  }

  if (states == SECONDL) {

    lcd.setCursor(0, 0);
    lcd.print("Fecha:");
    lcd.print("N/A");
    lcd.setCursor(0, 1);
    lcd.print("Tiempo:");
    lcd.print("N/A");
    lcd.setCursor(0, 2);
    lcd.print("Estado de desp.:");
    lcd.print(fogL);
    lcd.setCursor(0, 3);
    lcd.print("Bateria(%):");
    lcd.print( batL);

  }
  delay (1000);

}

//this will display the temperature, visibility,humidity
void Info2 () {
  lcd.clear();

  if (states == ENGLISH) {
    lcd.setCursor(0, 0);
    lcd.print("Temperature(C):");
    lcd.print(tempL);
    lcd.setCursor(0, 1);
    lcd.print("Visibility (m):");
    lcd.setCursor(0, 2);
    lcd.print(visL);
    lcd.setCursor(0, 3);
    lcd.print("Humidity(%):");
    lcd.print( humidL);

  }
  else if (states == SECONDL) {

    lcd.setCursor(0, 0);
    lcd.print("Temperatura(C):");
    lcd.print(tempL);
    lcd.setCursor(0, 1);
    lcd.print("Visibilidad (m):");
    lcd.setCursor(0, 2);
    lcd.print(visL);
    lcd.setCursor(0, 3);
    lcd.print("Humedad(%):");
    lcd.print( humidL);

  }

  delay (1000);
}
//display the water level, windspeed
void Info3 () {


  lcd.clear();

  if (states == ENGLISH) {
    lcd.setCursor(0, 0);
    lcd.print("Water Level(mm):");
    lcd.setCursor(5, 1);
    lcd.print(waterL);
    lcd.setCursor(0, 2);
    lcd.print("Wind Speed (m/s):");
    lcd.setCursor(5, 3);
    lcd.print(windL);
  }
  else if (states == SECONDL) {
    lcd.setCursor(0, 0);
    lcd.print("Estado de agua (mm):");
    lcd.setCursor(5, 1);
    lcd.print(waterL);
    lcd.setCursor(0, 2);
    lcd.print("Veloc. de Aire(m/s):");
    lcd.setCursor(5, 3);
    lcd.print(windL);
  }

  delay (1000);
}




void setup() {


  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);

  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode (buttonPin3, INPUT_PULLUP);


  //adding the interrupts
  attachInterrupt(digitalPinToInterrupt(buttonPin), check1, CHANGE);

  attachInterrupt(digitalPinToInterrupt(buttonPin2), check2, CHANGE);

  attachInterrupt(digitalPinToInterrupt(buttonPin3), check3, CHANGE);

  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.openWritingPipe(address2);
  radio.setAutoAck(1);
  radio.setRetries(2, 15);
  radio.setPALevel(RF24_PA_MIN);

  //printing the initialization string

  lcd.init();                      // initialize the lcd
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("We're doing the ");
  lcd.setCursor(0, 1);
  lcd.print("handshake to verify");
  lcd.setCursor(2, 2);
  lcd.print("the connection ");
  lcd.setCursor(2, 3);
  lcd.print("to the main node.");

  Serial.println ("The handshake signal will start once button 1 is pressed ");


}

void loop() {
  // put your main code here, to run repeatedly:


  switch (states) {

    //This state is for establishing connection with node.
    case HANDSHAKE:

      switch (handing) {

        //We are sending the handshake string of "FSG"
        case SEND:
          radio.stopListening ();

          if (buttonState == HIGH) {
            buttonState = LOW;

            Serial.println("The handshake signal is: ");
            radio.write(&hands, sizeof (hands));
            Serial.println(hands);

            handing = RECIEVE;
            //            radio.startListening();
            Serial.println("Handshake");

          }

          break;

        //We now begin to listen and wait for the fog chamber to send a string back "ABG"
        case RECIEVE:

          radio.startListening();
          delay(1000); 
       



          starts = millis();
          while ( !radio.available()) {
            // if the signal isn't recieved in 15 seconds we go back
            if (millis() - starts > 4000) {
              handShakeFailure();
              handing = SEND;
              Serial.println("HAD FIALED");
              return;
            }
          }

          radio.read(&getdata, sizeof(getdata));
          Serial.println(getdata);
          if (getdata[0] == 'A') {
            if (getdata[1] == 'B') {
              if (getdata[2] == 'G') {
                Serial.println("Connection Successful ");
                handShakeSuccess();
                states = TRANSFER;

              }
            }
          }

          break;
      }
      break;

    //In this state we send a string "data" in order to start recieve the data and storing it in the SD card
    case TRANSFER:

      switch (dataSending) {

        case SEND:

          radio.stopListening ();

          if (buttonState2 == HIGH) {
            buttonState2 = LOW;
            Serial.print("The instant signal sent  is :");
            radio.write(&datos, sizeof (datos));
            Serial.println(datos);

            dataSending = RECIEVE;

            // radio.startListening();
          }

          break;
        case RECIEVE:



          radio.startListening();
          delay(1000); 
          
       

          starts = millis();
          while ( !radio.available()) {
            // if the signal isn't recieved in 15 seconds we go back
            if (millis() - starts > 4000) {
              handShakeFailure();
              dataSending = SEND;
              Serial.println("HAD FIALED");
              return;
            }
          }

        //  while (radio.available()) {
            float dataRecieve[8];
            radio.read(&dataRecieve, sizeof (dataRecieve));

            humidL = dataRecieve[0];
            tempL = dataRecieve[1];
            visL = dataRecieve[2];
            windL = dataRecieve[3];
            waterL = dataRecieve[4];
            batL = dataRecieve[5] ;
            fogL = dataRecieve[6];
            endL = dataRecieve[7];

            if (fogL == 0) {

              fogIn[4] = "OFF";

            } else if (fogL == 1) {
              fogIn[4] = "ONN";
            }
            Serial.println(humidL);
            Serial.println(tempL);
            Serial.println(visL);
            Serial.println(windL);
            Serial.println(waterL);
            Serial.println(batL);
            Serial.println(fogL);
            Serial.print("End Val:");
            Serial.println(endL);

            if (dataRecieve[7] == 129.21) {
              states = READER;
              Serial.println("Success sending instant "); 
            }

            else {
              Serial.println("failure getting instant"); 
              dataSending = SEND;
            }

          //while radio.available}

         
          break;
      }

      break;


    case READER:

      switch (transferD) {

        case SEND:
          radio.stopListening ();

          if (buttonState3 == HIGH) {
            buttonState3 = LOW;

            Serial.println("The transfer signal is: ");
            radio.write(&sdcr, sizeof (sdcr));
            Serial.println(sdcr);
            DataScreen();
            transferD = RECIEVE;
            //            radio.startListening();
            Serial.println("Moving to receive data");

          }

          break;

        //We now begin to listen and wait for the fog chamber to send a string back "ABG"
        case RECIEVE:

          radio.startListening();


          if (!SD.begin(53)) {
            Serial.println("initialization failed!");
            while (1);
          }
          myFile = SD.open("HHD.txt", FILE_WRITE);

          if (myFile) {

            if (radio.available()) {

              radio.read(&text, sizeof(text));
              Serial.print(text);
              myFile.print(text);
            }
            myFile.close();

            for (i = 0; i < 32; i++) {
              rec[i] = text[i];
              if (rec[i] == '&') {
                successDataScreen();
                ;
              }
            }
          }

          break;
      }



      //Serial.println("READER");
      //data read and want to display english option
      if (buttonState2 == HIGH) {
        //reseet the button state
        buttonState2 = LOW;
        //initally we start with info 1 at english
        sdReader();
        sensorENG = INFO1;
        states = ENGLISH;

      } else if (buttonState3 == HIGH) {
        sdReader();
        sensor2L = INFO1;
        states = SECONDL;

      }


      break;

    case ENGLISH:

      switch (sensorENG) {

        case INFO1:
          //display the first set of information
          Serial.println("We are in INFO1");
          Info1();
          //check which button is being pressed.

          if (buttonState == HIGH) {
            //reset the buttonState to low
            buttonState = LOW;
            //Change the state read data again.
            states = TRANSFER;

          }
          //wants to display the next data
          else if (buttonState2 == HIGH) {
            //reset the buttonState
            buttonState2 = LOW;

            states = ENGLISH;
            sensorENG = INFO2;

          } else    if (buttonState3 == HIGH) {
            buttonState3 = LOW;
            states = SECONDL;
            sensor2L = INFO1;


          }

          break;

        case INFO2:


          Info2();
          if (buttonState == HIGH) {
            //reset the button and states
            buttonState = LOW;
            states = TRANSFER;
            sensorENG = INFO1;
          }
          //want to get the next data set
          else if (buttonState2 == HIGH) {
            //reset the buttonState
            buttonState2 = LOW;

            states = ENGLISH;
            sensorENG = INFO3;

          } else    if (buttonState3 == HIGH) {
            buttonState3 = LOW;
            states = SECONDL;
            sensor2L = INFO2;


          }

          break;

        case INFO3:

          Info3();

          if (buttonState == HIGH) {
            buttonState == LOW;
            states = TRANSFER;
            sensorENG = INFO1;
          }

          else if (buttonState2 == HIGH) {
            //reset the buttonState
            buttonState2 = LOW;

            states = ENGLISH;
            sensorENG = INFO1;

          } else    if (buttonState3 == HIGH) {
            buttonState3 = LOW;
            states = SECONDL;
            sensor2L = INFO3;


          }

          break;

      }


      break;

    case SECONDL:


      switch (sensor2L) {

        case INFO1:
          //display the first set of information

          Info1();


          //check which button is being pressed.

          if (buttonState == HIGH) {
            //reset the buttonState to low
            buttonState = LOW;
            //Change the state to  read data again.
            states = TRANSFER;

          }
          else if (buttonState2 == HIGH) {
            //reset the buttonState
            buttonState2 = LOW;

            states = ENGLISH;
            sensorENG = INFO1;

          }
          //wants to display the next data
          else    if (buttonState3 == HIGH) {
            buttonState3 = LOW;
            states = SECONDL;

            sensor2L = INFO2;


          }

          break;

        case INFO2:


          Info2();


          if (buttonState == HIGH) {
            //reset the button and states
            buttonState = LOW;
            states = TRANSFER;
            sensor2L = INFO1;
          }
          else if (buttonState2 == HIGH) {
            //reset the buttonState
            buttonState2 = LOW;

            states = ENGLISH;
            sensorENG = INFO2;

          }
          //want to get the next data set
          else    if (buttonState3 == HIGH) {
            buttonState3 = LOW;
            states = SECONDL;
            sensor2L = INFO3;


          }

          break;

        case INFO3:

          Info3();


          if (buttonState == HIGH) {
            buttonState == LOW;
            states = TRANSFER;
            sensor2L = INFO1;
          }
          else if (buttonState2 == HIGH) {
            //reset the buttonState
            buttonState2 = LOW;

            states = ENGLISH;
            sensorENG = INFO3;

          }

          else    if (buttonState3 == HIGH) {
            buttonState3 = LOW;
            states = SECONDL;
            sensor2L = INFO1;


          }

          break;

      }


      break;

  }
}


void check1() {

  if (millis() - lastDebounceTime < debounceDelay) {
    return;
  }
  lastDebounceTime = millis();


  buttonState = HIGH;

}

void check2 () {
  if (millis() - lastDebounceTime < debounceDelay) {
    return;
  }
  lastDebounceTime = millis();

  buttonState2 = HIGH;

}

void check3() {
  if (millis() - lastDebounceTime < debounceDelay) {
    return;
  }
  lastDebounceTime = millis();

  buttonState3 = HIGH;
}
