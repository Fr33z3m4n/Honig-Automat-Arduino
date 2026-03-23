#pragma once

// ─────────────────────────────────────────────
//  Config.h  –  Globale Konfiguration
//  Honig-Verkaufsautomat · Arduino Mega
// ─────────────────────────────────────────────

// ── Fächer ────────────────────────────────────
constexpr uint8_t NUM_COMPARTMENTS = 6;

// Pins der Fach-Aktoren (Relais / Servo / Magnet)
// Index 0 = Fach 1, Index 1 = Fach 2, …
constexpr uint8_t COMPARTMENT_PINS[NUM_COMPARTMENTS] = {22, 24, 26, 28, 30, 32};

// Standard-Preise in Cent (überschreibbar per EEPROM)
constexpr uint16_t DEFAULT_PRICES[NUM_COMPARTMENTS] = {500, 500, 500, 800, 800, 150};

// ── Keypad ────────────────────────────────────
constexpr uint8_t KEYPAD_ROWS = 4;
constexpr uint8_t KEYPAD_COLS = 4;
constexpr uint8_t KEYPAD_ROW_PINS[KEYPAD_ROWS] = {36, 37, 38, 39};
constexpr uint8_t KEYPAD_COL_PINS[KEYPAD_COLS] = {40, 41, 42, 43};

// ── LCD ───────────────────────────────────────
constexpr uint8_t LCD_RS   = 8;
constexpr uint8_t LCD_EN   = 9;
constexpr uint8_t LCD_D4   = 4;
constexpr uint8_t LCD_D5   = 5;
constexpr uint8_t LCD_D6   = 6;
constexpr uint8_t LCD_D7   = 7;
constexpr uint8_t LCD_COLS = 20;
constexpr uint8_t LCD_ROWS = 4;
// Pin für Hintergrundbeleuchtung (PWM)
constexpr uint8_t LCD_BACKLIGHT_PIN = 10;

// ── Münzzähler ────────────────────────────────
// Interrupt-fähiger Pin (Mega: 2, 3, 18, 19, 20, 21)
constexpr uint8_t COIN_INTERRUPT_PIN = 2;
// Münzwert je Impuls in Cent
constexpr uint8_t COIN_VALUE_PER_PULSE_CENT = 10;

// ── Aktor-Steuerung ───────────────────────────
// Wie lange der Aktor-Pin HIGH bleibt (ms)
constexpr uint16_t COMPARTMENT_OPEN_DURATION_MS = 500;

// ── Timeouts (alle in Millisekunden) ─────────
constexpr uint32_t TIMEOUT_IDLE_MS          = 30000UL;   // 30 s  → Rückkehr zu Idle
constexpr uint32_t TIMEOUT_SELECTION_MS     = 10000UL;   // 10 s  → Auswahl
constexpr uint32_t TIMEOUT_PAYMENT_MS       = 60000UL;   // 60 s  → Zahlung
constexpr uint32_t TIMEOUT_THANK_YOU_MS     = 20000UL;   // 20 s  → Abschluss
constexpr uint32_t TIMEOUT_ADMIN_LOCK_MS    = 600000UL;  // 10 min → Admin-Sperre
constexpr uint32_t TIMEOUT_POWERSAVE_MS     = 60000UL;   // 60 s  → LCD-Dimm
constexpr uint32_t TIMEOUT_POWEROFF_MS      = 180000UL;  // 3 min → LCD aus

// ── Energiesparmodus ──────────────────────────
constexpr uint8_t LCD_BRIGHTNESS_NORMAL = 255;
constexpr uint8_t LCD_BRIGHTNESS_DIM    = 60;
constexpr uint8_t LCD_BRIGHTNESS_OFF    = 0;

// ── Admin ─────────────────────────────────────
// ➜ Hier den gewünschten Admin-PIN eintragen und neu flashen.
//   Nur Ziffern (0–9), genau ADMIN_CODE_LENGTH Stellen.
#define ADMIN_CODE        "1234"
constexpr uint8_t ADMIN_CODE_LENGTH = 4;

// ── EEPROM-Adressen ───────────────────────────
// uint16_t Preise: 2 Byte × NUM_COMPARTMENTS
constexpr uint16_t EEPROM_ADDR_PRICES        = 0;    // 0 – 11
// uint8_t  Status: 1 Byte × NUM_COMPARTMENTS
constexpr uint16_t EEPROM_ADDR_STATUS        = 12;   // 12 – 17
// uint8_t  Magic-Byte – erkennt erstmalige Initialisierung
constexpr uint16_t EEPROM_ADDR_MAGIC         = 14;
constexpr uint8_t  EEPROM_MAGIC_VALUE        = 0xA7;

// ── Debug ─────────────────────────────────────
// Kommentiere die folgende Zeile aus, um Debug-Ausgaben zu deaktivieren
#define DEBUG_ENABLED
#ifdef DEBUG_ENABLED
  #define DBG_PRINT(x)   Serial.print(x)
  #define DBG_PRINTLN(x) Serial.println(x)
#else
  #define DBG_PRINT(x)
  #define DBG_PRINTLN(x)
#endif
