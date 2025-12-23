#include <Wire.h>
#include <ADCTouch.h>

#define MYADDRESS 0x06  // Slave address

struct message {
  uint8_t mid;
  uint8_t msg;
};

struct message repMsg = {0, 0}; // Default response message

// Reference values for capacitive touch sensors
int ref1, ref2, ref3;  
int threshold = 60;      
unsigned long debounceTime = 300;
unsigned long lastTouchTime1 = 0, lastTouchTime2 = 0, lastTouchTime3 = 0;
unsigned long lastUpdateTime = 0;
unsigned long gameStartTime = 0;
unsigned long gameDuration = 30000;
int score = 0;
int activeMole = -1;
unsigned long moleTimeLimit = 1000;
unsigned long moleStartTime = 0;
unsigned long moleAppearInterval = 1000;
unsigned long nextMoleTime = 0;
unsigned long redTimeout = 500;
unsigned long greenTimeout = 500;
bool moleHit = false;
bool gameActive = false;  


int passScore = 10;  // pass condition

// LED pin assignments
int redLED1 = 6, yellowLED1 = 5, greenLED1 = 4;
int redLED2 = 9, yellowLED2 = 8, greenLED2 = 7;
int redLED3 = 12, yellowLED3 = 11, greenLED3 = 10;

void setup() {
    Serial.begin(9600);
    delay(1000);

    Wire.begin(MYADDRESS);
    Wire.onReceive(dataRcv);
    Wire.onRequest(dataReq);

    ref1 = ADCTouch.read(A3, 100);
    ref2 = ADCTouch.read(A2, 100);
    ref3 = ADCTouch.read(A1, 100);

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
    if (!gameActive) return;

    if (millis() - gameStartTime > gameDuration) {
        Serial.println("Game Over!");
        Serial.print("Final Score: ");
        Serial.println(score);

        //Check if player passed or failed
        if (score >= passScore) {
            Serial.println("Game Passed!");
            repMsg = {1, 1};  // Message ID 4: Game Passed (1)
        } else {
            Serial.println("Game Failed!");
            repMsg = {1, 0};  // Message ID 4: Game Failed (0)
        }

        resetAllLEDs();
        gameActive = false;
        return;
    }

    if (millis() - lastUpdateTime > 50) {  
        lastUpdateTime = millis();
        int value1 = ADCTouch.read(A3, 100) - ref1;
        int value2 = ADCTouch.read(A2, 100) - ref2;
        int value3 = ADCTouch.read(A1, 100) - ref3;

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

    if (activeMole != -1 && millis() - moleStartTime > moleTimeLimit) {
        if (!moleHit) {
            Serial.println("Missed the mole!");
            triggerRedLightForActiveMole();
        }
        activeMole = -1;
        moleHit = false;
        resetAllLEDs();
        nextMoleTime = millis() + moleAppearInterval;
    }

    if (millis() >= nextMoleTime && activeMole == -1) {
        resetAllLEDs();
        activeMole = random(1, 4);
        moleStartTime = millis();
        Serial.print("Active Mole: ");
        Serial.println(activeMole);

        if (activeMole == 1) digitalWrite(yellowLED1, HIGH);
        if (activeMole == 2) digitalWrite(yellowLED2, HIGH);
        if (activeMole == 3) digitalWrite(yellowLED3, HIGH);
    }
}

void resetAllLEDs() {
    digitalWrite(redLED1, LOW); digitalWrite(yellowLED1, LOW); digitalWrite(greenLED1, LOW);
    digitalWrite(redLED2, LOW); digitalWrite(yellowLED2, LOW); digitalWrite(greenLED2, LOW);
    digitalWrite(redLED3, LOW); digitalWrite(yellowLED3, LOW); digitalWrite(greenLED3, LOW);
}

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
        triggerRedLightForActiveMole();
    }
}

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

// I2C RECEIVE HANDLER
void dataRcv(int numBytes) {
    if (Wire.available() == sizeof(message)) {
        message rcvMsg;
        Wire.readBytes((uint8_t*)&rcvMsg, sizeof(message));

        switch (rcvMsg.mid) {
            case 2:
                if (!gameActive) {
                    Serial.println("Starting Game via Master...");
                    gameStartTime = millis();
                    score = 0;
                    gameActive = true;
                    nextMoleTime = millis() + moleAppearInterval;
                }
                break;
            case 3:
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

// I2C REQUEST HANDLER
void dataReq() {
    Wire.write((byte *)&repMsg, sizeof(repMsg));
    repMsg = {0, 0};  
}
