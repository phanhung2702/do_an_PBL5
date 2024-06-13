#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <FirebaseESP8266.h>
#include <Servo.h>
Servo myservo;

#include <LiquidCrystal_I2C.h>     //D1,D2
LiquidCrystal_I2C lcd(0x27,16,2); 

//cảm biến gas
#define gas A0
//---------------------------------
/////// nhiệt độ độ ẩm
#include "DHT.h"  // Including library for dht
#define DHTPIN D3         //pin D3 esp where the dht11 is connected
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

//---------------------------------
// khai báo firebase
#define FIREBASE_HOST "smarthome-f5458-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "O7lhpgj6DRtlAvSKbjDG8GXVqggpZcBY0QArMnNE"
FirebaseData fbdo;
String dulieu;
float h,t,p;
//---------------------------------
const char* ssid     = "17B6";
const char* password = "khongbietduoc";
//======================================================================

void setup() {
  Serial.begin(9600); 
  dht.begin();
  lcd.begin();                      // initialize the lcd 
// Print a message to the LCD.
lcd.backlight();
//myservo.attach(D8);/// cửa 
pinMode(D0,OUTPUT);///đèn khách
pinMode(D8,OUTPUT);///đèn ngủ
pinMode(D5,OUTPUT);///đèn ăn
pinMode(D6,OUTPUT);///đèn vs
pinMode(D4,OUTPUT);/// quạt ngủ
//pinMode(3,INPUT);/// ss auto
pinMode(D7,OUTPUT);/// còi

//digitalWrite(D0, LOW);
//digitalWrite(D5, LOW);
//digitalWrite(D6, LOW);
//digitalWrite(D7, LOW);
//digitalWrite(D4, LOW);
//digitalWrite(D8, LOW);
Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\rWorking to connect");
  while (WiFi.status() != WL_CONNECTED) {delay(500); Serial.print(".");}
  Serial.println("");
  Serial.println("ESP8266 Web Server");
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

}

void loop() {
 h = dht.readHumidity();/// đọc độ ẩm
 t = dht.readTemperature();/// đọc nhiệt độ
int gasvalue = analogRead(gas);/// đọc giá trị khí gas
t = t*10;
//h=h*10;

 p = gasvalue;      ///đổi ra % 
//Serial.println(p);
lcd.setCursor(0,0);
  lcd.print("Nhiet Do:");
  lcd.setCursor(10,0);
  lcd.print(t);
  lcd.setCursor(14,0);
  lcd.write(223); 
  lcd.print("C");
  
  lcd.setCursor(0,1);
  lcd.print("Do Am:");
  lcd.setCursor(7,1);
  lcd.print(h);
  lcd.setCursor(12,1);
  lcd.print("%");
///gửi lên firebase
Firebase.setDouble(fbdo, "/smarthome/h", h);//// độ ẩm
Firebase.setDouble(fbdo, "/smarthome/t", t);///// nhiệt độ
Firebase.setDouble(fbdo, "/smarthome/p", p);///// gas

if (t > 20 )  {
digitalWrite (D7, HIGH);
}
else{
  digitalWrite (D7, LOW);
}    

//send firebase
if(Firebase.getString(fbdo,"/smarthome/led1"))  // 
 {
  dulieu = fbdo.stringData();
  dulieu.remove(0,2);
  dulieu.remove(dulieu.length() - 2, 2);
  
   if(dulieu == "A1"){// den khach
            digitalWrite(D0, HIGH);
          }
          if(dulieu == "A0"){
            digitalWrite(D0, LOW);
          }
    
             if(dulieu == "B1"){//den ngu
            digitalWrite(D8, HIGH);
          }
          if(dulieu == "B0"){
            digitalWrite(D8, LOW);
          }
    
          if(dulieu == "C1"){//den an
           
            digitalWrite(D5, HIGH);
          }
          if(dulieu == "C0"){
           
            digitalWrite(D5, LOW);
          }
          if(dulieu == "D1"){//den vs
           
            digitalWrite(D6, HIGH);
          }
          if(dulieu == "D0"){
           
            digitalWrite(D6, LOW);
          }

          if(dulieu == "F1"){//quat
           
            digitalWrite(D4, HIGH);
          }
          if(dulieu == "F0"){
           
            digitalWrite(D4, LOW);
          }

 }

}

void hienthi(){
//  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Nhiet Do:");
  lcd.setCursor(10,0);
  lcd.print(t);
  lcd.setCursor(14,0);
  lcd.write(223); 
  lcd.print("C");
  
  lcd.setCursor(0,1);
  lcd.print("Do Am:");
  lcd.setCursor(7,1);
  lcd.print(h);
  lcd.setCursor(12,1);
  lcd.print("%");
}
