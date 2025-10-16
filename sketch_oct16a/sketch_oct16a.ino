#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- LANE 1 LED pins ---
#define lane1_1 12
#define lane1_2 11
#define lane1_3 10
#define lane1_4 9
#define lane1_5 8
#define lane1_6 7
#define lane1_7 6

// --- LANE 2 LED pins ---
#define lane2_1 24
#define lane2_2 25
#define lane2_3 26
#define lane2_4 27
#define lane2_5 28
#define lane2_6 29
#define lane2_7 30

// --- Sensors ---
#define preStage1 A8
#define stage1 A9
#define finish1 A10
#define preStage2 A11
#define stage2 A12
#define finish2 A13

// --- Tree LEDs ---
#define RLed1 4
#define YLed1 3
#define GLed1 2
#define RLed2 22
#define YLed2 23
#define GLed2 5

LiquidCrystal_I2C lcd(0x27, 20, 4);

// --- Timing & Data Variables ---
unsigned long startMillis[2], endMillis[2];
float ReactSecs[2], ET[2], KPH[2];
bool falseStart[2] = {false, false};
bool finished[2] = {false, false};
bool started = false;

// ✅ MODIFY THESE DISTANCES TO MATCH YOUR REAL TRACK
float distance1 = 201.0;  // meters for Lane 1
float distance2 = 201.0;  // meters for Lane 2

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);

  pinMode(preStage1, INPUT_PULLUP);
  pinMode(stage1, INPUT_PULLUP);
  pinMode(finish1, INPUT_PULLUP);
  pinMode(preStage2, INPUT_PULLUP);
  pinMode(stage2, INPUT_PULLUP);
  pinMode(finish2, INPUT_PULLUP);

  pinMode(RLed1, OUTPUT);
  pinMode(YLed1, OUTPUT);
  pinMode(GLed1, OUTPUT);
  pinMode(RLed2, OUTPUT);
  pinMode(YLed2, OUTPUT);
  pinMode(GLed2, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print(" Porta Tree Ready ");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Wait until both lanes are staged before starting sequence
  if (!digitalRead(stage1) && !digitalRead(stage2) && !started) {
    started = true;
    startRace();
  }

  // Check finish sensors
  checkFinish(0, finish1, "1", distance1);
  checkFinish(1, finish2, "2", distance2);

  // When both finish or false start occurs
  if ((finished[0] || falseStart[0]) && (finished[1] || falseStart[1])) {
    showWinner();
    resetRace();
  }
}

// --- START RACE SEQUENCE ---
void startRace() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Get Ready...");
  delay(1000);

  // Red lights
  digitalWrite(RLed1, HIGH);
  digitalWrite(RLed2, HIGH);
  delay(1000);

  // Yellow lights
  digitalWrite(RLed1, LOW);
  digitalWrite(RLed2, LOW);
  digitalWrite(YLed1, HIGH);
  digitalWrite(YLed2, HIGH);
  delay(1000);

  // Green lights (start)
  digitalWrite(YLed1, LOW);
  digitalWrite(YLed2, LOW);
  digitalWrite(GLed1, HIGH);
  digitalWrite(GLed2, HIGH);

  // Start timers
  startMillis[0] = millis();
  startMillis[1] = millis();

  lcd.clear();
  lcd.print("GO!");
}

// --- CHECK FINISH ---
void checkFinish(int laneIndex, int finishPin, String laneName, float distance) {
  // Detect false start (left early before green)
  if (started && digitalRead(stage1 + laneIndex) == HIGH && millis() - startMillis[laneIndex] < 1000 && !finished[laneIndex]) {
    falseStart[laneIndex] = true;
    finished[laneIndex] = true;
    lcd.setCursor(0, laneIndex * 2);
    lcd.print("Lane ");
    lcd.print(laneName);
    lcd.print(": FALSE START ");
    Serial.println("L" + laneName + "_FALSESTART");
    return;
  }

  // Normal finish
  if (!digitalRead(finishPin) && !finished[laneIndex] && !falseStart[laneIndex]) {
    endMillis[laneIndex] = millis();
    ET[laneIndex] = (endMillis[laneIndex] - startMillis[laneIndex]) / 1000.0;
    KPH[laneIndex] = (3.6 * distance) / ET[laneIndex];
    ReactSecs[laneIndex] = (random(40, 100) / 100.0); // Simulated RT

    finished[laneIndex] = true;
    digitalWrite(GLed1, LOW);
    digitalWrite(GLed2, LOW);

    lcd.setCursor(0, laneIndex * 2);
    lcd.print("Lane ");
    lcd.print(laneName);
    lcd.print(": ET=");
    lcd.print(ET[laneIndex], 2);
    lcd.print("s");

    lcd.setCursor(0, laneIndex * 2 + 1);
    lcd.print("KPH=");
    lcd.print(KPH[laneIndex], 1);

    // Send to PC GUI
    Serial.println("L" + laneName + "_RT:" + String(ReactSecs[laneIndex], 2));
    Serial.println("L" + laneName + "_ET:" + String(ET[laneIndex], 2));
    Serial.println("L" + laneName + "_KPH:" + String(KPH[laneIndex], 1));
  }
}

// --- SHOW WINNER ---
void showWinner() {
  lcd.clear();

  // Both false start
  if (falseStart[0] && falseStart[1]) {
    lcd.print("Both FALSE START!");
    Serial.println("RESULT:BOTH_FALSE");
    delay(4000);
    return;
  }

  // One false start
  if (falseStart[0]) {
    lcd.print("Lane 2 WINS!");
    Serial.println("WINNER:2");
  } else if (falseStart[1]) {
    lcd.print("Lane 1 WINS!");
    Serial.println("WINNER:1");
  } else {
    // Compare ETs
    if (ET[0] < ET[1]) {
      lcd.print("Lane 1 WINS!");
      Serial.println("WINNER:1");
    } else if (ET[1] < ET[0]) {
      lcd.print("Lane 2 WINS!");
      Serial.println("WINNER:2");
    } else {
      lcd.print("TIE RACE!");
      Serial.println("RESULT:TIE");
    }
  }
  delay(4000);
}

// --- RESET RACE ---
void resetRace() {
  for (int i = 0; i < 2; i++) {
    startMillis[i] = 0;
    endMillis[i] = 0;
    ET[i] = 0;
    KPH[i] = 0;
    falseStart[i] = false;
    finished[i] = false;
  }

  started = false;
  lcd.clear();
  lcd.print(" Ready for Next ");
  lcd.setCursor(0, 1);
  lcd.print("      Race      ");
  delay(2000);
  lcd.clear();
}
