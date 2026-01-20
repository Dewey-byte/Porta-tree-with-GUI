//------global variables----//
#include <LiquidCrystal.h>
#define FEET_PER_MILE 4200
#define TRACK_LEN 1210
#define NUM_LANES 2
#define RESET_BTN 47
const byte RS = 52;
const byte EN = 49;
const byte D4 = 53;
const byte D5 = 50;
const byte D6 = 51;
const byte D7 = 48;
const byte BAUD_RATE = 115200;
const byte LED_Prestage[2] = {8,11};
const byte LED_Stage[2] ={ 7,22};
const byte LED_Y1 = 6;
const byte LED_Y2 = 5;
const byte LED_Y3 = 4;
const byte LED_Start = 3;
const byte LED_RED_Light = 2;
const byte LED_FIN = 9;
//LANE 2
const byte LED_Y12 = 23;
const byte LED_Y22 = 24;
const byte LED_Y32 = 25;
const byte LED_Start2 = 26;
const byte LED_RED_Light2 = 28;
const byte LED_FIN2 = 27;
const byte LED_RST = 29;
const byte LED_offsense = 31;
const byte Pre_Stage_Sensor[NUM_LANES] = {A1, A4};
const byte Stage_Sensor[NUM_LANES] = {A1, A4};
const byte Finish_Sensor[NUM_LANES] = {A8, A9};
const byte Start_Button = 46;
//how long is the countdown
const int  CountDownFin = 2000;
//define missing slash char for Flip ani..
uint8_t slash[8] = {
  0b10000,
  0b10000,
  0b01000,
  0b00100,
  0b00100,
  0b00010,
  0b00001,
  0b00001,
};
byte flip = 0;
unsigned long AniMilli = 0;
int IntervalAni = 50;
int state = 0;
bool StartFlag = false;
unsigned long countdownStart;
unsigned long raceStart;
bool Launched[NUM_LANES] = {false, false};
unsigned long reactionTime[NUM_LANES];
unsigned long vehicleStart[NUM_LANES];
bool FinishFlag[NUM_LANES];
unsigned long FinishET[NUM_LANES];
unsigned long lastSerialUpdate = 0;
//float ReactSec;
unsigned long laneStartTime[NUM_LANES];
unsigned long laneElapsedTime[NUM_LANES];
bool laneRunning[NUM_LANES] = {false, false};
int Pre_Stage_Sensor_Value[NUM_LANES];
int Stage_Sensor_Value[NUM_LANES];
int Finish_Sensor_Value[NUM_LANES];
bool Staged[NUM_LANES] = {false,false};
bool lastStaged[NUM_LANES] = {true,true};
bool needClear = true;
bool resultsPrinted = false;
bool bothFinishedPrinted = false;
int winnerLane = -1;
unsigned long finishOrderTime[NUM_LANES] = {0, 0};


LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

//------------------------------------------------------------------------//

void setup() {
  pinMode(LED_RST, OUTPUT);
  pinMode(LED_offsense, OUTPUT);
  
  pinMode(LED_Y1, OUTPUT);
  pinMode(LED_Y2, OUTPUT);
  pinMode(LED_Y3, OUTPUT);
  pinMode(LED_Start, OUTPUT);
  pinMode(LED_RED_Light, OUTPUT);
  pinMode(LED_FIN, OUTPUT);
  //LANE 2
  pinMode(LED_Y12, OUTPUT);
  pinMode(LED_Y22, OUTPUT);
  pinMode(LED_Y32, OUTPUT);
  pinMode(LED_Start2, OUTPUT);
  pinMode(LED_RED_Light2, OUTPUT);
  pinMode(LED_FIN2, OUTPUT);
  for (int i = 0; i < NUM_LANES; i++)
  {
    pinMode(LED_Prestage[i], OUTPUT);
    pinMode(LED_Stage[i], OUTPUT);
    pinMode(Pre_Stage_Sensor[i], INPUT);
    pinMode(Stage_Sensor[i], INPUT);
    pinMode(Finish_Sensor[i], INPUT);
  }
  pinMode(Start_Button, INPUT_PULLUP);
  pinMode(RESET_BTN, INPUT_PULLUP);
  //Serial.begin(BAUD_RATE);
  Serial.begin(115200);
 
  Serial.println("System Ready");
  Serial.println("Pre-Stage");
  Serial.println("Pre-Stage");


  
}

