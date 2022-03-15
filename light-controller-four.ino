#include <Wire.h>
#include <i2cEncoderMiniLib.h>
#include "pitches.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define PWMS 255 // LED display brightness levels 0-255 / PWM levels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// I2C Rotary Encoder
const int RotaryIntPin = A3;
i2cEncoderMiniLib Encoder(0x20);

// Global Variables
int aim = 1; // adjustment increment multiplier level 0=micro(x64) / 1=macro(x8)
float increments = 8; // init: number of steps along scale / must be float to keep calculations acurate
float adjustment_multiplier = PWMS/increments; // 255/8 ~ 31
int toneDuration = 12;
int tonePin = 5;
// int clickTones[] = {
  // NOTE_B1,
  // NOTE_C2, NOTE_D2, NOTE_E2, NOTE_F2,
  // NOTE_G2, NOTE_A2, NOTE_B2, NOTE_C3,
// };

//Callback when the CVAL is incremented
void encoder_increment(i2cEncoderMiniLib* obj) {
  Serial.print("Increment: ");
  Serial.println(Encoder.readCounterByte());
  update_levels(Encoder.readCounterByte());
}

//Callback when the CVAL is decremented
void encoder_decrement(i2cEncoderMiniLib* obj) {
  Serial.print("Decrement: ");
  Serial.println(Encoder.readCounterByte());
  update_levels(Encoder.readCounterByte());
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
  adjust_increment_multiplier(Encoder.readCounterByte());
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

void update_levels(int level) {
  update_light_level(level);
  update_display(level);
  // tone(tonePin, clickTones[level], toneDuration);
}

void update_light_level(int level) {

  // set the PWM
  float pwm = level*adjustment_multiplier;
  // if (pwm === 256) {}
   analogWrite(2, pwm);
   analogWrite(3, pwm);

  Serial.println("pwm ");
  Serial.println(pwm);


}

void update_display(float level) {

  int barLength = ((level*adjustment_multiplier)/PWMS)*SCREEN_WIDTH;
  int percent = ((level*adjustment_multiplier)/PWMS)*100;
  int display_level = level;
  int increments_int = increments;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true); // Use full 256 char 'Code Page 437' font

  display.setTextSize(1);
  display.setCursor(0, 46);
  display.print("Light Level");

  display.setCursor(0, 24);
  display.print(percent);
  display.print("% ");
  display.print(display_level);

  display.drawRect(0, 56, 128, 8, SSD1306_WHITE);
  display.fillRect(0, 56, barLength, 8, SSD1306_WHITE);

  display.display();

}

void adjust_increment_multiplier(float level) {

  if(0 == aim){
    // if micro, change to macro
    increments = 8;
    adjustment_multiplier = PWMS/increments;
    level = level/8;
    Encoder.writeCounter((int32_t) level); /* Reset the counter value */
    Encoder.writeMax((int32_t) increments); /* Set the maximum threshold*/
    update_display(level);
    aim = 1;
  } else {
    // if macro, change to micro
    increments = 64;
    level = level*8;
    adjustment_multiplier = PWMS/increments;
    Encoder.writeCounter((int32_t) level); /* Reset the counter value */
    Encoder.writeMax((int32_t) increments); /* Set the maximum threshold*/
    update_display(level);
    aim = 0;
  }

}

void setup(void) {


  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(RotaryIntPin, INPUT);
  Wire.begin();
  Serial.begin(115200);
  Serial.println("**** I2C Encoder Mini basic example ****");



  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  display.dim(false);



  Encoder.reset();
  Encoder.begin(i2cEncoderMiniLib::WRAP_DISABLE
                | i2cEncoderMiniLib::DIRE_LEFT | i2cEncoderMiniLib::IPUP_ENABLE
                | i2cEncoderMiniLib::RMOD_X1 );

  Encoder.writeCounter((int32_t) 0); /* Reset the counter value */
  Encoder.writeMax((int32_t) increments); /* Set the maximum threshold*/
  Encoder.writeMin((int32_t) 0); /* Set the minimum threshold */
  Encoder.writeStep((int32_t) 1); /* Set the step to 1*/
  Encoder.writeDoublePushPeriod(50); /*Set a period for the double push of 500ms */

  // Definition of the events
  Encoder.onIncrement = encoder_increment;
  Encoder.onDecrement = encoder_decrement;
  Encoder.onMax = encoder_max;
  Encoder.onMin = encoder_min;
  Encoder.onButtonPush = encoder_push;
  Encoder.onButtonRelease = encoder_released;
  Encoder.onButtonDoublePush = encoder_double_push;
  Encoder.onButtonLongPush = encoder_long_push;

  /* Enable the I2C Encoder V2 interrupts according to the previus attached callback */
  Encoder.autoconfigInterrupt();



}

void loop() {
  if (digitalRead(RotaryIntPin) == LOW) {
    /* Check the status of the encoder and call the callback */
    Encoder.updateStatus();
  }
}