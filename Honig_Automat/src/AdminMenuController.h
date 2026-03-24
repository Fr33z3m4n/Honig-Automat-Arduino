#pragma once
#include <Arduino.h>
#include "Config.h"
#include "DisplayService.h"
#include "KeypadService.h"
#include "CompartmentService.h"
#include "TextUtils.h"

// ─────────────────────────────────────────────
//  AdminMenuController.h  –  Admin-Modus
//  Zugang per PIN · Fachverwaltung · Preise
//  Automatische Sperre nach TIMEOUT_ADMIN_LOCK_MS
// ─────────────────────────────────────────────

enum class AdminState : uint8_t {
    LOCKED,              // Kein Admin-Modus
    ENTERING_PIN,        // PIN-Eingabe
    MAIN_MENU,           // Hauptmenü
    FACH_MENU,           // Fachverwaltung-Untermenü
    SINGLE_REFILL,       // Einzelnes Fach befüllen
    OPEN_ALL_CONFIRM,    // Alle Fächer öffnen – Bestätigung
    REFILL_ALL_CONFIRM,  // Alle Fächer befüllt markieren – Bestätigung
    PRICE_EDIT           // Preis bearbeiten
};

class AdminMenuController {
public:
    AdminMenuController(DisplayService& display, KeypadService& keypad,
                        CompartmentService& compartments)
        : _display(display), _keypad(keypad), _compartments(compartments) {}

    void begin() {
        _state = AdminState::LOCKED;
        strncpy(_adminCode, ADMIN_CODE, ADMIN_CODE_LENGTH + 1);
    }

    bool isActive() const { return _state != AdminState::LOCKED; }

    void activate() {
        if (_locked) {
            DBG_PRINTLN(F("Admin: Gesperrt!"));
            _display.showError("Admin gesperrt!");
            return;
        }
        _state    = AdminState::ENTERING_PIN;
        _pinInput = "";
        _display.showAdminLogin("");
        DBG_PRINTLN(F("Admin: PIN-Eingabe"));
    }

    void update() {
        if (_state == AdminState::LOCKED) return;

        if (millis() - _lastActivity > TIMEOUT_ADMIN_LOCK_MS) {
            leave("Timeout - Admin gesperrt");
            _locked    = true;
            _lockStart = millis();
            return;
        }
        if (_locked && millis() - _lockStart > TIMEOUT_ADMIN_LOCK_MS) {
            _locked = false;
        }

        char key = _keypad.getKey();
        if (key == KeypadService::NO_KEY) return;
        _lastActivity = millis();

        switch (_state) {
            case AdminState::ENTERING_PIN:       handlePin(key);          break;
            case AdminState::MAIN_MENU:          handleMenu(key);         break;
            case AdminState::FACH_MENU:          handleFachMenu(key);     break;
            case AdminState::SINGLE_REFILL:      handleSingleRefill(key); break;
            case AdminState::OPEN_ALL_CONFIRM:   handleOpenAll(key);      break;
            case AdminState::REFILL_ALL_CONFIRM: handleRefillAll(key);    break;
            case AdminState::PRICE_EDIT:         handlePrice(key);        break;
            default: break;
        }
    }

private:
    DisplayService&     _display;
    KeypadService&      _keypad;
    CompartmentService& _compartments;

    AdminState _state        = AdminState::LOCKED;
    String     _pinInput     = "";
    char       _adminCode[ADMIN_CODE_LENGTH + 1];

    uint8_t    _editIdx      = 0;
    String     _priceInput   = "";

    bool       _locked       = false;
    uint32_t   _lockStart    = 0;
    uint32_t   _lastActivity = 0;

    // ── PIN ───────────────────────────────────

    void handlePin(char key) {
        if (key >= '0' && key <= '9') {
            if (_pinInput.length() < ADMIN_CODE_LENGTH) {
                _pinInput += key;
                _display.showAdminLogin(TextUtils::repeat('*', _pinInput.length()));
            }
        } else if (key == '#') {
            checkPin();
        } else if (key == '*') {
            leave();
        }
    }

    void checkPin() {
        if (_pinInput == String(_adminCode)) {
            DBG_PRINTLN(F("Admin: PIN korrekt"));
            _state = AdminState::MAIN_MENU;
            refreshMenu();
        } else {
            DBG_PRINTLN(F("Admin: Falscher PIN"));
            _pinInput = "";
            _display.showAdminLogin("Falscher PIN!");
            delay(1500);
            _display.showAdminLogin("");
        }
    }

    // ── Hauptmenü ─────────────────────────────

    void handleMenu(char key) {
        switch (key) {
            case '1':
                _state = AdminState::FACH_MENU;
                refreshFachMenu();
                break;
            case '2':
                _state      = AdminState::PRICE_EDIT;
                _editIdx    = 0;
                _priceInput = "";
                refreshPriceEdit();
                break;
            case '4':
            case '*':
                leave("Admin beendet");
                break;
        }
    }

    void refreshMenu() {
        _display.showMessage(
            "=== ADMIN MENUE ===",
            "1 Fachverwaltung",
            "2 Preise aendern",
            "4 Beenden"
        );
    }

    // ── Fachverwaltung-Untermenü ──────────────