void loop()
{
  // -------- 1. READ ALL SENSORS --------
  for (int i = 0; i < NUM_LANES; i++) {
    Pre_Stage_Sensor_Value[i] = analogRead(Pre_Stage_Sensor[i]);
    Stage_Sensor_Value[i]     = analogRead(Stage_Sensor[i]);
    Finish_Sensor_Value[i]    = analogRead(Finish_Sensor[i]);
  }

  // -------- 2. UPDATE RUNNING TIMERS (STEP 3) --------
  for (int i = 0; i < NUM_LANES; i++) {
    if (laneRunning[i]) {
      laneElapsedTime[i] = millis() - laneStartTime[i];
    }
  }

  float laneSeconds[NUM_LANES];

for (int i = 0; i < NUM_LANES; i++) {
  laneSeconds[i] = laneElapsedTime[i] / 1000.0;
}

  //used later in the loop
  int j = 0;


//Serial.print("Finish: ");
   // Serial.println(Finish_Sensor_Value[0]);
   // Serial.println(Finish_Sensor_Value[1]);
   // delay (1000);
  /*
    Serial.print("PreStage: ");
    Serial.println(Pre_Stage_Sensor_Value[0]);
    Serial.print("Stage: ");
    Serial.println(Stage_Sensor_Value[0]);
    Serial.print("Finish: ");
    Serial.println(Finish_Sensor_Value[0]);
    Serial.print("State: ");
    Serial.println(state);
    Serial.println();
  */
//Serial.print("PreStage: ");
   //Serial.println(Pre_Stage_Sensor_Value[0]);
   //Serial.println(Pre_Stage_Sensor_Value[1]);
//Serial.print("Finish: ");
    //Serial.println(Finish_Sensor_Value[0]);
    //delay(600);
    //Serial.println(Finish_Sensor_Value[1]);

//-----enter the state machine..---------//
  switch (state) {
    case 0: //prestasge state..
      if (Pre_Stage_Sensor_Value[0] > 500 || Pre_Stage_Sensor_Value[1]  > 500 ) {
       if (Pre_Stage_Sensor_Value[0] > 500)
       { 
          
          
        digitalWrite(LED_Prestage[0], LOW);
        digitalWrite(LED_Stage[0], LOW);
        } else {
          
           
          digitalWrite(LED_Prestage[0], HIGH);
        }
       if (Pre_Stage_Sensor_Value[1] > 500)
       { 
          
           
        digitalWrite(LED_Prestage[1], LOW);
        digitalWrite(LED_Stage[1], LOW);
       } else {
          
          
          digitalWrite(LED_Prestage[1], HIGH);
       }
      }
      else {
        digitalWrite(LED_Prestage[0], HIGH);
        digitalWrite(LED_Prestage[1], HIGH);
       // lcd.clear();
       delay(2000);
        state = 1;
      }
      break;
//----------------------------------------------------------------------------------------------------//
    case 1: // Vehicle Staging State
      if (Stage_Sensor_Value[0] > 500 || Stage_Sensor_Value[1] > 500 ) {
       if (Stage_Sensor_Value[0] > 500){  
        digitalWrite(LED_Stage[0], LOW);
        Staged[0] = false;
        } else {
        digitalWrite(LED_Stage[0], HIGH);
        Staged[0] = true;
        }
       if (Stage_Sensor_Value[1] > 500){  
        digitalWrite(LED_Stage[1], LOW);
       Staged[1] = false;
       } else {
        digitalWrite(LED_Stage[1], HIGH);
       Staged[1] = true;
       }
        if (Staged[0]!= lastStaged[0]) {    
         if (Stage_Sensor_Value[0] > 500)  
          lcd.print("Lane1 : Please Stage"); 
        else
          Serial.println("Lane1 : Staged      "); 
          lastStaged[0] = Staged[0];
        }
        if (Staged[1]!= lastStaged[1]){
          lcd.setCursor(0,2);
         if (Stage_Sensor_Value[1] > 500)  
          lcd.print("Lane2 : Please Stage"); 
        else
          Serial.println("Lane2 : Staged      "); 
          lastStaged[1] = Staged[1];
          }
         // Staged = true;
          state--;
      }
      else {
        digitalWrite(LED_Stage[0], HIGH);
        digitalWrite(LED_Stage[1], HIGH);
        Serial.println("Lane1 : Staged      ");
        Serial.println("Lane2 : Staged      ");
        Staged[0] = true;
        Staged[1] = true;
        delay(3000);
        Serial.println("3");
        delay(1000);
        Serial.println("2");
        delay(1000);
        Serial.println("1");
        delay(1000);
        Serial.println("Vehicles Set");
        state++;
      }

      break;
//----------------------------------------------------------------------------------------------------//
    case 2: //check stage sensor and roll state back
      if (Stage_Sensor_Value[0] > 501 || Stage_Sensor_Value[1] > 501) {
        lcd.clear();
        state--;
        if (Stage_Sensor_Value[0] < 501) Staged[0] = false; lastStaged[0] = true;
        if (Stage_Sensor_Value[1] < 501) Staged[1] = false; lastStaged[1] = true;
      }
      else if (Stage_Sensor_Value[0] < 500 || Stage_Sensor_Value[1] < 500) 
      { //staged good .. check start button
          delay(500);
          digitalWrite(LED_Prestage[0], LOW);
          digitalWrite(LED_Stage[0], LOW);
          digitalWrite(LED_Prestage[1], LOW);
          digitalWrite(LED_Stage[1], LOW);
          delay(100);
          countdownStart = millis();
          state++;
      }
      break;
//----------------------------------------------------------------------------------------------------//
    case 3: //state 3 counts down leds and checks for early start..
      //check for early start first..
      if (!CheckEarlyStart()) {
        //not an early start, countdown
        if (millis() - countdownStart > 1150) //countdown done
        { //check sensor just before dropping the flag
          digitalWrite(LED_Stage[0], LOW);
          digitalWrite(LED_Stage[1], LOW);
          digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
          digitalWrite(LED_Y32, LOW);
          digitalWrite(LED_Start2, HIGH);
          StartFlag = true;
          raceStart = millis();//start counting race from here..
          for (int i = 0; i < NUM_LANES; i++) {
            laneStartTime[i] = raceStart;
            laneRunning[i] = true;
          }
          state++;
          
        }
        else if (millis() - countdownStart > 1000)
        {
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y3, HIGH);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y32, HIGH);
        }
        else if (millis() - countdownStart > 850)
        {
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y22, HIGH);
        }
        else if (millis() - countdownStart > 650)
        {
          digitalWrite(LED_Y1, HIGH);
           digitalWrite(LED_Y12, HIGH);
        }

      }
      break;
