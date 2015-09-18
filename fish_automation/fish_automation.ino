//importing the libraries

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>
#include <avr/wdt.h>
#include "DHT.h"


//Initialziing the temperature probe

#define ONE_WIRE_BUS 2 /*-(Connect to Pin 2 )-*/
OneWire ourWire(ONE_WIRE_BUS);
DallasTemperature sensors(&ourWire);

//Initializing the LCD

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

//Initializing time module

DS3231 Clock;
bool Century = false;
bool h12;
bool PM;
byte ADay, AHour, AMinute, ASecond, ABits;
bool ADy, A12h, Apm;

byte year, month, date, DoW, hour, minute, second;

//Initializing external temperature/humitity sensor

#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

////////////////////////Declaring global constants//////////////////////////////////
int waterchangeday = 30;
int filterchange = 45;
int meals_left = 3;
int days = 0;
int start_seconds = 0; 
int start_minutes = 0; 
int start_hours = 0;
int start_days = 0; 
int start_months = 0;
int start_years = 0;
int months[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; //leap year calculated later on...
boolean buzzer_on = true;
boolean warning = true;
boolean units_of_temperature = true; // true means Fahrenheit, false means Celcius
boolean pump_on = true;

int TEMPERATURE_MAX = 85;
int TEMPERATURE_MIN = 70;
int MENU_EXIT_TIME = 70; //around 70 SECONDS per menu
int MENU_TRANSITION_TIME = 300; //1 second or 1000 milliseconds
int CHANGE_MENU_TIME = 600; //2 seconds

int pump_relay = 0;
int nightlightpin = 3;
int pump_button = 4; 
int menu_button = 6;
int buzzer_button = 7;
int buzzerpin = 8; //using pin 8 for buzzer
int plus_button = 9;
int minus_button = 10;
int select_button = 11;
int confirm_button = 12;

void setup() {
  /*if running the function temperature check besure to uncomment
  the following code below, but comment out the comment */
  /*
  Serial.begin(9600);
  Serial.begin(115200);
  */
  pinMode(buzzerpin, OUTPUT);
  pinMode(menu_button, INPUT); //button for displaying menu
  pinMode(buzzer_button, INPUT); // turning buzzer on/off
  pinMode(select_button, INPUT); // select button for menu
  pinMode(plus_button, INPUT); // + button
  pinMode(minus_button, INPUT); // - button
  pinMode(confirm_button, INPUT);
  pinMode(pump_relay, OUTPUT);
  pinMode(pump_button, INPUT);
  pinMode(nightlightpin, OUTPUT);
  pinMode(A1, INPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  digitalWrite(pump_relay, LOW);
  noTone(buzzerpin);
  Wire.begin();
  sensors.begin();
  dht.begin();
  lcd.begin(20,4); //initialzing the LCD screen area(20 characters by 4 lines)
  startup();
  set_last_reset_time();
}

void first_screen (float temperature, int days_left_to_change_water, int days_left_to_change_filter, float external_temperature, float humidity_sensor) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Internal Temp: ");
  lcd.print(temperature);
  lcd.setCursor(0,1);
  lcd.print("External Temp: ");
  lcd.print(external_temperature);
  lcd.setCursor(0,2);
  lcd.print("Meals Left: ");
  lcd.setCursor(12, 2);
  lcd.print(meals_left);
  lcd.setCursor(0,3);
  lcd.print("Water Change: ");
  lcd.print(days_left_to_change_water);
  lcd.print("days");
  delay(3000);
  lcd.setCursor(0,2);
  lcd.print("                    ");
  delay(1000);
  lcd.setCursor(0,2);
  lcd.print("Humidity: ");
  lcd.print(humidity_sensor);
  lcd.print("%");
  delay(1000);
  lcd.setCursor(0,3);
  lcd.print("                    ");
  delay(1000);
  lcd.setCursor(0,3);
  lcd.print("Filter Change:");
  lcd.print(days_left_to_change_filter);
  lcd.print("days");
}

void menu0() {
  lcd.clear(); 
  lcd.setCursor(0,0);
  lcd.print("Menu Option 0/10");
  lcd.setCursor(0,2);
  lcd.print("Water Changed?");
  delay(CHANGE_MENU_TIME);
  for (int i = 0; i < MENU_EXIT_TIME; i++) {
    if (digitalRead(plus_button) == HIGH) {
      menu();
      break;
    } else if (digitalRead(minus_button) == HIGH) {
      menu9();
      break;
    } else if (digitalRead(select_button) == HIGH) {
      lcd.clear();
      while (digitalRead(confirm_button) != HIGH) {
        lcd.setCursor(0,0);
        lcd.print("Water Done!");
        lcd.setCursor(0,1);
        lcd.print("Confirm to reset day");
      }
      set_last_reset_time();
      lcd.clear();
      lcd.setCursor(7,2);
      lcd.print("Updated!!!");
      delay(3000);
      menu0();
    }
    delay(MENU_TRANSITION_TIME);
  }
  return;
}

void menu() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Menu Option: 1/10");
  lcd.setCursor(0,2);
  lcd.print("Water Change Days");
  lcd.setCursor(0,3);
  lcd.print("Current: ");
  lcd.print(waterchangeday);
  delay(CHANGE_MENU_TIME);
  for (int i = 0; i < MENU_EXIT_TIME; i++) {
    if (digitalRead(plus_button) == HIGH) {
      menu1();
      break;
    } else if (digitalRead(minus_button) == HIGH) {
      menu0();
    } else if (digitalRead(select_button) == HIGH) {
      lcd.clear();
      int temp = waterchangeday;
      while (digitalRead(confirm_button) != HIGH) {
        lcd.setCursor(0,0);
        lcd.print("Water Change Days:");
        lcd.setCursor(9,2);
        lcd.print(temp);
        if (digitalRead(plus_button) == HIGH) {
          temp++;
          if (temp > 99) {
            temp = 99;
          }
          delay(100);
        } else if (digitalRead(minus_button) == HIGH) {
          temp--;
          if (temp < 10) {
            temp = 10;
          }
          delay(100);
        }
      }
      if (waterchangeday != temp) {
        waterchangeday = temp;
        lcd.clear();
        lcd.setCursor(6,1);
        lcd.print("Updated");
        delay(2000);
        menu();
      }
    } else if (digitalRead(confirm_button) == HIGH) {
        break;
    }
    delay(MENU_TRANSITION_TIME);
  }
  return;
}

