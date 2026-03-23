# 📖 Anleitung – Honig-Automat

Konfiguration, Bedienung und Wartung des Honig-Verkaufsautomaten.

---

## Inhaltsverzeichnis

1. [Erstkonfiguration (Config.h)](#1-erstkonfiguration-configh)
2. [EEPROM & Erststart](#2-eeprom--erststart)
3. [User-Flow – Schritt für Schritt](#3-user-flow--schritt-für-schritt)
4. [Admin-Menü](#4-admin-menü)
5. [Timeouts im Überblick](#5-timeouts-im-überblick)
6. [Energiesparmodus](#6-energiesparmodus)
7. [Debug-Ausgabe](#7-debug-ausgabe)
8. [Häufige Fragen](#8-häufige-fragen)

---

## 1. Erstkonfiguration (`Config.h`)

Alle Einstellungen befinden sich in einer einzigen Datei:  
`Honig_Automat/src/Config.h`

Nach jeder Änderung muss der Sketch neu kompiliert und auf den Arduino geflasht werden.

---

### 1.1 Anzahl der Fächer

```cpp
constexpr uint8_t NUM_COMPARTMENTS = 6;
```

Maximale Anzahl der Verkaufsfächer. Muss mit den Arrays darunter übereinstimmen.

---

### 1.2 Fach-Pins (Aktoren)

```cpp
constexpr uint8_t COMPARTMENT_PINS[NUM_COMPARTMENTS] = {22, 24, 26, 28, 30, 32};
```

Jeder Wert ist der Arduino-Pin, der den Aktor (Relais / Magnetschloss / Servo) für das jeweilige Fach steuert.  
Index 0 = Fach 1, Index 1 = Fach 2, usw.

> ⚡ Die Pins werden kurz auf HIGH gezogen (`COMPARTMENT_OPEN_DURATION_MS`) und dann wieder auf LOW.

---

### 1.3 Standard-Preise

```cpp
constexpr uint16_t DEFAULT_PRICES[NUM_COMPARTMENTS] = {500, 500, 500, 800, 800, 150};
```

Preise in **Cent**. Diese Werte werden beim **allerersten Start** ins EEPROM geschrieben.  
Danach gelten die im Admin-Menü gespeicherten Preise. Zum Zurücksetzen → EEPROM löschen (siehe Abschnitt 2).

| Fach | Beispielpreis | Bedeutung |
|------|--------------|-----------|
| 1–3  | 500          | 5,00 EUR  |
| 4–5  | 800          | 8,00 EUR  |
| 6    | 150          | 1,50 EUR  |

---

### 1.4 Keypad-Pins

```cpp
constexpr uint8_t KEYPAD_ROW_PINS[4] = {36, 37, 38, 39};
constexpr uint8_t KEYPAD_COL_PINS[4] = {40, 41, 42, 43};
```

Zeilen- und Spaltenpins des 4×4-Matrix-Keypads.

---

### 1.5 LCD-Pins

```cpp
constexpr uint8_t LCD_RS = 8;
constexpr uint8_t LCD_EN = 9;
constexpr uint8_t LCD_D4 = 4;
constexpr uint8_t LCD_D5 = 5;
constexpr uint8_t LCD_D6 = 6;
constexpr uint8_t LCD_D7 = 7;
constexpr uint8_t LCD_BACKLIGHT_PIN = 10;  // PWM-Pin!
```

`LCD_BACKLIGHT_PIN` muss ein **PWM-fähiger Pin** sein (Mega: 2–13, 44–46).

---

### 1.6 Münzzähler

```cpp
constexpr uint8_t COIN_INTERRUPT_PIN     = 2;
constexpr uint8_t COIN_VALUE_PER_PULSE_CENT = 10;
```

- `COIN_INTERRUPT_PIN`: Muss ein **Interrupt-fähiger Pin** sein (Mega: 2, 3, 18, 19, 20, 21).
- `COIN_VALUE_PER_PULSE_CENT`: Wie viel Cent pro Impuls des Münzprüfers gutgeschrieben werden.  
  Beispiel: Münzprüfer sendet 5 Impulse für 50 Cent → Wert = `10`.

---

### 1.7 Öffnungsdauer Aktor

```cpp
constexpr uint16_t COMPARTMENT_OPEN_DURATION_MS = 500;
```

Wie lange der Aktor-Pin HIGH bleibt (in Millisekunden). Bei Magneten typisch 300–800 ms.

---

### 1.8 Admin-Code

```cpp
#define ADMIN_CODE        "1234"
constexpr uint8_t ADMIN_CODE_LENGTH = 4;
```

> 🔐 **Den PIN hier ändern, neu kompilieren und flashen** – fertig.  
> Nur Ziffern (`0`–`9`), genau `ADMIN_CODE_LENGTH` Stellen.  
> Der Code wird **nicht** im EEPROM gespeichert, sondern ist fest im Programm hinterlegt.

---

## 2. EEPROM & Erststart

Beim ersten Start (oder nach einem EEPROM-Reset) erkennt der Automat anhand eines **Magic-Bytes**, dass noch keine Daten vorhanden sind, und schreibt die Standardwerte aus `Config.h`:

| EEPROM-Inhalt | Adresse | Beschreibung |
|---|---|---|
| Preise (uint16_t × 6) | 0–11 | 2 Byte pro Fach |
| Fachstatus (uint8_t × 6) | 12–13 | 1 Byte pro Fach |
| Magic-Byte | 14 | Wert `0xA7` = initialisiert |

### EEPROM zurücksetzen

Um die Preise auf die `DEFAULT_PRICES` aus `Config.h` zurückzusetzen:

```cpp
// Einmalig in setup() einfügen, flashen, dann wieder entfernen:
EEPROM.write(EEPROM_ADDR_MAGIC, 0x00);
```

---

## 3. User-Flow – Schritt für Schritt

```
┌─────────────────────────────────────────────┐
│              IDLE-SCREEN                    │
│         *** HONIG-AUTOMAT ***               │
│           Frischer Honig                    │
│           direkt vom Imker                  │
│           Taste druecken...                 │
└────────────────────┬────────────────────────┘
                     │ Beliebige Zifferntaste
                     ▼
┌─────────────────────────────────────────────┐
│           FACHAUSWAHL                       │
│  Fach  Preis    Stat                        │
│    1   5,00 EUR  OK                         │
│    2   5,00 EUR  OK                         │
│    3   5,00 EUR  ---  (leer)                │
│  [#] Weiter   [*] Abbruch                   │
└────────────────────┬────────────────────────┘
                     │ Zifferntaste (z. B. "1")
                     ▼
┌─────────────────────────────────────────────┐
│           ZAHLUNGSAUFFORDERUNG              │
│  Fach 1: 5,00 EUR                           │
│  Bitte Muenzen                              │
│  einwerfen:                                 │
│  Eingeworfen: 0,00 EUR                      │
└────────────────────┬────────────────────────┘
                     │ Münzen einwerfen
                     │ (Anzeige aktualisiert sich)
                     │ Sobald Betrag ≥ Preis:
                     ▼
┌─────────────────────────────────────────────┐
│           FACH ÖFFNET                       │
│         Bitte warten...                     │
│              Fach 1                         │
│          wird geoeffnet                     │
└────────────────────┬────────────────────────┘
                     │ Aktor öffnet ~500 ms
                     ▼
┌─────────────────────────────────────────────┐
│              DANKE-SCREEN                   │
│            Vielen Dank!                     │
│            Guten Appetit!                   │
│        Bezahlt: 5,00 EUR                    │
│        * Honig vom Imker *                  │
└────────────────────┬────────────────────────┘
                     │ Taste oder 20 s Timeout
                     ▼
                  IDLE-SCREEN
```

### Abbruchmöglichkeiten

| Zustand | Abbruch per |
|---|---|
| Fachauswahl | `*` → zurück zu Idle |
| Zahlung | `*` → Guthaben verfällt, zurück zu Idle |
| Zahlung | 60 s ohne Münzeinwurf → automatischer Abbruch |

### Wichtige Regeln

- **Kein Rückgeld** – Überzahlung verfällt.
- **Guthaben verfällt nach 60 Sekunden** ohne weiteren Münzeinwurf.
- Ist ein Fach **leer / defekt / gesperrt**, erscheint `---` in der Fachauswahl.

---

## 4. Admin-Menü

### Zugang

Im **Idle-Screen** die Taste **`D`** drücken, dann den 4-stelligen PIN eingeben und mit **`#`** bestätigen.

```
-- ADMIN MODUS --
PIN eingeben:
****
# = Bestaetigen
```

> Nach **10 Minuten Inaktivität** wird das Admin-Menü automatisch gesperrt.  
> Nach einer Fehleingabe kann es sofort neu versucht werden.

---

### Menüstruktur

```
=== ADMIN MENUE ===
1  Fachstatus
2  Preise aendern
3  Befuellen
4  Beenden
```

---

### 4.1 Fachstatus anzeigen (`1`)

Zeigt je 3 Fächer pro Seite mit Status und Preis:

```
Fach Status    Preis
  1  VOLL   5,00 EUR
  2  LEER   5,00 EUR
  3  DFKT   5,00 EUR
```

| Status | Bedeutung |
|---|---|
| `VOLL` | Fach befüllt, verkaufbar |
| `LEER` | Fach wurde geleert (nach Verkauf) |
| `DFKT` | Fach defekt (manuell gesetzt) |
| `GSPR` | Fach gesperrt (manuell gesetzt) |

**`#`** blättert zur nächsten Seite, **`*`** zurück zum Menü.

---

### 4.2 Preise ändern (`2`)

Fächer werden nacheinander angezeigt. Preis in **Cent** eingeben:

```
Preis Fach 1
Aktuell: 5,00 EUR
Neu (Cent): 450_
# Speich.  * Abbruch
```

- Ziffern eingeben (max. 5 Stellen, z. B. `450` = 4,50 EUR)
- **`#`** speichert → nächstes Fach
- **`*`** überspringt das Fach ohne Änderung
- **`D`** beendet die Preisbearbeitung

Gespeicherte Preise werden sofort ins EEPROM geschrieben und bleiben dauerhaft erhalten.

---

### 4.3 Alle Fächer befüllen (`3`)

Setzt alle Fächer (außer DEFEKT und GESPERRT) auf den Status **BEFUELLT** zurück.

```
Alle befuellen?

# = Ja  * = Nein
```

---

### 4.4 Admin-Menü beenden (`4` oder `*`)

Kehrt zum normalen Idle-Screen zurück.

---

## 5. Timeouts im Überblick

| Situation | Timeout | Aktion |
|---|---|---|
| Fachauswahl | 10 Sekunden | Zurück zu Idle |
| Zahlungsvorgang | 60 Sekunden | Guthaben verfällt, zurück zu Idle |
| Danke-Screen | 20 Sekunden | Automatisch zurück zu Idle |
| Rückkehr allgemein | 30 Sekunden | – |
| Admin-Inaktivität | 10 Minuten | Admin gesperrt, zurück zu Idle |

---

## 6. Energiesparmodus

Der LCD-Hintergrundbeleuchtung wird nach Inaktivität automatisch gedimmt bzw. abgeschaltet:

| Zeit ohne Aktivität | Aktion |
|---|---|
| 60 Sekunden | LCD gedimmt (Helligkeit reduziert) |
| 3 Minuten | LCD vollständig aus |

Jede Taste oder jeder Münzeinwurf reaktiviert das Display sofort auf volle Helligkeit.

Die Schwellwerte sind in `Config.h` einstellbar:

```cpp
constexpr uint32_t TIMEOUT_POWERSAVE_MS = 60000UL;   // 60 s → Dimm
constexpr uint32_t TIMEOUT_POWEROFF_MS  = 180000UL;  // 3 min → Aus
constexpr uint8_t  LCD_BRIGHTNESS_NORMAL = 255;
constexpr uint8_t  LCD_BRIGHTNESS_DIM    = 60;
constexpr uint8_t  LCD_BRIGHTNESS_OFF    = 0;
```

---

## 7. Debug-Ausgabe

Während der Entwicklung / Inbetriebnahme gibt der Automat Statusmeldungen über den seriellen Monitor aus (**9600 Baud**):

```
Honig-Automat startet...
EEPROM: Erstinitialisierung
CoinService: Session gestartet
Flow: Fach gewaehlt: 2
Flow: Bezahlt!
Oeffne Fach 2
Fach 2 → LEER
```

**Debug deaktivieren** (für den Produktionsbetrieb) in `Config.h`:

```cpp
// Diese Zeile auskommentieren:
// #define DEBUG_ENABLED
```

---

## 8. Häufige Fragen

**Fach öffnet nicht?**  
→ `COMPARTMENT_OPEN_DURATION_MS` erhöhen (z. B. auf `800`). Pin-Nummer in `COMPARTMENT_PINS` prüfen.

**Münzen werden nicht erkannt?**  
→ `COIN_INTERRUPT_PIN` muss Interrupt-fähig sein. `COIN_VALUE_PER_PULSE_CENT` an Münzprüfer-Einstellung anpassen.

**Admin-PIN vergessen?**  
→ `ADMIN_CODE` in `Config.h` ändern und neu flashen.

**Preise stimmen nicht nach dem Flashen?**  
→ Das EEPROM speichert die zuletzt eingestellten Preise. Magic-Byte löschen (Abschnitt 2), um auf `DEFAULT_PRICES` zurückzusetzen.

**LCD zeigt nichts an?**  
→ Kontrast-Poti am LCD prüfen. `LCD_BACKLIGHT_PIN` auf PWM-fähigen Pin legen.
