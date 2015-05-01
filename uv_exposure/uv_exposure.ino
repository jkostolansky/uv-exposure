/*
  UV Exposure
  Countdown timer for UV exposure unit.

  by Juraj Kostolansky
*/

// pinout
#define SEG_A          1
#define SEG_B          5
#define SEG_C         11
#define SEG_D         A4
#define SEG_E         A3
#define SEG_F          0
#define SEG_G         10
#define SEG_CCD        9
#define SEG_CA1       A5
#define SEG_CA2       A2
#define SEG_CA3       A1
#define SEG_CA4       13
#define SEG_CAD       12
#define BTN_UP         8
#define BTN_DWN        7
#define BTN_OK         6
#define SIG            4
#define BUZZ           2

// settings
#define BTN_DELAY         500
#define SEG_BLINK_DELAY  1000
#define MODE_SETTINGS       0
#define MODE_COUNTER        1
#define MODE_FINISH         2

// global variables
const int seg_c_seq[7] = { SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G };
const int seg_a_seq[7] = { SEG_CA1, SEG_CA2, SEG_CA3, SEG_CA4 };
const byte numbers[10] = { 0b1000000, 0b1111001, 0b0100100, 0b0110000, 0b0011001, 0b0010010, 0b0000010, 0b1111000, 0b0000000, 0b0010000 };

unsigned long timeSet       = 300000;
unsigned long timeStart     = 0;
int           mode          = MODE_SETTINGS;
unsigned long btnDwnEnd     = 0;
unsigned long btnUpEnd      = 0;
unsigned long btnOkEnd      = 0;
unsigned long segEnd        = 0;
boolean       segDisplay    = true;
int           curDigit      = 0;

void setup() {
  // initialize display
  pinMode(SEG_A,   OUTPUT);
  pinMode(SEG_B,   OUTPUT);
  pinMode(SEG_C,   OUTPUT);
  pinMode(SEG_D,   OUTPUT);
  pinMode(SEG_E,   OUTPUT);
  pinMode(SEG_F,   OUTPUT);
  pinMode(SEG_G,   OUTPUT);
  pinMode(SEG_CCD, OUTPUT);
  pinMode(SEG_CA1, OUTPUT);
  pinMode(SEG_CA2, OUTPUT);
  pinMode(SEG_CA3, OUTPUT);
  pinMode(SEG_CA4, OUTPUT);
  pinMode(SEG_CAD, OUTPUT);

  // initialize buttons
  pinMode(BTN_UP, INPUT);
  pinMode(BTN_DWN, INPUT);
  pinMode(BTN_OK, INPUT_PULLUP);

  // initialize buzzer
  pinMode(BUZZ, OUTPUT);

  // initialize signal output
  pinMode(SIG, OUTPUT);
  digitalWrite(SIG, LOW);
}

