// Sketch for an Automated Lighting Control Project
//
// Copyright (c) 2018, Peter Dustin Azucena
// All rights reserved.
// Last Updated: 11/13/2018
// 

#include <stdio.h>
#include <DS1302.h>
#include <TimeLib.h>
#include <M5Stack.h>
#include "utility/MPU9250.h"

// Connections
// Relay Switch (GND --> M5Stack Ground, VCC --> M5Stack VCC, Signal --> M5Stack Pin 5)
// DS1302 RTC Module (GND --> M5Stack Ground, VCC --> M5Stack VCC, CLK --> M5Stack Pin 22, DAT --> M5Stack Pin 21, RST --> M5Stack Pin 18)
// LDR (Resistor 1K --> M5Stack Ground & Pin 35, LDR --> M5Stack VCC & Pin 35)




namespace {

// Set the appropriate digital I/O pin connections. These are the pin
// assignments for the Arduino as well for as the DS1302 chip. See the DS1302
// datasheet:
//
//   http://datasheets.maximintegrated.com/en/ds/DS1302.pdf
const int kCePin   = 18;                    // Chip Enable
const int kIoPin   = 21;                    // Input/Output
const int kSclkPin = 22;                    // Serial Clock
const int kGatePin = 19;                    // MI Pin
int sensorPin = 35;                         // select the input pin for the voltage measurement signal
int sensorValue = 0;                        // variable to store the value coming from the sensor
int debug = 1;                              // enable if you'd like to use serial monitor
int toggle_switch = 0;
int sleep_mode = 0;
int sleep_timer = 0;
int ambient_light = 0;
int auto_mode = 0;
int cursor_pos = 2;
int stat_changed = 1;
int adj_sleep_timer = 0;
int time_counter = 0;
int relay_sig = 5;
const int enable_rtc_time_set = 0;          // enable this if you want to set new time/date in RTC
int cur_time[6] = {2018,11,11,22,36,30};
int off_time[6]= {2018,11,12,6,0,0};
int on_time[6] = {2018,11,12,4,0,0};
int sleep_start = 0;
int timer_value = 5;

DS1302 rtc(kCePin, kIoPin, kSclkPin);       // Create a DS1302 object.

}  // namespace

void setup() {
  Serial.begin(115200);
  pinMode(relay_sig, OUTPUT);  
  if (enable_rtc_time_set) { 
    // Initialize a new chip by turning off write protection and clearing the
    // clock halt flag. These methods needn't always be called. See the DS1302
    // datasheet for details.
    rtc.writeProtect(false);
    rtc.halt(false);
    Time t(cur_time[0],cur_time[1],cur_time[2],cur_time[3],cur_time[4],cur_time[5], Time::kSunday);       // Make a new time object to set the date and time.
    rtc.time(t);                                   // Set the time and date on the chip.  
  }
  Time t = rtc.time();                              // Retrieve the time and date from RTC
  setTime(t.hr,t.min,t.sec,t.date,t.mon,t.yr);      // Set arduino internal clock the time and date retrieved from RTC
  M5.begin();                                       // initialize the M5Stack object
//  M5.startupLogo();
  
  lcd_text(10,20,2,WHITE,"Ambient Lighting:",0);    
  lcd_text(10,40,2,WHITE,"      Sleep Mode:",0);      
  lcd_text(10,60,2,WHITE,"   Sleep Timeout:",0);      
  lcd_text(10,80,2,WHITE,"       Auto Mode:",0);        
  lcd_text(10,100,2,WHITE,"        OFF Time: 6:00 AM",0);        
  lcd_text(10,120,2,WHITE,"         ON Time: 4:30 AM",0);     
  lcd_text(20,180,3,WHITE,"LIGHT STATUS:",0);        
  M5.lcd.setBrightness(100);
 
}

// Procedure to write a varying txt on LCD given x and y location, suze

void lcd_text(int x, int y, int tsize, int color, String txt, int varying) { 
  int xmult = 0; 
  int ymult = 0;
  M5.Lcd.setTextSize(tsize);
  if (varying==1){
    if (tsize==1){
      xmult = 6;
      ymult = 7;
    }else if (tsize==2){
      xmult = 12;
      ymult = 14;
    }else if (tsize==3){
      xmult = 18;
      ymult = 21;
    }else if (tsize==4){
      xmult = 24;
      ymult = 28;
    }
    M5.Lcd.drawRect(x-3, y-3, txt.length()*xmult+6, ymult+6, BLACK);
    M5.Lcd.fillRect(x-3, y-3, txt.length()*xmult+6, ymult+6, BLACK);
  }
  M5.Lcd.setCursor(x, y);
  M5.Lcd.setTextColor(color);
  M5.Lcd.print(txt);
}

void lcd_select_text(int x, int y, int tsize, String txt) { 
  int xmult = 0; 
  int ymult = 0;
  M5.Lcd.setTextSize(tsize);
  if (tsize==1){
    xmult = 6;
    ymult = 7;
  }else if (tsize==2){
    xmult = 12;
    ymult = 14;
  }else if (tsize==3){
    xmult = 18;
    ymult = 21;
  }else if (tsize==4){
    xmult = 24;
    ymult = 28;
  }
  M5.Lcd.drawRect(x-3, y-3, txt.length()*xmult+6, ymult+6, WHITE);
  M5.Lcd.fillRect(x-3, y-3, txt.length()*xmult+6, ymult+6, WHITE);
  M5.Lcd.setCursor(x, y);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.print(txt);
}

