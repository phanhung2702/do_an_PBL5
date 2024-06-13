
/*
  Bao dong (Uno R3):
  - 2 cảm biến chuyển động
  - 1 cảm biến nhiệt độ, độ ẩm: DHT11
  - Còi + button bao dong.
  - Servo: gara + button.
  - 1 LCD
  - Đèn + quạt gió (2 button) nhà vệ sinh.
  Web:
  - Chế độ báo động: 0,1
  - Gara: 2,3
  - Phát hiện xâm nhập: 4,5
  ***Lưu ý:
   - GND: DHT11, Relay(mức cao).
   - GND: Serial, LCD.
*/

#include <avr/interrupt.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DHT.h>
#include <Servo.h>

#define CBCD1 4
#define CBCD2 5
#define DHT_PIN A0
#define buzz 6
#define button_baodong 12
#define button_gara 8
#define pinServo 7
#define den_vs 10
#define relay_quatvs 9
#define button_denvs 2
#define button_quatvs 3

int state_baodong = 0, state_gara = 0, state_denvs = 0, state_quatvs = 0;
int data_send = 0, data_receive = 0, pos = 1, x_baodong = 0, x_gara = 0;
const int DHT_TYPE = DHT11;
int nhiet_do, do_am, i = 0, j;
double n = 20000;

LiquidCrystal_I2C lcd(0x3f, 16, 2);
DHT dht(DHT_PIN, DHT_TYPE);
Servo gara;

void setup() {

//  // Khởi tạo Timer 1____________________
//
//  TCCR1A = 0;
//  TCCR1B = 0;     // thanh ghi de cau hinh ti le chia cua Timer
//  TIMSK1 = 0 ;    // thanh ghi quy dinh hinh thuc ngat
//
//  // duoi day la cau hinh cho Timer
//  TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10); // ti le chia la 1/64
//  TCNT1 = 15535;      // thoi gian nhay vao ngat la sau moi 100ms
//  TIMSK1 = (1 << TOIE1); // hinh thuc ngat la ngat khi tran
//  sei() ;           // cho phep ngat toan cuc
//
//  //______________________________

  Serial.begin(9600);
  Serial.flush();
  lcd.init();
  lcd.backlight();
  pinMode(CBCD1, INPUT);
  pinMode(CBCD2, INPUT);
  pinMode(buzz, OUTPUT);
  pinMode(button_baodong, INPUT_PULLUP);
  pinMode(button_gara, INPUT_PULLUP);
  pinMode(den_vs, OUTPUT);
  pinMode(relay_quatvs, OUTPUT);
  pinMode(button_denvs, INPUT_PULLUP);
  pinMode(button_quatvs, INPUT_PULLUP);

  digitalWrite(buzz, 1);
  digitalWrite(den_vs, 1);
  digitalWrite(relay_quatvs, 1);
  lcd.print("Dang khoi dong..");
  delay(2000);
  digitalWrite(den_vs, state_denvs);
  digitalWrite(relay_quatvs, 0); // relay 1 kenh này kich o muc cao
  digitalWrite(buzz, 0);
  gara.attach(pinServo);
  gara.write(1);
}

void loop() {
  //Serial.println("loop");
    if (Serial.available()) {
      while (Serial.available() < 2);
      int b1 = Serial.read();
      int b2 = Serial.read();
      data_receive = b1 * 256 + b2;
      switch (data_receive) {
        case 0: // Tat che do bao dong
          state_baodong = 0;
          lcd.clear();
          lcd.print("T= ");
          lcd.print(nhiet_do);
          lcd.print("*C  ");
          lcd.print("H= ");
          lcd.print(do_am);
          lcd.print("%");
          lcd.setCursor(0, 1);
          lcd.print("BAO DONG: ");
          lcd.print("OFF");
          break;
  
        case 1:// Bat che do bao dong
          state_baodong = 1;
          lcd.clear();
          lcd.print("T= ");
          lcd.print(nhiet_do);
          lcd.print("*C  ");
          lcd.print("H= ");
          lcd.print(do_am);
          lcd.print("%");
          lcd.setCursor(0, 1);
          lcd.print("BAO DONG: ");
          lcd.print("ON");
          break;
//        case 2: // Dong gara
//          state_gara = 0;
//          while (pos != 1) {
//            pos --;
//            gara.write(pos);
//            delay(20);
//          }
//          break;
//        case 3: // Mo gara
//          state_gara = 1;
//          while (pos != 180) {
//            pos++;
//            gara.write(pos); // gara mo
//            delay(20);
//          }
//          break;
      }
    }

  //  Serial.println(i);
  //  Serial.println("/");
  //  Serial.println(n);
  //  //--------------------------------------
  attachInterrupt(digitalPinToInterrupt(button_quatvs), QUAT_VS, FALLING);
  attachInterrupt(digitalPinToInterrupt(button_denvs), DEN_VS, FALLING);
  //attachInterrupt(digitalPinToInterrupt(button_gara), GARA, FALLING);
  //if(digitalRead(button_denvs == 0) || digitalRead(button_quatvs == 0)) DEN_VA_QUAT_VS();
  if (digitalRead(button_gara) == 0) GARA();
  if (digitalRead(button_baodong) == 0) INT_BAO_DONG();

  if (state_baodong == 1 && i == n / 4) {
    CHECK_BAO_DONG();
  }
  else digitalWrite(buzz, 0);
  if ( i == n / 2) {
    //    data_send = 6;
    //    Serial.write(data_send / 256);
    //    Serial.write(data_send % 256);
    data_send = do_am;
    Serial.write(data_send / 256);
    Serial.write(data_send % 256);
    //---
    if (x_baodong == 1) {
      if (state_baodong == 1) {
        data_send = 1;
        Serial.write(data_send / 256);
        Serial.write(data_send % 256);
        delay(20);
      }
      else {
        data_send = 0;
        Serial.write(data_send / 256);
        Serial.write(data_send % 256);
        delay(20);
      }

      x_baodong = 2;
    }
    //----

//    if (x_gara == 1) {
//      if (state_gara == 1) {
//        data_send = 3;
//        Serial.write(data_send / 256);
//        Serial.write(data_send % 256);
//        delay(20);
//      }
//      if (state_gara == 0) {
//        data_send = 2;
//        Serial.write(data_send / 256);
//        Serial.write(data_send % 256);
//        delay(20);
//      }
//
//      x_gara = 2;
//    }
  }
  if (i == n) {
    LCD();
    //    data_send = 7;
    //    Serial.write(data_send / 256);
    //    Serial.write(data_send % 256);
    data_send = nhiet_do;
    Serial.write(data_send / 256);
    Serial.write(data_send % 256);
    i = 0;
  }
  i++;
}