//----------------------------------------------------------------------------------------------------//
    case 4: //stage to get reaction time
      //need to see car move before next state..
      for (j = 0; j < NUM_LANES; j++) {
        if (Stage_Sensor_Value[j] > 500 && StartFlag && !Launched[j])
        {
           ///digitalWrite(LED_offsense, HIGH);
          vehicleStart[j] = millis();
          reactionTime[j] = millis();
          Launched[j] =  true; 
        }
      }
      if (Launched[0] && Launched[1])
       state++;
      break;
//----------------------------------------------------------------------------------------------------//
    case 5:
      //prints go and reaction time..
      {
        digitalWrite(LED_offsense, HIGH);
        float ReactSecs[2] = {float(reactionTime[0] - raceStart) / 1000, float(reactionTime[1] - raceStart) / 1000};
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("RT1:"); lcd.print(ReactSecs[0], 2);
        lcd.setCursor(0, 3);
        lcd.print("RT2:"); lcd.print(ReactSecs[1], 2);
        Serial.print("Reaction Time 1: "); Serial.println(ReactSecs[0], 2);
        Serial.print("Reaction Time 2: "); Serial.println(ReactSecs[1], 2);
        state = 6;        
      }
      break;
 //----------------------------------------------------------------------------------------------------//
case 6: // race running / finish detection
{
  // ---- CONVERT ELAPSED TIME TO SECONDS ----
  float laneSeconds[NUM_LANES];
  for (int i = 0; i < NUM_LANES; i++) {
    laneSeconds[i] = laneElapsedTime[i] / 1000.0;
  }
  // ---- SERIAL MONITOR (ONLY WHILE RACE IS RUNNING) ----
  if (!FinishFlag[0] || !FinishFlag[1]) {
    if (millis() - lastSerialUpdate >= 100) {
      lastSerialUpdate = millis();
      Serial.print("L1 ET: ");
      Serial.print(laneSeconds[0], 2);
      Serial.print(" s | ");
      Serial.print("L2 ET: ");
      Serial.print(laneSeconds[1], 2);
      Serial.println(" s");
    }
  }
  // ---- FINISH DETECTION (PER LANE) ----
  for (int j = 0; j < NUM_LANES; j++) {
    if (Finish_Sensor_Value[j] < 10 && !FinishFlag[j]) {
      FinishFlag[j] = true;
      laneRunning[j] = false;
      FinishET[j] = laneElapsedTime[j];
      finishOrderTime[j] = millis();
      // FIRST FINISHER = WINNER
      if (winnerLane == -1) {
        winnerLane = j;
      }
      if (needClear) {
        lcd.clear();
        needClear = false;
      }
      float secs = FinishET[j] / 1000.0;
      float ReactSecs = (reactionTime[j] - raceStart) / 1000.0;
      float fps = (TRACK_LEN / secs);
      float kph = (fps / FEET_PER_MILE) / 0.00028;
      lcd.setCursor(0, j * 2);
      lcd.print("ET:");
      lcd.print(secs, 2);
      lcd.setCursor(8, j * 2);
      lcd.print("RT:");
      lcd.print(ReactSecs, 2);
      lcd.setCursor(0, j * 2 + 1);
      lcd.print("KPH:");
      lcd.print(kph, 2);
    }
  }
  // ---- BOTH LANES FINISHED ----
  if (FinishFlag[0] && FinishFlag[1] && !bothFinishedPrinted) {
    bothFinishedPrinted = true;
    float secs0 = FinishET[0] / 1000.0;
    float secs1 = FinishET[1] / 1000.0;
    float kph0 = ((TRACK_LEN / secs0) / FEET_PER_MILE) / 0.00028;
    float kph1 = ((TRACK_LEN / secs1) / FEET_PER_MILE) / 0.00028;
    Serial.println("===== RACE COMPLETE =====");
    Serial.print("Lane 1 ET: ");
    Serial.print(secs0, 2);
    Serial.print(" s | KPH: ");
    Serial.println(kph0, 2);
    Serial.print("Lane 2 ET: ");
    Serial.print(secs1, 2);
    Serial.print(" s | KPH: ");
    Serial.println(kph1, 2);
  // ---- Determine winners first ----
bool lane1Active = false;
bool lane2Active = false;
if (abs(FinishET[0] - FinishET[1]) <= 20) {
    // Tie → both lanes active
    lane1Active = true;
    lane2Active = true;
} else if (winnerLane == 0) {
    lane1Active = true;
} else {
    lane2Active = true;
}
// ---- SERIAL WINNER / TIE ----
if (abs(FinishET[0] - FinishET[1]) <= 20) {
    Serial.println("RESULT: TIE RACE");
} else {
    Serial.print("WINNER: Lane ");
    Serial.println(winnerLane + 1);
}
// ---- TURN ALL LEDS OFF FIRST ----
digitalWrite(LED_Prestage[0], LOW);
digitalWrite(LED_Prestage[1], LOW);
digitalWrite(LED_Stage[0], LOW);
digitalWrite(LED_Stage[1], LOW);
digitalWrite(LED_Y1, LOW);
digitalWrite(LED_Y12, LOW);
digitalWrite(LED_Y2, LOW);
digitalWrite(LED_Y22, LOW);
digitalWrite(LED_Y3, LOW);
digitalWrite(LED_Y32, LOW);
digitalWrite(LED_Start, LOW);
digitalWrite(LED_Start2, LOW);
digitalWrite(LED_RED_Light, LOW);
digitalWrite(LED_RED_Light2, LOW);

// ---- BLINKING SEQUENCE FOR WINNER LANES ONLY ----
for (int repeat = 0; repeat < 3; repeat++) {

    if (lane1Active) {
        digitalWrite(LED_Prestage[0], HIGH);
    } else {
        digitalWrite(LED_Prestage[0], LOW);
    }

    if (lane2Active) {
        digitalWrite(LED_Prestage[1], HIGH);
    } else {
        digitalWrite(LED_Prestage[1], LOW);
    }

      // Stage LEDs per lane
    if (lane1Active) digitalWrite(LED_Stage[0], HIGH); else digitalWrite(LED_Stage[0], LOW);
    if (lane2Active) digitalWrite(LED_Stage[1], HIGH); else digitalWrite(LED_Stage[1], LOW);
    delay(100);
    digitalWrite(LED_Stage[0], LOW);
    digitalWrite(LED_Stage[1], LOW);
    // Yellow sequence per lane
    if (lane1Active) {
        digitalWrite(LED_Y1, HIGH);
        digitalWrite(LED_Y2, HIGH);
        digitalWrite(LED_Y3, HIGH);
    }
    if (lane2Active) {
        digitalWrite(LED_Y12, HIGH);
        digitalWrite(LED_Y22, HIGH);
        digitalWrite(LED_Y32, HIGH);
    }
    delay(100);
    digitalWrite(LED_Y1, LOW);
    digitalWrite(LED_Y2, LOW);
    digitalWrite(LED_Y3, LOW);
    digitalWrite(LED_Y12, LOW);
    digitalWrite(LED_Y22, LOW);
    digitalWrite(LED_Y32, LOW);
    // Start LEDs per lane
    if (lane1Active) digitalWrite(LED_Start, HIGH);
    if (lane2Active) digitalWrite(LED_Start2, HIGH);
    delay(100);
    digitalWrite(LED_Start, LOW);
    digitalWrite(LED_Start2, LOW);
    // Red lights per lane
    if (lane1Active) digitalWrite(LED_RED_Light, HIGH);
    if (lane2Active) digitalWrite(LED_RED_Light2, HIGH);
    delay(100);
    digitalWrite(LED_RED_Light, LOW);
    digitalWrite(LED_RED_Light2, LOW);
}
// ---- KEEP PRESTAGE LED ON FOR WINNER/TIE AFTER SEQUENCE ----
if (lane1Active) digitalWrite(LED_Prestage[0], HIGH);
if (lane2Active) digitalWrite(LED_Prestage[1], HIGH);
    // ---- RESET FOR NEXT RACE ----
    StartFlag = false;
    FinishFlag[0] = false;
    FinishFlag[1] = false;
    Staged[0] = false;
    Staged[1] = false;
    needClear = true;
    bothFinishedPrinted = false;
    winnerLane = -1;
    finishOrderTime[0] = 0;
    finishOrderTime[1] = 0;
    state = 0;
    delay(3000);
    digitalWrite(LED_RST, HIGH);
  }
  break;
}
//----------------------------------------------------------------------------------------------------//
    case 7: // LANE 1
      {  
        Serial.println("LANE 1 WINS");
        Serial.println("LANE 2 LOSE");
        digitalWrite(LED_Start2, LOW);
        digitalWrite(LED_Start, LOW); 
         digitalWrite(LED_FIN, HIGH);
         digitalWrite(LED_Prestage[0], HIGH);
         digitalWrite(LED_Prestage[1], LOW);
         digitalWrite(LED_Stage[1], LOW);
           delay(100);
         digitalWrite(LED_Stage[0], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Y1, HIGH);
           delay(100);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y3, HIGH);
           delay(100);
          digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_RED_Light, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          delay(100);
         digitalWrite(LED_Stage[0], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Y1, HIGH);
           delay(100);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y3, HIGH);
           delay(100);
          digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_RED_Light, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          delay(100);
         digitalWrite(LED_Stage[0], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Y1, HIGH);
           delay(100);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y3, HIGH);
           delay(100);
          digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_RED_Light, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          delay(100);
         digitalWrite(LED_Stage[0], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Y1, HIGH);
           delay(100);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y3, HIGH);
           delay(100);
          digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_RED_Light, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          delay(100);
         digitalWrite(LED_Stage[0], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Y1, HIGH);
           delay(100);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y3, HIGH);
           delay(100);
          digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_RED_Light, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          delay(100);
         digitalWrite(LED_Stage[0], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Y1, HIGH);
           delay(100);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y3, HIGH);
           delay(100);
          digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_RED_Light, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          delay(100);
         digitalWrite(LED_Stage[0], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Y1, HIGH);
           delay(100);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y3, HIGH);
           delay(100);
          digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_RED_Light, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          delay(100);
         digitalWrite(LED_Stage[0], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Y1, HIGH);
           delay(100);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y3, HIGH);
           delay(100);
          digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_RED_Light, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          delay(100);
         digitalWrite(LED_Stage[0], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Y1, HIGH);
           delay(100);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y3, HIGH);
           delay(100);
          digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_RED_Light, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          delay(100);
         digitalWrite(LED_Stage[0], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Y1, HIGH);
           delay(100);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y3, HIGH);
           delay(100);
          digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_RED_Light, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          digitalWrite(LED_FIN, LOW);  
    if (digitalRead(RESET_BTN) == HIGH)
        state = 10;
      }
    break;
