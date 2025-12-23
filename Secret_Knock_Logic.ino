#include <Wire.h>        // Library for I2C communication
#include <ADCTouch.h>    // Library for capacitive touch sensing

#define MYADDRESS 0x06  // Define the I2C slave address

// Structure to hold I2C message
struct message {
  uint8_t mid;  // Message ID
  uint8_t msg;  // Message content
};

// Default response message
struct message repMsg = {0, 0}; 

// Capacitive touch sensor reference values
int ref1, ref2, ref3;
int threshold = 60;  // Touch sensitivity threshold

// Debounce time to prevent multiple rapid detections
unsigned long debounceTime = 300;
unsigned long lastTouchTime1 = 0, lastTouchTime2 = 0, lastTouchTime3 = 0;

// Game-related timing variables
unsigned long lastUpdateTime = 0;
unsigned long gameStartTime = 0;
unsigned long gameDuration = 30000;  // Game duration in milliseconds
int score = 0;  // Player's score
int activeMole = -1;  // Stores the currently active mole
unsigned long moleTimeLimit = 1000;  // Time before mole disappears
unsigned long moleStartTime = 0;
unsigned long moleAppearInterval = 1000;  // Interval for new mole appearance
unsigned long nextMoleTime = 0;

// LED timeout durations
unsigned long redTimeout = 500;
unsigned long greenTimeout = 500;

// Flags for game state
bool moleHit = false;
bool gameActive = false;

// Score threshold to pass the game
int passScore = 10;

// LED pin assignments for three mole positions
int redLED1 = 6, yellowLED1 = 5, greenLED1 = 4;
int redLED2 = 9, yellowLED2 = 8, greenLED2 = 7;
int redLED3 = 12, yellowLED3 = 11, greenLED3 = 10;

void setup() {
    Serial.begin(9600);  // Initialize serial communication
    delay(1000);  // Delay for stable startup

    Wire.begin(MYADDRESS);  // Initialize I2C with the assigned address
    Wire.onReceive(dataRcv);  // Register function to handle received data
    Wire.onRequest(dataReq);  // Register function to handle data requests

    // Read baseline touch values for calibration
    ref1 = ADCTouch.read(A3, 100);
    ref2 = ADCTouch.read(A2, 100);
    ref3 = ADCTouch.read(A1, 100);

    // Set LED pins as outputs
    pinMode(redLED1, OUTPUT);
    pinMode(yellowLED1, OUTPUT);
    pinMode(greenLED1, OUTPUT);
    pinMode(redLED2, OUTPUT);
    pinMode(yellowLED2, OUTPUT);
    pinMode(greenLED2, OUTPUT);
    pinMode(redLED3, OUTPUT);
    pinMode(yellowLED3, OUTPUT);
    pinMode(greenLED3, OUTPUT);

    Serial.println("Whack-a-Mole Ready! Waiting for Master Command...");
}

