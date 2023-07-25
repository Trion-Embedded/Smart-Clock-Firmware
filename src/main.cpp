#include <Arduino.h>
#include <LiquidCrystal.h>
#include <ESP32Time.h>
#include <button.h>

// Pin Number
#define ENCODER_CLK 34
#define ENCODER_DT 35
#define ENCODER_BTN 32
#define LED 13
#define LED2 18

#define ENCODER_CHANGE_CONST 7
#define MAX_MODE 5
#define MIN_MODE 0
#define ENCODER_COUNTER_MAX_LIM (MAX_MODE * ENCODER_CHANGE_CONST)
#define ENCODER_COUNTER_MIN_LIM (MIN_MODE * ENCODER_CHANGE_CONST)

// LCD PINOuts
LiquidCrystal lcd(12, 16, 15, 19, 5, 4);

typedef struct
{
  uint8_t dt_pin;
  uint8_t clk_pin;
  int pulse_pos; // Private
  int8_t mode;
} encoder_t;

void clicked();
void double_clicked();
void long_pressed();

button_t button = {
    .click_fun = &clicked,
    .double_click_fun = &double_clicked,
    .long_press_fun = &long_pressed};

void IRAM_ATTR button_isr()
{
  button_update(&button);
}
encoder_t encoder;
// Display which mode before choose
int pre_mode = 1;

// Choose which function that you want
int mode = 0;
int currentMode = 0;
int lastStep = 0;

// MODE state ref
int lastMode = 0;

// State of mode.
bool mode_state = false;
bool presscheck = false;
bool dblpresscheck = false;
bool sc = false;
bool click = false;
bool dblsc = false;

//  Value of minute.
int minute = 0;
// Value of second.
int second = 60;

// Mode Selection
String dir = "";
unsigned long last_run = 0;

// Offset in seconds GMT. (For rtc_clock funtion)
ESP32Time rtc(0);

// Timer for clock function.
hw_timer_t *My_timer = NULL;

// Timer for pomodoro function.
hw_timer_t *My_timer2 = NULL;

/*     encoder updating function          */
void encoder_update(encoder_t *e)
{
  // Read encoder inputs
  int clkValue = digitalRead(e->clk_pin);
  int dtValue = digitalRead(e->dt_pin);

  // If data and clock values are equal then encoder turns CCW, else CW
  if (dtValue == clkValue)
  {
    // Check for limits
    if (e->pulse_pos < ENCODER_COUNTER_MAX_LIM)
      e->pulse_pos++;
  }
  else
  {
    // Check for limits
    if (e->pulse_pos > ENCODER_COUNTER_MIN_LIM)
      e->pulse_pos--;
  }

  // Update mode counter with given change constant
  e->mode = e->pulse_pos / ENCODER_CHANGE_CONST;
}

//--------- Interuption -------------//

// Interupt for pomodor function.
void IRAM_ATTR onTimer2()
{
  // Increase second variable every second.
  second--;
}

void IRAM_ATTR ISR()
{
  encoder_update(&encoder);
}
/*-----------------Task Function-----------------------*/
// Task for menu function.
// TaskHandle_t Task_Menu;

// Task for rtc_clock function. (Mode 1)
TaskHandle_t Task_RTC;

// Task for pomodoro function. (Mode 2)
TaskHandle_t Task_Pomodoro;

// Task for send_text function. (Mode 3)
TaskHandle_t Task_Text;

// mode selection function
void modeSel()
{
  char str[30];

  // lcd.setCursor(0, 0);
  // lcd.print("                ");

  lcd.setCursor(0, 0);
  lcd.print(encoder.pulse_pos);

  sprintf(str, "Mode %d     ", encoder.mode);
  lcd.setCursor(6, 1);
  lcd.print(str);
}