//----------------------------------------------------------------------------------------------------//    
    case 8: // LANE 2
    {   
        Serial.println("LANE 2 WINS");
        Serial.println("LANE 1 LOSE");
        digitalWrite(LED_Start, LOW);
        digitalWrite(LED_Start2, LOW); 
         digitalWrite(LED_FIN2, HIGH);
         digitalWrite(LED_Prestage[1], HIGH);
         digitalWrite(LED_Prestage[0], LOW);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Stage[1], LOW);
           delay(100);
         digitalWrite(LED_Stage[1], HIGH);
           delay(100);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Y12, HIGH);
           delay(100);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y22, HIGH);
           delay(100);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y32, HIGH);
           delay(100);
          digitalWrite(LED_Y32, LOW);
          digitalWrite(LED_Start2, HIGH);
           delay(100);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light2, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light2, LOW);
           delay(100);
         digitalWrite(LED_Stage[1], HIGH);
           delay(100);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Y12, HIGH);
           delay(100);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y22, HIGH);
           delay(100);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y32, HIGH);
           delay(100);
          digitalWrite(LED_Y32, LOW);
          digitalWrite(LED_Start2, HIGH);
           delay(100);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light2, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light2, LOW);
           delay(100);
         digitalWrite(LED_Stage[1], HIGH);
           delay(100);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Y12, HIGH);
           delay(100);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y22, HIGH);
           delay(100);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y32, HIGH);
           delay(100);
          digitalWrite(LED_Y32, LOW);
          digitalWrite(LED_Start2, HIGH);
           delay(100);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light2, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light2, LOW);
           delay(100);
         digitalWrite(LED_Stage[1], HIGH);
           delay(100);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Y12, HIGH);
           delay(100);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y22, HIGH);
           delay(100);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y32, HIGH);
           delay(100);
          digitalWrite(LED_Y32, LOW);
          digitalWrite(LED_Start2, HIGH);
           delay(100);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light2, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light2, LOW);
           delay(100);
         digitalWrite(LED_Stage[1], HIGH);
           delay(100);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Y12, HIGH);
           delay(100);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y22, HIGH);
           delay(100);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y32, HIGH);
           delay(100);
          digitalWrite(LED_Y32, LOW);
          digitalWrite(LED_Start2, HIGH);
           delay(100);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light2, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light2, LOW);
           delay(100);
         digitalWrite(LED_Stage[1], HIGH);
           delay(100);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Y12, HIGH);
           delay(100);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y22, HIGH);
           delay(100);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y32, HIGH);
           delay(100);
          digitalWrite(LED_Y32, LOW);
          digitalWrite(LED_Start2, HIGH);
           delay(100);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light2, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light2, LOW);
           delay(100);
         digitalWrite(LED_Stage[1], HIGH);
           delay(100);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Y12, HIGH);
           delay(100);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y22, HIGH);
           delay(100);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y32, HIGH);
           delay(100);
          digitalWrite(LED_Y32, LOW);
          digitalWrite(LED_Start2, HIGH);
           delay(100);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light2, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light2, LOW);
           delay(100);
         digitalWrite(LED_Stage[1], HIGH);
           delay(100);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Y12, HIGH);
           delay(100);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y22, HIGH);
           delay(100);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y32, HIGH);
           delay(100);
          digitalWrite(LED_Y32, LOW);
          digitalWrite(LED_Start2, HIGH);
           delay(100);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light2, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light2, LOW);
           delay(100);
         digitalWrite(LED_Stage[1], HIGH);
           delay(100);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Y12, HIGH);
           delay(100);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y22, HIGH);
           delay(100);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y32, HIGH);
           delay(100);
          digitalWrite(LED_Y32, LOW);
          digitalWrite(LED_Start2, HIGH);
           delay(100);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light2, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light2, LOW);
          digitalWrite(LED_FIN2, LOW);
    if (digitalRead(RESET_BTN) == HIGH)
        state = 10;
      }
