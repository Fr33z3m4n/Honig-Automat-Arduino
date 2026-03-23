#pragma once
#include <Arduino.h>
#include "Config.h"
#include "DisplayService.h"
#include "KeypadService.h"
#include "CoinService.h"
#include "CompartmentService.h"
#include "PowerSaveService.h"
#include "TextUtils.h"

// ─────────────────────────────────────────────
//  CustomerFlowController.h  –  Kunden-Zustandsmaschine
//
//  Idle → SelectingCompartment → WaitingForPayment
//       → OpeningCompartment → ThankYou → Idle
//       \→ Error → Idle
//
//  Admin-Aktivierung über Taste 'D' in Idle
// ─────────────────────────────────────────────

enum class FlowState : uint8_t {
    IDLE,
    SELECTING_COMPARTMENT,
    WAITING_FOR_PAYMENT,
    OPENING_COMPARTMENT,
    THANK_YOU,
    ERROR
};

class CustomerFlowController {
public:
    CustomerFlowController(DisplayService&    display,
                           KeypadService&     keypad,
                           CoinService&       coins,
                           CompartmentService& compartments,
                           PowerSaveService&  powerSave)
        : _display(display), _keypad(keypad), _coins(coins),
          _compartments(compartments), _powerSave(powerSave) {}

    void begin() {
        transitionTo(FlowState::IDLE);
    }

    // Muss in loop() aufgerufen werden
    void update() {
        char key = _keypad.getKey();
        if (key != KeypadService::NO_KEY) {
            _powerSave.registerActivity();
        }

        // Münzeinwurf → Aktivität registrieren
        if (_coins.newCoinSinceLastCheck()) {
            _powerSave.registerActivity();
        }

        switch (_state) {
            case FlowState::IDLE:                  handleIdle(key);       break;
            case FlowState::SELECTING_COMPARTMENT: handleSelecting(key);  break;
            case FlowState::WAITING_FOR_PAYMENT:   handlePayment(key);    break;
            case FlowState::OPENING_COMPARTMENT:   handleOpening();       break;
            case FlowState::THANK_YOU:             handleThankYou(key);   break;
            case FlowState::ERROR:                 handleError(key);      break;
        }
    }

    FlowState getState() const { return _state; }

    // Gibt true zurück, wenn Admin-Taste gedrückt (wird von main.ino ausgewertet)
    bool adminRequested() {
        bool req = _adminRequest;
        _adminRequest = false;
        return req;
    }

private:
    DisplayService&    _display;
    KeypadService&     _keypad;
    CoinService&       _coins;
    CompartmentService& _compartments;
    PowerSaveService&  _powerSave;

    FlowState _state         = FlowState::IDLE;
    uint32_t  _stateEnterMs  = 0;
    uint8_t   _selectedIdx   = 0;   // 0-basiert
    uint16_t  _paidCents     = 0;
    bool      _adminRequest  = false;
    uint8_t   _selPage       = 0;

    // ── Zustandsübergang ──────────────────────

    void transitionTo(FlowState next) {
        _stateEnterMs = millis();
        _state = next;

        switch (next) {
            case FlowState::IDLE:
                _coins.reset();
                showIdle();
                break;

            case FlowState::SELECTING_COMPARTMENT:
                _selPage = 0;
                showSelectionPage();
                break;

            case FlowState::WAITING_FOR_PAYMENT:
                _coins.startSession();
                showPayment();
                break;

            case FlowState::OPENING_COMPARTMENT:
                _paidCents = _coins.endSession();
                _compartments.open(_selectedIdx);
                _display.showOpeningCompartment(_selectedIdx + 1);
                break;

            case FlowState::THANK_YOU:
                _display.showThankYou(_paidCents);
                break;

            case FlowState::ERROR:
                break; // Nachricht wird vom Aufrufer gesetzt
        }
    }

    // ── Idle ──────────────────────────────────

