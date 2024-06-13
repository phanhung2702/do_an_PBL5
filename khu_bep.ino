/*
  Chức năng:
   - Đèn + quạt.
   - Cảm biến khí gas, khói.
   - Cảm biến nhiệt độ.
   - Còi báo.
   - Servo(dây phơi) + cảm biến mưa.
  Web:
   - Đèn: 0,1
   - Quạt: 2,3
   - Báo khí gas: 4,5
*/


#include <Servo.h>

#define CBGAS 12
#define CBND A1
#define CBMUA 9
#define led_bep 4
#define fan 5
#define buzz 6
#define pinServo 8
#define button_led 2
#define button_fan 7
#define button_dayphoi 3

int nhiet_do, gas, i;
int state_led = 0, state_fan = 0, state_phoi = 0;
int pos = 0, data_send = 0, data_receive = 0;
Servo myservo;

void setup() {
//  //---Khoi tao timer
//  TCCR1A = 0;
//  TCCR1B = 0;     // thanh ghi de cau hinh ti le chia cua Timer
//  TIMSK1 = 0 ;    // thanh ghi quy dinh hinh thuc ngat
//
//  // duoi day la cau hinh cho Timer
//  TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10); // ti le chia la 1/64
//  TCNT1 = 63035;      // thoi gian nhay vao ngat la sau moi 100ms
//  TIMSK1 = (1 << TOIE1); // hinh thuc ngat la ngat khi tran
//  sei() ;           // cho phep ngat toan cuc
//  //----------------

  pinMode(CBGAS, INPUT);
  pinMode(CBND, INPUT);
  pinMode(led_bep, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(buzz, OUTPUT);
  pinMode(button_led, INPUT_PULLUP);
  pinMode(button_fan, INPUT_PULLUP);
  pinMode(button_dayphoi, INPUT_PULLUP);
  digitalWrite(led_bep, HIGH);
  digitalWrite(fan, HIGH);
  digitalWrite(buzz, LOW);
  myservo.attach(pinServo);
  myservo.write(pos);
  Serial.begin(9600);
}

void loop() {
  //Serial.println(digitalRead(CBGAS));
  // Nhan du lieu tu Webserver
  if (Serial.available()) {
    while (Serial.available() < 2);
    byte b1 = Serial.read();
    byte b2 = Serial.read();
    data_receive = b1 * 256 + b2;

    switch (data_receive) {
      case 0: // tat den
        digitalWrite(led_bep, 1);
        state_led = 0;
        break;
      case 1: // bat den
        digitalWrite(led_bep, 0);
        state_led = 1;
        break;
      case 2: // tat quat
        digitalWrite(fan, 1); // kich relay muc cao la TAT
        state_fan = 0;
        break;
      case 3: // bat quat
        digitalWrite(fan, 0); // kich relay muc thao la BAT
        state_fan = 1;
        break;
    }
  }
  // bat den
  if (digitalRead(button_led) == 0) // nut bat/tat den
  {
    while (digitalRead(button_led) == 0);
    state_led = !state_led;
    if (state_led == 1) {
      digitalWrite(led_bep, 0);
      data_send = 1;
      Serial.write(data_send / 256);
      Serial.write(data_send % 256);
    }
    else {
      digitalWrite(led_bep, 1);
      data_send = 0;
      Serial.write(data_send / 256);
      Serial.write(data_send % 256);
    }
  }
  // bat quat thong gio
  if (digitalRead(button_fan) == 0)
  {
    while (digitalRead(button_fan) == 0);
    state_fan = !state_fan;
    if (state_fan == 1) {
      digitalWrite(fan, 0);
      data_send = 3;
      Serial.write(data_send / 256);
      Serial.write(data_send % 256);
    }
    else {
      digitalWrite(fan, 1);
      data_send = 2;
      Serial.write(data_send / 256);
      Serial.write(data_send % 256);
    }
  }
  //canh bao khi co khi gas
  nhiet_do = 5.0 * 100.0 * analogRead(CBND) / 1024.0;
  //gas = 5.0 * 100.0 * analogRead(CBGAS) / 1024.0;
  //Serial.println(gas);
  //  Serial.print("T= ");
  // Serial.println(nhiet_do);
  //Serial.println(digitalRead(CBMUA));
  if (digitalRead(CBGAS) == 0 || nhiet_do > 60)
  {
    /*if (state_fan == 0) { // neu quat dang tat thi bat, con dang bat thi thoi
      digitalWrite(fan, 0);
      delay(100);
      data_send = 3;
      Serial.write(data_send / 256);
      Serial.write(data_send % 256);
    }*/
    digitalWrite(buzz, HIGH);
  }
  else
  {
    digitalWrite(buzz, LOW);
  }

  if (gas > 50) {
    data_send = 5;
    Serial.write(data_send / 256);
    Serial.write(data_send % 256);
  }
  else {
    data_send = 4;
    Serial.write(data_send / 256);
    Serial.write(data_send % 256);
  }
  // day phoi quan ao khi troi mua
  // che do tu dong bang cam bien
  if (state_phoi == 1) { // Che do tu dong day phoi dang bat
    if ((digitalRead(CBMUA) == 1) && (pos == 0)) // ko co mua
    {
      for (pos = 0; pos < 180; pos += 1) // cho servo quay từ 0->179 độ
      { // mỗi bước của vòng lặp tăng 1 độ
        myservo.write(pos);              // xuất tọa độ ra cho servo
        delay(15);                       // đợi 15 ms cho servo quay đến góc đó rồi tới bước tiếp theo
      }
    }
    if ((digitalRead(CBMUA) == 0) && (pos == 180)) // co mua
    {
      for (pos = 180; pos >= 1; pos -= 1) // cho servo quay từ 179-->0 độ
      {
        myservo.write(pos);              // xuất tọa độ ra cho servo
        delay(15);                       // đợi 15 ms cho servo quay đến góc đó rồi tới bước tiếp theo
      }
    }
  }
  //keo day phoi bang nut nhan
  if (digitalRead(button_dayphoi) == 0)
  {
    while (digitalRead(button_dayphoi) == 0);
    if (pos == 180)
    {
      for (pos = 180; pos > 0; pos--) // cho servo quay từ 0->179 độ
      { // mỗi bước của vòng lặp tăng 1 độ
        myservo.write(pos);              // xuất tọa độ ra cho servo
        delay(15);                       // đợi 15 ms cho servo quay đến góc đó rồi tới bước tiếp theo
      }
      state_phoi = 0;
    }

    else
    {
      for (pos = 0; pos < 180; pos++) // cho servo quay từ 179-->0 độ
      {
        myservo.write(pos);              // xuất tọa độ ra cho servo
        delay(15);                       // đợi 15 ms cho servo quay đến góc đó rồi tới bước tiếp theo
      }
      state_phoi = 1;
    }
  }
}