//----------------------------------------------------------------------------------------------------//      
      case 9:
      {
        digitalWrite(LED_Prestage[0], LOW);
      digitalWrite(LED_Prestage[1], LOW);
      digitalWrite(LED_Stage[1], LOW);
      digitalWrite(LED_Stage[0], LOW);
      digitalWrite(LED_Y12, LOW);
      digitalWrite(LED_Y1, LOW);
      digitalWrite(LED_Y2, LOW);
      digitalWrite(LED_Y22, LOW);
      digitalWrite(LED_Y32, LOW);
      digitalWrite(LED_Y3, LOW);
      digitalWrite(LED_Start, LOW);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light, LOW);
          digitalWrite(LED_RED_Light2, LOW);
        digitalWrite(LED_Prestage[0], HIGH);
         digitalWrite(LED_Prestage[1], HIGH);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Stage[0], LOW);
           delay(100);
         digitalWrite(LED_Stage[0], HIGH);
         digitalWrite(LED_Stage[1], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Y1, HIGH);
         digitalWrite(LED_Y12, HIGH);
           delay(100);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
          digitalWrite(LED_Y22, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y3, HIGH);
          digitalWrite(LED_Y32, HIGH);
           delay(100);
          digitalWrite(LED_Y32, LOW);
           digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
          digitalWrite(LED_Start2, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light, HIGH);
          digitalWrite(LED_RED_Light2, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          digitalWrite(LED_RED_Light2, LOW);

           if (digitalRead(RESET_BTN) == HIGH)
        state = 10;
      }