void loop() {
    // If game is not active, do nothing
    if (!gameActive) return;

    // Check if game time has elapsed
    if (millis() - gameStartTime > gameDuration) {
        Serial.println("Game Over!");
        Serial.print("Final Score: ");
        Serial.println(score);

        // Check if player meets the passing score
        if (score >= passScore) {
            Serial.println("Game Passed!");
            repMsg = {1, 1};  // Message indicating game passed
        } else {
            Serial.println("Game Failed!");
            repMsg = {1, 0};  // Message indicating game failed
        }

        resetAllLEDs();  // Turn off all LEDs
        gameActive = false;  // Stop the game
        return;
    }

    // Touch sensor reading and processing (every 50ms)
    if (millis() - lastUpdateTime > 50) {  
        lastUpdateTime = millis();
        int value1 = ADCTouch.read(A3, 100) - ref1;
        int value2 = ADCTouch.read(A2, 100) - ref2;
        int value3 = ADCTouch.read(A1, 100) - ref3;

        // Check if player touched any mole within debounce time
        if (value1 > threshold && millis() - lastTouchTime1 > debounceTime) {
            handleMoleHit(1, greenLED1, yellowLED1);
            lastTouchTime1 = millis();
        }
        if (value2 > threshold && millis() - lastTouchTime2 > debounceTime) {
            handleMoleHit(2, greenLED2, yellowLED2);
            lastTouchTime2 = millis();
        }
        if (value3 > threshold && millis() - lastTouchTime3 > debounceTime) {
            handleMoleHit(3, greenLED3, yellowLED3);
            lastTouchTime3 = millis();
        }
    }

    // Check if active mole has timed out
    if (activeMole != -1 && millis() - moleStartTime > moleTimeLimit) {
        if (!moleHit) {
            Serial.println("Missed the mole!");
            triggerRedLightForActiveMole();  // Indicate missed mole
        }
        activeMole = -1;  // Reset active mole
        moleHit = false;
        resetAllLEDs();
        nextMoleTime = millis() + moleAppearInterval;
    }

    // Generate new mole at a random position
    if (millis() >= nextMoleTime && activeMole == -1) {
        resetAllLEDs();
        activeMole = random(1, 4);
        moleStartTime = millis();
        Serial.print("Active Mole: ");
        Serial.println(activeMole);

        // Light up the yellow LED for the active mole
        if (activeMole == 1) digitalWrite(yellowLED1, HIGH);
        if (activeMole == 2) digitalWrite(yellowLED2, HIGH);
        if (activeMole == 3) digitalWrite(yellowLED3, HIGH);
    }
}

// Function to reset all LEDs
void resetAllLEDs() {
    digitalWrite(redLED1, LOW); digitalWrite(yellowLED1, LOW); digitalWrite(greenLED1, LOW);
    digitalWrite(redLED2, LOW); digitalWrite(yellowLED2, LOW); digitalWrite(greenLED2, LOW);
    digitalWrite(redLED3, LOW); digitalWrite(yellowLED3, LOW); digitalWrite(greenLED3, LOW);
}

// Function to handle a successful mole hit
void handleMoleHit(int mole, int greenLED, int yellowLED) {
    if (activeMole == mole) {
        score++;
        Serial.print("Whacked Mole ");
        Serial.println(mole);
        digitalWrite(greenLED, HIGH);
        digitalWrite(yellowLED, LOW);
        moleHit = true;
        delay(greenTimeout);
        digitalWrite(greenLED, LOW);
    } else {
        Serial.println("Wrong Mole! Mole got away.");
        triggerRedLightForActiveMole();  // Indicate wrong hit
    }
}

// Function to trigger red LED if player misses or hits the wrong mole
void triggerRedLightForActiveMole() {
    if (activeMole == 1) {
        digitalWrite(redLED1, HIGH); digitalWrite(yellowLED1, LOW);
        delay(redTimeout);
        digitalWrite(redLED1, LOW);
    } else if (activeMole == 2) {
        digitalWrite(redLED2, HIGH); digitalWrite(yellowLED2, LOW);
        delay(redTimeout);
        digitalWrite(redLED2, LOW);
    } else if (activeMole == 3) {
        digitalWrite(redLED3, HIGH); digitalWrite(yellowLED3, LOW);
        delay(redTimeout);
        digitalWrite(redLED3, LOW);
    }
}

// Function to handle I2C data reception
void dataRcv(int numBytes) {
    if (Wire.available() == sizeof(message)) {
        message rcvMsg;
        Wire.readBytes((uint8_t*)&rcvMsg, sizeof(message));

        switch (rcvMsg.mid) {
            case 2:  // Start game command
                if (!gameActive) {
                    Serial.println("Starting Game via Master...");
                    gameStartTime = millis();
                    score = 0;
                    gameActive = true;
                    nextMoleTime = millis() + moleAppearInterval;
                }
                break;
            case 3:  // Stop game command
                Serial.println("Stopping Game via Master...");
                resetAllLEDs();
                gameActive = false;
                break;
            default:
                Serial.println("Unknown Message ID");
        }
    } else {
        Serial.println("Incorrect data size received");
    }
}

// Function to handle I2C data request
void dataReq() {
    Wire.write((byte *)&repMsg, sizeof(repMsg));
    repMsg = {0, 0};  // Reset message after sending
}
