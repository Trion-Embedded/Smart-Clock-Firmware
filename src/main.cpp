// Include libraries.
#include <Arduino.h>
// Library for rtc_clock.
#include <ESP32Time.h>

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

/* -------------------------- */
/* Definitons for clock funtion. */

// Value of minute.
int minute = 0;
/* -------------------------- */

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
TaskHandle_t Task1;

// Task for clock function. (Mode 1)
TaskHandle_t Task2;

// Task for rtc_clock function. (Mode 2)
TaskHandle_t Task3;

// Task for pomodoro function. (Mode 3)
TaskHandle_t Task4;

// Task for send_text function. (Mode 4)
TaskHandle_t Task5;

// Task for menu funtion.
void Task1code(void *pvParameters)
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

      // When BUTTON_STATE button is on function works.
      if (digitalRead(BUTTON_STATE) == HIGH)
      {
        // Waits that until hand off the BUTTON_STATE button.
        while (digitalRead(BUTTON_STATE) == HIGH)
          ;

        // Assign pre_mode to mode to get in that mode.
        mode = pre_mode;

        Serial.print("mode: ");
        Serial.println(mode);

        // State is true because dont works again this function.
        mode_state = true;

        delay(10);
      }
    }
    //  mode_state = false;
    delay(100);
  }
}

// Task for clock function. (Mode 1)
void Task2code(void *pvParameters)
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

    Serial.print("Task 2: ");

  while (1)
  {
    if (mode == 1)
    {
      while (get_time_state == false)
      {
        // Input from user.
        while (Serial.available() > 0)
        {
          for (int i = 0; i < 4;)
          {
            // Read from input and assign to a variable.
            incoming = Serial.read();

            // If input is number that works.
            if (incoming >= 48 && incoming <= 58)
            {
              // Assign these input value to array.
              input_time[i] = incoming - 48;
              i++;
            }
          }
          Serial.println("\n");

          for (int i = 0; i < 4; i++)
          {
            // Print time.
            Serial.print(input_time[i]);
            delay(10);
          }
          Serial.println("\n");
          delay(100);

          // State is true because dont works again this function.
          get_time_state = true;

          // End of while.
          break;
        }
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
  delay(20);
}

// Task for rtc_clock function. (Mode 2)
void Task3code(void *pvParameters)
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

  while (1)
  {
    if (mode == 2)
    {
      // Input from user.
      while (Serial.available() > 0)
      {
        for (int i = 0; i < 14;)
        {
          // Read from input and assign to a variable.
          incoming = Serial.read();

          // If input is number that works.
          if (incoming >= 48 && incoming <= 58)
          {
            // Assign these input value to array.
            input_time[i] = incoming - 48;
            i++;
          }
        }
        // State is true because dont works again this function
        send_time_state = true;

        // Assign values to variables.
        hour = (input_time[0] * 10) + input_time[1];
        minute = (input_time[2] * 10) + input_time[3];
        sec = (input_time[4] * 10) + input_time[5];
        day = (input_time[6] * 10) + input_time[7];
        month = (input_time[8] * 10) + input_time[9];
        year = (input_time[10] * 1000) + (input_time[11] * 100) + (input_time[12] * 10) + input_time[13];

        // Set time.
        // sec / min / hour / day / month / year
        rtc.setTime(sec, minute, hour, day, month, year);

        delay(10);
        break;
      }

      if (send_time_state == true)
      {
        // Print time.
        Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));
        struct tm timeinfo = rtc.getTimeStruct();

        /*
        while (Serial.available() > 0)
        {
          // Read from input and assign to a variable.
          read_break = Serial.read();

          // If input enter, wait to get time again from input.
          if (read_break == 10)
          {
            send_time_state = false;
            break;
          }
          delay(20);
        }
        */

        delay(1000);
      }
      delay(20);
    }
    delay(20);
  }
  delay(20);
}

// Task for pomodoro funtion. (Mode 3)
void Task4code(void *pvParameters)
{
  int count = 0;

  int minute, last_second = 0;

  bool pom_state = false;

  while (1)
  {
    if (mode == 3)
    {
      while (pom_state == false)
      {
        if (digitalRead(INCREMENT) == HIGH)
        {
          while (digitalRead(INCREMENT) == HIGH)
            ;
          count += 5;
          delay(20);
        }
        if (digitalRead(BUTTON_STATE) == HIGH)
        {
          while (digitalRead(BUTTON_STATE) == HIGH)
            ;
          minute = count;

          pom_state = true;
          delay(20);
        }
      }

      minute = minute - 1;
      second = 59;

      delay(100);

      while (pom_state == true)
      {
        if (second != last_second)
        {
          if (second == -1)
          {
            minute--;
            second = 59;
          }

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
        last_second = second;
        delay(20);
      }
      delay(20);
    }
    delay(20);
  }
}

// Task for send_text funtion. (Mode 4)
void Task5code(void *pvParameters)
{
  char input_message[20];
  bool state = false;
  char incoming;

  while (1)
  {
    if (mode == 4)
    {
      // Input from user and works one time.
      while (Serial.available() > 0 && state == false)
      {
        for (int i = 0; i < 50;)
        {
          // Read input and assign to variable.
          incoming = Serial.read();

          if ((incoming >= 97 && incoming <= 122 && incoming != '\0') || (incoming >= 48 && incoming <= 58 && incoming != '\0')) // 97 122
          {
            // Assign input value to array.
            input_message[i] = incoming;

            Serial.print(input_message[i]);
            i++;
            delay(10);
          }
        }
        state = true;
        delay(10);
      }
      delay(20);
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
  // create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Task1code, /* Task function. */
      "Task1",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task1,    /* Task handle to keep track of created task */
      0);        /* pin task to core 0 */
  delay(500);

  // Task for clock function.
  // create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Task2code, /* Task function. */
      "Task2",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task2,    /* Task handle to keep track of created task */
      1);        /* pin task to core 1 */
  delay(500);

  // Task for rtc_clock function.
  // create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Task3code, /* Task function. */
      "Task3",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task3,    /* Task handle to keep track of created task */
      1);        /* pin task to core 1 */
  delay(500);

  // Task for pomodoro function.
  // create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Task4code, /* Task function. */
      "Task4",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task4,    /* Task handle to keep track of created task */
      1);        /* pin task to core 1 */
  delay(500);

  // Task for send_text function.
  // create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Task5code, /* Task function. */
      "Task5",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task5,    /* Task handle to keep track of created task */
      1);        /* pin task to core 1 */
  delay(500);
}

void loop()
{
  delay(20);
}