    void handleFachMenu(char key) {
        switch (key) {
            case '1':
                _state   = AdminState::SINGLE_REFILL;
                _editIdx = 0;
                refreshSingleRefill();
                break;
            case '2':
                _state = AdminState::OPEN_ALL_CONFIRM;
                _display.showMessage(
                    "Alle Faecher oeffnen?",
                    "",
                    "# = Ja   * = Nein"
                );
                break;
            case '3':
                _state = AdminState::REFILL_ALL_CONFIRM;
                _display.showMessage(
                    "Alle als BEFUELLT",
                    "markieren?",
                    "# = Ja   * = Nein"
                );
                break;
            case '*':
            case 'D':
                _state = AdminState::MAIN_MENU;
                refreshMenu();
                break;
        }
    }

    void refreshFachMenu() {
        _display.showMessage(
            "== FACHVERWALTUNG ==",
            "1 Einzeln befuellen",
            "2 Alle Faecher oeffnen",
            "3 Alle bef.mark. * Zur."
        );
    }

    // ── Einzelnes Fach befüllen ───────────────

    void handleSingleRefill(char key) {
        if (key >= '1' && key <= '0' + NUM_COMPARTMENTS) {
            uint8_t idx = key - '1';
            _compartments.refill(idx);
            _display.showMessage(
                "Fach " + String(idx + 1) + " befuellt!",
                "Status: BEFUELLT",
                "",
                "# weiter * Menue"
            );
            DBG_PRINT(F("Admin: Fach ")); DBG_PRINT(idx + 1); DBG_PRINTLN(F(" -> BEFUELLT"));
        } else if (key == '#') {
            refreshSingleRefill();
        } else if (key == '*' || key == 'D') {
            _state = AdminState::FACH_MENU;
            refreshFachMenu();
        }
    }

    void refreshSingleRefill() {
        // Zeige Fächerstatus kompakt (bis 6 Fächer in 3 Zeilen)
        String line1 = "", line2 = "", line3 = "";
        for (uint8_t i = 0; i < NUM_COMPARTMENTS && i < 2; i++)
            line1 += String(i + 1) + ":" + _compartments.getStatusString(i) + " ";
        for (uint8_t i = 2; i < NUM_COMPARTMENTS && i < 4; i++)
            line2 += String(i + 1) + ":" + _compartments.getStatusString(i) + " ";
        for (uint8_t i = 4; i < NUM_COMPARTMENTS && i < 6; i++)
            line3 += String(i + 1) + ":" + _compartments.getStatusString(i) + " ";

        _display.showMessage(
            "Fach waehlen (1-" + String(NUM_COMPARTMENTS) + "):",
            line1,
            line2,
            line3.length() > 0 ? line3 : "* = Zurueck"
        );
    }

    // ── Alle Fächer öffnen ────────────────────

    void handleOpenAll(char key) {
        if (key == '#') {
            _display.showMessage("Oeffne alle Faecher", "Bitte warten...");
            for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
                _compartments.open(i);
                delay(COMPARTMENT_OPEN_DURATION_MS + 100);
            }
            _display.showMessage("Alle Faecher", "geoeffnet!", "", "* Zurueck");
            DBG_PRINTLN(F("Admin: Alle Faecher geoeffnet"));
            delay(1500);
            _state = AdminState::FACH_MENU;
            refreshFachMenu();
        } else if (key == '*') {
            _state = AdminState::FACH_MENU;
            refreshFachMenu();
        }
    }

    // ── Alle befüllt markieren ────────────────

    void handleRefillAll(char key) {
        if (key == '#') {
            _compartments.refillAll();
            _display.showMessage("Alle Faecher", "als BEFUELLT markiert");
            DBG_PRINTLN(F("Admin: Alle Faecher BEFUELLT"));
            delay(1500);
            _state = AdminState::FACH_MENU;
            refreshFachMenu();
        } else if (key == '*') {
            _state = AdminState::FACH_MENU;
            refreshFachMenu();
        }
    }

    // ── Preis bearbeiten ──────────────────────

    void handlePrice(char key) {
        if (key >= '0' && key <= '9') {
            if (_priceInput.length() < 5) {
                _priceInput += key;
                refreshPriceEdit();
            }
        } else if (key == 'A') {
            if (_priceInput.length() > 0) {
                _priceInput.remove(_priceInput.length() - 1);
                refreshPriceEdit();
            }
        } else if (key == '#') {
            savePriceEdit();
        } else if (key == '*') {
            _editIdx = (_editIdx + 1) % NUM_COMPARTMENTS;
            _priceInput = "";
            refreshPriceEdit();
        } else if (key == 'D') {
            _state = AdminState::MAIN_MENU;
            refreshMenu();
        }
    }

    void refreshPriceEdit() {
        _display.showAdminPriceEdit(
            _editIdx + 1,
            _compartments.getPrice(_editIdx),
            _priceInput
        );
    }

    void savePriceEdit() {
        if (_priceInput.length() > 0) {
            uint16_t cents = static_cast<uint16_t>(_priceInput.toInt());
            if (cents > 0 && cents <= 9999) {
                _compartments.setPrice(_editIdx, cents);
                _display.showMessage("Preis gespeichert!", TextUtils::centsToString(cents));
                delay(1200);
            }
        }
        _editIdx    = (_editIdx + 1) % NUM_COMPARTMENTS;
        _priceInput = "";
        refreshPriceEdit();
    }

    // ── Hilfsmethoden ─────────────────────────

    void leave(const String& msg = "") {
        if (msg.length() > 0) DBG_PRINTLN(msg);
        _state    = AdminState::LOCKED;
        _pinInput = "";
    }
};

