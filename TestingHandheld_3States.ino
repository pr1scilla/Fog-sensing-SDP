/*
      Fog Sensing Wireless File Transfer

      by Andrew Calvo and Brandon Lem

*/

#include <printf.h>
#include <RF24_config.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SD.h>
#include <DigitalIO.h>


//Intializations
RF24 radio(33, 34); // CE, CSN
File myFile;
const byte address[6] = "00001";
const byte address2[6] = "00002";
//Global Declaration
char buf[32] = "";
char bs[32] = "";
int i = 0;
int k = 0;
char checker[32] = "ABG";
char reader [5] = "";
char reck[5] = "";
char sender[5] = "";

const int buttonPin = 2; //button interrupt
volatile byte buttonState = LOW;
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 1000;    // the debounce time; increase if the output flickers

//blue is csn

enum shaker {
  BS, CS
};
enum states {
  DATA, HANDSHAKE, TRANSFER, NORMAL,
};

states MAIN = HANDSHAKE;
shaker hander = BS;
shaker datos = BS;
shaker trnsf = BS;

//whole bunch of values that are obtained by the node

float humidL = 49;
float tempL = 27;
float visL = 967.79;
float windL = 7;
float waterL = 30;
float batL = 0.54;
float fogL = 0;
float endL = 129.21;


void setup() {
  pinMode (buttonPin, INPUT_PULLUP);



  //Turn on Serial port and Radio
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address2);
  radio.openWritingPipe(address);
  radio.setAutoAck(1);
  radio.setRetries(2, 15);
  radio.setPALevel(RF24_PA_MIN);
}

void loop() {

  switch (MAIN) {
    case HANDSHAKE:

      switch (hander) {

        case BS:
          radio.startListening();
          while (radio.available ()) {

            radio.read (&reader, sizeof(reader));

            Serial.println ("This is what we're recieving  ");
            Serial.println(reader);


            if (reader[0] == 'F') {
              if (reader[1] == 'S') {
                if (reader[2] == 'G') {

                  hander = CS;

                }
              }
            }
          }

          break;

        case CS:

          radio.stopListening();

          Serial.println("This what  we are sending:");
          radio.write (&checker, sizeof (checker));

          Serial.println(checker);

          MAIN = DATA;
          break;
      }



      // radio.write(&reader, sizeof(reader));
      //   Serial.println(reader);
      /*
        if (reader[0] == 'F') {
        if (reader[1] == 'S') {
          if (reader[2] == 'G') {



          }
        }

        }

      */



      //Serial.println("SHOULD WORK");

      //if (reader == "FSG\n") {

      //radio.write(&checker, sizeof(checker));
      //Serial.println(checker);
      //  }


      //if (reader == checker) {
      //MAIN = DATA;
      // }



      break;
    case DATA:


      switch (datos) {

        case BS:

          radio.startListening();
          while (radio.available ()) {

            radio.read (&reck, sizeof(reck));

            Serial.println ("This is what we're recieving  ");
            Serial.println(reck);


            if (reck[0] == 'd') {
              if (reck[1] == 'a') {
                if (reck[2] == 't') {
                  if (reck[3] == 'a') {

                    Serial.println("Success will send the data ");
                    datos = CS;
                  }
                }
              }
            }
            else if (reck[0] == 'F') {
              if (reck[1] == 'S') {
                if (reck[2] == 'G') {

                  Serial.println("Still Recieving FSG. Going to send ABG  ");

                  hander = CS;
                  MAIN = HANDSHAKE;
                }
              }
            }

          }


          break;

        case CS:

          radio.stopListening();
          float dataSending[8];
          dataSending[0] = humidL;

          dataSending[1] = tempL;
          dataSending[2] = visL;
          dataSending[3] = windL;
          dataSending[4] = waterL;
          dataSending[5] = batL;
          dataSending[6] = fogL;
          dataSending[7] = endL;

          radio.write(&dataSending, sizeof(dataSending));
          MAIN = TRANSFER;
          trnsf = BS;

          break;



      }
      break;

    case TRANSFER:

      switch (trnsf) {
        case BS:
          radio.startListening();

          while (radio.available ()) {

            radio.read (&sender, sizeof(sender));

            Serial.println ("This is what we're recieving for data transfer ");
            Serial.println(sender);


            if (sender[0] == 's') {
              if (sender[1] == 'd') {
                if (sender[2] == 'c') {
                  if (sender[3] == 'r') {

                    Serial.println("sending the data>wtf ");
                    trnsf = CS;
                  }
                }
              }
            }
            else if (sender[0] == 'd') {
              if (sender[1] == 'a') {
                if (sender[2] == 't') {
                  if (sender[3] == 'a') {




                    Serial.println("Theres an error ");

                    datos = BS;
                    MAIN = DATA;
                  }
                }
              }
            }


          }
          break;
        case CS:

          radio.stopListening();
          //SD card Initialization
          Serial.print("Initializing SD card...");

          if (!SD.begin(53)) {
            Serial.println("initialization failed!");
            while (1);
          }




          Serial.println("initialization done.");
          myFile = SD.open("testing.txt");



          // TIME TO SEND A FILE WIRELESSLY BABY
          // So basically while there is another character available to read from the file,
          // we'll read it, store it in our 32 character buffer, and send it

          while (myFile.available())
          {

            buf[i] = myFile.read(); //store character from file in buffer

            //increment index counter and enter conditional in the event we've filled our buffer
            i++;
            if (i == 31)
            {

              Serial.print((buf));
              radio.write(buf, sizeof(buf));
              delay(100);
              i = 0;
            }
          }


          //if there are any leftovers, they get sent here
          if (i > 0) {
            //  Serial.println("test1");
            radio.flush_tx();//make sure to clear the fifo since we're not completely filling it here
            radio.write(buf, i);
            Serial.write(buf, i);
            i = 0;
            Serial.println("");
            Serial.println("Last stream of less than 32 bytes");
          }

          Serial.println("end of code");

          for (k = 0; k < 32; k++) {
            bs[k] = buf[k];
            if (bs[k] = '&') {

              Serial.println("SUCCCCESSSS"); 

            }
          }
          
          break;
      }





      break;
    case NORMAL:
      radio.startListening();
      delay(1000);

      while (radio.available ()) {

        radio.read (&sender, sizeof(sender));

        Serial.println ("This is what we're recieving for data transfer ");
        Serial.println(sender);


        if (sender[0] == 's') {
          if (sender[1] == 'd') {
            if (sender[2] == 'c') {
              if (sender[3] == 'r') {

                Serial.println("sending the data>wtf ");
                trnsf = CS;
                MAIN = TRANSFER;
              }
            }
          }

        }
      }

      break;
  }


}


void bpress () {
  if (millis() - lastDebounceTime < debounceDelay) {
    return;
  }
  lastDebounceTime = millis();


  buttonState = HIGH;

}