void loop() {

  /**********************
   *       DISPLAY      *
   **********************/

  switch(mode) {
    case MODE_SETTINGS:
      displayTime(timeSet, false, false);
      break;
    case MODE_COUNTER:
      displayTime(timeSet - (millis() - timeStart), true, false);
      break;
    case MODE_FINISH:
      displayTime(0, true, true);
      break;
  }


  /**********************
   *       COUNTER      *
   **********************/

  if(mode == MODE_COUNTER) {
    if(timeStart == 0)
     timeStart = millis();

    if((millis() - timeStart) < timeSet) {
      digitalWrite(SIG, HIGH);
    } else {
      segEnd = 0;
      mode = MODE_FINISH;
    }
  }

  if(mode == MODE_FINISH) {
    digitalWrite(SIG, LOW);
    timeStart = 0;
  }

  /**********************
   *       CONTROL      *
   **********************/

  if(mode == MODE_SETTINGS) {
    // DOWN button
    if(digitalRead(BTN_DWN) == HIGH) {
      if(millis() > btnDwnEnd) {
        btnDwnEnd = millis() + BTN_DELAY;
        timeSet = adjustDigit(timeSet, curDigit, -1);
      }
    } else {
      btnDwnEnd = 0;
    }

    // UP button
    if(digitalRead(BTN_UP) == HIGH) {
      if(millis() > btnUpEnd) {
        btnUpEnd = millis() + BTN_DELAY;
        timeSet = adjustDigit(timeSet, curDigit, 1);
      }
    } else {
      btnUpEnd = 0;
    }

    // OK button
    if(digitalRead(BTN_OK) == HIGH) {
      if(millis() > btnOkEnd) {
        btnOkEnd = millis() + BTN_DELAY;
        curDigit++;
      }
    } else {
      btnOkEnd = 0;
    }

    if(curDigit >= 4) {
      curDigit = 0;
      mode = MODE_COUNTER;
    }
  }

  if(mode == MODE_COUNTER) {
    // OK button
    if(digitalRead(BTN_OK) == HIGH) {
      if(millis() > btnOkEnd) {
        btnOkEnd = millis() + BTN_DELAY;
        segEnd = 0;
        mode = MODE_FINISH;
      }
    } else {
      btnOkEnd = 0;
    }
  }

  if(mode == MODE_FINISH) {
    // OK button
    if(digitalRead(BTN_OK) == HIGH) {
      if(millis() > btnOkEnd) {
        btnOkEnd = millis() + BTN_DELAY;
        mode = MODE_SETTINGS;
      }
    } else {
      btnOkEnd = 0;
    }
  }
}

unsigned long adjustDigit(unsigned long time, int digit, int adjust) {
  unsigned timeString  = getTimeString(time);

  unsigned before    = (timeString / round(pow(10, 3-digit+1)));
  unsigned oldDigit  = (timeString / round(pow(10, 3-digit))) % 10;
  unsigned after     = (timeString % round(pow(10, 3-digit)));

  unsigned newDigit;
  int digMax = (digit == 2 ? 5 : 9);
  if(adjust == 1) {
    newDigit = (oldDigit+1) % (digMax+1);
  } else {
    if(oldDigit == 0)
      newDigit = digMax;
    else
      newDigit = oldDigit-1;
  }

  unsigned adjusted = (before * round(pow(10, 3-digit+1))) + (newDigit * round(pow(10, 3-digit))) + after;
  return getTimeFromString(adjusted);
}

void displayTime(unsigned long time, boolean blinkColon, boolean blinkDisplay) {
  if(blinkDisplay) {
   if(segEnd == 0)
     segEnd = millis();
   if(millis() > segEnd) {
     segEnd += SEG_BLINK_DELAY;
     segDisplay = !segDisplay;
   }
  }

  unsigned timeString  = getTimeString(time);
  for(int digit = 0; digit < 4; digit++) {
    int number = (timeString / round(pow(10, 3-digit)) ) % 10;
    // ON
    if(!blinkDisplay || segDisplay) {
      digitalWrite(seg_a_seq[digit], HIGH);
      if(!blinkColon || (timeString % 2 == 0)) {
        digitalWrite(SEG_CAD, HIGH);
        digitalWrite(SEG_CCD, LOW);
      }
      for (int i = 0; i < 7; i++)
        digitalWrite(seg_c_seq[i], bitRead(numbers[number], i));
    }
    // wait
    delay(2);
    // OFF
    digitalWrite(seg_a_seq[digit], LOW);
    digitalWrite(SEG_CAD, LOW);
    digitalWrite(SEG_CCD, HIGH);
    for (int i = 0; i < 7; i++)
      digitalWrite(seg_c_seq[i], HIGH);
  }
}

unsigned getTimeString(unsigned long time) {
  unsigned timeSeconds = (time/1000) % 60;
  unsigned timeMinutes = ((time/1000) - timeSeconds) / 60;
  return timeMinutes * 100 + timeSeconds;
}

unsigned long getTimeFromString(unsigned string) {
  unsigned long timeSeconds = (unsigned long)string % 100;
  unsigned long timeMinutes = ((unsigned long)string - timeSeconds) / 100;
  return (timeMinutes*60 + timeSeconds) * 1000;
}