void menu1() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Menu Option: 2/10");
  lcd.setCursor(0,2);
  lcd.print("Filter Change Days");
  lcd.setCursor(0,3);
  lcd.print("Current: ");
  lcd.print(filterchange);
  delay(CHANGE_MENU_TIME);
  for (int i = 0; i < MENU_EXIT_TIME; i++) {
    if (digitalRead(plus_button) == HIGH) {
      menu2();
      break;
    } else if (digitalRead(minus_button) == HIGH) {
      menu(); 
      break;
    } else if (digitalRead(select_button) == HIGH) {
        lcd.clear();
        int temp = filterchange;
        while (digitalRead(confirm_button) != HIGH) {
          lcd.setCursor(0,0);
          lcd.print("Filter Change Days:");
          lcd.setCursor(9,2);
          lcd.print(temp);
          if (digitalRead(plus_button) == HIGH) {
            temp++;
              if (temp > 99) {
                temp = 99;
              }
          } else if (digitalRead(minus_button) == HIGH) {
            temp--;
            if (temp < 10) {
              temp = 10;
            }
          }
          delay(100);
        }
        if (filterchange != temp) {
          filterchange = temp;
          lcd.clear();
          lcd.setCursor(6,1);
          lcd.print("Updated");
          delay(2000);
          menu1();
        }
    } else if (digitalRead(confirm_button) == HIGH) {
      return;
    }
  delay(MENU_TRANSITION_TIME);
  }
  return;
}

