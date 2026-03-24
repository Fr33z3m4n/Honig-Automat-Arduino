# 🔌 Hardware-Anleitung – Honig-Automat

Technische Verdrahtung aller Komponenten am Arduino Mega 2560.

---

## Inhaltsverzeichnis

1. [Arduino Mega 2560 – Pinübersicht](#1-arduino-mega-2560--pinübersicht)
2. [LCD Display (I2C)](#2-lcd-display-i2c)
3. [Keypad 4×4](#3-keypad-44)
4. [Münzzähler](#4-münzzähler)
5. [Fach-Aktoren (Relais)](#5-fach-aktoren-relais)
6. [Stromversorgung](#6-stromversorgung)
7. [Gesamtübersicht Pinbelegung](#7-gesamtübersicht-pinbelegung)

---

## 1. Arduino Mega 2560 – Pinübersicht

```
                    ┌──────────────────────────────┐
                    │      Arduino Mega 2560        │
                    │                              │
     LCD I2C ───── │  SDA = Pin 20                │
     LCD I2C ───── │  SCL = Pin 21                │
                    │                              │
  Münzzähler ───── │  Pin  2  (INT0, Interrupt)   │
                    │                              │
   Keypad R1 ───── │  Pin 46                      │
   Keypad C1 ───── │  Pin 47                      │
   Keypad R2 ───── │  Pin 48                      │
   Keypad C2 ───── │  Pin 49                      │
   Keypad R3 ───── │  Pin 50                      │
   Keypad C3 ───── │  Pin 51                      │
   Keypad R4 ───── │  Pin 52                      │
   Keypad C4 ───── │  Pin 53                      │
                    │                              │
     Fach 1  ───── │  Pin 22                      │
     Fach 2  ───── │  Pin 24                      │
     Fach 3  ───── │  Pin 26                      │
     Fach 4  ───── │  Pin 28                      │
     Fach 5  ───── │  Pin 30                      │
     Fach 6  ───── │  Pin 32                      │
                    └──────────────────────────────┘
```

---

## 2. LCD Display (I2C)

Das Display ist ein **20×4 Zeichen LCD** mit aufgelötetem **I2C-Adapter-Modul** (PCF8574).  
Der I2C-Adapter reduziert die Kabelanzahl auf 4 Leitungen.

### Verdrahtung

| I2C-Adapter Pin | Arduino Mega Pin | Beschreibung |
|---|---|---|
| `GND` | `GND` | Masse |
| `VCC` | `5V` | Versorgungsspannung |
| `SDA` | `Pin 20` | I2C Datenleitung |
| `SCL` | `Pin 21` | I2C Taktleitung |

```
I2C-Adapter               Arduino Mega
┌─────────┐               ┌──────────┐
│ GND     │───────────────│ GND      │
│ VCC     │───────────────│ 5V       │
│ SDA     │───────────────│ Pin 20   │
│ SCL     │───────────────│ Pin 21   │
└─────────┘               └──────────┘
```

### I2C-Adresse ermitteln

Die meisten I2C-LCD-Adapter verwenden `0x27` oder `0x3F`.  
Falls das Display leer bleibt, in `Config.h` die Adresse wechseln:

```cpp
constexpr uint8_t LCD_I2C_ADDR = 0x27;  // oder 0x3F
```

**I2C-Scanner-Sketch** zum Ermitteln der Adresse:

```cpp
#include <Wire.h>
void setup() {
    Serial.begin(9600);
    Wire.begin();
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.print("I2C Geraet gefunden: 0x");
            Serial.println(addr, HEX);
        }
    }
}
void loop() {}
```

### Kontrast einstellen

Am I2C-Adapter befindet sich ein blaues **Potentiometer**. Damit wird der Kontrast des Displays eingestellt. Falls die Anzeige zu hell oder zu dunkel ist → Poti drehen bis Text gut lesbar ist.

---

## 3. Keypad 4×4

Das Keypad hat **8 Anschlüsse** (4 Reihen + 4 Spalten).  
Die Drähte wechseln sich alternierend ab: **Reihe, Spalte, Reihe, Spalte …**

### Verdrahtung (von links nach rechts am Keypad-Stecker)

```
Keypad-Stecker (von links):
Pin 1  ──── Arduino Pin 46  (Reihe 1)
Pin 2  ──── Arduino Pin 47  (Spalte 1)
Pin 3  ──── Arduino Pin 48  (Reihe 2)
Pin 4  ──── Arduino Pin 49  (Spalte 2)
Pin 5  ──── Arduino Pin 50  (Reihe 3)
Pin 6  ──── Arduino Pin 51  (Spalte 3)
Pin 7  ──── Arduino Pin 52  (Reihe 4)
Pin 8  ──── Arduino Pin 53  (Spalte 4)
```

### Keypad-Matrix

```
         Spalte 1  Spalte 2  Spalte 3  Spalte 4
         (Pin 47)  (Pin 49)  (Pin 51)  (Pin 53)
          │         │         │         │
Reihe 1 ──┼─────────┼─────────┼─────────┤
(Pin 46)  │    1    │    2    │    3    │    A    │
Reihe 2 ──┼─────────┼─────────┼─────────┤
(Pin 48)  │    4    │    5    │    6    │    B    │
Reihe 3 ──┼─────────┼─────────┼─────────┤
(Pin 50)  │    7    │    8    │    9    │    C    │
Reihe 4 ──┼─────────┼─────────┼─────────┤
(Pin 52)  │    *    │    0    │    #    │    D    │
          └─────────┴─────────┴─────────┘
```

### Funktionstasten

| Taste | Funktion im Normalbetrieb | Funktion im Admin |
|---|---|---|
| `1`–`6` | Fach wählen | Menüpunkt / Fach wählen |
| `#` | Bestätigen / Blättern | Bestätigen / Speichern |
| `*` | Abbrechen / Zurück | Überspringen / Zurück |
| `A` | – | Letzte Ziffer löschen (Backspace) |
| `D` | Admin-Menü öffnen | Zurück / Beenden |

### Hinweis zur Entprellung

Die Entprellung erfolgt softwareseitig mit `delayMicroseconds(100)` im Scan-Zyklus.  
Kein zusätzlicher Hardware-Kondensator notwendig.

---

## 4. Münzzähler

Der Münzprüfer gibt bei jeder akzeptierten Münze **Impulse** aus.  
Die Anzahl der Impulse pro Münze ist am Münzprüfer einstellbar.

### Verdrahtung

```
Münzprüfer                Arduino Mega
┌──────────┐              ┌──────────┐
│ GND      │──────────────│ GND      │
│ VCC      │──────────────│ 5V       │
│ Impuls   │──────────────│ Pin 2    │  ← Interrupt-Pin INT0
└──────────┘              └──────────┘
```

> ⚠️ **Pin 2 muss verwendet werden** (oder einer der anderen Interrupt-Pins: 3, 18, 19, 20, 21).  
> Der Eingang wird mit `INPUT_PULLUP` konfiguriert – fallende Flanke = Impuls.

### Konfiguration in `Config.h`

```cpp
constexpr uint8_t COIN_INTERRUPT_PIN        = 2;
constexpr uint8_t COIN_VALUE_PER_PULSE_CENT = 10;  // Cent pro Impuls
```

**Beispiel:** Münzprüfer sendet für eine 50-Cent-Münze 5 Impulse → `COIN_VALUE_PER_PULSE_CENT = 10`.

---

## 5. Fach-Aktoren (Relais)

Jedes Fach wird über einen eigenen Digital-Pin gesteuert, der ein Relais ansteuert.  
Das Relais schaltet die eigentliche Last (Magnet, Motor, Servo).

### Verdrahtung (pro Fach)

```
Arduino Mega              Relaismodul               Last (Magnet o.ä.)
┌──────────┐              ┌──────────┐              ┌──────────┐
│ Pin 22   │──────────────│ IN1      │              │          │
│ GND      │──────────────│ GND      │    ┌─────────│ +        │
│ 5V       │──────────────│ VCC      │    │         │ -        │────┐
└──────────┘              │ COM      │────┘         └──────────┘    │
                          │ NO       │──────────────────────────────┘
                          └──────────┘         (Normally Open)
```

### Pin-Zuordnung

| Fach | Arduino-Pin | `COMPARTMENT_PINS` Index |
|---|---|---|
| Fach 1 | Pin 22 | Index 0 |
| Fach 2 | Pin 24 | Index 1 |
| Fach 3 | Pin 26 | Index 2 |
| Fach 4 | Pin 28 | Index 3 |
| Fach 5 | Pin 30 | Index 4 |
| Fach 6 | Pin 32 | Index 5 |

### Öffnungsdauer anpassen

```cpp
constexpr uint16_t COMPARTMENT_OPEN_DURATION_MS = 500;  // ms
```

Der Pin wird für diese Dauer auf HIGH gezogen und dann automatisch wieder auf LOW gesetzt.  
Typische Werte: **300–800 ms** je nach Aktor.

> ⚠️ Bei induktiven Lasten (Magneten) eine **Freilaufdiode** parallel zum Magneten einbauen!

---

## 6. Stromversorgung

| Komponente | Spannung | Strom (ca.) |
|---|---|---|
| Arduino Mega | 7–12 V DC (Barrel Jack) oder 5 V (USB) | 80 mA |
| LCD + I2C-Adapter | 5 V (vom Arduino) | 20 mA |
| Keypad | 5 V (vom Arduino, passiv) | < 1 mA |
| Relaismodul (je Kanal) | 5 V (Spule) | 70–90 mA |
| Münzprüfer | 12 V DC (separat) | 150 mA |
| Aktoren / Magnete | 12 V DC (separat) | je nach Typ |

> 💡 Empfehlung: Arduino über **externes Netzteil (12 V / 2 A)** versorgen.  
> Relais und Aktoren über eine **separate 12-V-Quelle** – nicht über den Arduino!

---

## 7. Gesamtübersicht Pinbelegung

| Arduino-Pin | Funktion | Richtung |
|---|---|---|
| `2` | Münzzähler Impuls (INT0) | INPUT_PULLUP |
| `20` | LCD SDA (I2C) | I2C |
| `21` | LCD SCL (I2C) | I2C |
| `22` | Aktor Fach 1 | OUTPUT |
| `24` | Aktor Fach 2 | OUTPUT |
| `26` | Aktor Fach 3 | OUTPUT |
| `28` | Aktor Fach 4 | OUTPUT |
| `30` | Aktor Fach 5 | OUTPUT |
| `32` | Aktor Fach 6 | OUTPUT |
| `46` | Keypad Reihe 1 | OUTPUT |
| `47` | Keypad Spalte 1 | INPUT_PULLUP |
| `48` | Keypad Reihe 2 | OUTPUT |
| `49` | Keypad Spalte 2 | INPUT_PULLUP |
| `50` | Keypad Reihe 3 | OUTPUT |
| `51` | Keypad Spalte 3 | INPUT_PULLUP |
| `52` | Keypad Reihe 4 | OUTPUT |
| `53` | Keypad Spalte 4 | INPUT_PULLUP |
