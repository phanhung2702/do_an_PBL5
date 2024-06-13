#include <Key.h>
#include <Keypad.h>

/*
  Arduino Mega:
   - LCD ngoài + Keyboard
   - Cửa chính: 2 servo + 5 button
   - Đèn phòng khách + 1 button
   - Cảm biến ánh sáng + 1 button
   - Code Webserver.
  Giao tiếp Serial với các phòng và WebServer
   - Phòng ngủ: Serial1
    + Đèn: 0,1
    + Điều hòa: 2,3
    + Rèm: 4,5
   - Bếp: Serial2
    + Đèn: 0,1
    + Điều hòa: 2,3
    + Báo khí gas: 4,5
   - Báo động: Serial3
    + Bật tắt chế độ báo động: 0,1
    + Gara: 2,3
    + Phát hiện xâm nhập: 4,5
   ***Lưu ý:
     - GND: CBND, 2 Servo.
*/
// Khai bao cac thu vien
#include <Servo.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include<avr/interrupt.h> // dung cac chan 49 47 45 43 41 39
#include <Keypad.h> // dung cac chan 48 46 44 42 40 38 36 34
#include <SPI.h> 
#include <Ethernet.h>

// Khai bao cac chan
#define pinServo1 32 // canh phai
#define pinServo2 30 // canh trai
#define relay_denpk 26  // led ngoai hien
#define relay_dieuhoapk 28
#define button_dieuhoapk 20
#define button_denpk 2
#define button_dongcua 3
#define CBND1 A0
#define CBND2 A1

// Khai bao cac bien
int x, pos1 = 90, pos2 = 90; // cua mo khi pos1 = 180, pos 2 = 0, cua dong khi pos1 = pos2 = 90
int data_send = 0, data_receive = 0, x_baodong = 0, x_gara = 0;
int nhiet_do_pk, do_am_pk, nhiet_do_pn, nhiet_do_bep;

// Bien trang thai cua thiet bi trong nha
byte state_denpk = 0, state_dieuhoapk = 0, state_door = 0 ; // phong khach
byte state_denpn = 0, state_dieuhoapn = 0, state_autorempn = 0; // phong ngu
byte state_denbep = 0, state_dieuhoabep = 0, state_gas = 0; // bep
byte state_baodongmode = 0, state_xamnhap = 0, state_gara = 0; // bao dong va gara

// Khai bao cua Keypad - PassWord cho cua chinh
const byte ROWS = 4;
const byte COLS = 4;
int k = 0; // đếm số kí tự trong Pass đúng
int i = 0; //giới hạn kí tự của pass
int error = 0;
char keys[ROWS][COLS] =
{ {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
}; // Keypad Form

char pass[] = {'1', '2', '3', '4', '5', '6'}; // pass nguoi dung dat
char newpass[6];
byte rowPins[ROWS] = {34, 36, 38, 40};
byte colPins[COLS] = {42, 44, 46, 48};
int f = 0; // to Enter Clear Display one time
//
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

LiquidCrystal lcd(49, 47, 45, 43, 41, 39); // cac chan theo thu tu RS, E, D4, D5, D6, D7

Servo motor1_cuachinh, motor2_cuachinh;

// nhap dia chi IP va dia chi MAC
// Dia chi Ip phu thuoc vao vi tri su dung mang internet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Khai bao IP trong mang LAN
IPAddress ip(192, 168, 0, 58);

EthernetServer server(80);

String readString;
char c;



//__________________________________________________BAT DAU_________________________________________________________//

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  Serial.flush();
  Serial1.flush();
  Serial2.flush();
  Serial3.flush();
  //----
  while (!Serial) {
    ; // ham su dung khi ket noi serialport
  }

  // Khởi tạo Timer 1____________________

  TCCR1A = 0;
  TCCR1B = 0;     // thanh ghi de cau hinh ti le chia cua Timer
  TIMSK1 = 0 ;    // thanh ghi quy dinh hinh thuc ngat

  // duoi day la cau hinh cho Timer
  TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10); // ti le chia la 1/64
  TCNT1 = 53035;      // thoi gian nhay vao ngat la sau moi 100ms
  TIMSK1 = (1 << TOIE1); // hinh thuc ngat la ngat khi tran
  sei() ;           // cho phep ngat toan cuc

  //______________________________

  Ethernet.begin(mac, ip);
  server.begin();

  //----
  motor1_cuachinh.attach(pinServo1);
  motor2_cuachinh.attach(pinServo2);// cua chinh dung 2 servo
  lcd.begin(16, 2);
  lcd.print("Smart Home K58 !");
  delay(2000);
  lcd.clear();
  lcd.print("Xin moi nhap");
  lcd.setCursor(0, 1);
  lcd.print("mat khau...");
  delay(1500);
  lcd.clear();
  lcd.print("   Mat khau:");

  pinMode(relay_denpk, OUTPUT); // den phong khach
  pinMode(relay_dieuhoapk, OUTPUT);
  pinMode(button_denpk, INPUT_PULLUP);
  pinMode(button_dieuhoapk, INPUT_PULLUP);
  pinMode(button_dongcua, INPUT_PULLUP);
  pinMode(CBND1, INPUT);

  digitalWrite(relay_denpk, 1); // tat den
  digitalWrite(relay_dieuhoapk, 1); // tat dieu hoa phong khach

}

