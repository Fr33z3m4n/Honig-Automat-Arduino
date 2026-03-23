#include <LCD.h>

// --- INIT DISPLAY ---
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// 5x8-Bitmaps (oben zwei Punkte für ä/ö/ü/Ä/Ö/Ü, simple Formen)
byte CH_AE[8] = { B01010,B00000,B01110,B00001,B01111,B10001,B01111,B00000 }; // ä
byte CH_OE[8] = { B01010,B00000,B01110,B10001,B10001,B10001,B01110,B00000 }; // ö
byte CH_UE[8] = { B01010,B00000,B10001,B10001,B10001,B10001,B01111,B00000 }; // ü
byte CH_AE_CAP[8]={ B01010,B01110,B10001,B11111,B10001,B10001,B10001,B00000 }; // Ä
byte CH_OE_CAP[8]={ B01010,B01110,B10001,B10001,B11111,B10001,B01110,B00000 }; // Ö
byte CH_UE_CAP[8]={ B01010,B10001,B10001,B10001,B10001,B10001,B11111,B00000 }; // Ü
byte CH_SS[8]  = { B00000,B01110,B10001,B01110,B10001,B10001,B01110,B00000 }; // ß (vereinfachtes B)
byte CH_ARROW[8]={B00100,B00010,B11111,B00010,B00100,B00000,B00000,B00000};

// LCD Initialisieren
void initLCD() {
  lcd.begin(20, 4);
  lcd.backlight();         // Hintergrundbeleuchtung an
  lcd.clear();

  loadCustomChars();
}

void displayClear() {
  lcd.clear();
  if (debug) {
    Serial.println("Display: --- clear ---");
  }
}

void display(int position, String text, bool clear = false) {
  if (clear) {
    displayClear();
  }

  lcdPrintWrapped(position, text);
}

void displayClearLine(int row) {
  lcd.setCursor(0, row);         // Anfang der Zeile
  for (int i = 0; i < 20; i++) { // bei 20x4 Display → 20 Spalten
    lcd.print(" ");
  }
  lcd.setCursor(0, row);         // Cursor zurücksetzen
}

// Optional: sauberer 20x4-Umbruch
void lcdPrintWrapped(int rowStart, String text) {
  text = lcdize(text);

  if (debug) {
    Serial.println("Display: " + text);
  }

  int row = rowStart, col = 0;
  for (size_t i = 0; i < text.length() && row < 4; i++) {
    lcd.setCursor(col, row);
    lcd.write((uint8_t)text[i]);       // write statt print für Custom-Bytes
    if (++col >= 20) { col = 0; row++; }
  }
}

// UTF-8 → LCD-Zeichen (ersetzt Umlaute durch Custom-Slots, ° durch 223)
String lcdize(String s) {
  s.replace("ä", String((char)1));
  s.replace("ö", String((char)2));
  s.replace("ü", String((char)3));
  s.replace("Ä", String((char)4));
  s.replace("Ö", String((char)5));
  s.replace("Ü", String((char)6));
  s.replace("ß", String((char)7));
  return s;
}

// map: 0..7
void loadCustomChars() {
  lcd.createChar(1, CH_AE);
  lcd.createChar(2, CH_OE);
  lcd.createChar(3, CH_UE);
  lcd.createChar(4, CH_AE_CAP);
  lcd.createChar(5, CH_OE_CAP);
  lcd.createChar(6, CH_UE_CAP);
  lcd.createChar(7, CH_SS);
  lcd.createChar(0, CH_ARROW);   // unkritisch auf 0
}


