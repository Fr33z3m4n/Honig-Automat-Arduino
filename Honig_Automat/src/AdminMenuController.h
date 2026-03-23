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
    LOCKED,          // Kein Admin-Modus
    ENTERING_PIN,    // PIN-Eingabe
    MAIN_MENU,       // Hauptmenü
    STATUS_VIEW,     // Fachstatus anzeigen
    PRICE_EDIT,      // Preis bearbeiten
    REFILL_CONFIRM   // Befüll-Bestätigung
};

class AdminMenuController {
public:
    AdminMenuController(DisplayService& display, KeypadService& keypad,
                        CompartmentService& compartments)
        : _display(display), _keypad(keypad), _compartments(compartments) {}

    void begin() {
        _state = AdminState::LOCKED;
        // Admin-Code kommt direkt aus Config.h (kein EEPROM)
        strncpy(_adminCode, ADMIN_CODE, ADMIN_CODE_LENGTH + 1);
    }

    // Gibt true zurück, wenn Admin-Modus aktiv
    bool isActive() const {
        return _state != AdminState::LOCKED;
    }

    // Aktiviert PIN-Eingabe (aufgerufen wenn 'D' gedrückt)
    void activate() {
        if (_locked) {
            DBG_PRINTLN(F("Admin: Gesperrt!"));
            _display.showError("Admin gesperrt!");
            return;
        }
        _state     = AdminState::ENTERING_PIN;
        _pinInput  = "";
        _display.showAdminLogin("");
        DBG_PRINTLN(F("Admin: PIN-Eingabe"));
    }

    // Muss in loop() aufgerufen werden
    void update() {
        if (_state == AdminState::LOCKED) return;

        // Timeout-Prüfung
        if (millis() - _lastActivity > TIMEOUT_ADMIN_LOCK_MS) {
            leave("Timeout – Admin gesperrt");
            _locked     = true;
            _lockStart  = millis();
            return;
        }

        // Sperre aufheben?
        if (_locked && millis() - _lockStart > TIMEOUT_ADMIN_LOCK_MS) {
            _locked = false;
        }

        char key = _keypad.getKey();
        if (key == KeypadService::NO_KEY) return;

        _lastActivity = millis();

        switch (_state) {
            case AdminState::ENTERING_PIN:    handlePin(key);    break;
            case AdminState::MAIN_MENU:       handleMenu(key);   break;
            case AdminState::STATUS_VIEW:     handleStatus(key); break;
            case AdminState::PRICE_EDIT:      handlePrice(key);  break;
            case AdminState::REFILL_CONFIRM:  handleRefill(key); break;
            default: break;
        }
    }

private:
    DisplayService&    _display;
    KeypadService&     _keypad;
    CompartmentService& _compartments;

    AdminState _state        = AdminState::LOCKED;
    String     _pinInput     = "";
    char       _adminCode[ADMIN_CODE_LENGTH + 1];

    uint8_t    _menuSel      = 0;   // 0-3
    uint8_t    _statusPage   = 0;
    uint8_t    _editIdx      = 0;
    String     _priceInput   = "";

    bool       _locked       = false;
    uint32_t   _lockStart    = 0;
    uint32_t   _lastActivity = 0;

    // ── PIN-Eingabe ───────────────────────────

    void handlePin(char key) {
        if (key >= '0' && key <= '9') {
            if (_pinInput.length() < ADMIN_CODE_LENGTH) {
                _pinInput += key;
                String mask = TextUtils::repeat('*', _pinInput.length());
                _display.showAdminLogin(mask);
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
            _state    = AdminState::MAIN_MENU;
            _menuSel  = 0;
            refreshMenu();
        } else {
            DBG_PRINTLN(F("Admin: Falscher PIN"));
            _pinInput = "";
            _display.showAdminLogin("Falscher PIN!");
            delay(1500); // einmaliger delay nur im Fehlerfall
            _display.showAdminLogin("");
        }
    }

    // ── Hauptmenü ─────────────────────────────

    void handleMenu(char key) {
        switch (key) {
            case '1':
                _state = AdminState::STATUS_VIEW;
                _statusPage = 0;
                refreshStatus();
                break;
            case '2':
                _state = AdminState::PRICE_EDIT;
                _editIdx    = 0;
                _priceInput = "";
                refreshPriceEdit();
                break;
            case '3':
                _state = AdminState::REFILL_CONFIRM;
                _display.showMessage("Alle befuellen?", "", "# = Ja  * = Nein");
                break;
            case '4':
            case '*':
                leave("Admin beendet");
                break;
        }
    }

    void refreshMenu() {
        // Statisches Menü, keine Selektion per Cursor nötig
        _display.showMessage(
            "=== ADMIN MENUE ===",
            "1 Fachstatus",
            "2 Preise aendern",
            "4 Beenden  3 Fuell."
        );
    }

    // ── Fachstatus ────────────────────────────

    void handleStatus(char key) {
        if (key == '#' || key == 'D') {
            _statusPage = (_statusPage + 1) % ((NUM_COMPARTMENTS + 2) / 3);
            refreshStatus();
        } else if (key == '*') {
            _state = AdminState::MAIN_MENU;
            refreshMenu();
        }
    }

    void refreshStatus() {
        String statuses[NUM_COMPARTMENTS];
        String prices[NUM_COMPARTMENTS];
        for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
            statuses[i] = _compartments.getStatusString(i);
            prices[i]   = TextUtils::centsToString(_compartments.getPrice(i));
        }
        _display.showAdminCompartmentStatus(_statusPage, NUM_COMPARTMENTS, statuses, prices);
    }

    // ── Preis bearbeiten ──────────────────────

    void handlePrice(char key) {
        if (key >= '0' && key <= '9') {
            if (_priceInput.length() < 5) {
                _priceInput += key;
                refreshPriceEdit();
            }
        } else if (key == '#') {
            savePriceEdit();
        } else if (key == '*') {
            // Abbruch / nächstes Fach
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

    // ── Befüllen ──────────────────────────────

    void handleRefill(char key) {
        if (key == '#') {
            _compartments.refillAll();
            _display.showMessage("Alle Faecher", "als BEFUELLT markiert", "", "# weiter");
            delay(1500);
            _state = AdminState::MAIN_MENU;
            refreshMenu();
        } else if (key == '*') {
            _state = AdminState::MAIN_MENU;
            refreshMenu();
        }
    }

    // ── Hilfsmethoden ─────────────────────────

    void leave(const String& msg = "") {
        if (msg.length() > 0) {
            DBG_PRINTLN(msg);
        }
        _state    = AdminState::LOCKED;
        _pinInput = "";
    }



};
