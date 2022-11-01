#include <Wire.h>
#include <i2cEncoderMiniLib.h>
#include "pitches.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LED1PIN 11
#define LED2PIN 5
#define LED3PIN 6
#define LED4PIN 9

#define BUTTON_0_PIN 7
#define BUTTON_1_PIN 10
#define BUTTON_2_PIN 8
#define BUTTON_3_PIN 4
#define BUTTON_4_PIN 2

// debounce variables
// the current reading from the input pin
int buttonState0;
int buttonState1;
int buttonState2;
int buttonState3;
int buttonState4;
// the previous reading from the input pin
int lastButtonState0 = HIGH;
int lastButtonState1 = HIGH;
int lastButtonState2 = HIGH;
int lastButtonState3 = HIGH;
int lastButtonState4 = HIGH;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 25;    // the debounce time; increase if the output flickers

// Define and Init I2C Rotary Encoder
#define ROTARY_INT_PIN 3
#define ROTARY_ADDRESS 0x20 // OLED Address 0x3C for 128x64
i2cEncoderMiniLib Encoder(ROTARY_ADDRESS);

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // OLED Address 0x3C for 128x64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define and Set Global Variables
float dutyCycle = 255; // LED display brightness levels 0-255 / pwm levels
float steps = 64; // init: number of steps along dutyCycle
float stepLength = dutyCycle/steps; // 255/64 ~ 3.98
float pwm = 0;
float level = 0;
bool multiplierMacro = false; // adjustment increment stepLength level 0=micro(x64) / 1=macro(x8)

/* Tones */
// int toneDuration = 12;
// int tonePin = 5;
// int clickTones[] = {
//   NOTE_B1,
//   NOTE_C2, NOTE_D2, NOTE_E2, NOTE_F2,
//   NOTE_G2, NOTE_A2, NOTE_B2, NOTE_C3,
// };


//Callback when the CVAL is incremented
void encoder_increment(i2cEncoderMiniLib* obj) {
  Serial.print("Increment: ");
  Serial.println(Encoder.readCounterByte());
  runRotaryStep(Encoder.readCounterByte());
}

//Callback when the CVAL is decremented
void encoder_decrement(i2cEncoderMiniLib* obj) {
  Serial.print("Decrement: ");
  Serial.println(Encoder.readCounterByte());
  runRotaryStep(Encoder.readCounterByte());
}

//Callback when CVAL reach MAX
void encoder_max(i2cEncoderMiniLib* obj) {
  Serial.print("Maximum threshold: ");
  Serial.println(Encoder.readCounterByte());
}

//Callback when CVAL reach MIN
void encoder_min(i2cEncoderMiniLib* obj) {
  Serial.print("Minimum threshold: ");
  Serial.println(Encoder.readCounterByte());
}

//Callback when the encoder is pushed
void encoder_push(i2cEncoderMiniLib* obj) {
  Serial.println("Encoder is pushed!");
  adjust_increment_stepLength();
}

//Callback when the encoder is released
void encoder_released(i2cEncoderMiniLib* obj) {
  Serial.println("Encoder is released");
}

//Callback when the encoder is double pushed
void encoder_double_push(i2cEncoderMiniLib* obj) {
  Serial.println("Encoder is double pushed!");
}

//Callback when the encoder is long pushed
void encoder_long_push(i2cEncoderMiniLib* obj) {
  Serial.println("Encoder is long pushed!");
}

void runRotaryStep(float stepNum) {
  update_light_level(stepNum);
  update_display(stepNum);
  // tone(tonePin, clickTones[level], toneDuration);
}

void update_light_level(float stepNum) {

  Serial.print("update_light_level: ");
  Serial.print(stepNum);
  Serial.print(" ");
  Serial.println(stepLength);

  // set the pwm
  pwm = stepNum*stepLength;

  analogWrite(LED1PIN, pwm);
  analogWrite(LED2PIN, pwm);
  analogWrite(LED3PIN, pwm);
  analogWrite(LED4PIN, pwm);

  Serial.print("PWN Level: ");
  Serial.println(pwm);

}