void loop() {
  // Code WebServer
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        c = client.read();
        Serial.write(c);
        if (readString.length() < 100) {
          readString += c;
        }
        if (c == '\n') {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<b>");
          client.println("<meta http-equiv=\"Refresh\" content=\"3; url=/tt\"/>");
          client.println("<meta name = \"viewport\" content=\"width=device-width, initial-scale=0.5\"/>");
          client.println("<meta http-equiv=\"Content-Type\" content=\"application/vnd.wap.xhtml+xml; charset=utf-8\" />");
          client.println("<HEAD> <TITLE>HE THONG DIEU KHIEN QUA MANG LAN < / TITLE > < / HEAD >");
          client.println("<body  text = rgb(0, 0, 255)>");

          client.println(" <style>");
          client.println(" .logo {");
          client.println(" width: 750;");
          client.println(" height: 130;");
          client.println(" margin - top: 10;");
          client.println(" margin - left: 20;");
          client.println("}");
          client.println("< / style >");
          client.println("< / HEAD >");
          client.println("<h1>");
          //client.println("<img class = \"logo\" src = "file:///G:/Webserver/Public/Image/logo-bk-rgb.png">");
          client.println("< / h1 >");
          client.println(" <BODY bgcolor = \"FFFFFF\"text = \"FFFFFF\">");

          client.println("<meta http-equiv=\"Refresh\" content=\"3\"; url=/tt\"/>");
          client.println("<meta name = \"viewport\" content=\"width=device-width, initial-scale=0.5\"/>");
          client.println("<meta http-equiv=\"Content-Type\" content=\"application/vnd.wap.xhtml+xml; charset=utf-8\" />");
          client.println("<link type=\"text/css\" rel=\"stylesheet\" href=\"https://googledrive.com/host/0B-H__6fwNtWrWlAzV3E0T0pYYjg\" />");
          client.println("<TITLE>Control Device Via Internet</TITLE>");
          client.println("</HEAD>");

          client.println("<body align \"center\">");
          client.println(" <h1><center>HỆ THỐNG GIÁM SÁT VÀ ĐIỀU KHIỂN</center></h1>");


          client.println("<table border=\"2\" align \"center\" cellspacing=\"0\" cellpadding=\"4\">");
          client.println("<th width=\"400 px \"  bgcolor=\" violet\">&nbsp;&nbspVỊ TRÍ</th>");
          client.println("<th width=\"350 px \"bgcolor=\"violet\">&nbsp;&nbsp;CẢNH BÁO</th>");
          client.println("<th width=\"300 px \"bgcolor=\"violet\">&nbsp;&nbsp;THÔNG SỐ</th>");
          client.println("<th width=\"300 px \"bgcolor=\"violet\">&nbsp;&nbsp;TRẠNG THÁI</th>");
          client.println("<th width=\"100 px \"bgcolor=\"violet\">&nbsp;&nbsp;THAO TÁC</th>");


          client.println("<tr>");
          client.println("<th rowspan = \"5\">PHÒNG KHÁCH</th>");
          client.println("<th align=\"center\" rowspan = \"5\"> Nhiệt độ:      ");
          client.println(nhiet_do_pk);
          client.println(" *C");
          client.println("<br/> ");
          if (nhiet_do_pk >= 60) client.println("NGUY HIỂM !!!");
          else client.println("Bình thường");
          client.println("<br/> ");
          client.println(" <br/>Độ ẩm:    ");
          client.println(do_am_pk);
          client.println(" %");
          client.println("</th>");
          client.println("<td align= \"center\"> Cửa chính</td>");
          if (state_door == 0) {
            client.println("<td  align=\"center\"> Đóng");
            client.println("  </td>");
            client.println("  <td  align=\"center\">");
            client.println("<a href = \"CUACHINHON\"><button type=\"button\">Mở</a>");
          }
          else {
            client.println("<td  align=\"center\"> Mở");
            client.println("  </td>");
            client.println("  <td  align=\"center\">");
            client.println("  <a href = \"CUACHINHOFF\"><button type=\"button\">Đóng</a>");
          }
          client.println("  </td>");
          client.println("</tr>");
          client.println(" <tr>");

          client.println("<td align= \"center\">Đèn chiếu sáng</td>");
          if (state_denpk == 0) {
            client.println("<td  align=\"center\"> Tắt");
            client.println("</td>");

            client.println("<td  align=\"center\">");
            client.println("<a href = \"DENPKON\"><button type=\"button\">Bật</a>");
          }
          else {
            client.println("<td  align=\"center\"> Bật");
            client.println("</td>");

            client.println("<td  align=\"center\">");
            client.println("<a href = \"DENPKOFF\"><button type=\"button\">Tắt</a>");
          }

          client.println("  </td>");
          client.println("</tr>");
          client.println("<tr>");

          client.println(" <td align =\"center\">Điều hòa</td>");
          if (state_dieuhoapk == 0) {
            client.println("<td  align=\"center\"> Tắt");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DHPKON\"><button type=\"button\">Bật</a>");
          }
          else {
            client.println("<td  align=\"center\"> Bật");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DHPKOFF\"><button type=\"button\">Tắt</a>");
          }

          client.println("</td>");
          client.println("</tr>");

          client.println("<tr>");
          client.println("</td>");

          client.println("</tr>");
          client.println("<tr>");

          client.println("<tr>");
          client.println("<th rowspan = \"4\">&nbsp;&nbsp;PHÒNG NGỦ</th>"); //PHONG NGUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU

          client.println("<th rowspan = \"3\"> &nbsp;&nbsp;Nhiệt độ:     ");
          client.println(nhiet_do_pn);
          client.println(" *C");
          client.println("<br/> ");
          client.println("<br/>  ");
          if (nhiet_do_pn >= 60) client.println("NGUY HIỂM !!!");
          else client.println("Bình thường");
          client.println(" </th>");



          client.println("<td align =\"center\">&nbsp;&nbsp;Đèn chiếu sáng</td>");
          if (state_denpn == 0) {
            client.println("<td  align=\"center\"> Tắt");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DENNGUON\"><button type=\"button\">Bật</a>");
          }
          else {
            client.println("<td  align=\"center\"> Bật");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DENNGUOFF\"><button type=\"button\">Tắt</a>");
          }
          client.println("</td>");
          client.println("</tr>");
          client.println("<tr>");

          client.println("  <td align= \"center\">&nbsp;&nbsp;Rèm tự động</td>");
          if (state_autorempn == 0) {
            client.println("  <td  align=\"center\"> Tắt");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"REMON\"><button type=\"button\">Bật</a>");
          }
          else {
            client.println("  <td  align=\"center\"> Bật");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"REMOFF\"><button type=\"button\">Tắt</a>");
          }
          client.println("</td>");
          client.println("</tr>");
          client.println("<tr>");

          client.println("   <td align =\"center\">&nbsp;&nbsp;Điều hòa</td>");
          if (state_dieuhoapn == 0) {
            client.println("<td  align=\"center\"> Tắt");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("  <a href = \"DHNGUON\"><button type=\"button\">Bật</a>");
          }
          else {
            client.println("<td  align=\"center\"> Bật");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("  <a href = \"DHNGUOFF\"><button type=\"button\">Tắt</a>");
          }

          client.println("  </td>");
          client.println("  </tr>");
          client.println("  <tr>");
          client.println("<tr>");

          client.println("<th rowspan = \"4\">&nbsp;&nbsp;KHU BẾP</th>"); // KHU BEPPPPPPPPPPPPPPPPPP

          client.println("<th rowspan = \"2\">&nbsp;&nbsp Nhiệt độ:      ");
          client.println(nhiet_do_bep);
          client.println(" *C");
          client.println("<br/>");
          if (nhiet_do_bep >= 60) client.println("NGUY HIỂM !!!");
          else client.println("Bình thường");

          client.println("</th>");
          client.println("</tr>");
          client.println("<tr>");

          client.println("<td align =\"center\" rowspan = \"2\">&nbsp;&nbsp;Đèn chiếu sáng</td>");
          if (state_denbep == 0) {
            client.println("<td  align=\"center\"> Tắt");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("<a href = \"DENBEPON\"><button type=\"button\">Bật</a>");
          }
          else {
            client.println("    <td  align=\"center\"> Bật");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("  <a href = \"DENBEPOFF\"><button type=\"button\">Tắt</a>");
          }

          client.println("</td>");
          client.println("  </tr>");
          client.println("<tr>");
          client.println("<th rowspan = \"2\"> Khí gas  ");
          client.println("  <br/>");
          if (state_gas == 0) client.println("Bình thường");
          else client.println("NGUY HIỂM !!!");
          client.println("</th>");

          client.println("</tr>");
          client.println("  <tr>");


          client.println("    <td align =\"center\" rowspan = \"2\">&nbsp;&nbsp;Điều hòa</td>");
          if (state_dieuhoabep == 0) {
            client.println("    <td  align=\"center\"> Tắt");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("  <a href = \"DHBEPON\"><button type=\"button\">Bật</a>");
          }
          else {
            client.println("    <td  align=\"center\"> Bật");
            client.println("</td>");
            client.println("<td  align=\"center\">");
            client.println("  <a href = \"DHBEPOFF\"><button type=\"button\">Tắt</a>");
          }
          client.println("</td>");
          client.println("  </tr> ");
          client.println("</tr></table>");// Ket thuc bang dieu khien cac phonggggggggggggggggggggggggggg

