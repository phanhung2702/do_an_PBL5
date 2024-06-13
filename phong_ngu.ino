#include <LiquidCrystal_I2C.h>

/*
  Phòng ngủ - Arduino Uno R3:
  - Đèn hiên.
  - Cảm biến ánh sáng, cảm biến nhiệt độ.
  - LCD.
  - Động cơ bước + drive.
  - Động cơ Servo(rèm) + button
  - Đèn phòng ngủ.
  Giao tiếp:
  - Đèn : 0,1
  - Điều hòa(Động cơ bước): 2,3
  - Rem: 4,5
  ***Lưu ý:
   - GND của Servo, LCD, CBND chung.
   - GND của relay riêng.
*/

#include <Servo.h>
#include <avr/interrupt.h>
#include <Wire.h>

#define den_hien 4
#define relay_denpn 5 // den phong ngu//********
#define CBAS A0
#define CBND A1
#define pinServo 6
#define step_pin 7
#define step_enable 8
#define step_dir 9
#define button_denpn 2
#define button_dieuhoapn 3
#define button_rem 10

int step_time, pos = 179, i = 0;
int nhiet_do, v1;
int data_send = 0, data_receive = 0;
boolean state_denpn = 0, state_dieuhoapn = 0, state_rem = 0;

LiquidCrystal_I2C lcd(0x3F, 16, 2);
Servo rem ;

void setup() {

  TCCR1A = 0;
  TCCR1B = 0;     // thanh ghi de cau hinh ti le chia cua Timer
  TIMSK1 = 0 ;    // thanh ghi quy dinh hinh thuc ngat

  // duoi day la cau hinh cho Timer
  TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10); // ti le chia la 1/64
  TCNT1 = 53035;      // thoi gian nhay vao ngat la sau moi 100ms
  TIMSK1 = (1 << TOIE1); // hinh thuc ngat la ngat khi tran
  sei() ;           // cho phep ngat toan cuc
  //***************************

  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.flush();
  lcd.init();
  lcd.backlight();
  pinMode(den_hien, OUTPUT);
  pinMode(relay_denpn, OUTPUT);
  pinMode(step_pin, OUTPUT);
  pinMode(step_enable, OUTPUT);
  pinMode(step_dir, OUTPUT);
  pinMode(button_denpn, INPUT_PULLUP);
  pinMode(button_dieuhoapn, INPUT_PULLUP);
  pinMode(button_rem, INPUT_PULLUP);
  pinMode(CBAS, INPUT);

  digitalWrite(relay_denpn, 1); // ban dau tat den
  digitalWrite(den_hien, 1);
  delay(500);
  digitalWrite(relay_denpn, 0); // ban dau tat den
  digitalWrite(den_hien, 0);
  //digitalWrite(step_dir, 1);
  digitalWrite(step_enable, 1); // enable la BAT o muc thap --> ban dau dieu hoa tay
  lcd.print("Dang khoi dong..");
  rem.attach(pinServo);
  for (pos = 100; pos < 179; pos++) {
    rem.write(pos);
    delay(20);
  }
  delay(500);
}

void loop() {
  //Serial.println(step_time);
  if (Serial.available()) {
    while (Serial.available() < 2);
    int b1 = Serial.read();
    int b2 = Serial.read();
    data_receive = b1 * 256 + b2;

    switch (data_receive) {
      case 0: // Tat den
        digitalWrite(relay_denpn, 0);
        state_denpn = 0;
        break;
      case 1:// Bat den
        digitalWrite(relay_denpn, 1);
        state_denpn = 1;
        break;
      case 2: // Tat dieu hoa
        digitalWrite(step_enable, 1);
        state_dieuhoapn = 0;
        break;
      case 3: //Bat dieu hoa
        digitalWrite(step_enable, 0); // Chân Enable kích mức thấp
        state_dieuhoapn = 1;
        break;
      case 4: // Kéo rèm và tắt chế độ tự động
        while (pos != 0) {
          pos--;
          rem.write(pos);
          delay(20);
        }
        state_rem = 0; //rèm đóng và tắt chế độ tự động
        break;
      case 5: //Bật chế độ tự động điều chỉnh của rèm
        state_rem = 1;
    }
  }
  //--------------------------------------------------------
  attachInterrupt(digitalPinToInterrupt(button_denpn), DEN_VA_DIEU_HOA, FALLING);
  attachInterrupt(digitalPinToInterrupt(button_dieuhoapn), DEN_VA_DIEU_HOA, FALLING);

  if (state_dieuhoapn == 1) {
    digitalWrite(step_dir, 1);
    DIEU_HOA();
  }

  if (i == 2000) LCD_PHONG_NGU();

  if (state_rem == 1) REM(); // che do tu dong thay doi độ mở rèm theo ánh sáng ngoài trời

  if (digitalRead(button_rem) == 0) {
    while (digitalRead(button_rem) == 0);
    state_rem = !state_rem;
    if (state_rem == 0) {
      while (pos != 179) { // dong rem
        pos++;
        rem.write(pos);
        delay(20);
      }
      data_send = 4;
      Serial.write(data_send / 256);
      Serial.write(data_send % 256);
    }
    else {
      data_send = 5;
      Serial.write(data_send / 256);
      Serial.write(data_send % 256);
    }
  }
  DEN_NGOAI_HIEN();
  i++;
  //Serial.println(analogRead(CBAS));
  //Serial.println(state_rem);

}