void menu2() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Menu Option: 3/10");
  lcd.setCursor(0,2);
  lcd.print("Last Reset Date/Time");
  delay(CHANGE_MENU_TIME);
  for (int i = 0; i < MENU_EXIT_TIME; i++) {
    if (digitalRead(plus_button) == HIGH) {
      menu3();
      break;
    } else if (digitalRead(minus_button) == HIGH) {
      menu1();
      break;
    } else if (digitalRead(select_button) == HIGH) {
        lcd.clear();
        while (digitalRead(confirm_button) != HIGH) {
          show_original_time();
          delay(500);
        } 
        menu2();
    }
  delay(MENU_TRANSITION_TIME);
  } 
  return;
}

void menu3() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Menu Option: 4/10");
  lcd.setCursor(0,2);
  lcd.print("Max Temperature:");
  lcd.setCursor(0,3);
  lcd.print("Current: ");
  lcd.print(TEMPERATURE_MAX);
  lcd.print(" F");
  delay(CHANGE_MENU_TIME);
  for (int i = 0; i < MENU_EXIT_TIME; i++) {
    if (digitalRead(plus_button) == HIGH) {
      menu4();
      break;
    } else if (digitalRead(minus_button) == HIGH) {
      menu2();
      break;
    } else if (digitalRead(select_button) == HIGH) {
        lcd.clear();
        int temp = TEMPERATURE_MAX;
        while (digitalRead(confirm_button) != HIGH) {
        lcd.setCursor(0,0);
        lcd.print("Set Max Temp:");
        lcd.setCursor(9,2);
        lcd.print(temp);
        if (digitalRead(plus_button) == HIGH) {
        temp++;
          if (temp > 99) {
            temp = 99;
          }
        delay(100);
        } else if (digitalRead(minus_button) == HIGH) {
          temp--;
          if (temp < 10) {
            temp = 10;
          }
          delay(100);
        }
      }
      if (TEMPERATURE_MAX != temp) {
        TEMPERATURE_MAX = temp;
        lcd.clear();
        lcd.setCursor(6,1);
        lcd.print("Updated");
        delay(2000);
        menu3();    
      }
    } else if (digitalRead(confirm_button) == HIGH) {
        return;
    }
  delay(MENU_TRANSITION_TIME);
  }
  return;
}

void menu4(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Menu Option: 5/10");
  lcd.setCursor(0,2);
  lcd.print("Min Temperature:");
  lcd.setCursor(0,3);
  lcd.print("Current: ");
  lcd.print(TEMPERATURE_MIN);
  lcd.print(" F");
  delay(CHANGE_MENU_TIME);
  for (int i = 0; i < MENU_EXIT_TIME; i++) {
    if (digitalRead(plus_button) == HIGH) {
      menu5();
      break;
    } else if (digitalRead(minus_button) == HIGH) {
      menu3();
      break;
    } else if (digitalRead(select_button) == HIGH) {
        lcd.clear();
        int temp = TEMPERATURE_MIN;
        while (digitalRead(confirm_button) != HIGH) {
        lcd.setCursor(0,0);
        lcd.print("Set min temp:");
        lcd.setCursor(9,2);
        lcd.print(temp);
        if (digitalRead(plus_button) == HIGH) {
        temp++;
          if (temp > 99) {
            temp = 99;
          }
        delay(100);
        } else if (digitalRead(minus_button) == HIGH) {
          temp--;
          if (temp < 10) {
            temp = 10;
          }
          delay(100);
        }
      }
      if (TEMPERATURE_MIN != temp) {
        TEMPERATURE_MIN = temp;
        lcd.clear();
        lcd.setCursor(6,1);
        lcd.print("Updated");
        delay(2000);
        menu4();
      }
    } else if (digitalRead(confirm_button) == HIGH) {
        return;
    }
    delay(MENU_TRANSITION_TIME);
  }
  return;
}

