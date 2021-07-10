// Copyright 2015 chaeplin
// Copyright 2017 xpokor22
// This is based on:
// https://github.com/z3t0/Arduino-IRremote/blob/master/examples/LGACSendDemo/LGACSendDemo.ino

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

IRsend irsend(14);  // An IR LED is controlled by GPIO pin 14 (D5)





// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
const char auth[] = "epLHp0dZ82_WbhMZ0UYAe5QLEU6Ucd7H";
// Network settings
const char ssid[] = "TP-Link_1AC0";
const char pass[] = "89290101";



// 0 : TOWER
// 1 : WALL
const unsigned int kAc_Type  = 1;

// 0 : cooling
// 1 : heating
unsigned int ac_heat = 0;

// 0 : off
// 1 : on
unsigned int ac_power_on = 0;

// 0 : off
// 1 : on --> power on
unsigned int ac_air_clean_state = 0;

// temperature : 18 ~ 30
unsigned int ac_temperature = 24;

// 0 : low
// 1 : mid
// 2 : high
// if kAc_Type = 1, 3 : change
unsigned int ac_flow = 0;

const uint8_t kAc_Flow_Tower[3] = {0, 4, 6};
const uint8_t kAc_Flow_Wall[4] = {0, 2, 4, 5};

uint32_t ac_code_to_sent;

void Ac_Send_Code(uint32_t code) {
  Serial.print("code to send : ");
  Serial.print(code, BIN);
  Serial.print(" : ");
  Serial.println(code, HEX);

#if SEND_LG
  irsend.sendLG(code, 28);
#else  // SEND_LG
  Serial.println("Can't send because SEND_LG has been disabled.");
#endif  // SEND_LG
}

void Ac_Activate(unsigned int temperature, unsigned int air_flow,
                 unsigned int heat) {
  ac_heat = heat;
  unsigned int ac_msbits1 = 8;
  unsigned int ac_msbits2 = 8;
  unsigned int ac_msbits3 = 0;
  unsigned int ac_msbits4;
  if (ac_heat == 1)
    ac_msbits4 = 4;  // heating
  else
    ac_msbits4 = 0;  // cooling
  unsigned int ac_msbits5 =  (temperature < 15) ? 0 : temperature - 15;
  unsigned int ac_msbits6 = 0;

  if (air_flow <= 2) {
    if (kAc_Type == 0)
      ac_msbits6 = kAc_Flow_Tower[air_flow];
    else
      ac_msbits6 = kAc_Flow_Wall[air_flow];
  }

  // calculating using other values
  unsigned int ac_msbits7 = (ac_msbits3 + ac_msbits4 + ac_msbits5 +
                             ac_msbits6) & B00001111;
  ac_code_to_sent = ac_msbits1 << 4;
  ac_code_to_sent = (ac_code_to_sent + ac_msbits2) << 4;
  ac_code_to_sent = (ac_code_to_sent + ac_msbits3) << 4;
  ac_code_to_sent = (ac_code_to_sent + ac_msbits4) << 4;
  ac_code_to_sent = (ac_code_to_sent + ac_msbits5) << 4;
  ac_code_to_sent = (ac_code_to_sent + ac_msbits6) << 4;
  ac_code_to_sent = (ac_code_to_sent + ac_msbits7);

  Ac_Send_Code(ac_code_to_sent);

  ac_power_on = 1;
  ac_temperature = temperature;
  ac_flow = air_flow;
}

void Ac_Change_Air_Swing(int air_swing) {
  if (kAc_Type == 0) {
    if (air_swing == 1)
      ac_code_to_sent = 0x881316B;
    else
      ac_code_to_sent = 0x881317C;
  } else {
    if (air_swing == 1)
      ac_code_to_sent = 0x8813149;
    else
      ac_code_to_sent = 0x881315A;
  }
  Ac_Send_Code(ac_code_to_sent);
}

void Ac_Power_Down() {
  ac_code_to_sent = 0x88C0051;

  Ac_Send_Code(ac_code_to_sent);

  ac_power_on = 0;
}

