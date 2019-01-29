
#include <FastLED.h>
#include "lightPWM.h"

#define DEBUG 0

#define NUM_STRIPS 4
#define NUM_LEDS_PER_STRIP 10
CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];

void breath_leds();

#define LT 0 // left top
#define LB 1 // left bottom
#define RT 2 // right top
#define RB 3 // right bottom

// pin definitions
#define LT_PIN 9
#define LB_PIN 10
#define RT_PIN A0
#define RB_PIN A1

#define BLINK_COLOR 255, 100, 0
#define INNER_BLINK_LED 3
#define OUTER_BLINK_LED 9
/*
 *
 * 
 * commands for lights are coded into bits of "bitwise_command"
 * commands and their bits are:
 * 
     enum light_pwm_order {
      PWM_NOT_USED0,
      PWM_NOT_USED1,
      PWM_NOT_USED2,
      PWM_NOT_USED3,
      PWM_NOT_USED4,
      PWM_OFFSET,
      PWM_ODD_PARITY0,
      PWM_ODD_PARITY1,
      PWM_BLINK_LEFT,
      PWM_BLINK_RIGHT,
      PWM_BREAK,
      PWM_RC_LED,
      PWM_ARM,
    };
 *  
 *  -> bits are encoded into the PWM signal with approx. 96 - 7968 microseconds pulse width
 *  -> first 5 bits can be ignored
 *  -> offset bit is not to read. its only there to stabalize the signal
 *  -> PWM_ODD_PARITY0 bit in combination with PWM_ODD_PARITY1 bit functions as checksum
 *    -> PWM_ODD_PARITY0 is the odd parity bit
 *    -> PWM_ODD_PARITY1 is allways 0
 */
enum com_order
{
  PWM_BLINK_LEFT,
  PWM_BLINK_RIGHT,
  PWM_BREAK,
  PWM_RC_LED,
  PWM_ARM,
  COM_ARRAY_SIZE
};

uint8_t commands[COM_ARRAY_SIZE] = {1, 1, 0, 1, 1};
uint16_t lost_frames = 0;

uint8_t RGBintensity[3] = {255, 255, 255};

