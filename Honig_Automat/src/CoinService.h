#pragma once
#include <Arduino.h>
#include "Config.h"

// ─────────────────────────────────────────────
//  CoinService.h  –  Münzzähler per Polling
//  Funktioniert auf jedem Digital-Pin (kein Interrupt nötig)
//  Kein Rückgeld · Überzahlung verfällt
//  Guthaben verfällt nach TIMEOUT_PAYMENT_MS
// ─────────────────────────────────────────────

class CoinService {
public:
    void begin() {
        pinMode(COIN_PIN, INPUT_PULLUP);
        reset();
    }

    // Muss in loop() aufgerufen werden – erkennt Impulse und prüft Timeout
    void update() {
        detectPulse();

        if (_cents > 0 && _active) {
            if (millis() - _lastPulseTime > TIMEOUT_PAYMENT_MS) {
                DBG_PRINTLN(F("CoinService: Guthaben verfallen (Timeout)"));
                reset();
            }
        }
    }

    void startSession() {
        reset();
        _active = true;
        DBG_PRINTLN(F("CoinService: Session gestartet"));
    }

    uint16_t endSession() {
        uint16_t paid = _cents;
        reset();
        return paid;
    }

    uint16_t getInsertedCents() const { return _cents; }

    bool hasEnough(uint16_t priceCents) const { return _cents >= priceCents; }

    bool isActive() const { return _active; }

    bool newCoinSinceLastCheck() {
        if (_newCoin) {
            _newCoin = false;
            return true;
        }
        return false;
    }

    void reset() {
        _cents        = 0;
        _newCoin      = false;
        _active       = false;
        _lastPulseTime = 0;
        _lastPinState  = HIGH;  // Pull-up → Ruhezustand HIGH
    }

private:
    uint16_t _cents         = 0;
    uint32_t _lastPulseTime = 0;
    bool     _newCoin       = false;
    bool     _active        = false;
    bool     _lastPinState  = HIGH;

    // Flanken-Erkennung: fallende Flanke = Impuls
    void detectPulse() {
        bool currentState = digitalRead(COIN_PIN);

        if (_lastPinState == HIGH && currentState == LOW) {
            // Fallende Flanke erkannt → Münzimpuls
            _cents        += COIN_VALUE_PER_PULSE_CENT;
            _lastPulseTime = millis();
            _newCoin       = true;
            DBG_PRINT(F("CoinService: Impuls, Gesamt: "));
            DBG_PRINTLN(_cents);
        }

        _lastPinState = currentState;
    }
};