// Task for menu funtion.
void Task_Menu()
{
  //  if mode_state false this funtion will work.

  while (mode_state == false)
  {
    // When INCREMENT button is on function works.
    lcd.setCursor(0, 0);
    lcd.print("pre_mode: ");
    lcd.print(pre_mode);
    if (encoder.mode != lastMode)
    {
      pre_mode++;

      // when pre_mode pass 3, turn to 1. Because there are 3 modes.
      if (pre_mode == 4)
      {
        pre_mode = 1;
      }

      delay(20);
      lastMode = encoder.mode;
    }
    if (click == true)
    {
      // Assign pre_mode to mode to get in that mode.
      mode = pre_mode;
      currentMode = 1;
      lcd.setCursor(0, 1);
      lcd.print("mode: ");
      Serial.print("mode: ");
      lcd.print(mode);

      // State is true because dont works again this function.
      mode_state = true;
      delay(20);
    }

    delay(20);
  }
  delay(20);
}

// Task for rtc_clock function. (Mode 1)
void Task_RTC_Code(void *pvParameters)
{
  // Create a place to hold incoming messages.
  int incoming;

  // To set time again.
  int read_break;

  // Time definitions.
  int sec, minute, hour, day, month, year;

  // State is true because dont works again this function.
  bool send_time_state = true;

  // Input time from user.
  int input_time[14];

  // time value.
  int value = 0;

  // Step of time.
  int i = 0;
  int i_time = 0;
  sc = false;
  while (1)
  {
    if (mode == 1 && mode_state == true && sc == true && currentMode == 2)
    {
      lcd.clear();
      // Step of time.
      i = 0;
      send_time_state = false;
      i_time = 1;

      // there are 14 digit to input.
      while (i < 18)
      {
        // Increase the time value.
        if (encoder.pulse_pos != lastStep)
        {
          value++;

          if (value == 10)
          {
            value = 0;
          }

          // Clock config

          // First digit of time max may be 2.
          if (i == 0 && value == 3)
          {
            value = 0;
          }
          // Second digit of time max may be 4 if digit one is 2.
          else if (i == 1 && input_time[0] == 2 && value == 4)
          {
            value = 0;
          }
          // Third digit of time max may be 5.
          else if (i == 3 && value == 6)
          {
            value = 0;
          }
          // Fifth digit of time max may be 5.
          else if (i == 6 && value == 6)
          {
            value = 0;
          }

          // Date config.
          else if (i == 8 && value == 4)
          {
            value = 0;
          }
          else if (i == 9 && input_time[8] == 3)
          {
            value = 0;
          }
          else if (i == 11 && value == 2)
          {
            value = 0;
          }
          else if (i == 12 && input_time[11] == 1 && value == 3)
          {
            value = 0;
          }
          if (i < 8)
          {
            lcd.setCursor(i, 0);
            lcd.print(value);
            lcd.print('\b');
          }
          if (i > 7)
          {
            lcd.setCursor(i - 8, 1);
            lcd.print(value);
            lcd.print('\b');
          }
          lastStep = encoder.pulse_pos;
        }

        delay(20);

        if (digitalRead(ENCODER_BTN) == HIGH)
        {
          while (digitalRead(ENCODER_BTN) == HIGH)
            ;

          input_time[i] = value;
          if (i < 8)
          {
            lcd.setCursor(i, 0);
            lcd.print(value);
          }
          if (i > 7)
          {
            lcd.setCursor(i - 8, 1);
            lcd.print(value);
          }

          i++;

          if (i == 2 || i == 5)
          {
            lcd.print(":");
            i++;
          }

          if (i == 10 || i == 13)
          {
            lcd.print("/");
            i++;
          }

          // Its for year first two digit will be same "20**"
          if (i == 14)
          {
            lcd.print("20");
            i = 16;
          }

          value = 0;
        }

        delay(20);
      }

      lcd.println("\n-----------------");

      // State is true because dont works again this function
      send_time_state = true;

      // Assign values to variables.
      hour = (input_time[0] * 10) + input_time[1];
      minute = (input_time[3] * 10) + input_time[4];
      sec = (input_time[6] * 10) + input_time[7];
      day = (input_time[8] * 10) + input_time[9];
      month = (input_time[11] * 10) + input_time[12];
      year = (2 * 1000) + (0 * 100) + (input_time[14] * 10) + input_time[15];

      // Set time.
      // sec / min / hour / day / month / year
      rtc.setTime(sec, minute, hour, day, month, year);

      sc == false;
      delay(20);
      lcd.clear();
    }
    else if (currentMode == 0)
    {
      lcd.println("ok");
      // end of the config of time, activete mode selection again.
      mode_state = false;
      sc == false;
      Task_Menu();
    }

    delay(20);

    // Display time
    if (send_time_state == true && mode == 1 && currentMode == 1)
    {
      // Print time.
      lcd.setCursor(0, 0);
      lcd.print(rtc.getTime("%A,%B%d%Y"));
      lcd.setCursor(5, 1);
      lcd.print(rtc.getTime("%H:%M:%S"));
      struct tm timeinfo = rtc.getTimeStruct();
      delay(1000);
      lcd.clear();
    }
    delay(20);
  }

  delay(20);
}
// Task for pomodoro funtion. (Mode 2)
void Task_Pomodoro_Code(void *pvParameters)
{

  // count back from that value.
  int count = 5;

  // value of time.
  int minute, last_second = 0;

  // State variable for configure one time.
  bool pom_state = false;

  // Variable for each digit.
  int one_dig;
  int sec_dig;

  while (1)
  {
    if (mode == 2 && mode_state == true && sc == true && currentMode == 2)
    {
      lcd.clear();
      // To config funtion work.
      pom_state = false;

      count = 5;

      while (pom_state == false)
      {
        // When INCREMENT button is on function works.
        if (encoder.pulse_pos != lastStep)
        {
          // every push to increment that increase count.
          count += 5;

          // Max count value is 60.
          if (count > 60)
          {
            count == 5;
          }

          // To get time for each digit.
          sec_dig = count / 10;
          one_dig = count % 10;
          lcd.setCursor(0, 0);
          lcd.print(sec_dig);
          lcd.print(one_dig);

          delay(100);
          lastStep = encoder.pulse_pos;
        }
        // When ENCODER_BTN button is on function works.
        if (digitalRead(ENCODER_BTN) == HIGH)
        {
          // Waits that until hand off the ENCODER_BTN button.
          while (digitalRead(ENCODER_BTN) == HIGH)
            ;
          // Assign count to minute variable.
          minute = count;

          lcd.print(sec_dig);
          lcd.print(one_dig);

          pom_state = true;
          delay(20);
        }
      }

      // set time after config.
      minute = minute - 1;
      second = 59;

      delay(20);
    }

    // if second is decrease that works.
    if (second != last_second && (currentMode == 1 || currentMode == 2) && mode == 2)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      // if second goes to -1 that equals 59 second.
      if (second == -1)
      {
        minute--;
        second = 59;
      }
      last_second = second;

      delay(20);

      if (pom_state == true)
      {
        // It is because look better to with 0s.
        if (minute < 10)
        {
          lcd.print("0");
        }
        lcd.print(minute);
        lcd.print(":");

        if (second < 10)
        {
          lcd.print("0");
        }
        lcd.println(second);
        delay(10);
      }
      if (currentMode == 0)
      {
        Serial.println("ok");
        // end of the config of time, activete mode selection again.
        mode_state = false;

        sc == false;
      }
    }
    delay(20);
  }
  delay(20);
}