void menu5() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Menu Option: 6/10");
  lcd.setCursor(0,2);
  lcd.print("Set Date");
  delay(CHANGE_MENU_TIME);
  for (int i = 0; i < MENU_EXIT_TIME; i++) {
    if (digitalRead(plus_button) == HIGH) {
      menu6();
      break;
    } else if (digitalRead(minus_button) == HIGH) {
      menu4();
      break;
    } else if (digitalRead(select_button) == HIGH) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Current Month:");
      lcd.setCursor(9,2);
      int month = 1;

      //updating the month first! 
      while (digitalRead(confirm_button) != HIGH) {
        lcd.setCursor(9,2);
        lcd.print(month);
        if (digitalRead(plus_button) == HIGH) {
          month++;
          if (month > 12) {
            month = 12;
          }
        } else if (digitalRead(minus_button) == HIGH) {
          month--;
          if (month < 1) {
            month = 1;
          }
        }
        delay(100);
      }
      if (month != Clock.getMonth(Century)) {
        lcd.clear();
        Clock.setMonth(month);
        lcd.setCursor(6,1);
        lcd.print("Updated");
        delay(2000);
      }

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Currrent Day:");
      lcd.setCursor(9,2);
      int day = 1;

      //updating the days
      while (digitalRead(confirm_button) != HIGH) {
        lcd.setCursor(9,2);
        lcd.print(day);
        if (digitalRead(plus_button) == HIGH) {
          day++;
          if (day > 31) {
            day = 31;
          }
        } else if (digitalRead(minus_button) == HIGH) {
          day--;
          if (day < 1) {
            day = 1;
          }
        }
        delay(100);
      }
      if (day != Clock.getDate()) {
        lcd.clear();
        Clock.setDate(day); 
        lcd.setCursor(6,1);
        lcd.print("Updated");
        delay(2000);
      }

      //updating the year
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Current Year:");
      lcd.setCursor(9,2);
      int year = 0;
      while (digitalRead(confirm_button) != HIGH) {
        lcd.setCursor(9,2);
        lcd.print(year);
        if (digitalRead(plus_button) == HIGH) {
          year++;
        } else if (digitalRead(minus_button) == HIGH) {
          year--;
        }
        delay(100);
      }
      if (year != Clock.getYear()) {
        lcd.clear();
        Clock.setYear(year);
        lcd.setCursor(6,1);
        lcd.print("Updated");
        delay(2000);
      }
      menu5();
    }
    delay(MENU_TRANSITION_TIME);
  }
  return;
}

void menu6() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Menu Option: 7/10");
  lcd.setCursor(0,2);
  lcd.print("Set Time");
  delay(CHANGE_MENU_TIME);
  for (int i = 0; i < MENU_EXIT_TIME; i++) {
    if (digitalRead(plus_button) == HIGH) {
      menu7();
      break;
    } else if (digitalRead(minus_button) == HIGH) {
      menu5();
      break;
    } else if (digitalRead(select_button) == HIGH) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Current hour:");
      lcd.setCursor(0,1);
      lcd.print("24 Hour Time");
      int hour = 0;
      while (digitalRead(confirm_button) != HIGH) {
        lcd.setCursor(6,3);
        lcd.print(hour);
        if (digitalRead(plus_button) == HIGH) {
          hour++;
          if (hour > 24) {
            hour = 24;
          }
        } else if (digitalRead(minus_button) == HIGH) {
          hour--;
          if (hour < 0) {
            hour = 0;
          }
        }
        delay(100);
      }

      delay(2000);
      //setting the module with the hour
      Clock.setHour(hour);

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Current minute:");
      int minute = 0;
      while (digitalRead(confirm_button) != HIGH) {
        lcd.setCursor(6,2);
        lcd.print(minute);
        if (digitalRead(plus_button) == HIGH) {
          minute++;
          if (minute > 59) {
            minute = 59;
          }
        } else if (digitalRead(minus_button) == HIGH) {
          minute--;
          if (minute < 0) {
            minute = 0;
          }
        }
        delay(100);
      }

      Clock.setMinute(minute);

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Current second:");
      lcd.setCursor(0,1);
      lcd.print("If you want random");
      lcd.setCursor(0,2);
      lcd.print("generated seconds");
      lcd.setCursor(0,3);
      lcd.print("Select 0!!!");
      delay(3000);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Current second:");
      int second = 0;
      while (digitalRead(confirm_button) == HIGH) {
        lcd.setCursor(6,2);
        lcd.print(second);
        if (digitalRead(plus_button) == HIGH) {
          second++;
          if (second > 59) {
            second = 59;
          }
        } else if (digitalRead(minus_button) == HIGH) {
          second--;
          if (second < 0) {
            second = 0;
          }
        }
        delay(100);
      }

      if (second == 0) {
        Clock.setSecond(random(1,60));
      } else {
        Clock.setSecond(second);
      }

      lcd.clear();
      lcd.setCursor(3,1);
      lcd.print("Updated Time");
      delay(2000);
      menu6();
    }
    delay(MENU_TRANSITION_TIME);
  }
  return;
}