//----------------------------------------------------------------------------------------------------//      
    case 10:
      if (digitalRead(RESET_BTN) == HIGH)
      { digitalWrite(LED_Prestage[0], LOW);
      digitalWrite(LED_Prestage[1], LOW);
      digitalWrite(LED_Stage[1], LOW);
      digitalWrite(LED_Stage[0], LOW);
      digitalWrite(LED_Y12, LOW);
      digitalWrite(LED_Y1, LOW);
      digitalWrite(LED_Y2, LOW);
      digitalWrite(LED_Y22, LOW);
      digitalWrite(LED_Y32, LOW);
      digitalWrite(LED_Y3, LOW);
      digitalWrite(LED_Start, LOW);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light, LOW);
          digitalWrite(LED_RED_Light2, LOW);
        digitalWrite(LED_Prestage[0], HIGH);
         digitalWrite(LED_Prestage[1], HIGH);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Stage[0], LOW);
           delay(100);
         digitalWrite(LED_Stage[0], HIGH);
         digitalWrite(LED_Stage[1], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Y1, HIGH);
         digitalWrite(LED_Y12, HIGH);
           delay(100);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
          digitalWrite(LED_Y22, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y3, HIGH);
          digitalWrite(LED_Y32, HIGH);
           delay(100);
          digitalWrite(LED_Y32, LOW);
           digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
          digitalWrite(LED_Start2, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light, HIGH);
          digitalWrite(LED_RED_Light2, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          digitalWrite(LED_RED_Light2, LOW);
        StartFlag = false;
        FinishFlag[0] = false;
        Staged[0] = false;
        Staged[1] = false;
        FinishFlag[1] = false;
        needClear = true;
        state = 0;
        delay(3000);
        digitalWrite(LED_RST, HIGH);
      }
      break;
  }
}// END LOOP BRACKETS

