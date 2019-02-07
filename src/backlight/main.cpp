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
#define LEFT_PIN A1
#define RIGHT_PIN A0
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
 * the time, when blinking started the last time can be obtained by calling "command_start_times" array with the same enum
 * from "lightPWM.h": uint32_t command_start_times[5];
 * 
 *  e.g. start_time_left = command_start_times[PWM_BLINK_LEFT];
 * 
 * functions to call:
 *  setup_PWM()
 * 
 *  get_pwm_commands(commands) // writes the commands into the array "commands"
 * 
*/

uint8_t red_intensity = 255;
uint8_t dimmed_light = 0;
uint32_t last_breath_time = 0;

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
    leds[TOP][i].setRGB(0, 0, 0);
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
  for (uint8_t j = 255; j >= 30; j--)
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
  
  dimmed_light = red_intensity;

  while (millis() < 2500) ;
}

void loop()
{
  if (got_pulse == 1 || DEBUG)
  {
#if !DEBUG
    lost_frames = get_pwm_commands(commands);
#endif
    leds[TOP][0].setRGB(0, 0, 0);
    if (commands[PWM_ARM])
    {
      for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
      {
        leds[LEFT][i].setRGB(red_intensity, 0, 0);
        leds[RIGHT][i].setRGB(red_intensity, 0, 0);
      }
      dimmed_light = red_intensity;
      last_breath_time = 0;
    }
    else
    {
      breath_leds();
    }
    if (commands[PWM_BRAKE] && command_start_times[PWM_BRAKE] >= 35)
    {
      for (int i = 0; i < 3; i++)
      {
        leds[LEFT][i].setRGB(255, 0, 0);
        leds[RIGHT][i].setRGB(255, 0, 0);
      }
      for (int i = 3; i < 14; i++)
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
      if ((millis() - command_start_times[PWM_BLINK_LEFT]) % 500 < 250)
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
      if ((millis() - command_start_times[PWM_BLINK_RIGHT]) % 500 < 250)
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
      if ((millis() - command_start_times[PWM_BLINK_LEFT]) % 500 < 250)
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
    if (commands[PWM_RC_LED] && commands[PWM_ARM])
    {
      if ((millis() - command_start_times[PWM_RC_LED]) % 1000 < 500)
      {
        leds[TOP][0].setRGB(0, 0, 255);
        /*
        for (uint8_t i = 11; i < 12; i++)
        {
          leds[LEFT][i].setRGB(0, 0, 255);
          leds[RIGHT][i].setRGB(0, 0, 255);
        }
        */
      }
      else
      {
        leds[TOP][0].setRGB(0, 0, 0);
        /*
        for (uint8_t i = 11; i < 12; i++)
        {
          leds[LEFT][i].setRGB(0, 0, 0);
          leds[RIGHT][i].setRGB(0, 0, 0);
        }
        */
      }
    }
#if DEBUG
    delay(20);
#else
    got_pulse = 0;
    while (micros() - cur_pwm_micros < 3000);
#endif
    FastLED.show();
  }
}

void breath_leds()
{
  static int incrementer = 1;
  uint32_t cur_breath_time = millis() - command_start_times[PWM_ARM];
  if (cur_breath_time - last_breath_time >= 15)
  {
    if (dimmed_light <= 5)
    {
      incrementer = 1;
    }
    else if (dimmed_light >= red_intensity)
    {
      incrementer = -1;
    }

    dimmed_light += incrementer;
    //Serial.println(dimmed_light);
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
    {
      leds[LEFT][i].setRGB(dimmed_light, 0, 0);
      leds[RIGHT][i].setRGB(dimmed_light, 0, 0);
    }
    Leds[TOP][0] = setRGB(0, map(dimmed_light, 5, 30, 10, 255), 0)
    last_breath_time = cur_breath_time;
  }
}