//          client.println("<br/>");
//          client.println("<br/>");
//          client.println("<table border = \"1\" align=\"center\">");
//          client.println("<th width=\"250 px \">GARA</th>");
//          client.println("<td width=\"150 px \" align=\"center\" >");
//          if (state_gara == 1) {
//            client.println("<a href = \"GARAOFF\"><button type=\"button\">Đóng");
//            client.println("</td>");
//            client.println("<td width=\"200px \"><b>Đang mở</b></td>");
//          }
//          else {
//            client.println("<a href = \"GARAON\"><button type=\"button\">Mở");
//            client.println("</td>");
//            client.println("<td width=\"200px \"><b>    Đang đóng</b></td>");
//          }
//          client.println("</table>");

          client.println("<br/>");
          client.println("<br/>");
          client.println("<table border = \"1\" align=\"center\">");
          client.println("<th width=\"250 px \">CHẾ ĐỘ AN NINH</th>"); // CHẾ ĐỘ AN NINHHHHHHHHHHHHHHH
          client.println("<td width=\"150 px \" align=\"center\" >");
          if (state_baodongmode == 1) {
            client.println("<a href = \"BAODONGOFF\"><button type=\"button\">Tắt");
            client.println("</td>");
            if (state_xamnhap == 1) client.println("<td width=\"200px \"><b>    CÓ XÂM NHẬP !!!</b></td>");
            else if (state_xamnhap == 0) {
              client.println("<td width=\"200px \"><b>       An toàn</b></td>");
            }
          }
          else {
            client.println("<a href = \"BAODONGON\"><button type=\"button\">Bật");
          }
          client.println("</table>");

          client.println("<h3>GVHD: TS. Đặng Thái Việt ");
          client.println("<br/>");
          client.println(" <br/>");
          client.println(" Nhóm sinh viên thực hiện: Smart Home K58- ĐH Bách Khoa Hà Nội");
          client.println(" </b>");
          client.println(" </h3>");
          client.println(" <h6> || Hệ thống giám sát và điều khiển nhà thông minh</h6> ");
          client.println(" </body>");

          delay(50);
          client.stop();

          // CUA CHINH
          if (readString.indexOf("CUACHINHON") > 0) {
            state_door = 1;
            x = 1;
          }
          if (readString.indexOf("CUACHINHOFF") > 0) {
            state_door = 0;
            x = 1;
          }

          //// dieu khien den phong khach
          if (readString.indexOf("DENPKON") > 0) {
            digitalWrite(relay_denpk, 0);
            state_denpk = 1;
          }
          if (readString.indexOf("DENPKOFF") > 0) {
            digitalWrite(relay_denpk, 1);
            state_denpk = 0;
          }

          //// Dieu hoa phong khach
          if (readString.indexOf("DHPKON") > 0) {
            digitalWrite(relay_dieuhoapk, 0);
            state_dieuhoapk = 1;
          }
          if (readString.indexOf("DHPKOFF") > 0) {
            digitalWrite(relay_dieuhoapk, 1);
            state_dieuhoapk;
          }
          //// Den phong ngu
          if (readString.indexOf("DENNGUON") > 0) {
            data_send = 1;
            Serial1.write(data_send / 256);
            Serial1.write(data_send % 256);
            state_denpn = 1;
          }
          if (readString.indexOf("DENNGUOFF") > 0) {
            data_send = 0;
            Serial1.write(data_send / 256);
            Serial1.write(data_send % 256);
            state_denpn = 0;
          }

          //// Rem phong ngu
          if (readString.indexOf("REMON") > 0) {// Bat che do tu dong dieu chinh
            data_send = 5;
            Serial1.write(data_send / 256);
            Serial1.write(data_send % 256);
            state_autorempn = 1;
          }

          if (readString.indexOf("REMOFF") > 0) {// Tat che do tu dong va dong rem
            data_send = 4;
            Serial1.write(data_send / 256);
            Serial1.write(data_send % 256);
            state_autorempn = 1;
          }

          //// Dieu hoa phong ngu
          if (readString.indexOf("DHNGUON") > 0) {
            data_send = 3;
            Serial1.write(data_send / 256);
            Serial1.write(data_send % 256);
            state_dieuhoapn = 1;
          }
          if (readString.indexOf("DHNGUOFF") > 0) {
            data_send = 2;
            Serial1.write(data_send / 256);
            Serial1.write(data_send % 256);
            state_dieuhoapn = 0;
          }

          ////Den bep
          if (readString.indexOf("DENBEPON") > 0) {
            data_send = 1;
            Serial2.write(data_send / 256);
            Serial2.write(data_send % 256);
            state_denbep = 1;
          }
          if (readString.indexOf("DENBEPOFF") > 0) {
            data_send = 0;
            Serial2.write(data_send / 256);
            Serial2.write(data_send % 256);
            state_denbep = 0;
          }
          // Dieu hoa bep
          if (readString.indexOf("DHBEPON") > 0) {
            data_send = 3;
            Serial2.write(data_send / 256);
            Serial2.write(data_send % 256);
            state_dieuhoabep = 1;
          }
          if (readString.indexOf("DHBEPOFF") > 0) {
            data_send = 2;
            Serial2.write(data_send / 256);
            Serial2.write(data_send % 256);
            state_dieuhoabep = 0;
          }
