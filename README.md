# 🍯 Honig-Automat-Arduino

Arduino-Mega-Software für einen selbstgebauten Honig-Verkaufsautomaten.  
Der Automat verkauft Honig und Imkerei-Zubehör vollautomatisch gegen Münzeinwurf – ohne Rückgeld, ohne externe Abhängigkeiten.

---

## 🎯 Zielsetzung

Ein kleiner, robuster Verkaufsautomat direkt vom Imker:

- Kunden wählen ein Fach per Zifferntaste, werfen Münzen ein und erhalten ihr Produkt automatisch
- Preise und Fachstatus werden dauerhaft im EEPROM gespeichert
- Admin-Code ist fest im Quellcode hinterlegt (kein EEPROM)
- Kein PC, kein WLAN, kein Cloud-Dienst notwendig – vollständig autark

---

## ⚙️ Hardware

| Komponente | Details |
|---|---|
| Mikrocontroller | Arduino Mega 2560 |
| Display | LCD 20×4 mit **I2C-Adapter** (Adresse `0x27`) |
| Eingabe | 4×4-Matrix-Keypad |
| Münzerkennung | Münzprüfer mit Impulsausgang (1 Impuls = konfigurierbarer Cent-Wert) |
| Fach-Aktoren | Relais / Magnetschloss / Servo – 1 Pin pro Fach |
| Speicher | Internes EEPROM des Arduino Mega |
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
| `A` | Letzte Eingabe löschen (Backspace, im Admin) |
| `D` | Admin-Menü öffnen |

➡️ Vollständige Verdrahtung: **[HARDWARE.md](HARDWARE.md)**

---

## 🗂️ Projektstruktur

```
Arduino_Programm/                  ← Git-Root
├── README.md                      ← Diese Datei
├── ANLEITUNG.md                   ← Bedienungs- und Konfigurationsanleitung
├── HARDWARE.md                    ← Technische Verdrahtungsanleitung
├── .gitignore
├── .github/
│   ├── honig-automat-copilot.md   ← Copilot-Spezifikation
│   └── main/                      ← Älterer Referenzcode
└── Honig_Automat/                 ← Arduino-Sketch-Ordner
    ├── Honig_Automat.ino           ← Einstiegspunkt (setup / loop)
    └── src/
        ├── Config.h                ← ⚙️ Alle Einstellungen hier
        ├── TextUtils.h             ← LCD-Hilfsklasse
        ├── DisplayService.h        ← LCD I2C-Abstraktion (alle Screens)
        ├── KeypadService.h         ← Nicht-blockierendes Keypad-Polling
        ├── CoinService.h           ← Münzzähler (Interrupt-basiert)
        ├── CompartmentService.h    ← Fachverwaltung + EEPROM-Persistenz
        ├── PowerSaveService.h      ← LCD Ein/Aus nach Inaktivität
        ├── AdminMenuController.h   ← Admin-Menü (PIN, Fachverwaltung, Preise)
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

  IDLE ──[D]──► AdminMenuController
                  └─ FACH_MENU
                       ├─ Einzeln befüllen
                       ├─ Alle öffnen
                       └─ Alle befüllt markieren
                  └─ PRICE_EDIT
```

---

## 🛠️ Tech-Stack

| Bereich | Technologie |
|---|---|
| Sprache | C++11 (Arduino-Dialekt) |
| Framework | Arduino Core für AVR |
| Bibliotheken | `LiquidCrystal_I2C`, `EEPROM` (built-in) |
| Timing | `millis()` – kein blockierendes `delay()` im Normalbetrieb |
| ISR | `attachInterrupt()` für Münzimpulse |
| Persistenz | EEPROM (Preise + Fachstatus); Admin-Code fest im Code |
| Upload-Tool | arduino-cli oder Arduino IDE 2.x |

---

## 🚀 Schnellstart

1. Repository klonen
   ```bash
   git clone https://github.com/Fr33z3m4n/Honig-Automat-Arduino.git
   ```
2. `Honig_Automat/Honig_Automat.ino` in der Arduino IDE öffnen
3. Pins, Preise und Admin-Code in `src/Config.h` anpassen
4. Board: **Arduino Mega 2560** auswählen, Port wählen
5. Kompilieren & hochladen

**Oder per arduino-cli:**
```bash
arduino-cli compile --fqbn arduino:avr:mega Honig_Automat
arduino-cli upload  --fqbn arduino:avr:mega --port COM3 Honig_Automat
```

➡️ Konfiguration & Bedienung: **[ANLEITUNG.md](ANLEITUNG.md)**  
➡️ Verdrahtung & Hardware: **[HARDWARE.md](HARDWARE.md)**

---

## 📄 Lizenz

Dieses Projekt ist für den privaten und gewerblichen Einsatz durch den Auftraggeber freigegeben.
