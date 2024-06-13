/*
nhiệm vụ:
- Điều khiển bơm theo 2 cấp theo % độ ẩm
- Hiển thị độ ẩm đất theo %
- Hiển thị cấp động cơ bơm nước
- Có nút bấm Bật/ Tắt động cơ thủ công
 */


#define CBHN1 8 //cong
#define CBHN2 9 //hien
#define led_cong 11
#define led_hien 10
#define step_pin 6     // 7,8,9 la cac chan arduino ket noi voi dong co buoc
#define step_enable 5
#define step_dir 7
#define CB_doam 12   // Khô = 1, ẩm = 0

void setup() {
  // put your setup code here, to run once:
  pinMode(CBHN1, INPUT);
  pinMode(CBHN2, INPUT);
  pinMode(led_cong, OUTPUT);
    pinMode(led_hien, OUTPUT);
  digitalWrite(led_cong, 1);
  digitalWrite(led_hien, 1);
  
  delay(1000);
    digitalWrite(led_cong, 0);
  digitalWrite(led_hien, 0);

  pinMode(step_pin, OUTPUT);
  pinMode(step_enable, OUTPUT);
  pinMode(step_dir, OUTPUT);
  pinMode(CB_doam, INPUT);
  digitalWrite(step_enable, 1); // 1 = tat, 0 = chạy
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println(digitalRead(CB_doam));
  //if(analogRead(8));
  if(digitalRead(CBHN1) == 0) {
    digitalWrite(led_cong, 1);
    delay(3000);
  }
  else digitalWrite(led_cong, 0);
    
    
 if(digitalRead(CBHN2) == 0) {
    digitalWrite(led_hien, 1);
    delay(3000);
  }
  else digitalWrite(led_hien, 0);

  if (digitalRead(CB_doam) == 1) { //khi đất khô (độ nhạy tùy thuộc biến trở) thì cảm biến độ ẩm báo 1
    digitalWrite(step_enable, 0); // bật động cơ bơm
    while (digitalRead(CB_doam) == 1){// đặt xung tốc độ cho động cơ
      digitalWrite(step_pin, 1);
      delayMicroseconds(1200);
      digitalWrite(step_pin, 0);
      delayMicroseconds(1200);
    }
  }
    digitalWrite(step_enable, 1); 
}
