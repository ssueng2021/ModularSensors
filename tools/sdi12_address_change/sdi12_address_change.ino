/*
########################
#        OVERVIEW      #
########################

 Example B: Changing the address of a sensor.

 This is a simple demonstration of the SDI-12 library for arduino.
 It discovers the address of the attached sensor and allows you to change it.

#########################
#      THE CIRCUIT      #
#########################

 The circuit: You should not have more than one SDI-12 device attached for this
example.

 See:
 https://raw.github.com/Kevin-M-Smith/SDI-12-Circuit-Diagrams/master/basic_setup_no_usb.png
 or
 https://raw.github.com/Kevin-M-Smith/SDI-12-Circuit-Diagrams/master/compat_setup_usb.png

###########################
#      COMPATIBILITY      #
###########################

 This library requires the use of pin change interrupts (PCINT).
 Not all Arduino boards have the same pin capabilities.
 The known compatibile pins for common variants are shown below.

 Arduino Uno:     All pins.

 Arduino Mega or Mega 2560:
 10, 11, 12, 13, 14, 15, 50, 51, 52, 53, A8 (62),
 A9 (63), A10 (64), A11 (65), A12 (66), A13 (67), A14 (68), A15 (69).

 Arduino Leonardo:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI)

#########################
#      RESOURCES        #
#########################

 Written by Kevin M. Smith in 2013.
 Contact: SDI12@ethosengineering.org

 The SDI-12 specification is available at: http://www.sdi-12.org/
 The library is available at: https://github.com/EnviroDIY/Arduino-SDI-12
*/


#include <EnableInterrupt.h>
#include <SDI12.h>

#define SERIAL_BAUD 115200  // The baud rate for the output serial port
#define DATA_PIN 7          // The pin of the SDI-12 data bus
#define POWER_PIN 22  // The // Sensor power pin (or -1 if not switching power)

// Define the SDI-12 bus
SDI12 mySDI12(DATA_PIN);

String myCommand  = "";   // empty to start
char   oldAddress = '!';  // invalid address as placeholder


// this checks for activity at a particular address
// expects a char, '0'-'9', 'a'-'z', or 'A'-'Z'
boolean
checkActive(byte i) {  // this checks for activity at a particular address
    Serial.print("\n\rChecking address ");
    Serial.print(static_cast<char>(i));
    Serial.print("...");
    myCommand = "";
    myCommand +=
        static_cast<char>(i);  // sends basic 'acknowledge' command [address][!]
    myCommand += "!";

    for (int j = 0; j < 3; j++) {  // goes through three rapid contact attempts
        mySDI12.clearBuffer();
        mySDI12.sendCommand(myCommand);
        delay(30);
        if (int avl = mySDI12.available()) {  // If we here anything, assume we have an
                                    // active sensor
            Serial.print(avl);
            Serial.print("]Occupied ~ ");
            mySDI12.clearBuffer();
            myCommand = "";
            myCommand += static_cast<char>(i);  //  [address][!]
            myCommand += "I!";
            mySDI12.sendCommand(myCommand);
            delay(30);
            String sdiResponse = mySDI12.readStringUntil('\n');
            sdiResponse.trim();
            Serial.println( sdiResponse);
            return true;
        } else {
            Serial.print("  Vacant");  // otherwise it is vacant.
            //mySDI12.clearBuffer();
        }
    }
    return false;
}


void setup() {
    Serial.begin(SERIAL_BAUD);
    while (!Serial)
        ;

    // Enable interrupts for the recieve pin
    pinMode(DATA_PIN, INPUT_PULLUP);
    enableInterrupt(DATA_PIN, SDI12::handleInterrupt, CHANGE);

    // Power the sensors;
    if (POWER_PIN > 0) {
        Serial.println("Powering up sensors...");
        pinMode(POWER_PIN, OUTPUT);
        digitalWrite(POWER_PIN, HIGH);
        delay(200);
    }

    Serial.println("\n\r\n\rr1 Opening SDI-12 bus...");
    mySDI12.begin();
    delay(500);  // allow things to settle

    checkActive('0'); //Throw away
}

void loop() {
    boolean found = false;  // have we identified the sensor yet?

    for (byte i = '0'; i <= '9'; i++) {  // scan address space 0-9
        if (found) break;
        if (checkActive(i)) {
            found      = true;
            oldAddress = i;
        }
    }

    for (byte i = 'a'; i <= 'z'; i++) {  // scan address space a-z
        if (found) break;
        if (checkActive(i)) {
            found      = true;
            oldAddress = i;
        }
    }

    for (byte i = 'A'; i <= 'Z'; i++) {  // scan address space A-Z
        if (found) break;
        if (checkActive(i)) {
            found      = true;
            oldAddress = i;
        }
    }

    if (!found) {
        Serial.println(
            "\n\rNo sensor detected. Check physical connections.");  // couldn't
                                                                 // find a
                                                                 // sensor.
                                                                 // check
                                                                 // connections..
    } else {
        Serial.print("Sensor active at address ");  // found a sensor!
        Serial.print(oldAddress);
        Serial.println(".");

        Serial.println("Enter new address.");  // prompt for a new address
        while (!Serial.available())
            ;
        char newAdd = Serial.read();

        // wait for valid response
        while (((newAdd < '0') || (newAdd > '9')) &&
               ((newAdd < 'a') || (newAdd > 'z')) &&
               ((newAdd < 'A') || (newAdd > 'Z'))) {
            if (!(newAdd == '\n') || (newAdd == '\r') || (newAdd == ' ')) {
                Serial.println("Not a valid address. Please enter '0'-'9', "
                               "'a'-'A', or 'z'-'Z'.");
            }
            while (!Serial.available())
                ;
            newAdd = Serial.read();
        }

        // the syntax of the change address command
        // is:[currentAddress]A[newAddress]!
        Serial.print("Readdressing sensor. From ");
        Serial.print(oldAddress);
        Serial.print(" to ");
        Serial.println(newAdd);
        myCommand = "";
        myCommand += static_cast<char>(oldAddress);
        myCommand += "A";
        myCommand += static_cast<char>(newAdd);
        myCommand += "!";
        mySDI12.sendCommand(myCommand);

        // wait for the response then throw it away by clearing the buffer with
        // clearBuffer()
        delay(300);
        mySDI12.clearBuffer();

        Serial.println(" Rescanning for verification.");
    }
}
