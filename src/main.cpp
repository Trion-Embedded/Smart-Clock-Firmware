/* Differences -> It will change the increasing time value with a button instead of entering from the keyboard.  */

// Include libraries.
#include <Arduino.h>
// Library for rtc_clock.
#include <ESP32Time.h>
#include <stdio.h>

// Define buttons.
#define INCREMENT 33
#define BUTTON_STATE 32

/* -------------------------- */

/* Definitions for menu function. */

// Display which mode before choose
int pre_mode = 0;

// Choose which function that you want
int mode = 0;

// State of mode.
bool mode_state = false;

/* -------------------------- */

/* Definitons for clock funtion. */

// Value of minute.
int minute = 0;

/* -------------------------- */

/* Definitons for pomodoro funtion. */

// Value of second.
int second = 60;

/* -------------------------- */

// Offset in seconds GMT. (For rtc_clock funtion)
ESP32Time rtc(0);

// Timer for clock function.
hw_timer_t *My_timer = NULL;

// Timer for pomodoro function.
hw_timer_t *My_timer2 = NULL;

// Interrupt for clock function.
void IRAM_ATTR onTimer()
{
  // Increase minute variable every minute.
  minute++;
}

// Interrupt for pomodor function.
void IRAM_ATTR onTimer2()
{
  // Increase second variable every second.
  second--;
}

// Task for menu function.
TaskHandle_t Task_Menu;

// Task for clock function. (Mode 1)
TaskHandle_t Task_Clock;

// Task for rtc_clock function. (Mode 2)
TaskHandle_t Task_RTC;

// Task for pomodoro function. (Mode 3)
TaskHandle_t Task_Pomodoro;

// Task for send_text function. (Mode 4)
TaskHandle_t Task_Text;

// Task for menu funtion.
void Task_Menu_Code(void *pvParameters)
{
  while (1)
  {
    // if mode_state false this funtion will work.
    while (mode_state == false)
    {
      // When INCREMENT button is on function works.
      if (digitalRead(INCREMENT) == HIGH)
      {
        // Waits that until hand off the INCREMENT button.
        while (digitalRead(INCREMENT) == HIGH)
          ;

        pre_mode++;

        // when pre_mode pass 4, turn to 1. Because there are 4 mods.
        if (pre_mode == 5)
        {
          pre_mode = 1;
        }

        Serial.print("pre_mode: ");
        Serial.println(pre_mode);

        delay(20);
      }

      // When BUTTON_STATE is on function works.
      if (digitalRead(BUTTON_STATE) == HIGH)
      {
        // Waits that until hand off the BUTTON_STATE button.
        while (digitalRead(BUTTON_STATE) == HIGH)
          ;

        // Assign pre_mode to mode to get in that mode.
        mode = pre_mode;

        Serial.println("-----------------");
        Serial.print("mode: ");
        Serial.println(mode);
        Serial.println("-----------------");

        // State is true because dont works again this function.
        mode_state = true;

        delay(20);
      }
      delay(20);
    }
    delay(20);
  }
}