//          // Dong/mo gara
//          if (readString.indexOf("GARAON") > 0) {
//            //            data_send = 3;
//            //            Serial3.write(data_send / 256);
//            //            Serial3.write(data_send % 256);
//            state_gara = 1;
//            x_gara = 1;
//
//          }
//          if (readString.indexOf("GARAOFF") > 0) {
//            //            data_send = 2;
//            //            Serial3.write(data_send / 256);
//            //            Serial3.write(data_send % 256);
//            state_gara = 0;
//            x_gara = 1;
//          }
          // Bat/ tat che do bao dong
          if (readString.indexOf("BAODONGON") > 0) {
//            data_send = 1;
//            Serial3.write(data_send / 256);
//            Serial3.write(data_send % 256);
            state_baodongmode = 1;
            x_baodong = 1;
          }
          if (readString.indexOf("BAODONGOFF") > 0) {
//            data_send = 0;
//            Serial3.write(data_send / 256);
//            Serial3.write(data_send % 256);
            state_baodongmode = 0;
            x_baodong = 0;
          }

          readString = "";
          client.println("</html>");
        }
      }
    }
  }

  //_______________________________________Code cơ cấu chấp hành___________________________________________________

  attachInterrupt(digitalPinToInterrupt(button_dongcua), DONG_CUA, FALLING);
  attachInterrupt(digitalPinToInterrupt(button_denpk), DEN_PHONG_KHACH, FALLING);
  attachInterrupt(digitalPinToInterrupt(button_dieuhoapk), DIEU_HOA_PHONG_KHACH, FALLING);
  nhiet_do_pn = 5.0 * (analogRead(CBND1)) * 100.0 / 1024.0;
  nhiet_do_bep = 5.0 * (analogRead(CBND2)) * 100.0 / 1024.0;
  //Serial.println(state_baodongmode);
  //------------------------------------------