//----------------------Het ham loop
//---

void INT_BAO_DONG() {
  while (digitalRead(button_baodong) == 0);
  //Serial.println("Bao dong");
  state_baodong = !state_baodong;
  //if (state_baodong == 0) digitalWrite(buzz, 0);
  lcd.clear();
  lcd.print("T= ");
  lcd.print(nhiet_do);
  lcd.print("*C  ");
  lcd.print("H= ");
  lcd.print(do_am);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("BAO DONG: ");
  if (state_baodong == 0) {
    lcd.print("OFF");
    digitalWrite(buzz, 0);
    //    data_send = 0;
    //    //delay(50);
    //    Serial.write(data_send / 256);
    //    Serial.write(data_send % 256);
    n = 20000;
    //delay(50);
  }
  else {
    lcd.print("ON");
    n = 88;
    //    data_send = 1;
    //    delay(50);
    //    Serial.write(data_send / 256);
    //    Serial.write(data_send % 256);
    //delay(50);
  }

  i = 0;
  x_baodong = 1;
}

void CHECK_BAO_DONG() {
  //Serial.println("Check BD");
  Serial.print("CBCD1=");
  Serial.println(digitalRead(CBCD1));
  Serial.print("CBCD2=");
  Serial.println(digitalRead(CBCD2));
  if ((digitalRead(CBCD1) == 1) || (digitalRead(CBCD2) == 1)) {
    digitalWrite(buzz, 1); // phat bao dong
    data_send = 5;
    Serial.write(data_send / 256);
    Serial.write(data_send % 256);
    delay(50);
  }
  else {
    digitalWrite(buzz, 0);
    data_send = 4;
    Serial.write(data_send / 256);
    Serial.write(data_send % 256);
    delay(50);
  }
}

void LCD() {
  //Serial.println("LCD");
  nhiet_do = dht.readTemperature(); //Đọc nhiệt độ
  do_am = dht.readHumidity();    //Đọc độ ẩm
  lcd.clear();
  lcd.print("T= ");
  lcd.print(nhiet_do);
  lcd.print("*C  ");
  lcd.print("H= ");
  lcd.print(do_am);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("BAO DONG: ");
  if (state_baodong == 1) lcd.print("ON");
  else lcd.print("OFF");
}

void GARA() {
  //Serial.println("GARA");
  while (digitalRead(button_gara) == 0);
  state_gara = !state_gara;
  if (state_gara == 0) {
    while (pos != 1) {
      pos--;
      gara.write(pos);
      delay(20);
    }
    //Serial.println(pos);
    //      data_send = 2; // gara dong
    //      Serial.write(data_send / 256);
    //      Serial.write(data_send % 256);
    delay(50);
    return;
  }
  else {
    while (pos != 95) {
      pos++;
      gara.write(pos); // gara mo
      delay(20);
    }
    //Serial.println(pos);
//    data_send = 3;
//    Serial.write(data_send / 256);
//    Serial.write(data_send % 256);
//    delay(50);
  }
  
  //x_gara = 1;
}

void DEN_VS() {
  //Serial.println("DEN/QUAT");
  //  lcd.clear();
  //  lcd.print("NGON");
  if (digitalRead(button_denvs) == 0) { // BAT-TAT den bang nut an
    while (digitalRead(button_denvs) == 0);
    state_denvs = !state_denvs;
    digitalWrite(den_vs, state_denvs);
  }
}

void QUAT_VS() {
  //Serial.println("QUAT_VS");
  if (digitalRead(button_quatvs) == 0) { // BAT-TAT quat bang nut an
    while (digitalRead(button_quatvs) == 0);
    state_quatvs = !state_quatvs;
    if (state_quatvs == 1) {
      digitalWrite(relay_quatvs, 0); // BAT
    }
    else {
      digitalWrite(relay_quatvs, 1); // TAT
    }
  }
}