void menu7() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Menu Option: 8/10");
  lcd.setCursor(0,2);
  lcd.print("Number of Meals");
  for (int i = 0; i < MENU_EXIT_TIME; i++) {
    delay(MENU_TRANSITION_TIME);
    if (digitalRead(plus_button) == HIGH) {
      menu8();
      break;
    } else if (digitalRead(minus_button) == HIGH) {
      menu6();
      break;
    } else if (digitalRead(select_button) == HIGH) {
        lcd.clear();
        int temp = meals_left;
        while (digitalRead(confirm_button) != HIGH) {
          lcd.setCursor(0,0);
          lcd.print("Set meals left:");
          lcd.setCursor(9,2);
          lcd.print(temp);
          if (digitalRead(plus_button) == HIGH) {
          temp++;
            if (temp > 99) {
              temp = 99;
            }
          delay(100);
          } else if (digitalRead(minus_button) == HIGH) {
            temp--;
            if (temp < 1) {
              temp = 1;
            }
            delay(100);
          }
        }
        if (meals_left != temp) {
          meals_left = temp;
          lcd.clear();
          lcd.setCursor(6,1);
          lcd.print("Updated");
          delay(2000);
          menu7();
        }
    } else if (digitalRead(confirm_button) == HIGH) {
        return;
    }
    delay(MENU_TRANSITION_TIME);
  }
  return;
}

void menu8() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Menu Option: 9/10");
  lcd.setCursor(0,2);
  lcd.print("Units");
  lcd.setCursor(0,3);
  lcd.print("Current: ");
  if (units_of_temperature) {
    lcd.print("Fahrenheit");
  } else {
    lcd.print("Celsius");
  }
  delay(MENU_TRANSITION_TIME);
  for (int i = 0; i < MENU_EXIT_TIME; i++) {
    if (digitalRead(plus_button) == HIGH) {
      menu9();
      break;
    } else if (digitalRead(minus_button) == HIGH) {
      menu7();
      break; 
    } else if (digitalRead(select_button) == HIGH) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Current Units:");
      lcd.setCursor(0,1);
      if (units_of_temperature) {
        lcd.print("Fahrenheit");
      } else {
        lcd.print("Celsius");
      }
      while (digitalRead(confirm_button) != HIGH) {
        lcd.setCursor(0,2);
        lcd.print("Change to: ");
        if (!units_of_temperature) {
          lcd.print("Fahrenheit");
        } else {
          lcd.print("Celsius");
        } 
        if (digitalRead(menu_button) == HIGH) {
          menu8();
          return;
        }  
      }
      units_of_temperature = !units_of_temperature;
      lcd.clear();
      lcd.setCursor(6,1);
      lcd.print("Updated");
      delay(2000);
      menu8();
    }
    delay(MENU_TRANSITION_TIME);
  }
  return;
}