bool CheckEarlyStart()
{
  bool early = false;
  if (Stage_Sensor_Value[0] > 501 & Stage_Sensor_Value[1] > 501)
  {reactionTime[0] = CountDownFin - (millis() - countdownStart);
   float ReactSecs = (float(reactionTime[0]) / 1000) * -1;
    
    Serial.println("!!DOUBLE Bad Start!!");
    
    lcd.print("RT:"); lcd.print(ReactSecs, 2);
    Serial.print("Reaction Time :"); Serial.println(ReactSecs, 2);
     early = true;
  
    digitalWrite(LED_Prestage[0], LOW);
    digitalWrite(LED_Stage[0], LOW);
    digitalWrite(LED_Prestage[1], LOW);
    digitalWrite(LED_Stage[1], LOW);
    digitalWrite(LED_RED_Light, HIGH);
    digitalWrite(LED_RED_Light2, HIGH);
     early = true;
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(500);
    digitalWrite(LED_RED_Light, LOW);
    digitalWrite(LED_RED_Light2, LOW);
      
     digitalWrite(LED_Prestage[0], HIGH);
         digitalWrite(LED_Prestage[1], HIGH);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Stage[0], LOW);
           delay(100);
         digitalWrite(LED_Stage[0], HIGH);
         digitalWrite(LED_Stage[1], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Y1, HIGH);
         digitalWrite(LED_Y12, HIGH);
           delay(100);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
          digitalWrite(LED_Y22, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y3, HIGH);
          digitalWrite(LED_Y32, HIGH);
           delay(100);
          digitalWrite(LED_Y32, LOW);
           digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
          digitalWrite(LED_Start2, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light, HIGH);
          digitalWrite(LED_RED_Light2, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          digitalWrite(LED_RED_Light2, LOW);
    
    state = 10;
    }
 else if (Stage_Sensor_Value[0] > 501) {
    reactionTime[0] = CountDownFin - (millis() - countdownStart);
    digitalWrite(LED_Prestage[0], LOW);
    digitalWrite(LED_Stage[0], LOW);
    digitalWrite(LED_Prestage[1], LOW);
    digitalWrite(LED_Stage[1], LOW);
    digitalWrite(LED_Start2, HIGH);
    digitalWrite(LED_RED_Light, HIGH);
    float ReactSecs = (float(reactionTime[0]) / 1000) * -1;
    
    Serial.println("LANE 1!!Bad Start!!");
    
    lcd.print("RT:"); lcd.print(ReactSecs, 2);
    Serial.print("Reaction Time :"); Serial.println(ReactSecs, 2);
     early = true;
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light, LOW);
    delay(200);
    digitalWrite(LED_RED_Light, HIGH);
    delay(500);
    digitalWrite(LED_RED_Light, LOW);
       digitalWrite(LED_Prestage[0], HIGH);
         digitalWrite(LED_Prestage[1], HIGH);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Stage[0], LOW);
           delay(100);
         digitalWrite(LED_Stage[0], HIGH);
         digitalWrite(LED_Stage[1], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Y1, HIGH);
         digitalWrite(LED_Y12, HIGH);
           delay(100);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
          digitalWrite(LED_Y22, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y3, HIGH);
          digitalWrite(LED_Y32, HIGH);
           delay(100);
          digitalWrite(LED_Y32, LOW);
           digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
          digitalWrite(LED_Start2, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light, HIGH);
          digitalWrite(LED_RED_Light2, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          digitalWrite(LED_RED_Light2, LOW);
   
    state = 10;
  }
  else if (Stage_Sensor_Value[1] > 501){
    digitalWrite(LED_Prestage[0], LOW);
    digitalWrite(LED_Stage[0], LOW);
    digitalWrite(LED_Prestage[1], LOW);
    digitalWrite(LED_Stage[1], LOW);
    digitalWrite(LED_Start, HIGH);
    digitalWrite(LED_RED_Light2, HIGH);
    reactionTime[0] = CountDownFin - (millis() - countdownStart);
    float ReactSecs = (float(reactionTime[0]) / 1000) * -1;
    
   Serial.println("LANE 2!!Bad Start!!");
    
    lcd.print("RT:"); lcd.print(ReactSecs, 2);
    Serial.print("Rection Time :"); Serial.println(ReactSecs, 2);
    early = true;
    delay(200);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(200);
    digitalWrite(LED_RED_Light2, LOW);
    delay(200);
    digitalWrite(LED_RED_Light2, HIGH);
    delay(500);
    digitalWrite(LED_RED_Light, LOW);
        digitalWrite(LED_RED_Light2, LOW);
        digitalWrite(LED_Stage[0], LOW);
        digitalWrite(LED_Y1, LOW);
         digitalWrite(LED_Y2, LOW);
         digitalWrite(LED_Y3, LOW);
         digitalWrite(LED_RED_Light, LOW);
          digitalWrite(LED_RED_Light2, LOW);
     digitalWrite(LED_Prestage[0], HIGH);
         digitalWrite(LED_Prestage[1], HIGH);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Stage[0], LOW);
           delay(100);
         digitalWrite(LED_Stage[0], HIGH);
         digitalWrite(LED_Stage[1], HIGH);
           delay(100);
         digitalWrite(LED_Stage[0], LOW);
         digitalWrite(LED_Stage[1], LOW);
         digitalWrite(LED_Y1, HIGH);
         digitalWrite(LED_Y12, HIGH);
           delay(100);
          digitalWrite(LED_Y12, LOW);
          digitalWrite(LED_Y1, LOW);
          digitalWrite(LED_Y2, HIGH);
          digitalWrite(LED_Y22, HIGH);
           delay(100);
          digitalWrite(LED_Y2, LOW);
          digitalWrite(LED_Y22, LOW);
          digitalWrite(LED_Y3, HIGH);
          digitalWrite(LED_Y32, HIGH);
           delay(100);
          digitalWrite(LED_Y32, LOW);
           digitalWrite(LED_Y3, LOW);
          digitalWrite(LED_Start, HIGH);
          digitalWrite(LED_Start2, HIGH);
           delay(100);
          digitalWrite(LED_Start, LOW);
          digitalWrite(LED_Start2, LOW);
          digitalWrite(LED_RED_Light, HIGH);
          digitalWrite(LED_RED_Light2, HIGH);
           delay(100);
          digitalWrite(LED_RED_Light, LOW);
          digitalWrite(LED_RED_Light2, LOW);
    
    state = 9;
  }
  return early;
}



void FlipAni() {
  if (millis() - AniMilli >= IntervalAni)
  {
    AniMilli = millis();
    switch (flip) {
      case 0: flip = 1; lcd.setCursor(0, 0); lcd.print("|"); lcd.setCursor(19, 0); lcd.print("|"); break;
      case 1: flip = 2; lcd.setCursor(0, 0); lcd.print("/"); lcd.setCursor(19, 0); lcd.print("/"); break;
      case 2: flip = 3; lcd.setCursor(0, 0); lcd.print("-"); lcd.setCursor(19, 0); lcd.print("-"); break;
      case 3: flip = 4; lcd.setCursor(0, 0); lcd.print("\x01"); lcd.setCursor(19, 0); lcd.print("\x01"); break;
      case 4: flip = 0; lcd.setCursor(0, 0); lcd.print("|"); lcd.setCursor(19, 0); lcd.print("|"); break;
    }
  }
}