void Ac_Air_Clean(int air_clean) {
  if (air_clean == '1')
    ac_code_to_sent = 0x88C000C;
  else
    ac_code_to_sent = 0x88C0084;

  Ac_Send_Code(ac_code_to_sent);

  ac_air_clean_state = air_clean;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  irsend.begin();
  Blynk.begin(auth, ssid, pass);
 
}

void loop() {

  Blynk.run();


  /**
  char b = ' ';
  Serial.println("# a : mode or temp    b : air_flow, temp, swing, clean,"
                 " cooling/heating");
  Serial.println("# 0 : off             0");
  Serial.println("# 1 : on              0");
  Serial.println("# 2 : air_swing       0 or 1");
  Serial.println("# 3 : air_clean       0 or 1");
  Serial.println("# 4 : air_flow        0 ~ 2 : flow");
  Serial.println("# + : temp + 1");
  Serial.println("# - : temp - 1");
  Serial.println("# c : cooling");
  Serial.println("# h : heating");
  Serial.println("# m : change cooling to air clean, air clean to cooling");

  Serial.println("a=");  // Prompt User for input
  while (Serial.available() == 0) {  // Wait for user input
  }
  char a = Serial.read();  // Read user input into a
  switch (a) {
    case '0':
    case '1':
    case '+':
    case '-':
    case 'c':
    case 'h':
    case 'm':
      break;
    default:
      Serial.println("b=");  // Prompt User for input
      while (Serial.available() == 0) {}
      b = Serial.read();
  }


  Serial.print("a : ");
  Serial.print(a);
  Serial.print("  b : ");
  Serial.println(b);

  switch (a) {
    case '0':  // off
      Ac_Power_Down();
      break;
    case '1':  // on
      Ac_Activate(ac_temperature, ac_flow, ac_heat);
      break;
    case '2':
      if (b == '0')
        Ac_Change_Air_Swing(0);
      else
        Ac_Change_Air_Swing(1);
      break;
    case '3':  // 1  : clean on, power on
      if (b == '0' || b == '1')
        Ac_Air_Clean(b);
      break;
    case '4':
      switch (b) {
        case '1':
          Ac_Activate(ac_temperature, 1, ac_heat);
          break;
        case '2':
          Ac_Activate(ac_temperature, 2, ac_heat);
          break;
        case '3':
          Ac_Activate(ac_temperature, 3, ac_heat);
          break;
        default:
          Ac_Activate(ac_temperature, 0, ac_heat);
      }
      break;
    case '+':
      if (18 <= ac_temperature && ac_temperature <= 29)
        Ac_Activate((ac_temperature + 1), ac_flow, ac_heat);
      break;
    case '-':
      if (19 <= ac_temperature && ac_temperature <= 30)
        Ac_Activate((ac_temperature - 1), ac_flow, ac_heat);
      break;
    case 'c':
      ac_heat = 0;
      Ac_Activate(ac_temperature, ac_flow, ac_heat);
      break;
    case 'h':
      ac_heat = 1;
      Ac_Activate(ac_temperature, ac_flow, ac_heat);
      break;
    case 'm':
    
      if (ac_power_on == 1) {
        Ac_Power_Down();
        delay(100);
        Ac_Air_Clean(1);
      } else {
        if (ac_air_clean_state == 1) {
          Ac_Air_Clean(0);
          delay(100);
        }
        Ac_Activate(ac_temperature, ac_flow, ac_heat);
      }
      break;
  } 
  
  */

}



BLYNK_WRITE(V1) {
 int pinValue = param.asInt(); // Assigning incoming value from pin V3 to a variable
 if (pinValue == 0) {
    Ac_Power_Down();
  } else {
    ac_flow=2;
    ac_heat = 0;
    Ac_Activate(ac_temperature, ac_flow, ac_heat);
 }
}

BLYNK_WRITE(V2) {
 
 int pinValue = param.asInt(); // Assigning incoming value from pin V3 to a variable
 ac_heat = 0;
 ac_flow=2;
 Ac_Activate(pinValue, ac_flow, ac_heat);
}


BLYNK_READ(V3) {
 int ac_temperature=24;
 Blynk.virtualWrite(V3,ac_temperature);
}

 


BLYNK_WRITE(V0) {
 int pinValue = param.asInt(); // Assigning incoming value from pin V3 to a variable
 Ac_Change_Air_Swing(pinValue);
 }
