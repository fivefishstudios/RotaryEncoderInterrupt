#include "mbed.h"
#include "LCD_DISCO_F429ZI.h"
#include "PinDetect.h"   // URL: http://mbed.org/users/AjK/libraries/PinDetect
using namespace std;

LCD_DISCO_F429ZI lcd;
char lcdBuffer[20];   // lcd display buffer

// Interrupt for User PushButton switch 
InterruptIn Button(PA_0);
DigitalOut led1(PG_13);
DigitalOut greenLed(PD_12);

// Our Interrupt Handler Routine, for Button(PA_0)
void PBHandler(){
	led1 = !led1; // toggle LED
	if (led1){
		lcd.DisplayStringAt(0, 250, (uint8_t *) " Interrupt! ", CENTER_MODE); 
	} else {
		lcd.DisplayStringAt(0, 250, (uint8_t *) " Another IRQ! ", CENTER_MODE); 
	}
}

// Interrupt for Encoder Switch
PinDetect  EncoderSwitch(PF_9);   // PF_9 working
DigitalOut led2(PG_14); 

void SwitchHandler(){
  led2 = !led2;
  // greenLed = !greenLed;
}


// Interrupt for Encoder Rotary Out A/B
PinDetect EncoderOutA(PC_12, PullUp);  
PinDetect EncoderOutB(PC_11, PullUp);
int EncoderOutA_LastState = 0;
int EncoderOutB_LastState = 0;
int EncoderOutA_State;
int EncoderOutB_State;
int rotation_value = 0;

void UpdateRotaryValue(){
  // display on LCD
  lcd.SetFont(&Font24);
  lcd.SetBackColor(LCD_COLOR_RED);   
  lcd.SetTextColor(LCD_COLOR_WHITE);
  sprintf(lcdBuffer, "%03d", rotation_value);
  lcd.DisplayStringAt(1, 100, (uint8_t *)lcdBuffer, CENTER_MODE);   
}

// Check Rotary Encoder status (switch + rotation)
// based on datasheet (CW Rotation)
// position |  OutA  | OutB
//    1     |    0   |  0
//    2     |    1   |  0
//    3     |    1   |  1   // starting
//    4     |    0   |  1

// Interrupt Handlers for A/B
void RotaryEncoderHandlerA_assert(){
  EncoderOutA_State = 1;    
  led1 = 1;
  if (EncoderOutA_State != EncoderOutA_LastState)
  {
    // check if we moved CW, increment for each detent
    if (EncoderOutA_State != EncoderOutB_State)
    {
      // CW
      rotation_value += 1;
    } else {
      rotation_value -= 1;
    }
  }  
  EncoderOutA_LastState = EncoderOutA_State;
  UpdateRotaryValue();
}

void RotaryEncoderHandlerA_deasserted(){
  EncoderOutA_State = 0;    
  led1 = 0;
  if (EncoderOutA_State != EncoderOutA_LastState)
  {
    // check if we moved CW, increment for each detent
    if (EncoderOutA_State != EncoderOutB_State)
    {
      // CW
      rotation_value += 1;
    } else {
      // CCW
      rotation_value -= 1;
    }
  }  
  EncoderOutA_LastState = EncoderOutA_State;
  UpdateRotaryValue();
}

void RotaryEncoderHandlerB_assert(){
  EncoderOutB_State = 1;    
  led2 = 1;
  EncoderOutB_LastState = EncoderOutB_State;
}

void RotaryEncoderHandlerB_deasserted(){
  EncoderOutB_State = 0;    
  led2 = 0;
  if (EncoderOutB_LastState){
    if (!EncoderOutA_State){
      rotation_value -= 1;
    }
  }
  EncoderOutB_LastState = EncoderOutB_State;  
  UpdateRotaryValue();
}




int main(){

  // setup Interrupt for devboard PB Switch
  Button.rise(&PBHandler);

  // setup Interrupts for Encoder Switch
  EncoderSwitch.attach_asserted(&SwitchHandler);
  EncoderSwitch.setSampleFrequency(10000);  // Start sampling pb input using interrupts (us)

  // setup Interrupts for Encoder Output A/B
  EncoderOutA.attach_asserted(&RotaryEncoderHandlerA_assert);
  EncoderOutA.attach_deasserted(&RotaryEncoderHandlerA_deasserted);
  EncoderOutA.setSampleFrequency(500);   // in (us)
  
  EncoderOutB.attach_asserted(&RotaryEncoderHandlerB_assert);
  EncoderOutB.attach_deasserted(&RotaryEncoderHandlerB_deasserted); 
  EncoderOutB.setSampleFrequency(500);   // in (us)

  // setup LCD Display
  lcd.Clear(LCD_COLOR_RED);
  lcd.SetFont(&Font24);
  lcd.SetBackColor(LCD_COLOR_RED);      // text background color
  lcd.SetTextColor(LCD_COLOR_WHITE);    // text foreground color
  lcd.DisplayStringAt(0, 10, (uint8_t *)" Encoder v1.3 ", CENTER_MODE);
  lcd.DisplayStringAt(0, 290, (uint8_t *)" by owel.codes ", CENTER_MODE);
  lcd.SetFont(&Font20);
  lcd.DisplayStringAt(0, 220, (uint8_t *)" Interrupt Driven ", CENTER_MODE);
  lcd.DisplayStringAt(0, 240, (uint8_t *)" Rotary Encoder ", CENTER_MODE);

  UpdateRotaryValue();  // initial display of value
  // start of infinite loop....
  while(1){   
    // we're relying on interrupts for all switches and encoders to work
    // no polling done inside this loop
    // Note: We must have LCD update inside the Interrupt routine so the display is not affected by blocking delays

    greenLed = !greenLed;
    wait(2);

  }

  return 0;
}