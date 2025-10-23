#include <Arduino.h>

#define NUM_LANES 2
#define RESET_BTN 47
#define BAUD_RATE 115200

// LED pins
const byte LED_Prestage[NUM_LANES] = {8, 11};
const byte LED_Stage[NUM_LANES] = {7, 22};
const byte LED_Green[NUM_LANES] = {3, 2};
const byte LED_Red[NUM_LANES] = {9, 10};
const byte LED_Y1 = 6, LED_Y2 = 5, LED_Y3 = 4;

// Sensor pins
const byte SENSOR_Start[NUM_LANES] = {23, 24};
const byte SENSOR_Finish[NUM_LANES] = {25, 26};

// State variables
bool prestaged[NUM_LANES], staged[NUM_LANES], finished[NUM_LANES], falseStart[NUM_LANES];
unsigned long startTime[NUM_LANES], finishTime[NUM_LANES], reactionTime[NUM_LANES];
bool raceStarted = false, resultShown = false;
bool countdownRunning = false;
// --- RESET ---
void resetRace() {
  raceStarted = false;
  resultShown = false;
  for (int i = 0; i < NUM_LANES; i++) {
    prestaged[i] = staged[i] = finished[i] = falseStart[i] = false;
    startTime[i] = finishTime[i] = reactionTime[i] = 0;
    digitalWrite(LED_Prestage[i], LOW);
    digitalWrite(LED_Stage[i], LOW);
    digitalWrite(LED_Green[i], LOW);
    digitalWrite(LED_Red[i], LOW);
  }
  digitalWrite(LED_Y1, LOW);
  digitalWrite(LED_Y2, LOW);
  digitalWrite(LED_Y3, LOW);
  Serial.println("RESET");
}

// --- COUNT STAGED LANES ---
int stagedCount() {
  int count = 0;
  for (int i = 0; i < NUM_LANES; i++) if (staged[i]) count++;
  return count;
}

// --- START COUNTDOWN ---

void startCountdown() {
  countdownRunning = true; // countdown is active
  Serial.println("COUNTDOWN START");
  delay(500);

  digitalWrite(LED_Y1, HIGH); Serial.println("YELLOW 1"); delay(700);
  digitalWrite(LED_Y2, HIGH); Serial.println("YELLOW 2"); delay(700);
  digitalWrite(LED_Y3, HIGH); Serial.println("YELLOW 3"); delay(700);
  delay(random(500,1500));

  // GREEN light
  for (int i = 0; i < NUM_LANES; i++) if(staged[i]) digitalWrite(LED_Green[i], HIGH);
  Serial.println("GO");

  raceStarted = true;
  countdownRunning = false; // countdown finished
  unsigned long now = millis();
  for (int i = 0; i < NUM_LANES; i++) if(staged[i]) startTime[i] = now;

  digitalWrite(LED_Y1, LOW);
  digitalWrite(LED_Y2, LOW);
  digitalWrite(LED_Y3, LOW);
}
// --- SETUP ---
void setup() {
  Serial.begin(BAUD_RATE);
  while(!Serial);

  for(int i=0;i<NUM_LANES;i++){
    pinMode(LED_Prestage[i], OUTPUT);
    pinMode(LED_Stage[i], OUTPUT);
    pinMode(LED_Green[i], OUTPUT);
    pinMode(LED_Red[i], OUTPUT);
    pinMode(SENSOR_Start[i], INPUT_PULLUP);
    pinMode(SENSOR_Finish[i], INPUT_PULLUP);
  }
  pinMode(LED_Y1, OUTPUT); pinMode(LED_Y2, OUTPUT); pinMode(LED_Y3, OUTPUT);
  pinMode(RESET_BTN, INPUT_PULLUP);

  Serial.println("SYSTEM READY");
  resetRace();
}

// --- LOOP ---
void loop() {
  // --- RESET ---
  if(digitalRead(RESET_BTN) == LOW){
    resetRace(); 
    delay(800); 
  }

  // --- PRE-STAGE & STAGE ---
  for(int i=0; i<NUM_LANES; i++){
    int sensor = digitalRead(SENSOR_Start[i]);

    // Pre-stage
    if(!prestaged[i] && sensor==LOW){ 
      prestaged[i] = true; 
      digitalWrite(LED_Prestage[i], HIGH); 
      Serial.print("L"); Serial.print(i+1); Serial.println("_PRESTAGE"); 
      delay(150); 
    }

    // Stage
    if(prestaged[i] && !staged[i] && sensor==LOW){ 
      staged[i] = true; 
      digitalWrite(LED_Stage[i], HIGH); 
      Serial.print("L"); Serial.print(i+1); Serial.println("_STAGE"); 
      delay(150); 
    }

    // --- FALSE START DURING COUNTDOWN ONLY ---
    if(countdownRunning && staged[i] && sensor==LOW && !falseStart[i]){
      falseStart[i] = true;
      digitalWrite(LED_Red[i], HIGH);
      Serial.print("L"); Serial.print(i+1); Serial.println("_FALSESTART");
    }
  }

  // --- START COUNTDOWN ONLY IF ALL LANES PRE-STAGED ---
  bool allPrestaged = true;
  for(int i=0; i<NUM_LANES; i++) if(!prestaged[i]) allPrestaged = false;

  if(allPrestaged && !raceStarted && !countdownRunning){
    startCountdown();
  }

  // --- AFTER GO: track running time ---
  if(raceStarted){
    for(int i=0;i<NUM_LANES;i++){
      if(staged[i] && !finished[i]){
        unsigned long elapsed = millis() - startTime[i];
        int kph = random(120,180);
        Serial.print("L"); Serial.print(i+1); Serial.print("|ET:"); 
        Serial.print(elapsed/1000); 
        Serial.print("s|KPH:"); 
        Serial.println(kph);
      }
    }
  }

  // --- FINISH ---
  for(int i=0;i<NUM_LANES;i++){
    int sensor = digitalRead(SENSOR_Finish[i]);
    if(raceStarted && !finished[i] && sensor==LOW){
      finished[i] = true;
      finishTime[i] = millis();
      Serial.print("L"); Serial.print(i+1); Serial.println("_FINISH");
    }
  }

  // --- WINNER / LOSER ---
  if(raceStarted && !resultShown && (finished[0] || finished[1])){
    resultShown = true;
    int winner = 0;

    if(finished[0] && finished[1]){
      unsigned long et1 = finishTime[0]-startTime[0];
      unsigned long et2 = finishTime[1]-startTime[1];
      if(et1 < et2) winner = 1;
      else if(et2 < et1) winner = 2;
      else winner = 3; // Tie
    } 
    else if(finished[0]) winner = 1;
    else if(finished[1]) winner = 2;

    if(winner == 1){ digitalWrite(LED_Green[0], HIGH); Serial.println("WINNER:1"); }
    else if(winner == 2){ digitalWrite(LED_Green[1], HIGH); Serial.println("WINNER:2"); }
    else Serial.println("RESULT:TIE");

    // Display final stats (ET + KPH only)
    for(int i=0; i<NUM_LANES; i++){
      unsigned long et = finished[i] ? (finishTime[i]-startTime[i]) : 0;
      int kph = random(120,180);
      Serial.print("L"); Serial.print(i+1); Serial.print("|ET:"); Serial.print(et/1000);
      Serial.print("s|KPH:"); Serial.println(kph);
    }
  }

  delay(50);
}