//
//  if (x_gara == 1) {
//    if (state_gara == 1) {
//      data_send = 3;
//      Serial3.write(data_send / 256);
//      Serial3.write(data_send % 256);
//      //      lcd.clear();
//      //      lcd.print("Gara_on....");
//      //      delay(1000);
//      delay(20);
//    }
//    else {
//      data_send = 2;
//      Serial3.write(data_send / 256);
//      Serial3.write(data_send % 256);
//      delay(20);
//    }
//    x_gara = 2;
//  }

  if (x_baodong == 1) {
    if (state_baodongmode == 1) {
      data_send = 1;
      Serial3.write(data_send / 256);
      Serial3.write(data_send % 256);
      //      lcd.clear();
      //      lcd.print("Gara_on....");
      //      delay(1000);
      delay(20);
    }
    else {
      data_send = 0;
      Serial3.write(data_send / 256);
      Serial3.write(data_send % 256);
      delay(20);
    }
    x_baodong = 2;
  }

  if (state_door == 1 && x == 1) { // Mo cua
    lcd.clear();
    lcd.print("Smart Home K58");
    lcd.setCursor(0, 1);
    lcd.print(" Xin chao ^.^ !");
    while (pos1 != 180) {
      pos1++;
      pos2--;
      motor1_cuachinh.write(pos1);
      motor2_cuachinh.write(pos2);
      delay(20);
    }
    x = 2;
  }
  else if (state_door == 0 && x == 1) {
    while (pos1 != 90) {
      pos1--;
      pos2++;
      motor1_cuachinh.write(pos1);
      motor2_cuachinh.write(pos2);
      delay(20);
    }
    x = 2;
    char newpass[] = {'0', '0', '0', '0', '0', '0'};
    k = 0;
    i = 0;
    f = 0;
    print1();
  }
  if (error == 5) Sai5lan();
  READ_SERIAL();
}
//______________________________________CÁC HÀM CON________________________________________________