void DEN_NGOAI_HIEN() {
  //Serial.println(analogRead(CBAS));
  if (analogRead(CBAS) <= 250) { // neu troi sang thi tat den hien
    digitalWrite(den_hien, 0);
  }
  else digitalWrite(den_hien, 1); // toi thi bat den hien len
}

void REM() { // pos tăng là mở thêm, giảm là khép bớt: 180 là mở hẳn, 0 là khép hẳn
  // ánh sáng từ sáng tối từ 40-600.
  if (analogRead(CBAS) <= 150) { //trời sáng thì kh
    while (pos != 179) {
      pos++;
      rem.write(pos);
      delay(20);
    }
  }
  //   else {
  //      while (pos < 50) {
  //        pos++;
  //        rem.write(pos);
  //        delay(20);
  //      }
  //    }

  if ( (analogRead(CBAS) > 150) && (analogRead(CBAS) <= 500) ) {
    if (pos > 80) {
      while (pos != 80) {
        pos--;
        rem.write(pos);
        delay(20);
      }
    }
    else {
      while (pos != 80) {
        pos++;
        rem.write(pos);
        delay(20);
      }
    }
  }

  if (analogRead(CBAS) > 500) { // troi toi
    while (pos != 1) {
      pos--;
      rem.write(pos);
      delay(20);
    }
  }
}

void DEN_VA_DIEU_HOA() {
  // BAT/TAT den
  if ( (digitalRead(button_denpn) == 0) ) {
    while (digitalRead(button_denpn) == 0);
    state_denpn = !state_denpn;
    if (state_denpn == 1) {
      digitalWrite(relay_denpn, 1); // bat len
      data_send = 1;
      Serial.write(data_send / 256);
      Serial.write(data_send % 256);
    }
    else {
      digitalWrite(relay_denpn, 0); // tat di
      data_send = 0;
      Serial.write(data_send / 256);
      Serial.write(data_send % 256);
    }
    return;
  }

  // BAT/TAT dieu hoa
  if (digitalRead(button_dieuhoapn) == 0) {
    while (digitalRead(button_dieuhoapn) == 0);
    state_dieuhoapn = !state_dieuhoapn;

    if (state_dieuhoapn == 1) {
      digitalWrite(step_enable, 0); // Bat len
      data_send = 3;
      Serial.write(data_send / 256);
      Serial.write(data_send % 256);
    }
    else {
      digitalWrite(step_enable, 1); // tat di
      data_send = 2;
      Serial.write(data_send / 256);
      Serial.write(data_send % 256);
      step_time = 0;
    }
  }
}
//---

void LCD_PHONG_NGU() {
  i = 0;
  nhiet_do = 5.0 * (analogRead(CBND)) * 100.0 / 1024.0 - 2.0;
  lcd.clear();
  lcd.print("Nhiet do: ");
  lcd.print(nhiet_do);
  lcd.print("*C");
  lcd.setCursor(0, 1); // chỉnh con trỏ xuống cột 1, dòng 2.
  lcd.print("v = ");   // Lệnh in ra màn hình
  lcd.print(v1);
  lcd.print("rpm        ");
  //TINH_VAN_TOC();
}
//---

void TINH_VAN_TOC() {
  double v;
  //int v1;
  v = 60000 / (9.6 * (step_time)) ; // động cơ 96 bước/vòng
  v1 = int(55 * v);
  //    lcd.setCursor(0,1);
  //    lcd.print("v = ");   // Lệnh in ra màn hình
  //    lcd.print(v1);
  //    lcd.println("rpm        ");
  //Serial.println(v1);
  return;
}
//---

void DIEU_HOA() {
  if (nhiet_do < 30) {
    step_time = 850;
    TURN(step_time);
    return;
  }

  if ( (nhiet_do > 30) && (nhiet_do <= 40) ) {
    step_time = 750;
    TURN(step_time);
    return;
  }

  if (nhiet_do > 40) {
    step_time = 560;
    TURN(step_time);
    return;
  }
}
//---

void TURN(int t) {
  digitalWrite(step_pin, 1);
  delayMicroseconds(t);
  digitalWrite(step_pin, 0);
  delayMicroseconds(t);
}
//---

ISR(TIMER1_OVF_vect) { // Ngắt của timer 1
  TINH_VAN_TOC();
  TCNT1 = 53035;
}