void menu9() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Menu Option: 10/10");
  lcd.setCursor(0,2);
  lcd.print("Reset");
  delay(CHANGE_MENU_TIME);
  for (int i = 0; i < MENU_EXIT_TIME; i++) {
    if (digitalRead(plus_button) == HIGH) {
      menu();
      break;
    } else if (digitalRead(minus_button) == HIGH) {
      menu8();
      break;
    } else if (digitalRead(select_button) == HIGH) {
      lcd.clear();
      for (int i = 0; i < MENU_TRANSITION_TIME; i++) {
        lcd.setCursor(0,0);
        lcd.print("CONFIRM RESET??");
        lcd.setCursor(0,1);
        lcd.print("All DATA WILL");
        lcd.setCursor(0,2);
        lcd.print("BE LOST FOREVER!!!");
        if (digitalRead(confirm_button) == HIGH) {
          lcd.clear();
          delay(700);
          lcd.setCursor(0,0);
          lcd.print("ARUDINO RESETTING IN");
          wdt_enable(WDTO_8S);
          for (int i = 8; i >= 0; i--) {
            lcd.setCursor(6,2);
            lcd.print(i);
            lcd.print(" seconds");
            delay(1000);
          }
        }
        delay(MENU_TRANSITION_TIME);
      }
    }
    delay(MENU_TRANSITION_TIME);
  }
  return;
}


void buttonpressed() {
  if (digitalRead(menu_button) == HIGH) {
    lcd.clear();
     menu0();
  }
  if (digitalRead(buzzer_button) == HIGH) {
    while (digitalRead(buzzer_button) == HIGH) {
      lcd.clear();
      lcd.setCursor(3,1);
      if (buzzer_on) {
        lcd.print("Buzzer is OFF");
        digitalWrite(13, LOW);
      } else {
        lcd.print("Buzzer is ON");
        digitalWrite(13, HIGH); 
      }
      delay(1500);
    }
    buzzer_on = !buzzer_on; 
  }
  if (digitalRead(pump_button) == HIGH) {
    while (digitalRead(pump_button) == HIGH) {
      lcd.clear();
      lcd.setCursor(5,1);
      if (pump_on) {
        lcd.print("PUMP is OFF");
        digitalWrite(pump_relay, HIGH);
      } else {
        lcd.print("PUMP is ON");
        digitalWrite(pump_relay, LOW); 
      }
      delay(1500);
    }
    pump_on = !pump_on;
  }
}

void listener() {
  for (int i = 0; i < 35; i++) {
    buttonpressed();
    delay(300);
  }
}

void nightlight() {
  float photocell;
  photocell = analogRead(A1) * (5.0 / 1023.0);
  if (photocell < 0.50) {
    digitalWrite(nightlightpin, HIGH);
  } else {
    digitalWrite(nightlightpin, LOW);
  }
}