void DONG_CUA() {
  if (digitalRead(button_dongcua) == 0 ) {
    while (digitalRead(button_dongcua) == 0);
    state_door = 0;
    //    char newpass[] = {'0', '0', '0', '0', '0', '0'};
    //    k = 0;
    //    i = 0;
    //    f = 0;
    x = 1;
  }
}

void passWord() {
  char key = keypad.getKey();
  if (key != NO_KEY && i < 6)
  {
    if (f == 0)
    {
      lcd.clear();
      f = 1;
    }
    lcd.setCursor(0, 0);
    lcd.print("Mat khau:");
    lcd.setCursor(i, 1);
    lcd.print("*");
    newpass[i] = key;
    if (newpass[i] == pass[i]) k++;
    i++;
  }

  if (k == 6) {
    //    lcd.clear();
    //    lcd.print("Loading...");
    //    delay(1000);
    //    lcd.clear();
    //    lcd.print("*** WELCOME ***");
    //    Mo cua chinh
    //    while (pos1 != 180) {
    //      pos1++;
    //      pos2--;
    //      motor1_cuachinh.write(pos1);
    //      motor2_cuachinh.write(pos2);
    //      delay(20);
    //    }
    state_door = 1; //
    x = 1;

    if (state_denpk == 0) { // neu den dang tat thi bat den len
      digitalWrite(relay_denpk, 0); // bat den
      state_denpk = 1; // doi trang thai den la ON
    }
    if (state_dieuhoapk == 0) { // neu dieu hoa dang tat thi bat len
      digitalWrite(relay_dieuhoapk, 0); // bat den
      state_dieuhoapk = 1; // doi trang thai den la ON
    }
    k = 7;
    return;
  }
  else if (k < 6 && i == 6) {
    //    delay(30);
    //    lcd.clear();
    //    lcd.print("Loading...");
    //    delay(1000);
    lcd.clear();
    lcd.print("Mat khau sai !");
    delay(1000);
    i = 0;
    k = 0;
    error++;
    if (error < 5) {
      print1();
    }
    f = 0;
    return;
  }
}

