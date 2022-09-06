#include <Wire.h>
#include <i2cEncoderMiniLib.h>
#include "pitches.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // OLED Address 0x3C for 128x64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LED1PIN 11
#define LED2PIN 5
#define LED3PIN 6
#define LED4PIN 9

// Define and Init I2C Rotary Encoder
#define ROTARY_INT_PIN 3
#define ROTARY_ADDRESS 0x20 // OLED Address 0x3C for 128x64
i2cEncoderMiniLib Encoder(ROTARY_ADDRESS);

// Define and Set Global Variables
float pulses = 254.9; // LED display brightness levels 0-255 / pwm levels
float steps = 64; // init: number of steps along pulses
float stepLength = pulses/steps; // 255/64 ~ 3.98
float pwm = 0;
bool multiplierMacro = false; // adjustment increment stepLength level 0=micro(x64) / 1=macro(x8)

/* Tones */
// int toneDuration = 12;
// int tonePin = 5;
// int clickTones[] = {
  // NOTE_B1,
  // NOTE_C2, NOTE_D2, NOTE_E2, NOTE_F2,
  // NOTE_G2, NOTE_A2, NOTE_B2, NOTE_C3,
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
  // adjust_increment_stepLength();
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

  float barLength = ((stepNum*stepLength)/pulses)*SCREEN_WIDTH;
  float percent = ((stepNum*stepLength)/pulses)*100;
  int display_level = stepNum;

  display.clearDisplay();
  // display.setTextColor(SSD1306_WHITE); // Draw white text
  // display.cp437(true); // Use full 256 char 'Code Page 437' font

  display.setTextSize(1);
  display.setCursor(0, 46);
  display.print("Light Level: ");

  display.setCursor(0, 24);
  display.print(percent);
  display.print("% ");
  display.print(display_level);

  display.drawRect(0, 56, 128, 8, SSD1306_WHITE);
  display.fillRect(0, 56, barLength, 8, SSD1306_WHITE);

  display.display();

}

void adjust_increment_stepLength() {

  // if(true == multiplierMacro){
  //   // if macro, change to micro
  //   steps = 64;
  //   stepLength = pulses/steps; // 255/64 = ~3.98
  //   // level = level*8;
  //   // Encoder.writeCounter((int32_t) level); /* Reset the counter value */
  //   Encoder.writeMax((int32_t) steps); /* Set the maximum threshold*/
  //   multiplierMacro = false;
  // } else {
  //   // if micro, change to macro
  //   steps = 8;
  //   stepLength = pulses/steps; // 255/8 = ~31.87
  //   // // level = level/8;
  //   // Encoder.writeCounter((int32_t) level); /* Reset the counter value */
  //   Encoder.writeMax((int32_t) steps); /* Set the maximum threshold*/
  //   multiplierMacro = true;
  // }
  // update_display(level);

}

void setup(void) {

  /* Define Pins */
  // pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ROTARY_INT_PIN, INPUT);
  // pinMode(button1Pin, INPUT);
  // pinMode(button2Pin, INPUT);
  // pinMode(button3Pin, INPUT);
  // pinMode(button4Pin, INPUT);
  // pinMode(button5Pin, INPUT);

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
  if (digitalRead(ROTARY_INT_PIN) == LOW) {
    /* Check the status of the encoder and call the callback */
    Encoder.updateStatus();
  }
}