// Task for clock function. (Mode 1)
void Task_Clock_Code(void *pvParameters)
{
  // Create a place to hold incoming messages.
  int incoming;

  // Value of last_minute.
  int last_minute = 0;

  // Value of hour.
  int hour = 0;

  // Input time from user.
  int input_time[4];

  // For one time config
  bool get_time_state = false;
  bool send_time_state = false;

  // it is value of the time that each digit.
  int value = 0;

  // Digit value.
  int i = 0;

  while (1)
  {
    if (mode == 1)
    {
      // when mode equals 1 that activate set time functons.
      get_time_state = false;
      send_time_state = false;

      // Assign digit value to 0, to every start on funtion .
      i = 0;

      while (get_time_state == false)
      {
        // Display time before config with 0s.
        Serial.print("0");
        Serial.print("0");
        Serial.print(":");
        Serial.print("0");
        Serial.print("0");

        // Backspace to first value of time.
        Serial.print('\b');
        Serial.print('\b');
        Serial.print('\b');
        Serial.print('\b');
        Serial.print('\b');

        while (i < 4)
        {
          // Increment button is High.
          if (digitalRead(INCREMENT) == HIGH)
          {
            // Wait for push back from button.
            while (digitalRead(INCREMENT) == HIGH)
              ;
            delay(20);

            // Increase time value for each digit.
            value++;

            // max value of time digit is 9.
            if (value == 10)
            {
              value = 0;
            }

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
            else if (i == 2 && value == 6)
            {
              value = 0;
            }

            Serial.print(value);

            // backspace.
            Serial.print('\b');
          }

          delay(20);

          // Confirm the time value.
          if (digitalRead(BUTTON_STATE) == HIGH)
          {
            // Wait for push back from button.
            while (digitalRead(BUTTON_STATE) == HIGH)
              ;
            // Assign time value to array.
            input_time[i] = value;
            i++;

            Serial.print(value);

            // Its for looks good.
            if (i == 2)
            {
              Serial.print(":");
            }
            value = 0;
          }

          delay(20);
        }
        Serial.println("\n-----------------");

        delay(100);

        // State is true because dont works again this function.
        get_time_state = true;

        // End of while.
        break;
      }

      while (send_time_state == false)
      {
        // Assign time values to variables.
        hour = input_time[0] * 10 + input_time[1];
        minute = input_time[2] * 10 + input_time[3];

        // State is true because dont works again this function.
        send_time_state = true;

        delay(10);
      }

      // to mode 0.
      mode = 0;

      // end of the config of time, activete mode selection again.
      mode_state = false;
    }

    // Works after every one second.
    if (last_minute != minute)
    {
      // End of the 60 minutes, minute turns to 0.
      if (minute == 60)
      {
        minute = 0;

        // Increase one hour.
        hour += 1;

        // End of the 24 hours, hour turns to 0.
        if (hour == 24)
        {
          hour = 0;
        }
      }

      last_minute = minute;

      // if works when premode is 1
      if (pre_mode == 1 && send_time_state == true)
      {
        // if hour smaller than 10, print a 0.
        if (hour < 10)
        {
          Serial.print('0');
        }
        Serial.print(hour);

        Serial.print(":");

        // if minute smaller than 10, print a 0.
        if (minute < 10)
        {
          Serial.print('0');
        }
        Serial.println(last_minute);
      }
    }
    delay(20);
  }
}

// Task for rtc_clock function. (Mode 2)
void Task_RTC_Code(void *pvParameters)
{
  // Create a place to hold incoming messages.
  int incoming;

  // To set time again.
  int read_break;

  // Time definitions.
  int sec, minute, hour, day, month, year;

  // State is true because dont works again this function.
  bool send_time_state = false;

  // Input time from user.
  int input_time[14];

  // time value.
  int value = 0;

  // Step of time.
  int i = 0;

  while (1)
  {
    if (mode == 2)
    {
      // Step of time.
      i = 0;

      // to config time funtion.
      send_time_state = false;

      // Display time before config with 0s.
      Serial.print("0");
      Serial.print("0");
      Serial.print(":");
      Serial.print("0");
      Serial.print("0");
      Serial.print(":");
      Serial.print("0");
      Serial.print("0");

      // Backspace to first value of time.
      Serial.print('\b');
      Serial.print('\b');
      Serial.print('\b');
      Serial.print('\b');
      Serial.print('\b');
      Serial.print('\b');
      Serial.print('\b');
      Serial.print('\b');

      // there are 14 digit to input.
      while (i < 14)
      {
        // Increase the time value.
        if (digitalRead(INCREMENT) == HIGH)
        {
          // Wait push back from button.
          while (digitalRead(INCREMENT) == HIGH)
            ;
          delay(20);

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
          else if (i == 2 && value == 6)
          {
            value = 0;
          }
          // Fifth digit of time max may be 5.
          else if (i == 4 && value == 6)
          {
            value = 0;
          }

          // Date config.
          else if (i == 6 && value == 4)
          {
            value = 0;
          }
          else if (i == 7 && input_time[6] == 3)
          {
            value = 0;
          }
          else if (i == 8 && value == 2)
          {
            value = 0;
          }
          else if (i == 9 && input_time[8] == 1 && value == 3)
          {
            value = 0;
          }

          Serial.print(value);
          Serial.print('\b');
        }

        delay(20);

        if (digitalRead(BUTTON_STATE) == HIGH)
        {
          while (digitalRead(BUTTON_STATE) == HIGH)
            ;

          input_time[i] = value;

          Serial.print(value);

          i++;

          if (i == 2 || i == 4)
          {
            Serial.print(":");
          }
          if (i == 6)
          {
            Serial.print("\n");

            // Display time before config with 0s.
            Serial.print("0");
            Serial.print("0");
            Serial.print("/");
            Serial.print("0");
            Serial.print("0");
            Serial.print("/");
            Serial.print("0");
            Serial.print("0");
            Serial.print("0");
            Serial.print("0");

            // Backspace to first value of time.
            Serial.print('\b');
            Serial.print('\b');
            Serial.print('\b');
            Serial.print('\b');
            Serial.print('\b');
            Serial.print('\b');
            Serial.print('\b');
            Serial.print('\b');
            Serial.print('\b');
            Serial.print('\b');
          }

          if (i == 8 || i == 10)
          {
            Serial.print("/");
          }

          // Its for year first two digit will be same "20**"
          if (i == 10)
          {
            Serial.print("20");
            i = 12;
          }

          value = 0;
        }

        delay(20);
      }

      Serial.println("\n-----------------");

      // State is true because dont works again this function
      send_time_state = true;

      // Assign values to variables.
      hour = (input_time[0] * 10) + input_time[1];
      minute = (input_time[2] * 10) + input_time[3];
      sec = (input_time[4] * 10) + input_time[5];
      day = (input_time[6] * 10) + input_time[7];
      month = (input_time[8] * 10) + input_time[9];
      // year = (input_time[10] * 1000) + (input_time[11] * 100) + (input_time[12] * 10) + input_time[13];
      year = (2 * 1000) + (0 * 100) + (input_time[12] * 10) + input_time[13];

      // Set time.
      // sec / min / hour / day / month / year
      rtc.setTime(sec, minute, hour, day, month, year);

      // to mode 0.
      mode = 0;

      // end of the config of time, activete mode selection again.
      mode_state = false;

      delay(20);
    }

    delay(20);

    // Display time when no configrating.
    if (send_time_state == true && pre_mode == 2)
    {
      // Print time.
      Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));
      struct tm timeinfo = rtc.getTimeStruct();

      delay(1000);
    }
    delay(20);
  }
  delay(20);
}