// Task for send_text funtion. (Mode 3)
void Task_Text_Code(void *pvParameters)
{
  // Input from user.
  char input_message[20];

  // Variable to print text.
  bool state = false;

  // Digit of array.
  int digit = 0;

  // Letters type of int. (65-90)
  int value = 65;

  // int to char.
  char buffer[1];

  while (1)
  {
    if (mode == 3 && mode_state == true && sc == true && currentMode == 2)
    {
      lcd.clear();
      //  Letter digit.
      digit = 0;
      int pos = 0;

      // Clear all array.
      for (int i = 0; i < 20; i++)
      {
        input_message[i] = '\0';
        delay(1);
      }

      // Function starts with A letter.
      lcd.setCursor(0,0);
      lcd.print('A');

      while (1)
      {
        //  Every push change the letter.
        if (encoder.pulse_pos != lastStep)
        {

          value++;
          if (value == 91)
          {
            value = 64;
          }

          Serial.print('\b');

          // Convert int to char.
          sprintf(buffer, "%c", value);
          lcd.setCursor(pos,0);
          lcd.print(buffer);

          lastStep = encoder.pulse_pos;
          delay(10);
        }

        if (digitalRead(ENCODER_BTN) == HIGH)
        {
          while (digitalRead(ENCODER_BTN) == HIGH)
            ;
          //int i = 0;
          //lcd.setCursor(i, 0);
          if (digit <= 19)
          {

            // Print array.
            input_message[digit] = value;

            if (input_message[digit] == 64)
            {
              // To next time read null.
              input_message[digit] == 'A';
              break;
            }

            lcd.print('A');
            digit++;

            value = 65;
            pos++;
          }

          delay(10);
        }
      }
      delay(10);

      Serial.print("\n");

      if (currentMode == 0)
      {
        Serial.println("ok");
        mode_state = false;
        sc == false;
      }
      state = true;

      delay(10);
    }
    delay(20);

    if (state == true && currentMode == 1 && mode == 3)
    {
      for (int i = 0; i < 20; i++)
      {
        if (input_message[i] == 64)
        {
          break;
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(input_message[i]);
        delay(10);
      }
      Serial.print("\n");

      state = false;
      delay(20);
    }
    delay(20);
  }
}

//---------------------------SETUP--------------------------------------------//
void setup()
{
  Serial.begin(115200);
  encoder.clk_pin = ENCODER_CLK;
  encoder.dt_pin = ENCODER_DT;
  pinMode(encoder.clk_pin, INPUT);
  pinMode(encoder.dt_pin, INPUT);
  button_add_default(&button, ENCODER_BTN);
  button_init(&button_isr);

  attachInterrupt(digitalPinToInterrupt(encoder.clk_pin), ISR, CHANGE);
  lcd.begin(16, 2);
  My_timer2 = timerBegin(1, 80, true);

  // Attach it to an ISR
  timerAttachInterrupt(My_timer2, &onTimer2, true);

  // 1000000us is 1 second
  // third variable: interrupt generated periodically.
  timerAlarmWrite(My_timer2, 1000000, true);

  // enable timer.
  timerAlarmEnable(My_timer2);

  // Task for rtc_clock function.
  xTaskCreatePinnedToCore(
      Task_RTC_Code, /* Task function. */
      "Task_RTC",    /* name of task. */
      10000,         /* Stack size of task */
      NULL,          /* parameter of the task */
      1,             /* priority of the task */
      &Task_RTC,     /* Task handle to keep track of created task */
      1);            /* pin task to core 1 */
  delay(500);

  // Task for pomodoro function.
  xTaskCreatePinnedToCore(
      Task_Pomodoro_Code, /* Task function. */
      "Task_Pomodoro",    /* name of task. */
      10000,              /* Stack size of task */
      NULL,               /* parameter of the task */
      1,                  /* priority of the task */
      &Task_Pomodoro,     /* Task handle to keep track of created task */
      1);                 /* pin task to core 1 */
  delay(500);

  // Task for send_text function.
  xTaskCreatePinnedToCore(
      Task_Text_Code, /* Task function. */
      "Task_Text",    /* name of task. */
      10000,          /* Stack size of task */
      NULL,           /* parameter of the task */
      1,              /* priority of the task */
      &Task_Text,     /* Task handle to keep track of created task */
      1);             /* pin task to core 1 */
  delay(500);
}

void clicked()
{
  if (button.mode == CLICKED)
  {
    Serial.println("Clicked !");
    click = true;
    button.mode = NONE;
  }
}
void double_clicked()
{
  if (button.mode == DOUBLE_CLICKED)
  {
    Serial.println("Double Clicked !");
    sc = true;
    click = false;
    currentMode++;

    button.mode = NONE;
  }
}
void long_pressed()
{
  if (button.mode == LONG_PRESSED)
  {
    Serial.println("Long Pressed !");
    sc = false;
    click = false;
    currentMode--;
    button.mode = NONE;
  }
}

void loop()
{
  while (1)
  {
    delay(20);
  }
}