void print1()
{
  lcd.clear();
  lcd.print("  Xin moi nhap");
  lcd.setCursor(0, 1);
  lcd.print("Mat khau:");
}

void DEN_PHONG_KHACH() {
  if ((digitalRead(button_denpk) == 0)) {
    while (digitalRead(button_denpk) == 0);
    if (state_denpk == 0) { // neu den dang tat thi bat den len
      digitalWrite(relay_denpk, 0); // bat den
      state_denpk = 1; // doi trang thai den la ON
    }
    else {
      digitalWrite(relay_denpk, 1);
      state_denpk = 0;
    }
  }
}

void DIEU_HOA_PHONG_KHACH() {
  if (digitalRead(button_dieuhoapk) == 0) {
    while (digitalRead(button_dieuhoapk) == 0);
    if (state_dieuhoapk == 0) { // neu dieu hoa dang tat thi bat len
      digitalWrite(relay_dieuhoapk, 0); // bat dieu hoa
      state_dieuhoapk = 1; // doi trang thai dieu hoa la ON
    }
    else {
      digitalWrite(relay_dieuhoapk, 1); // tat dieu hoa
      state_dieuhoapk = 0;
    }
  }
}

void READ_SERIAL() {
  //  1. Phong xem phim - Serial
  //  if(Serial.available()) {
  //    while(Serial.available() < 2);
  //    byte b1 = Serial.read();
  //    byte b2 = Serial.read();
  //    data_receive = b1*256 + b2;
  //    switch(data_receive) {
  //      case 0: // den dang tat
  //        state_denxp = 0;
  //        break;
  //      case 1: // den dang bat
  //        state_denxp = 1;
  //        break;
  //      case 2: // dieu hoa dang tat
  //        state_dieuhoaxp = 0;
  //      case 3: // dieu hoa dang bat
  //        state_dieuhoaxp = 1;
  //        break;
  //      case 4: // che do xem phim dang tat
  //        state_autoxp = 0;
  //        break;
  //      case 5: // che do xem phim dang bat
  //        state_autoxp = 1;
  //        break;
  //    }
  //  }
  // 2. Phòng ngủ -  Serial1
  if (Serial1.available()) {
    while (Serial1.available() < 2);
    byte b1 = Serial1.read();
    byte b2 = Serial1.read();
    data_receive = b1 * 256 + b2;
    switch (data_receive) {
      case 0: // den dang tat
        state_denpn = 0;
        break;
      case 1: // den dang bat
        state_denpn = 1;
        break;
      case 2: // dieu hoa dang tat
        state_dieuhoapn = 0;
        break;
      case 3: // dieu hoa dang bat
        state_dieuhoapn = 1;
        break;
      case 4: // Rem dang dong va che do tu dong dang tat
        state_autorempn = 0;
        break;
      case 5: // che do tu dong dang bat
        state_autorempn = 1;
        break;
    }
    //Serial1.flush();
  }
  // 3. Bếp - Serial2
  if (Serial2.available()) {
    while (Serial2.available() < 2);
    byte b1 = Serial2.read();
    byte b2 = Serial2.read();
    data_receive = b1 * 256 + b2;
    switch (data_receive) {
      case 0: // den dang tat
        state_denbep = 0;
        break;
      case 1: // den dang bat
        state_denbep = 1;
        break;
      case 2: // dieu hoa dang tat
        state_dieuhoabep = 0;
        break;
      case 3: // dieu hoa dang bat
        state_dieuhoabep = 1;
      case 4: // Không có khí gas rò rỉ
        state_gas = 0;
        break;
      case 5: // có khí gas rò rỉ
        state_gas = 1;
    }
    //    while(Serial2.available() < 4);
    //    byte b3 = Serial2.read();
    //    byte b4 = Serial2.read();
    //    if(data_receive == 6) {
    //      nhiet_do_bep = b3*256 + b4;
    //    }
    //Serial2.flush();
  }
  // 4. Báo động - Serial3
  if (Serial3.available()) {
    while (Serial3.available() < 2);
    byte b1 = Serial3.read();
    byte b2 = Serial3.read();
    data_receive = b1 * 256 + b2;
    //---
//    lcd.clear();
//    lcd.print("rec = ");
//    lcd.print(data_receive);
    //---
    switch (data_receive) {
      case 0: // che do bao dong dang tat
        state_baodongmode = 0;
        break;
      case 1: // che do bao dong dang bat
        state_baodongmode = 1;
        break;
//      case 2: // gara dang dong
//        state_gara = 0;
//        break;
//      case 3: // gara dang mo
//        //---
//        //        lcd.clear();
//        //        lcd.print("Mo__Gara...");
//        //        delay(1000);
//        //---
//        state_gara = 1;
//        break;
      case 4: // khong co xam nhap
        state_xamnhap = 0;
        break;
      case 5: // co xam nhap
        state_xamnhap = 1;
        break;
      default:
        if (data_receive <= 50) nhiet_do_pk = data_receive;
        else if (data_receive > 50) do_am_pk = data_receive;
    }

    //    while(Serial3.available() < 4);
    //    byte b3 = Serial3.read();
    //    byte b4 = Serial3.read();
    //    if(data_receive == 6) {
    //      do_am_pk = b3*256 + b4;
    //    }
    //    else if(data_receive == 7) {
    //      nhiet_do_pk = b3*256 + b4;
    //    }
  }
}
//---
void Sai5lan() {
  lcd.clear();
  lcd.print("Sai 5 lan !");
  lcd.setCursor(0, 1);
  lcd.print("Nhap lai sau 5s.");
  delay(5000);
  error = 0;
  print1();
  f = 0;
}
//---

ISR(TIMER1_OVF_vect) { // Ngắt của timer 1
  passWord();
  //READ_SERIAL();
  TCNT1 = 53035;
}


