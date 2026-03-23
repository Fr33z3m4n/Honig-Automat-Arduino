# 🍯 Honig-Verkaufsautomat – Arduino Software Spezifikation

## 🎯 Zielsetzung
Für einen Honig-Produzenten sollen Verkaufsautomaten entwickelt werden, die Produkte (Honig, Zubehör) automatisiert verkaufen.

Ein Arduino Mega übernimmt:
- Steuerung der Fächer
- Zahlungslogik
- Benutzerführung über LCD + Keypad
- Admin-Funktionen zur Wartung

Die Software soll **modular, wartbar und konfigurierbar** sein.

---

## ⚙️ Hardware

- Arduino Mega
- LCD Display 20x4 (Crystal)
- 4x4 Keypad  
  `(1,2,3,A),(4,5,6,B),(7,8,9,C),(*,0,#,D)`
- Münzzähler (unterschiedliche Impulse je Münzwert)
- Relais / Magnet / Servo pro Fach

---

## 🧱 Architektur (Zielstruktur)

- main.ino / main.cpp
- Config.h
- DisplayService
- KeypadService
- CoinService
- CompartmentService
- AdminMenuController
- CustomerFlowController
- PowerSaveService
- TextUtils

---

## 🔄 Zustandsmaschine

Idle, SelectingCompartment, WaitingForPayment, OpeningCompartment, ThankYou, AdminLogin, AdminMenu, Error

---

## 📦 Fachlogik

- Fach: Nummer, Preis (Cent), Pin, Status
- Status: BEFUELLT, LEER, DEFEKT, GESPERRT
- Nach Öffnung → LEER
- Alle Fächer befüllt → Status zurücksetzen

---

## 💰 Zahlungslogik

- Nur Münzen
- Kein Rückgeld
- Überzahlung verfällt
- Guthaben verfällt nach 60 Sekunden
- Preise als Integer (Cent)

---

## ⏱️ Timeouts

- Auswahl: 10 Sek
- Zahlung: 60 Sek
- Abschluss: 20 Sek
- Rückkehr: 30 Sek
- Admin Sperre: 10 Min

---

## 🔐 Admin-Menü

Konfig-Modus mit Fachverwaltung, Preise, Beenden

---

## 💾 Persistenz

EEPROM:
- Preise
- Fachstatus
- Admin-Code

---

## 🖥️ Anzeige

Start, Auswahl, Zahlung, Abschluss wie definiert.

---

## ⚡ Energiesparmodus

LCD dimmen/aus nach Inaktivität

---

## 🧪 Qualität

- Kein delay()
- millis()
- Modular
- Debug optional

---

## 🧠 Copilot Verhalten

- Ganze Dateien erzeugen
- Klassen nutzen
- Architektur einhalten