void loop() {
  nightlight();
  buttonpressed();
  sensors.requestTemperatures();
  float temperature;
  float external_temperature;
  if (units_of_temperature) {
    external_temperature = temperature_in_F();
    temperature = sensors.getTempFByIndex(0);
  } else {
    external_temperature = temperature_in_C();
    temperature = sensors.getTempCByIndex(0);
  }
  float humidity_sensor = humidity();
  int days_left_to_change_water = waterchangeday - differenceday();
  int days_left_to_change_filter = filterchange - differenceday();
  if (temperature == -196 && warning) {
    check_temperature_probe_connections();
  } else if (temperature < TEMPERATURE_MAX && temperature > TEMPERATURE_MIN || !warning) {
    if (days_left_to_change_water < 0 || days_left_to_change_filter < 0) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("PLEASE CHANGE");
        lcd.setCursor(0,2);
        if (days_left_to_change_water < 0) {
          lcd.print("THE WATER!!!");
        } else {
          lcd.print("THE FILTER!!!");
        }
        buzzer();
        delay(500000);
    } 
    
    //first screen display 
    first_screen(temperature, days_left_to_change_water, days_left_to_change_filter, external_temperature, humidity_sensor);

    listener();
    
    //second screen
    lcd.clear(); 
    lcd.setCursor(0,0); 
    lcd.print("Calculated Uptime:");
    int display_second;
    int display_minute; 
    int display_hour;
    int display_day;

    display_minute = 59 - Clock.getMinute();
    display_hour = 24 - Clock.getHour(h12, PM);


    if (display_minute < start_minutes) {
      int temp = 60 - start_minutes;
      display_minute = temp + display_minute;
      if (abs(display_hour - start_hours) > 1) {
        display_hour = abs(display_hour - start_hours) - 1;
      }
    } else {
      display_minute = abs(display_minute - start_minutes);
      display_hour = abs(display_hour - start_hours);
    }
    
    lcd.setCursor(0,1);
    lcd.print("Days: ");
    lcd.print(differenceday());
  
    lcd.setCursor(0,2);
    lcd.print("Hours: ");
    lcd.print(display_hour);
    
    lcd.setCursor(0,3);
    lcd.print("Minutes: ");
    lcd.print(display_minute);
    
    listener();
    lcd.clear();
    
    //third screen
    ReadDS3231();
    listener();

  } else {
    lcd.clear();
    buzzer();
    lcd.setCursor(0,0);
    lcd.print("WARNING!!!!!!!!!!!!!");
    lcd.setCursor(0,1);
    if (temperature >= TEMPERATURE_MAX) {
      lcd.print("TEMPERATURE TOO HIGH");
      lcd.setCursor(0,2);
      lcd.print("LOWER TEMPERATURE BY");
    } else {
      lcd.print("TEMPERATURE TOO LOW");
      lcd.setCursor(0,2);
      lcd.print("HEAT TEMPERATURE BY");
    }
    lcd.setCursor(5,3);
    lcd.print(abs(temperature - 78));
    lcd.print(" F");
    delay(30000);
  }
}

//getting the time from time module
void ReadDS3231()
{
  int second,minute,hour,date,month,year,temperature; 
  second=Clock.getSecond();
  minute=Clock.getMinute();
  hour=Clock.getHour(h12, PM);
  date=Clock.getDate();
  month=Clock.getMonth(Century);
  year=Clock.getYear();
  
  temperature=Clock.getTemperature();
  temperature = (temperature * 1.8) + 32;
  
  lcd.setCursor(0,0);
  lcd.print("Today's Date:");
  lcd.setCursor(0,1);
  lcd.print(month, DEC); 
  lcd.print("/");
  lcd.print(date, DEC);
  lcd.print("/");
  lcd.print(year, DEC);
  lcd.setCursor(0,2);
  lcd.print("Time:");
  lcd.setCursor(0,3);
  if (hour > 12) {
    hour = hour - 12;
    lcd.print(hour,DEC);
    lcd.print(':');
    lcd.print(minute,DEC);
    lcd.print(':');
    lcd.print(second,DEC);
    lcd.print(" PM");
  } else {
    lcd.print(hour,DEC);
    lcd.print(':');
    lcd.print(minute,DEC);
    lcd.print(':');
    lcd.print(second,DEC);
    lcd.print(" AM");
  }
  
  /* The following code below is good for diagnosis use using 
  the serial monitor
  Serial.print("20");
  Serial.print(year,DEC);
  Serial.print('-');
  Serial.print(month,DEC);
  Serial.print('-');
  Serial.print(date,DEC);
  Serial.print(' ');
  Serial.print(hour,DEC);
  Serial.print(':');
  Serial.print(minute,DEC);
  Serial.print(':');
  Serial.print(second,DEC);
  Serial.print('\n');
  Serial.print("Temperature=");
  Serial.print(temperature); 
  Serial.print('\n');
  */
}