void setup()
{
  Serial.begin(115200);

  setup_PWM();

  // initialize all strips
  FastLED.addLeds<NEOPIXEL, LT_PIN>(leds[LT], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, LB_PIN>(leds[LB], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, RT_PIN>(leds[RT], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, RB_PIN>(leds[RB], NUM_LEDS_PER_STRIP);

  // set all strips black just in case
  for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
  {
    leds[LT][i].setRGB(0, 0, 0);
    leds[LB][i].setRGB(0, 0, 0);
    leds[RT][i].setRGB(0, 0, 0);
    leds[RB][i].setRGB(0, 0, 0);
  }
  FastLED.show();
  delay(500);

  // code for the welcome swipe
  for (int32_t j = 0; j <= 255 * NUM_LEDS_PER_STRIP; j += 9)
  {
    for (int32_t i = 0; i < NUM_LEDS_PER_STRIP; i++)
    {
      leds[LT][i].setRGB(constrain(j - (255 * i), 0, 255), constrain(j - (255 * i), 0, 255), constrain(j - (255 * i), 0, 255));
      leds[LB][i].setRGB(constrain(j - (255 * i), 0, 255), constrain(j - (255 * i), 0, 255), constrain(j - (255 * i), 0, 255));
      leds[RT][i].setRGB(constrain(j - (255 * i), 0, 255), constrain(j - (255 * i), 0, 255), constrain(j - (255 * i), 0, 255));
      leds[RB][i].setRGB(constrain(j - (255 * i), 0, 255), constrain(j - (255 * i), 0, 255), constrain(j - (255 * i), 0, 255));
    }
    FastLED.show();
    //delay(1);
  }

  // reduce brightness a little for more pleasant look
  for (uint8_t j = 255; j >= 60; j--)
  {
    RGBintensity[0] = j;
    RGBintensity[1] = j;
    RGBintensity[2] = j;
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
    {
      leds[LT][i].setRGB(RGBintensity[0], RGBintensity[1], RGBintensity[2]);
      leds[LB][i].setRGB(RGBintensity[0], RGBintensity[1], RGBintensity[2]);
      leds[RT][i].setRGB(RGBintensity[0], RGBintensity[1], RGBintensity[2]);
      leds[RB][i].setRGB(RGBintensity[0], RGBintensity[1], RGBintensity[2]);
    }
    FastLED.show();
    delay(2);
  }
  delay(500);
}

void loop()
{
  if (got_pulse == 1 || DEBUG)
  {
#if !DEBUG
    lost_frames = get_pwm_commands(commands);
#endif
    //Serial.println(commands[PWM_ARM]);
    // no loop functionality yet
    if (commands[PWM_ARM])
    {
      for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
      {
        leds[LT][i].setRGB(RGBintensity[0], RGBintensity[1], RGBintensity[2]);
        leds[LB][i].setRGB(RGBintensity[0], RGBintensity[1], RGBintensity[2]);
        leds[RT][i].setRGB(RGBintensity[0], RGBintensity[1], RGBintensity[2]);
        leds[RB][i].setRGB(RGBintensity[0], RGBintensity[1], RGBintensity[2]);
      }
    }
    else
    {
      //Serial.println(commands[PWM_ARM]);
      breath_leds();
    }
    if (commands[PWM_BLINK_LEFT])
    {
      if ((micros() - blink_start[0]) % 500000 < 250000)
      {
        for (uint8_t i = INNER_BLINK_LED; i < OUTER_BLINK_LED; i++)
        {
          leds[LB][i].setRGB(BLINK_COLOR);
        }
      }
      else
      {
        for (uint8_t i = INNER_BLINK_LED; i < OUTER_BLINK_LED; i++)
        {
          leds[LB][i].setRGB(0, 0, 0);
        }
      }
    }
    if (commands[PWM_BLINK_RIGHT])
    {
      if ((micros() - blink_start[1]) % 500000 < 250000)
      {
        for (uint8_t i = INNER_BLINK_LED; i < OUTER_BLINK_LED; i++)
        {
          leds[RB][i].setRGB(BLINK_COLOR);
        }
      }
      else
      {
        for (uint8_t i = INNER_BLINK_LED; i < OUTER_BLINK_LED; i++)
        {
          leds[RB][i].setRGB(0, 0, 0);
        }
      }
    }
    if (commands[PWM_BLINK_LEFT] && commands[PWM_BLINK_RIGHT])
    {
      if ((micros() - blink_start[0]) % 500000 < 250000)
      {
        for (uint8_t i = INNER_BLINK_LED; i < OUTER_BLINK_LED; i++)
        {
          leds[LB][i].setRGB(BLINK_COLOR);
          leds[RB][i].setRGB(BLINK_COLOR);
        }
      }
      else
      {
        for (uint8_t i = INNER_BLINK_LED; i < OUTER_BLINK_LED; i++)
        {
          leds[LB][i].setRGB(0, 0, 0);
          leds[RB][i].setRGB(0, 0, 0);
        }
      }
    }
    if (commands[PWM_RC_LED])
    {
      if ((micros() - blink_start[2]) % 1000000 < 500000 && commands[PWM_ARM])
      {
        leds[LT][7].setRGB(0, 0, 0);
        leds[RT][7].setRGB(0, 0, 0);
        for (uint8_t i = 8; i < 10; i++)
        {
          leds[LT][i].setRGB(0, 0, 255);
          leds[RT][i].setRGB(0, 0, 255);
        }
      }
      else
      {
        for (uint8_t i = 7; i < 10; i++)
        {
          leds[LT][i].setRGB(0, 0, 0);
          leds[RT][i].setRGB(0, 0, 0);
        }
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
  static int brightness[3] = {RGBintensity[0], RGBintensity[1], RGBintensity[2]};
  static int incrementer = 1;
  static uint32_t last_mil = 0;
  uint32_t cur_mil = millis();
  if (cur_mil - last_mil >= 20)
  {
    if (brightness[0] <= 5)
    {
      incrementer = 1;
    }
    else if (brightness[0] >= RGBintensity[0])
    {
      incrementer = -1;
    }

    brightness[0] += incrementer;
    brightness[1] += incrementer;
    brightness[2] += incrementer;
    Serial.println(brightness[2]);
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
    {
      leds[LT][i].setRGB(brightness[0], brightness[1], brightness[2]);
      leds[LB][i].setRGB(brightness[0], brightness[1], brightness[2]);
      leds[RT][i].setRGB(brightness[0], brightness[1], brightness[2]);
      leds[RB][i].setRGB(brightness[0], brightness[1], brightness[2]);
    }
    last_mil = cur_mil;
  }
}