// Task for pomodoro function. (Mode 3)
void Task_Pomodoro_Code(void *pvParameters)
{
  // task watchdog got triggered. the following tasks did not reset the watchdog in time ??

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
    if (mode == 3)
    {
      // To config funtion work.
      pom_state = false;

      count = 5;

      Serial.print("0");
      Serial.print("5");
      Serial.print(":");
      Serial.print("0");
      Serial.print("0");

      Serial.print("\b");
      Serial.print("\b");
      Serial.print("\b");
      Serial.print("\b");
      Serial.print("\b");

      while (pom_state == false)
      {
        // When INCREMENT button is on function works.
        if (digitalRead(INCREMENT) == HIGH)
        {
          // Waits that until hand off the INCREMENT button.
          while (digitalRead(INCREMENT) == HIGH)
            ;
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

          Serial.print(sec_dig);
          Serial.print(one_dig);
          Serial.print("\b");
          Serial.print("\b");

          delay(100);
        }
        // When BUTTON_STATE button is on function works.
        if (digitalRead(BUTTON_STATE) == HIGH)
        {
          // Waits that until hand off the BUTTON_STATE button.
          while (digitalRead(BUTTON_STATE) == HIGH)
            ;
          // Assign count to minute variable.
          minute = count;

          Serial.print(sec_dig);
          Serial.print(one_dig);
          Serial.println("\n-----------------");

          pom_state = true;
          delay(20);
        }
      }

      // set time after config.
      minute = minute - 1;
      second = 59;

      // to mode 0.
      mode = 0;

      // end of the config of time, activete mode selection again.
      mode_state = false;

      delay(20);
    }

    // if second is decrease that works.
    if (second != last_second)
    {
      // if second goes to -1 that equals 59 second.
      if (second == -1)
      {
        minute--;
        second = 59;
      }
      last_second = second;

      delay(20);

      if (pom_state == true && pre_mode == 3)
      {
        // It is because look better to with 0s.
        if (minute < 10)
        {
          Serial.print("0");
        }
        Serial.print(minute);
        Serial.print(":");

        if (second < 10)
        {
          Serial.print("0");
        }
        Serial.println(second);
        delay(10);
      }
    }
    delay(20);
  }
  delay(20);
}

// Task for send_text function. (Mode 4) // Using keyboard input.
void Task_Text_Code2(void *pvParameters) // It will be developed.
{
  char input_message[50];
  bool state = false;
  char incoming;

  bool display_state = false;

  while (1)
  {
    if (mode == 4)
    {
      state = false;

      // Input from user and works one time.
      while (Serial.available() > 0 && state == false)
      {

        /* *********************** */
        // Read character.
        for (int i = 0;;) // Read 50 character -> ; i<50 ;.
        {
          // Assign input to variable
          incoming = Serial.read();

          // Assign only small letters and numbers to array.
          if (((incoming >= 97 && incoming <= 122) || (incoming >= 48 && incoming <= 58) || incoming == ' ') && (incoming != '\0'))
          {
            // Assign variable to array.
            input_message[i] = incoming;

            if (i <= 49)
            {
              // Print array.
              Serial.print(input_message[i]);
              i++;
            }

            delay(10);
          }

          // When input backspace
          else if (incoming == '\b')
          {
            i--;

            if (i < 0)
            {
              i = 0;
            }

            // Assign backspace to array.
            input_message[i] = incoming;

            // Print backspace.
            Serial.print(input_message[i]);

            delay(10);
          }
        }
        delay(10);

        state = true;

        // to mode 0.
        mode = 0;

        // end of the config of time, activete mode selection again.
        mode_state = false;

        delay(10);
      }
      delay(20);
    }

    if (display_state == false)
    {
      if (state == true && pre_mode == 4)
      {
        for (int i = 0; i < 20; i++)
        {
          if (input_message[i] == '\0')
          {
            break;
          }
          Serial.print(input_message[i]);
          delay(10);
        }

        display_state = true;
      }
    }

    if (pre_mode != 4)
    {
      display_state = false;
    }

    delay(20);
  }
}