void update_display(float stepNum) {

  int adjustedScreenWidth = SCREEN_WIDTH - 16;

  float barLengthW = (((stepNum*stepLength)/dutyCycle)*adjustedScreenWidth);
  float barLengthH = ((((stepNum*stepLength)/dutyCycle)*SCREEN_HEIGHT)-SCREEN_HEIGHT)*-1;
  float percent = ((stepNum*stepLength)/dutyCycle)*100;
  int display_level = stepNum;

  display.clearDisplay();

  // horizontal meters

  // indicators
  display.fillCircle(2, 5, 2, SSD1306_WHITE);
  display.fillCircle(2, 5+16, 2, SSD1306_WHITE);
  display.fillCircle(2, 5+16+16, 2, SSD1306_WHITE);
  display.fillCircle(2, 5+16+16+16, 2, SSD1306_WHITE);

  // identifier numbers
  display.setTextSize(1);
  display.setCursor(8, 0+2);
  display.print("1");
  display.setCursor(8, 16+2);
  display.print("2");
  display.setCursor(8, 32+2);
  display.print("3");
  display.setCursor(8, 48+2);
  display.print("4");
  
  // meters
  display.drawRoundRect(16, 0, adjustedScreenWidth, 12, 4, SSD1306_WHITE);
  display.fillRoundRect(16, 0, barLengthW, 12, 4, SSD1306_WHITE);

  display.drawRoundRect(16, 16, adjustedScreenWidth, 12, 4, SSD1306_WHITE);
  display.fillRoundRect(16, 16, barLengthW, 12, 4, SSD1306_WHITE);

  display.drawRoundRect(16, 32, adjustedScreenWidth, 12, 4, SSD1306_WHITE);
  display.fillRoundRect(16, 32, barLengthW, 12, 4, SSD1306_WHITE);

  display.drawRoundRect(16, 48, adjustedScreenWidth, 12, 4, SSD1306_WHITE);
  display.fillRoundRect(16, 48, barLengthW, 12, 4, SSD1306_WHITE);

  // vertical meters

  // display.drawRect(72, 0, 8, SCREEN_HEIGHT, SSD1306_WHITE);
  // display.fillRect(72, barLengthH, 8, SCREEN_HEIGHT, SSD1306_WHITE);
  
  // display.drawRect(88, 0, 8, SCREEN_HEIGHT, SSD1306_WHITE);
  // display.fillRect(88, barLengthH, 8, SCREEN_HEIGHT, SSD1306_WHITE);
  
  // display.drawRect(104, 0, 8, SCREEN_HEIGHT, SSD1306_WHITE);
  // display.fillRect(104, barLengthH, 8, SCREEN_HEIGHT, SSD1306_WHITE);

  // display.drawRect(120, 0, 8, SCREEN_HEIGHT, SSD1306_WHITE);
  // display.fillRect(120, barLengthH, 8, SCREEN_HEIGHT, SSD1306_WHITE);

  display.display();

}

void adjust_increment_stepLength() {

  if(true == multiplierMacro){
    // if macro, change to micro
    steps = 64;
    stepLength = dutyCycle/steps; // 255/64 = ~3.98
    level = level*8;
    Encoder.writeCounter((int32_t) level); /* Reset the counter value */
    Encoder.writeMax((int32_t) steps); /* Set the maximum threshold*/
    multiplierMacro = false;
  } else {
    // if micro, change to macro
    steps = 8;
    stepLength = dutyCycle/steps; // 255/8 = ~31.87
    level = level/8;
    Encoder.writeCounter((int32_t) level); /* Reset the counter value */
    Encoder.writeMax((int32_t) steps); /* Set the maximum threshold*/
    multiplierMacro = true;
  }
  update_display(level);

}

void press_button_0() {
  Serial.println("press_button_0");
}
void press_button_1() {
  Serial.println("press_button_1");
}
void press_button_2() {
  Serial.println("press_button_2");
}
void press_button_3() {
  Serial.println("press_button_3");
}
void press_button_4() {
  Serial.println("press_button_4");
}

