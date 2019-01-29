#include "Arduino.h"
#include <FastLED.h>
#include "lightPWM.h"

void breath_leds();

#define NUM_STRIPS 3
#define NUM_LEDS_PER_STRIP 20
CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];

#define DEBUG 0

#define LEFT 0
#define RIGHT 1
#define TOP 2

// pin definitions
#define LEFT_PIN A0
#define RIGHT_PIN A1
#define TOP_PIN A2

#define BLINK_COLOR 255, 70, 0
#define INNER_BLINK_LED 18
#define OUTER_BLINK_LED 20

enum com_order
{
  PWM_BLINK_LEFT,
  PWM_BLINK_RIGHT,
  PWM_BRAKE,
  PWM_RC_LED,
  PWM_ARM,
  COM_ARRAY_SIZE
};

uint8_t commands[COM_ARRAY_SIZE] = {1, 1, 1, 0, 1};
uint16_t lost_frames = 0;

/* get the commands by using the enum "com_order"
 *  example for getting blink left comando:
 * 
 *  if (commands[PWM_BLINK_LEFT] == true) {
 *    // go to blink routine
 *  }
 * 
 * the time, when blinking started the last time can be obtained by calling "blink_start" array with the same enum
 * from "lightPWM.h": uint32_t blink_start[2];
 * 
 *  e.g. start_time_left = blink_start[PWM_BLINK_LEFT];
 * 
 * functions to call:
 *  setup_PWM()
 * 
 *  get_pwm_commands(commands) // writes the commands into the array "commands"
 * 
*/

uint8_t red_intensity = 255;

void setup()
{
  // initialize the pwm read functionality
  setup_PWM();
  //Serial.begin(115200);
  // initialize all strips
  FastLED.addLeds<NEOPIXEL, LEFT_PIN>(leds[LEFT], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, RIGHT_PIN>(leds[RIGHT], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, TOP_PIN>(leds[TOP], NUM_LEDS_PER_STRIP);

  // set all strips black just in case
  for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
  {
    leds[LEFT][i].setRGB(0, 0, 0);
    leds[RIGHT][i].setRGB(0, 0, 0);
  }
  FastLED.show();
  delay(500);

  // code for the welcome swipe
  for (int32_t j = 0; j <= 5100; j += 17)
  {
    for (int32_t i = 0; i < NUM_LEDS_PER_STRIP; i++)
    {
      leds[LEFT][i].setRGB(constrain(j - (255 * i), 0, 255), 0, 0);
      leds[RIGHT][i].setRGB(constrain(j - (255 * i), 0, 255), 0, 0);
    }
    FastLED.show();
    //delay(1);
  }

  // reduce brightness a little for more pleasant look
  for (uint8_t j = 255; j >= 60; j--)
  {
    red_intensity = j;
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
    {
      leds[LEFT][i].setRGB(red_intensity, 0, 0);
      leds[RIGHT][i].setRGB(red_intensity, 0, 0);
    }
    FastLED.show();
    delay(2);
  }
}

void loop()
{
  if (got_pulse == 1 || DEBUG)
  {
#if !DEBUG
    lost_frames = get_pwm_commands(commands);
#endif

#define BRAKE_THRESHOLD 30
    static uint8_t brake_on = 0;
    uint8_t brake_show = 0;
    if (commands[PWM_BRAKE] && brake_on < BRAKE_THRESHOLD * 2)
    {
      brake_on++;
    }
    else if (!commands[PWM_BRAKE] && brake_on > 0)
    {
      brake_on--;
    }
    if (brake_on > BRAKE_THRESHOLD)
    {
      brake_show = 1;
    }
    if (commands[PWM_ARM])
    {
      for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
      {
        leds[LEFT][i].setRGB(red_intensity, 0, 0);
        leds[RIGHT][i].setRGB(red_intensity, 0, 0);
      }
    }
    else
    {
      breath_leds();
    }
    if (brake_show)
    {
      for (int i = 0; i < 4; i++)
      {
        leds[LEFT][i].setRGB(255, 0, 0);
        leds[RIGHT][i].setRGB(255, 0, 0);
      }
      for (int i = 4; i < 14; i++)
      {
        leds[LEFT][i].setRGB(0, 0, 0);
        leds[RIGHT][i].setRGB(0, 0, 0);
      }
      for (int i = 14; i < NUM_LEDS_PER_STRIP; i++)
      {
        leds[LEFT][i].setRGB(255, 0, 0);
        leds[RIGHT][i].setRGB(255, 0, 0);
      }
    }
    if (commands[PWM_BLINK_LEFT])
    {
      if ((micros() - blink_start[0]) % 500000 < 250000)
      {
        for (uint8_t i = INNER_BLINK_LED; i < OUTER_BLINK_LED; i++)
        {
          leds[LEFT][i].setRGB(BLINK_COLOR);
        }
      }
      else
      {
        for (uint8_t i = INNER_BLINK_LED; i < OUTER_BLINK_LED; i++)
        {
          leds[LEFT][i].setRGB(0, 0, 0);
        }
      }
    }
    if (commands[PWM_BLINK_RIGHT])
    {
      if ((micros() - blink_start[1]) % 500000 < 250000)
      {
        for (uint8_t i = INNER_BLINK_LED; i < OUTER_BLINK_LED; i++)
        {
          leds[RIGHT][i].setRGB(BLINK_COLOR);
        }
      }
      else
      {
        for (uint8_t i = INNER_BLINK_LED; i < OUTER_BLINK_LED; i++)
        {
          leds[RIGHT][i].setRGB(0, 0, 0);
        }
      }
    }
    if (commands[PWM_BLINK_LEFT] && commands[PWM_BLINK_RIGHT])
    {
      if ((micros() - blink_start[0]) % 500000 < 250000)
      {
        for (uint8_t i = INNER_BLINK_LED; i < OUTER_BLINK_LED; i++)
        {
          leds[LEFT][i].setRGB(BLINK_COLOR);
          leds[RIGHT][i].setRGB(BLINK_COLOR);
        }
      }
      else
      {
        for (uint8_t i = INNER_BLINK_LED; i < OUTER_BLINK_LED; i++)
        {
          leds[LEFT][i].setRGB(0, 0, 0);
          leds[RIGHT][i].setRGB(0, 0, 0);
        }
      }
    }
    if (commands[PWM_RC_LED])
    {
      if ((micros() - blink_start[2]) % 1000000 < 500000)
      {
        leds[TOP][0].setRGB(0, 0, 255);
      }
    }
    FastLED.show();
#if DEBUG
    delay(20);
#else 
    got_pulse = 0;
#endif
  }
}

void breath_leds()
{
  static int brightness = red_intensity;
  static int incrementer = 1;
  static uint32_t last_mil = 0;
  uint32_t cur_mil = millis();
  if (cur_mil - last_mil >= 20)
  {
    if (brightness <= 5)
    {
      incrementer = 1;
    }
    else if (brightness >= red_intensity)
    {
      incrementer = -1;
    }

    brightness += incrementer;
    //Serial.println(brightness);
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
    {
      leds[LEFT][i].setRGB(brightness, 0, 0);
      leds[RIGHT][i].setRGB(brightness, 0, 0);
    }
    last_mil = cur_mil;
  }
}
