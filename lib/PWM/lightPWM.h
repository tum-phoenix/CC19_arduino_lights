#ifndef PWM_H_
#define PWM_H_

#include "Arduino.h"
#include "PinChangeInterrupt.h"

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

volatile uint32_t blink_start[3] = {0, 0, 0};

void setup_PWM();
uint16_t get_pwm_commands(uint8_t *tar_array);

/////////////////////////////////////////////////////////////////////////////////////////////

volatile uint32_t cur_pwm_micros = 0;
volatile uint8_t cur_pwm_state = 0;
volatile uint32_t pulse_pwm_start = 0;
volatile uint32_t cur_pwm_pulse = 0;
volatile uint16_t bitwise_command = 0;
volatile uint16_t bitwise_command_prev = 0;

uint16_t pwm_lost_frames = 0;
uint8_t got_pulse = 0;
#define COM_NUM 5
/*
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
 * 
 *  "bitwise_command" bit order
 *    0 - PWM_BLINK_LEFT
      1 - PWM_BLINK_RIGHT
      2 - PWM_BREAK
      3 - PWM_RC_LED
      4 - PWM_ARM
      5-7 - not used
 */
void ISR_function(void);

void setup_PWM()
{
  pinMode(11, INPUT);
  attachPCINT(digitalPinToPCINT(11), ISR_function, CHANGE);
}

void ISR_function(void)
{
  cur_pwm_micros = micros();
  cur_pwm_state = digitalRead(11); // reads current state of pin
  if (cur_pwm_state == 0)
  {                                                     // falling edge means stop of signal
    bitwise_command_prev = bitwise_command;             // save previous command in case we got an invalid new one
    cur_pwm_pulse = (cur_pwm_micros - pulse_pwm_start); // calculate the pulse length
    got_pulse = 1;
    //Serial.println(cur_pwm_pulse);
    cur_pwm_pulse = cur_pwm_pulse >> 6;              // getting rid of the offset pulse, discard first 6 bit
    bitwise_command = (uint8_t)(cur_pwm_pulse >> 2); // dicard the parity for now
    if ((bitwise_command & 1) && !(bitwise_command_prev & 1))
    {
      blink_start[0] = cur_pwm_micros;
    }
    if (((bitwise_command >> 1) & 1) && !((bitwise_command_prev >> 1) & 1))
    {
      blink_start[1] = cur_pwm_micros;
    }
    if (((bitwise_command >> 3) & 1) && !((bitwise_command_prev >> 3) & 1))
    {
      blink_start[2] = cur_pwm_micros;
    }
    uint8_t par_count = 0;
    for (uint8_t i = 0; i < COM_NUM; i++)
    { // calculate expected parity bit
      par_count += (bitwise_command >> i & 1);
    }

    if (par_count % 2 != (cur_pwm_pulse & 1)) // the parity bit needs to be the oposite of the even/odd parity count
    {
      if (!(cur_pwm_pulse >> 1 & 1))
      { // if parity bit 0 is correct and parity bit 1 is low everything seems correct
        //        if (pwm_lost_frames)
        //        {
        //          pwm_lost_frames--;
        //        }
        return;
      }
      else
      {
        bitwise_command = bitwise_command_prev;
        pwm_lost_frames++;
        return;
      }
    }
  }
  else
  {
    pulse_pwm_start = cur_pwm_micros;
    return;
  }
}

uint16_t get_pwm_commands(uint8_t *tar_array)
{ // writes commands into array and return (dirrctly in a row) lost frames
  for (uint8_t i = 0; i < COM_NUM; i++)
  {
    tar_array[i] = (bitwise_command >> i) & 1;
  }
  return pwm_lost_frames;
}
#endif // PWM_H_