    void handleIdle(char key) {
        if (key == KeypadService::NO_KEY) {
            // Timeout → bleibt in Idle (nichts zu tun)
            return;
        }
        if (key == 'D') {
            // Admin anfordern
            _adminRequest = true;
            return;
        }
        // Beliebige andere Taste → Auswahl starten
        if (!_compartments.anyAvailable()) {
            _display.showError("Alle Faecher leer");
            _state = FlowState::ERROR;
            return;
        }
        transitionTo(FlowState::SELECTING_COMPARTMENT);
    }

    // ── Fachauswahl ───────────────────────────

    void handleSelecting(char key) {
        // Timeout
        if (millis() - _stateEnterMs > TIMEOUT_SELECTION_MS) {
            DBG_PRINTLN(F("Flow: Auswahl-Timeout"));
            transitionTo(FlowState::IDLE);
            return;
        }

        if (key == KeypadService::NO_KEY) return;

        if (key == '*') {
            transitionTo(FlowState::IDLE);
            return;
        }

        // Blättern mit '#'
        if (key == '#') {
            uint8_t pages = (NUM_COMPARTMENTS + 2) / 3;
            _selPage = (_selPage + 1) % pages;
            showSelectionPage();
            return;
        }

        // Zifferntaste → Fach wählen
        if (key >= '1' && key <= '0' + NUM_COMPARTMENTS) {
            uint8_t idx = key - '1';
            if (!_compartments.isAvailable(idx)) {
                _display.showError("Fach nicht verfuegbar");
                delay(1200);
                showSelectionPage();
                return;
            }
            _selectedIdx = idx;
            DBG_PRINT(F("Flow: Fach gewaehlt: "));
            DBG_PRINTLN(idx + 1);
            transitionTo(FlowState::WAITING_FOR_PAYMENT);
        }
    }

    // ── Zahlung ───────────────────────────────

    void handlePayment(char key) {
        // Timeout Guthaben verfällt
        if (millis() - _stateEnterMs > TIMEOUT_PAYMENT_MS) {
            DBG_PRINTLN(F("Flow: Zahlungs-Timeout"));
            _coins.reset();
            _display.showError("Timeout - Zahlung");
            delay(1500);
            transitionTo(FlowState::IDLE);
            return;
        }

        // Abbruch
        if (key == '*') {
            _coins.reset();
            transitionTo(FlowState::IDLE);
            return;
        }

        // Display aktualisieren wenn neue Münze
        uint16_t inserted = _coins.getInsertedCents();
        _display.showPaymentUpdate(inserted);

        // Bezahlt?
        uint16_t price = _compartments.getPrice(_selectedIdx);
        if (_coins.hasEnough(price)) {
            DBG_PRINTLN(F("Flow: Bezahlt!"));
            transitionTo(FlowState::OPENING_COMPARTMENT);
        }
    }

    // ── Öffnen ────────────────────────────────

    void handleOpening() {
        _compartments.update(); // Aktor-Pulse steuern

        if (!_compartments.isOpening()) {
            transitionTo(FlowState::THANK_YOU);
        }
    }

    // ── Danke ─────────────────────────────────

    void handleThankYou(char key) {
        bool timeout = (millis() - _stateEnterMs > TIMEOUT_THANK_YOU_MS);
        if (timeout || key != KeypadService::NO_KEY) {
            transitionTo(FlowState::IDLE);
        }
    }

    // ── Fehler ────────────────────────────────

    void handleError(char key) {
        if (key != KeypadService::NO_KEY) {
            transitionTo(FlowState::IDLE);
        }
    }

    // ── Display-Helfer ────────────────────────

    void showIdle() {
        _display.showIdle();
    }

    void showSelectionPage() {
        String prices[NUM_COMPARTMENTS];
        bool   available[NUM_COMPARTMENTS];
        for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
            prices[i]    = TextUtils::centsToString(_compartments.getPrice(i));
            available[i] = _compartments.isAvailable(i);
        }
        _display.showSelectionPage(_selPage, NUM_COMPARTMENTS, prices, available);
    }

    void showPayment() {
        _display.showPayment(
            _selectedIdx + 1,
            _compartments.getPrice(_selectedIdx),
            0
        );
    }
};
