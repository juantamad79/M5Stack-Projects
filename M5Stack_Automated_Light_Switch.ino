// Sketch for an Automated Lighting Control Project
//
// Copyright (c) 2018, Peter Dustin Azucena
// All rights reserved.
// Last Updated: 11/12/2018
// 

#include <stdio.h>
#include <DS1302.h>
#include <TimeLib.h>
#include <M5Stack.h>
#include "utility/MPU9250.h"

namespace {

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
int cursor_pos = 1;
int stat_changed = 1;
int time_counter = 0;
const int enable_rtc_time_set = 0;          // enable this if you want to set new time/date in RTC
int cur_time[6] = {2018,11,11,22,36,30};
int off_time[6]= {2018,11,12,6,0,0};
int on_time[6] = {2018,11,12,4,0,0};

DS1302 rtc(kCePin, kIoPin, kSclkPin);       // Create a DS1302 object.

}  // namespace

void setup() {
  Serial.begin(115200);
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
  
 lcd_text(20,140,4,WHITE,"LIGHT STATUS",0);        
 lcd_text(110,180,4,WHITE,"OFF",0);        
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
  lcd_text(10,20,2,WHITE,"Ambient Lighting:",0);    
  lcd_text(10,40,2,WHITE,"      Sleep Mode:",0);      
  lcd_text(10,60,2,WHITE,"       Auto Mode:",0);        
  lcd_text(10,80,2,WHITE,"        OFF Time: 6:00 AM",0);        
  lcd_text(10,100,2,WHITE,"         ON Time: 4:30 AM",0);     
  M5.lcd.setBrightness(100);
  printTime();                                      // Loop and print the time every second.
  sensorValue = analogRead(sensorPin);
  if (debug==1) {
    Serial.print(" Timer Value ");
    Serial.print(millis());
    Serial.print(" Difference Value ");
    Serial.println(millis()%1000);
    
  }

  //lcd_text(20,70,4,WHITE,String(sensorValue),0);    
  if ((sensorValue > 4000)&&(toggle_switch==0)){
    toggle_switch = 1;
  }
  if ((sensorValue < 1000)&&(toggle_switch==1)){
    toggle_switch = 0;
  }


  if (stat_changed == 1){
    if (ambient_light == 1){
      lcd_text(225,20,2,WHITE,"TRUE ",1);    
    }
    else{
      lcd_text(225,20,2,WHITE,"FALSE",1);    
    }
    if (sleep_mode == 1){
      lcd_text(225,40,2,WHITE,"TRUE ",1);    
    }
    else{
      lcd_text(225,40,2,WHITE,"FALSE",1);    
    }
    if (auto_mode == 1){
      lcd_text(225,60,2,WHITE,"TRUE ",1);    
    }
    else{
      lcd_text(225,60,2,WHITE,"FALSE",1);    
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
        if (auto_mode== 1){
          lcd_select_text(225,60,2,"TRUE ");    
        }
        else{
          lcd_select_text(225,60,2,"FALSE");    
        }
        break;
      default:
        break;
    }
    stat_changed = 0;
  }  
  if(M5.BtnA.wasPressed()) {
    if (cursor_pos>1){
      cursor_pos = cursor_pos - 1;
    }
    else{
      cursor_pos = 3;
    }
    stat_changed = 1;
  }
  if(M5.BtnB.wasPressed()) {
    if (cursor_pos<3){
      cursor_pos = cursor_pos + 1;
    }
    else{
      cursor_pos = 1;
    }
    Serial.println(cursor_pos);    
    stat_changed = 1;    
  } 
  
  if(M5.BtnC.wasPressed()) {
    switch (cursor_pos){
      case 1:
        ambient_light = !ambient_light;
        break;
      case 2:
        sleep_mode = !sleep_mode;
        break;
      case 3:
        auto_mode = !auto_mode;
        break;
      default:
        break;
    }
    
    stat_changed = 1;
  } 
  //delay(100);
  M5.update(); 
}