void printTime() {                          // Function to display current time/date in serial monitor
  // Get the current time and date from the chip.
  time_t timeNow = now();
  // Name the day of the week.
  //const String day = dayAsString(t.day);

  // Format the time and date and insert into the temporary buffer.
  char buf[50];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
           year(timeNow), month(timeNow), day(timeNow),
           hour(timeNow), minute(timeNow), second(timeNow));

  // Print the formatted string to serial so we can see the time.
  if (millis()%1000 < 50){
    lcd_text(90,220,1,WHITE,buf,1);
  }
  
}


void loop() {

  printTime();                                      // Loop and print the time every second.
  sensorValue = analogRead(sensorPin);

  // When sleep timer has been reached this will disable sleep_mode and reset sleep_timer to zero, it will also flag toggle_switch which disable the relay
  
  if ((sleep_timer>0)&&((millis()-sleep_timer)>(timer_value*60000))){
    sleep_timer = 0;
    sleep_mode = 0;
    stat_changed = 1;
    toggle_switch = 0;
  }
  
  // If the sensor value of the LDR is high enough, ambient light flag will be set to normal, otherwise, its considered dark in the room
  if ((sensorValue > 4000) && (ambient_light == 0)){
    stat_changed = 1;
    ambient_light = 1;
    toggle_switch = 0;
  } 
  else if ((sensorValue < 1000)&&(ambient_light == 1)){
    stat_changed = 1;
    toggle_switch = 1;
    ambient_light = 0;
  }
  
  if (debug==1) {
    //Serial.print(" Timer Value ");
    //Serial.print(millis());
    //Serial.print(" Difference Value ");
    //Serial.println(millis()%1000);
    Serial.print("LDR Value: ");
    Serial.println(sensorValue);
  }
  
  if (stat_changed == 1){
    if (toggle_switch == 1){
      lcd_text(260,180,3,WHITE,"ON ",1);        
      digitalWrite(relay_sig, HIGH);
    }else{
      lcd_text(260,180,3,WHITE,"OFF",1);        
      digitalWrite(relay_sig, LOW);
    }
 
    if (ambient_light == 1){
      lcd_text(225,20,2,WHITE,"TRUE ",1);    
    }
    else{
      lcd_text(225,20,2,WHITE,"FALSE",1);    
    }
    if (sleep_mode == 1){
      lcd_text(225,40,2,WHITE,"TRUE ",1);    
      lcd_text(225,60,2,WHITE,String(timer_value) + " MINS",1);    
    }
    else{
      lcd_text(225,40,2,WHITE,"FALSE",1); 
      lcd_text(225,60,2,WHITE,"N/A     ",1);    
      sleep_timer = 0;   
    }
    if (auto_mode == 1){
      lcd_text(225,80,2,WHITE,"TRUE ",1);    
    }
    else{
      lcd_text(225,80,2,WHITE,"FALSE",1);    
    }
    
    switch (cursor_pos){
      case 1:
        if (ambient_light == 1){
          lcd_select_text(225,20,2,"TRUE ");    
        }
        else{
          lcd_select_text(225,20,2,"FALSE");    
        }
        break;
      case 2:
        if (sleep_mode== 1){
          lcd_select_text(225,40,2,"TRUE ");    
        }
        else{
          lcd_select_text(225,40,2,"FALSE");    
        }
        break;
      case 3:
        if (sleep_mode == 1){
          lcd_select_text(225,60,2,String(timer_value)+ " MINS");    
        }else{
          lcd_select_text(225,60,2,"N/A");    
        }
        break;
      case 4:
        if (auto_mode== 1){
          lcd_select_text(225,80,2,"TRUE ");    
        }
        else{
          lcd_select_text(225,80,2,"FALSE");    
        }
        break;
      default:
        break;
    }
    stat_changed = 0;
  }  
  if(M5.BtnA.wasPressed()) {
    if (adj_sleep_timer == 1){
        if (timer_value>1){
          timer_value = timer_value - 1;
        }
    }
    else{
      if (cursor_pos>2){
        cursor_pos = cursor_pos - 1;
      }
      else{
        cursor_pos = 4;
      }
    }
    stat_changed = 1;
  }
  if(M5.BtnB.wasPressed()) {
    if (adj_sleep_timer == 1){
        if (timer_value<=120){
          timer_value = timer_value + 1;
        }
    }
    else{
      if (cursor_pos<4){
        cursor_pos = cursor_pos + 1;
      }
      else{
        cursor_pos = 2;
      }
      Serial.println(cursor_pos);    
    }
    stat_changed = 1;    
  } 
  
  if(M5.BtnC.wasPressed()) {
    if (adj_sleep_timer == 1){
      adj_sleep_timer = 0;
    }
    else{
      switch (cursor_pos){
        case 1:
          ambient_light = !ambient_light;
          break;
        case 2:
          sleep_mode = !sleep_mode;
          sleep_timer = millis();      
          break;
        case 3:
          adj_sleep_timer = 1;
          break;          
        case 4:
          auto_mode = !auto_mode;
          break;
        default:
          break;
      }
      stat_changed = 1;
    }
  } 
  //delay(100);
  M5.update(); 
}

