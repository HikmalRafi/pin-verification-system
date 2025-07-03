#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

// =====================================
// Struct & Class
// =====================================
struct EdgeButton {
  int LastButtonC = HIGH;
  int LastButtonB = HIGH;
}; EdgeButton edge;

struct EdgeJoy {
  int Center = 512;
  int Range = 100;
  bool LastY = false;
}; EdgeJoy edgeJoy;

struct MenuData {
  int Page = 1;
  int Pin = 0;
  int Index = 0;
  int CekBenar = 0;
  const int PinCode[4] = {1, 2, 3, 4};
}; MenuData menu;

struct UserInput {
  int Data[4] = {0, 0, 0, 0};
}; UserInput user;

class SetLed {
  public:
    int Red, Green;
    SetLed(int r, int g): Red(r), Green(g) {}
    void begin() {
      pinMode(Red, OUTPUT);
      pinMode(Green, OUTPUT);
    }
    void onRed() { digitalWrite(Red, HIGH); }
    void offRed() { digitalWrite(Red, LOW); }
    void onGreen() { digitalWrite(Green, HIGH); }
}; SetLed led(9, 10);

class SetButton {
  public:
    int C, B;
    SetButton(int c, int b): C(c), B(b) {}
    void begin() {
      pinMode(C, INPUT_PULLUP);
      pinMode(B, INPUT_PULLUP);
    }
}; SetButton button(4, 3);

class SetJoy {
  public:
    int X, Y;
    void update() {
      X = analogRead(A0);
      Y = analogRead(A1);
    }
}; SetJoy joy;

class Debounce {
  private:
    unsigned long delayMs = 50;
    unsigned long lastTime = 0;
    int lastState = HIGH;
    int stableState = HIGH;
  public:
    void update(int input) {
      if (input != lastState) lastTime = millis();
      if ((millis() - lastTime) > delayMs) {
        if (input != stableState) stableState = input;
      }
      lastState = input;
    }
    int read() { return stableState; }
}; Debounce dbC, dbB;

// =====================================
// Utility Function
// =====================================
void LcdPrint(const char* word, int x, int y) {
  lcd.setCursor(x, y);
  lcd.print(word);
}

void LcdPrintNum(int num, int x, int y) {
  lcd.setCursor(x, y);
  lcd.print(num);
}

void resetPinInput() {
  for (int i = 0; i < 4; i++) user.Data[i] = 0;
  menu.Index = 0;
  menu.Pin = 0;
  menu.CekBenar = 0;
}

bool cekPinBenar() {
  for (int i = 0; i < 4; i++) {
    if (user.Data[i] != menu.PinCode[i]) return false;
  }
  return true;
}

// =====================================
// Setup
// =====================================
void setup() {
  lcd.init();
  lcd.backlight();
  led.begin();
  button.begin();
  Serial.begin(9600);
}

// =====================================
// Loop
// =====================================
void loop() {
  // Update input
  dbC.update(digitalRead(button.C));
  dbB.update(digitalRead(button.B));
  joy.update();

  int btnC = dbC.read();
  int btnB = dbB.read();

  // ========= Page 1 =========
  if (menu.Page == 1) {
    LcdPrint("Send Pin:", 0, 0);

    // Joy naik turun
    if (joy.Y < (edgeJoy.Center - edgeJoy.Range) && menu.Pin < 9) {
      if (!edgeJoy.LastY) { edgeJoy.LastY = true; menu.Pin++; }
    } else if (joy.Y > (edgeJoy.Center + edgeJoy.Range) && menu.Pin > 0) {
      if (!edgeJoy.LastY) { edgeJoy.LastY = true; menu.Pin--; }
    } else {
      edgeJoy.LastY = false;
    }

    // Tampilkan input
    for (int i = 0; i < 4; i++) {
      if (i < menu.Index)
        LcdPrint("*", i, 1);
      else if (i == menu.Index)
        LcdPrintNum(menu.Pin, i, 1);
      else
        LcdPrint("_", i, 1);
    }

    // Simpan digit
    if (btnC == LOW && edge.LastButtonC == HIGH && menu.Index < 4) {
      user.Data[menu.Index++] = menu.Pin;
      menu.Pin = 0;
    }

    // Cek PIN
    if (btnB == LOW && edge.LastButtonB == HIGH) {
      if (menu.Index == 4 && cekPinBenar()) {
        menu.Page = 2;
        lcd.clear();
      } else {
        lcd.clear();
        LcdPrint("Pin Salah!", 0, 0);
        led.onRed();
        delay(500);
        led.offRed();
        lcd.clear();
        resetPinInput();
      }
    }
  }

  // ========= Page 2 =========
  else if (menu.Page == 2) {
    LcdPrint("Pin Benar", 0, 0);
    led.onGreen();
    delay(500);
  }

  // Update edge detection
  edge.LastButtonB = btnB;
  edge.LastButtonC = btnC;

  delay(50);
}
