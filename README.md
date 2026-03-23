# 🍯 Honig-Automat-Arduino

Arduino-Mega-Software für einen selbstgebauten Honig-Verkaufsautomaten.  
Der Automat verkauft Honig und Imkerei-Zubehör vollautomatisch gegen Münzeinwurf – ohne Rückgeld, ohne externe Abhängigkeiten.

---

## 🎯 Zielsetzung

Ein kleiner, robuster Verkaufsautomat direkt vom Imker:

- Kunden wählen ein Fach per Zifferntaste, werfen Münzen ein und erhalten ihr Produkt automatisch
- Preise, Fachstatus und Admin-Code sind dauerhaft im EEPROM bzw. direkt im Quellcode konfigurierbar
- Kein PC, kein WLAN, kein Cloud-Dienst notwendig – vollständig autark

---

## ⚙️ Hardware

| Komponente | Details |
|---|---|
| Mikrocontroller | Arduino Mega 2560 |
| Display | LCD 20×4 (HD44780-kompatibel, 4-Bit-Modus) |
| Eingabe | 4×4-Matrix-Keypad |
| Münzerkennung | Münzprüfer mit Impulsausgang (1 Impuls = konfigurierbarer Cent-Wert) |
| Fach-Aktoren | Relais / Magnetschloss / Servo – 1 Pin pro Fach |
| Speicher | Internes EEPROM des Arduino Mega (mind. 512 Byte) |
| Stromversorgung | 12 V DC (extern) |

### Keypad-Belegung

```
┌───┬───┬───┬───┐
│ 1 │ 2 │ 3 │ A │
├───┼───┼───┼───┤
│ 4 │ 5 │ 6 │ B │
├───┼───┼───┼───┤
│ 7 │ 8 │ 9 │ C │
├───┼───┼───┼───┤
│ * │ 0 │ # │ D │
└───┴───┴───┴───┘
```

| Taste | Funktion |
|---|---|
| `1`–`6` | Fach auswählen |
| `#` | Bestätigen / Weiterblättern |
| `*` | Abbrechen / Zurück |
| `D` | Admin-Menü öffnen |

---

## 🗂️ Projektstruktur

```
Arduino_Programm/                  ← Git-Root
├── README.md
├── ANLEITUNG.md
├── .gitignore
├── .github/
│   ├── honig-automat-copilot.md   ← Copilot-Spezifikation
│   └── main/                      ← Älterer Referenzcode
└── Honig_Automat/                 ← Arduino-Sketch-Ordner
    ├── Honig_Automat.ino           ← Einstiegspunkt (setup / loop)
    └── src/
        ├── Config.h                ← ⚙️ Alle Einstellungen hier
        ├── TextUtils.h             ← LCD-Hilfsklasse
        ├── DisplayService.h        ← LCD-Abstraktion (alle Screens)
        ├── KeypadService.h         ← Nicht-blockierendes Keypad-Polling
        ├── CoinService.h           ← Münzzähler (Interrupt-basiert)
        ├── CompartmentService.h    ← Fachverwaltung + EEPROM-Persistenz
        ├── PowerSaveService.h      ← LCD-Dimmen / Abschalten
        ├── AdminMenuController.h   ← Admin-Menü (PIN, Preise, Status)
        └── CustomerFlowController.h← Kunden-Zustandsmaschine
```

---

## 🔄 Zustandsmaschine

```
              Taste gedrückt
  ┌──────────────────────────────────────────┐
  ▼                                          │
IDLE ──[Ziffer]──► SELECTING_COMPARTMENT ───┘
  │                        │
  │                   Fach gewählt
  │                        ▼
  │              WAITING_FOR_PAYMENT
  │                        │
  │                  Betrag erreicht
  │                        ▼
  │              OPENING_COMPARTMENT
  │                        │
  │                   Fach offen
  │                        ▼
  │                   THANK_YOU
  │                        │
  └────────────────────────┘  (Timeout / Taste)

  IDLE ──[D]──► AdminMenuController (eigene Zustandsmaschine)
```

---

## 🛠️ Tech-Stack

| Bereich | Technologie |
|---|---|
| Sprache | C++11 (Arduino-Dialekt) |
| Framework | Arduino Core für AVR |
| Bibliotheken | `LiquidCrystal` (built-in), `EEPROM` (built-in) |
| Timing | `millis()` – kein blockierendes `delay()` im Normalbetrieb |
| ISR | `attachInterrupt()` für Münzimpulse |
| Persistenz | EEPROM (Preise + Fachstatus) |
| IDE | Arduino IDE 2.x oder PlatformIO |

---

## 🚀 Schnellstart

1. Repository klonen
   ```bash
   git clone https://github.com/Fr33z3m4n/Honig-Automat-Arduino.git
   ```
2. `Honig_Automat/Honig_Automat.ino` in der Arduino IDE öffnen
3. Pins und Preise in `src/Config.h` anpassen
4. Board: **Arduino Mega 2560** auswählen
5. Kompilieren & hochladen

➡️ Detaillierte Konfigurations- und Bedienungsanleitung: **[ANLEITUNG.md](ANLEITUNG.md)**

---

## 📄 Lizenz

Dieses Projekt ist für den privaten und gewerblichen Einsatz durch den Auftraggeber freigegeben.