// Task for send_text function. (Mode 4) // Using button for letters.
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
    if (mode == 4)
    {
      // Letter digit.
      digit = 0;

      // Clear all array.
      for (int i = 0; i < 20; i++)
      {
        input_message[i] = '\0';
        delay(1);
      }

      // Function starts with A letter.
      Serial.print('A');

      while (1)
      {
        // Every push change the letter.
        if (digitalRead(INCREMENT) == HIGH)
        {
          while (digitalRead(INCREMENT) == HIGH)
            ;

          value++;

          if (value == 91)
          {
            value = 64;
          }

          // Backspace
          Serial.print('\b');

          // Convert int to char.
          sprintf(buffer, "%c", value);

          Serial.print(buffer);

          delay(10);
        }

        if (digitalRead(BUTTON_STATE) == HIGH)
        {
          while (digitalRead(BUTTON_STATE) == HIGH)
            ;
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

            // It is new digit starts.
            Serial.print('A');
            digit++;

            value = 65;
          }
          delay(10);
        }
      }
      delay(10);

      Serial.print("\n");

      // to mode 0.
      mode = 0;

      // end of the config of time, activete mode selection again.
      mode_state = false;

      state = true;

      delay(10);
    }
    delay(20);

    if (pre_mode == 4 && state == true)
    {
      for (int i = 0; i < 20; i++)
      {
        // HOWWWW????
        if (input_message[i] == 64)
        {
          break;
        }
        Serial.print(input_message[i]);
        delay(10);
      }
      Serial.print("\n");

      state = false;
      delay(20);
    }

    if (pre_mode != 4)
    {
      state = true;
    }
    delay(20);
  }
}

void setup()
{
  // Serial comm begin.
  Serial.begin(115200);

  pinMode(INCREMENT, INPUT);
  pinMode(BUTTON_STATE, INPUT);

  //  minute = 0;

  /* begin of timer settings for clock and pomodoro function. */

  // First variable: number of the timer.(from 0 to 3)
  // Second variable: prescaler.
  // Third variable: count up.(true)
  My_timer = timerBegin(1, 80, true);
  My_timer2 = timerBegin(0, 80, true);

  // Attach it to an ISR
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAttachInterrupt(My_timer2, &onTimer2, true);

  // 1000000us is 1 second
  // third variable: interrupt generated periodically.
  timerAlarmWrite(My_timer, 1000000, true);
  timerAlarmWrite(My_timer2, 1000000, true);

  // enable timer.
  timerAlarmEnable(My_timer);
  timerAlarmEnable(My_timer2);

  /* End of timer settings for clock and pomodoro function. */

  // Task for menu function.
  // create a task that will be executed in the Task_Menucode() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Task_Menu_Code, /* Task function. */
      "Task_Menu",    /* name of task. */
      10000,          /* Stack size of task */
      NULL,           /* parameter of the task */
      1,              /* priority of the task */
      &Task_Menu,     /* Task handle to keep track of created task */
      0);             /* pin task to core 0 */
  delay(500);

  // Task for clock function.
  // create a task that will be executed in the Task_Menucode() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Task_Clock_Code, /* Task function. */
      "Task_Clock",    /* name of task. */
      10000,           /* Stack size of task */
      NULL,            /* parameter of the task */
      1,               /* priority of the task */
      &Task_Clock,     /* Task handle to keep track of created task */
      1);              /* pin task to core 1 */
  delay(500);

  // Task for rtc_clock function.
  // create a task that will be executed in the Task_Menucode() function, with priority 1 and executed on core 0
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
  // create a task that will be executed in the Task_Menucode() function, with priority 1 and executed on core 0
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
  // create a task that will be executed in the Task_Menucode() function, with priority 1 and executed on core 0
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

void loop()
{
  delay(20);
}
