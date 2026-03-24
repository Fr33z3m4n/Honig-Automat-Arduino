#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Config.h"
#include "TextUtils.h"

// ─────────────────────────────────────────────
//  DisplayService.h  –  LCD-Abstraktion (I2C)
//  Screens: Idle, Auswahl, Zahlung, ThankYou,
//           Error, AdminLogin, AdminMenu
// ─────────────────────────────────────────────

class DisplayService {
public:
    DisplayService()
        : _lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS)
    {}

    void begin() {
        _lcd.init();
        _lcd.backlight();
        _backlightOn = true;
        _lcd.noCursor();
    }

    void setBacklightOn(bool on) {
        if (on) { _lcd.backlight();   _backlightOn = true;  }
        else    { _lcd.noBacklight(); _backlightOn = false; }
    }

    bool isBacklightOn() const { return _backlightOn; }

    void clear() { _lcd.clear(); }

    // ── Screens ───────────────────────────────

    void showIdle() {
        _lcd.clear();
        _lcd.setCursor(0, 0); _lcd.print(TextUtils::center("* HONIG-AUTOMAT *"));
        _lcd.setCursor(0, 1); _lcd.print(TextUtils::center("Bereit."));
        _lcd.setCursor(0, 2); _lcd.print(TextUtils::center("Fachnummer eingeben"));
        _lcd.setCursor(0, 3); _lcd.print(TextUtils::center("* = Abbruch"));
    }

    // numStr: "1" bis "6", priceStr: "5,00 EUR"
    void showSelection(uint8_t numCompartments, const String prices[], const bool available[]) {
        _lcd.clear();
        _lcd.setCursor(0, 0); _lcd.print(TextUtils::padRight("Fach  Preis   Verf."));
        _lcd.setCursor(0, 1); _lcd.print(TextUtils::repeat('-', LCD_COLS));

        // Bis zu 2 Fächer gleichzeitig anzeigen (Seiten-Logik via showSelectionPage)
        // Diese Methode zeigt Zeile 2+3 mit Fächern. Aufruf aus CustomerFlowController.
        (void)numCompartments; (void)prices; (void)available;
    }

    // Zeigt eine Seite der Fachauswahl (je 2 Fächer)
    void showSelectionPage(uint8_t page, uint8_t numCompartments,
                           const String prices[], const bool available[]) {
        _lcd.clear();
        _lcd.setCursor(0, 0); _lcd.print(TextUtils::padRight("Fach  Preis    Stat"));

        for (uint8_t row = 0; row < 3; row++) {
            uint8_t idx = page * 3 + row;
            if (idx >= numCompartments) break;
            String line = "  ";
            line += String(idx + 1);
            line += "   ";
            String priceStr = prices[idx];
            line += TextUtils::padRight(priceStr, 9);
            line += available[idx] ? "OK " : "---";
            _lcd.setCursor(0, row + 1);
            _lcd.print(TextUtils::padRight(line));
        }
    }

    void showPayment(uint8_t compartmentNum, uint16_t priceCents, uint16_t insertedCents) {
        _lcd.clear();
        String line1 = "Fach " + String(compartmentNum) + ": " + TextUtils::centsToString(priceCents);
        _lcd.setCursor(0, 0); _lcd.print(TextUtils::padRight(line1));
        _lcd.setCursor(0, 1); _lcd.print(TextUtils::padRight("Bitte Muenzen"));
        _lcd.setCursor(0, 2); _lcd.print(TextUtils::padRight("einwerfen:"));
        String inserted = "Eingeworfen: " + TextUtils::centsToString(insertedCents);
        _lcd.setCursor(0, 3); _lcd.print(TextUtils::padRight(inserted));
    }

    void showPaymentUpdate(uint16_t insertedCents) {
        String inserted = "Eingeworfen: " + TextUtils::centsToString(insertedCents);
        _lcd.setCursor(0, 3);
        _lcd.print(TextUtils::padRight(inserted));
    }

    void showOpeningCompartment(uint8_t compartmentNum) {
        _lcd.clear();
        _lcd.setCursor(0, 0); _lcd.print(TextUtils::center("Bitte warten..."));
        _lcd.setCursor(0, 1); _lcd.print(TextUtils::center("Fach " + String(compartmentNum)));
        _lcd.setCursor(0, 2); _lcd.print(TextUtils::center("wird geoeffnet"));
    }

    void showThankYou(uint16_t paidCents) {
        _lcd.clear();
        _lcd.setCursor(0, 0); _lcd.print(TextUtils::center("Vielen Dank!"));
        _lcd.setCursor(0, 1); _lcd.print(TextUtils::center("Guten Appetit!"));
        _lcd.setCursor(0, 2); _lcd.print(TextUtils::center("Bezahlt: " + TextUtils::centsToString(paidCents)));
        _lcd.setCursor(0, 3); _lcd.print(TextUtils::center("* Honig vom Imker *"));
    }

    void showError(const String& msg) {
        _lcd.clear();
        _lcd.setCursor(0, 0); _lcd.print(TextUtils::center("!!! FEHLER !!!"));
        _lcd.setCursor(0, 1); _lcd.print(TextUtils::center(msg));
        _lcd.setCursor(0, 3); _lcd.print(TextUtils::center("Taste zum Fortfahren"));
    }

    void showAdminLogin(const String& enteredMask) {
        _lcd.clear();
        _lcd.setCursor(0, 0); _lcd.print(TextUtils::center("-- ADMIN MODUS --"));
        _lcd.setCursor(0, 1); _lcd.print(TextUtils::center("PIN eingeben:"));
        _lcd.setCursor(0, 2); _lcd.print(TextUtils::center(enteredMask));
        _lcd.setCursor(0, 3); _lcd.print(TextUtils::center("# = Bestaetigen"));
    }

    // Hauptmenü Admin
    void showAdminMenu(uint8_t selectedItem) {
        const char* items[] = {"1 Fachstatus", "2 Preise", "3 Befuellen", "4 Beenden"};
        _lcd.clear();
        _lcd.setCursor(0, 0); _lcd.print(TextUtils::padRight("=== ADMIN MENUE ==="));
        for (uint8_t i = 0; i < 4; i++) {
            _lcd.setCursor(0, i);
            if (i == 0) {
                _lcd.print(TextUtils::padRight("=== ADMIN MENUE ==="));
            } else {
                String line = (i - 1 == selectedItem) ? ">" : " ";
                line += items[i - 1];
                _lcd.print(TextUtils::padRight(line));
            }
        }
    }

    // Fachstatus-Übersicht
    void showAdminCompartmentStatus(uint8_t page, uint8_t numCompartments,
                                    const String statuses[], const String prices[]) {
        _lcd.clear();
        _lcd.setCursor(0, 0); _lcd.print(TextUtils::padRight("Fach Status    Preis"));
        for (uint8_t row = 0; row < 3; row++) {
            uint8_t idx = page * 3 + row;
            if (idx >= numCompartments) break;
            String line = "  " + String(idx + 1) + "  ";
            line += TextUtils::padRight(statuses[idx], 7);
            line += prices[idx];
            _lcd.setCursor(0, row + 1);
            _lcd.print(TextUtils::padRight(line));
        }
    }

    // Preis-Edit-Screen
    void showAdminPriceEdit(uint8_t compartmentNum, uint16_t currentCents, const String& input) {
        _lcd.clear();
        _lcd.setCursor(0, 0); _lcd.print(TextUtils::padRight("Preis Fach " + String(compartmentNum)));
        _lcd.setCursor(0, 1); _lcd.print(TextUtils::padRight("Aktuell: " + TextUtils::centsToString(currentCents)));
        _lcd.setCursor(0, 2); _lcd.print(TextUtils::padRight("Neu (Cent): " + input + "_"));
        _lcd.setCursor(0, 3); _lcd.print(TextUtils::padRight("# Speich. A=Del * Skip"));
    }

    // Allgemeine Nachricht (2 Zeilen)
    void showMessage(const String& line1, const String& line2 = "",
                     const String& line3 = "", const String& line4 = "") {
        _lcd.clear();
        _lcd.setCursor(0, 0); _lcd.print(TextUtils::padRight(line1));
        _lcd.setCursor(0, 1); _lcd.print(TextUtils::padRight(line2));
        _lcd.setCursor(0, 2); _lcd.print(TextUtils::padRight(line3));
        _lcd.setCursor(0, 3); _lcd.print(TextUtils::padRight(line4));
    }

private:
    LiquidCrystal_I2C _lcd;
    bool              _backlightOn = true;
};
