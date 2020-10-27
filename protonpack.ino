#include <Adafruit_NeoPixel.h>
#define LED_PIN   6
#define LED_COUNT 8
#define LIGHT_PIN 7
const int redButton = 3;
const int buzzerPin = 8;

Adafruit_NeoPixel strip(LED_COUNT + 3, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel backpackstrip(6, 9, NEO_GRB + NEO_KHZ800);


// 0 - red,   10922  -- yellow, 21845 - green
uint32_t ledOff = strip.ColorHSV(0, 0, 0);
uint32_t _1 = strip.gamma32(strip.ColorHSV(0, 255, 100));
uint32_t _2 = strip.gamma32(strip.ColorHSV(0, 255, 100));
uint32_t _3 = strip.gamma32(strip.ColorHSV(0, 255, 100));
uint32_t _4 = strip.gamma32(strip.ColorHSV(2000, 255, 100));
uint32_t _5 = strip.gamma32(strip.ColorHSV(6000, 255, 100));
uint32_t _6 = strip.gamma32(strip.ColorHSV(13000, 255, 100));
uint32_t _7 = strip.gamma32(strip.ColorHSV(18000, 255, 100));
uint32_t _8 = strip.gamma32(strip.ColorHSV(21845, 255, 100));
uint32_t white = strip.Color(255, 255, 255);

uint32_t fireBlue = strip.gamma32(backpackstrip.ColorHSV(43690, 255, 255));
uint32_t firePink = strip.gamma32(backpackstrip.ColorHSV(54613, 255, 255));

uint32_t power[] = {_8, _7, _6, _5, _4, _3, _2, _1};

uint32_t backpackLedOff = backpackstrip.ColorHSV(0, 0, 0);
uint32_t red = backpackstrip.gamma32(backpackstrip.ColorHSV(0, 255, 255));
uint32_t blue = backpackstrip.gamma32(backpackstrip.ColorHSV(43690, 255, 255));

const int fireSound = 100;
const int warningSound = 440;
const int readySound = 600;

const int upInterval = 200;
const int downInterval = 1000;

unsigned long lastTime = 0;
unsigned long lastCycleTime = 0;

int currentCharge = 7;
boolean currRedKey = false;
boolean prevRedKey = currRedKey;

int direction = 0;
bool redButtonPressed = false;


void setup() {
  Serial.begin(9600);

  pinMode(buzzerPin, OUTPUT);
  pinMode(redButton, INPUT);

  backpackstrip.begin();
  backpackstrip.show();
  backpackstrip.setBrightness(255);

  strip.begin();
  strip.show();
  strip.setBrightness(255);

  //LED in proton gun, THis is always on white.
  strip.setPixelColor(8, 255, 0, 0);


 // startupSequence();
  lastCycleTime = lastTime = millis();
}


void loop() {

  boolean isRedPressed = isRedButtonPressed();

  cycleBackpackLights(isRedPressed);

  if (isRedPressed && !prevRedKey) {
    //start pressing button
    direction = 1;
  }

  if (!isRedPressed && prevRedKey) {
    //just released button
    //start climbing again
    direction = -1;
  }

  if (isRedPressed) {
    fireLights();
    fireNoise();
  } else {
    turnOffGunLights();
  }

  prevRedKey = isRedPressed;

  if (direction == 1) {
    //go down
    //slowly
    long currentTime = millis();
    if (currentTime > lastTime + downInterval) {
      lastTime = currentTime;
      currentCharge--;
      if (currentCharge < 0) {
        currentCharge = 0;
        failureLights();
        //play full recharge sequence
        playEmptyWarning();
        startupSequence();
      }
    }
  } else {
    //go up
    //faster
    long currentTime = millis();
    if (currentTime > lastTime + upInterval) {
      lastTime = currentTime;
      currentCharge++;
      if (currentCharge >= 7) {
        currentCharge = 7;
      }
    }
  }

  updateStripLights();
  backpackstrip.show();

  Serial.println(currentCharge);
}

void failureLights() {
  //turn off blue lights.
  backpackstrip.setPixelColor(4, ledOff);
  backpackstrip.setPixelColor(5, ledOff);

  //turn on all red
  backpackstrip.setPixelColor(0, red);
  backpackstrip.setPixelColor(1, red);
  backpackstrip.setPixelColor(2, red);
  backpackstrip.setPixelColor(3, red);
  backpackstrip.show();
}

int next = 0;

const int cycleTime = 1000;
void cycleBackpackLights(boolean firing) {

  backpackstrip.setPixelColor(4, blue);
  backpackstrip.setPixelColor(5, blue);
  int currentCycleTime = cycleTime;
  if(firing) {
    currentCycleTime = cycleTime/2;
    backpackstrip.setPixelColor(4, ledOff);
  } else {
    backpackstrip.setPixelColor(4, blue);
  }
  //Cycle lights
  long currentTime = millis();
  if (currentTime > lastCycleTime + currentCycleTime) {
     
    lastCycleTime = currentTime;
    next++;
    if (next > 3) {
      next = 0;
    }
  }

  for (int i = 0; i < 4; i++) {
    if (i == next) {
      backpackstrip.setPixelColor(i, red);
    } else {
      backpackstrip.setPixelColor(i, backpackLedOff);
    }
  }
}

boolean nextBlue = false;
void fireLights()
{
  if (nextBlue) {
    nextBlue = false;
    strip.setPixelColor(9, fireBlue);
    strip.setPixelColor(10, ledOff);
  } else {
    nextBlue = true;
    strip.setPixelColor(9, ledOff);
    strip.setPixelColor(10, firePink);
  }
  // flash lights
}

void turnOffGunLights() {
  strip.setPixelColor(9, ledOff);
  strip.setPixelColor(10, ledOff);
}

void updateStripLights() {

  int turnOffUntil = 7 - currentCharge;
  for (int i = 0; i <= LED_COUNT; i++) {

    if (i < turnOffUntil) {
      turnOffLight(i);
    } else {
      strip.setPixelColor(i, power[i]);
    }
  }

  strip.show();
}

void turnOffLight(int pos) {
  strip.setPixelColor(pos, ledOff);
}

void fireNoise() {
    tone(buzzerPin, fireSound, 200);
}

void playWarning() {
  tone(buzzerPin, warningSound, 200);
  delay(300);
  tone(buzzerPin, warningSound, 200);
}

void playEmptyWarning() {
  tone(buzzerPin, warningSound, 100);
  delay(300);
  tone(buzzerPin, warningSound, 100);
  delay(300);
  tone(buzzerPin, warningSound, 100);
}

void playLongWarning() {
  tone(buzzerPin, readySound, 500);
}

void test() {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, power[i]);

  }
  strip.show();
}

void startupSequence() {
  Serial.println("startup sequence ...");
  strip.setPixelColor(7, power[7]);
  strip.show();
  playWarning();


  for (int i = LED_COUNT - 2; i >= 0; i--) {
    delay(500);
    strip.setPixelColor(i, power[i]);
    strip.show();
  }

  playLongWarning();
  currentCharge = 7;
  Serial.println("Ready!!");
}

boolean isRedButtonPressed()
{
  return (digitalRead(redButton) == HIGH);
}