void setup(void) {

  /* Define Pins */
  // pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ROTARY_INT_PIN, INPUT);
  pinMode(BUTTON_0_PIN, INPUT);
  pinMode(BUTTON_1_PIN, INPUT);
  pinMode(BUTTON_2_PIN, INPUT);
  pinMode(BUTTON_3_PIN, INPUT);
  pinMode(BUTTON_4_PIN, INPUT);

  pinMode(LED1PIN, OUTPUT);
  pinMode(LED2PIN, OUTPUT);
  pinMode(LED3PIN, OUTPUT);
  pinMode(LED4PIN, OUTPUT);

  Wire.begin();
  Serial.begin(115200);

  Serial.println("**** Light Controller Init ****");

  /*
  * Setup OLED
  * SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  */ 
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("**** SSD1306 allocation failed ****"));
    for(;;); // Don't proceed, loop forever
  } else {
    Serial.println(F("** SSD1306 allocation Successful **"));
  }

  /*
  * Setup Rotary Controller
  */ 
  Encoder.reset();
  Encoder.begin(i2cEncoderMiniLib::WRAP_DISABLE
                | i2cEncoderMiniLib::DIRE_LEFT | i2cEncoderMiniLib::IPUP_ENABLE
                | i2cEncoderMiniLib::RMOD_X1 );

  Encoder.writeCounter((int32_t) 0); /* Reset the counter value */
  Encoder.writeMax((int32_t) steps); /* Set the maximum threshold*/
  Encoder.writeMin((int32_t) 0); /* Set the minimum threshold */
  Encoder.writeStep((int32_t) 1); /* Set the stepLength to 1*/
  Encoder.writeDoublePushPeriod(50); /*Set a period for the double push of 500ms */

  // Definition of the events
  Encoder.onIncrement = encoder_increment;
  Encoder.onDecrement = encoder_decrement;
  // Encoder.onMax = encoder_max;
  Encoder.onMin = encoder_min;
  Encoder.onButtonPush = encoder_push;
  Encoder.onButtonRelease = encoder_released;
  Encoder.onButtonDoublePush = encoder_double_push;
  Encoder.onButtonLongPush = encoder_long_push;

  /* Enable the I2C Encoder V2 interrupts according to the previus attached callback */
  Encoder.autoconfigInterrupt();

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true); // Use full 256 char 'Code Page 437' font
  display.display();
  display.dim(false);
  update_display(0);

}

void loop() {

  //Encoder
  if (digitalRead(ROTARY_INT_PIN) == LOW) {
    Encoder.updateStatus();
  }

  // TODO: loop
  // 1
  int readingButton1 = digitalRead(BUTTON_1_PIN);
  if (readingButton1 != lastButtonState1) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (readingButton1 != buttonState1) {
      buttonState1 = readingButton1;
      if (buttonState1 == LOW) {
        press_button_1();
      }
    }
  }
  lastButtonState1 = readingButton1;

  // 2
  int readingButton2 = digitalRead(BUTTON_2_PIN);
  if (readingButton2 != lastButtonState2) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (readingButton2 != buttonState2) {
      buttonState2 = readingButton2;
      if (buttonState2 == LOW) {
        press_button_2();
      }
    }
  }
  lastButtonState2 = readingButton2;

  // 3
  int readingButton3 = digitalRead(BUTTON_3_PIN);
  if (readingButton3 != lastButtonState3) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (readingButton3 != buttonState3) {
      buttonState3 = readingButton3;
      if (buttonState3 == LOW) {
        press_button_3();
      }
    }
  }
  lastButtonState3 = readingButton3;

  // 4
  int readingButton4 = digitalRead(BUTTON_4_PIN);
  if (readingButton4 != lastButtonState4) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (readingButton4 != buttonState4) {
      buttonState4 = readingButton4;
      if (buttonState4 == LOW) {
        press_button_4();
      }
    }
  }
  lastButtonState4 = readingButton4;

}