//the following method is to test the temperature probe independently
//diagnosis ONLY!
void temperaturecheck() {
  Serial.println();
  Serial.print("Requesting temperature...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  delay(1000);
  Serial.print("Device 1 (index 0) = ");
  Serial.print(sensors.getTempFByIndex(0));
  Serial.println(" Degrees F");
  lcd.setCursor(0, 3);
  lcd.print(sensors.getTempFByIndex(0));
}

void startup() {
  //printing first line of text
  delay(2000);
  lcd.setCursor(2 , 0); //Start at character 3 on line 0
  String name = "Hello Tan Family!";
  for (int i = 0; i < name.length(); i++) {
    lcd.print(name.charAt(i)); 
    delay(400);  
  }
  
  //printing the second line of text
  delay(1000);
  lcd.setCursor(2 , 1);
  String second = "Initializing";
  delay(500);
  lcd.print(second);
  String dots = "...";
  String empty = "   ";
  int count = 0;
  
  //getting the dots to repeat over and over again
  for (int j = 0; j < 3; j++) {
    lcd.setCursor((second.length() + 2) , 1);
    for (int i = 0; i < dots.length(); i++) {
      lcd.print(dots.charAt(i));
      delay(500);  
    }
    if (count < 2) {
      lcd.setCursor((second.length() + 2) , 1);
      lcd.print(empty);
      delay(500);
    }
    count++;
  }
  
  //printing third line of text
  delay(2000);  
  lcd.setCursor(2,2);
  sensors.requestTemperatures();
  long temperature = sensors.getTempFByIndex(0);
  if (warning) {
    while (temperature == -196) {
      buttonpressed();
      sensors.requestTemperatures();
      long temperature = sensors.getTempFByIndex(0);
      check_temperature_probe_connections();
    } 
  } else {
      lcd.print("Equipment Ready"); 
   }
    
  //printing fourth line of text
  delay(2000);
  lcd.setCursor(2,3); 
  String last = "Automation in";  
  lcd.print(last);
  String numbers = "3s2s1s0s";
  for (int i = 0; i < numbers.length(); i+= 2) {
    lcd.setCursor((last.length() + 3) , 3);
    lcd.print(numbers.charAt(i));
    lcd.print(numbers.charAt(i + 1));
    if (i < 6) {
      delay(1000);
    }
  }
  lcd.clear();
}

float humidity() {
  return dht.readHumidity();
} 

float temperature_in_C() {
  return dht.readTemperature();
}

float temperature_in_F() {
  return dht.readTemperature(true);
}

void show_original_time() { 
  lcd.setCursor(0,0);
  lcd.print("Last Reset:"); 
  lcd.setCursor(0,1);
  lcd.print("Date: ");
  lcd.print(start_months);
  lcd.print("/");
  lcd.print(start_days);
  lcd.print("/20");
  lcd.print(start_years);
  lcd.setCursor(0,2);
  lcd.print("Time: ");
  lcd.print(start_hours);
  lcd.print(":");
  lcd.print(start_minutes);
  lcd.print(":");
  lcd.print(start_seconds);
}

void set_last_reset_time() {
  start_seconds = Clock.getSecond();
  start_minutes = Clock.getMinute();
  start_hours = Clock.getHour(h12, PM);
  start_days = Clock.getDate();
  start_months = Clock.getMonth(Century);
  start_years = Clock.getYear();
}

void check_temperature_probe_connections() {
  buzzer();
  buttonpressed();
  lcd.setCursor(0,0); 
  lcd.print("ERROR:");
  lcd.setCursor(0,1);
  lcd.print("Temperature Probe");
  lcd.setCursor(0,2);
  lcd.print("Please check wiring!");
  delay(5000); 
  lcd.clear();
}

int differenceday() {
   int day = Clock.getDate();
   int month = Clock.getMonth(Century);
   if (month == start_months) {
     return day - start_days;
   } else if (month > start_months) {
     int temp;
     for (int i = start_months + 1; i < month; i++) {
       temp = temp + months[i];
     }
     return day + (months[start_months] - start_days) + temp;
   } else if (month < start_months) {
      int temp;
      for (int i = 1; i < month; i++) {
        temp = temp + months[i];
      }
    return day + (months[start_months] - start_days) + temp;
   }
}

void button_push_warning() {
  warning = !warning;
}

void buzzer() {
  if (buzzer_on) {
    tone(buzzerpin, 2000);
    delay(3000);
    noTone(buzzerpin);
  }
}
