#pragma once
#include <Arduino.h>
#include "Config.h"

// ─────────────────────────────────────────────
//  CoinService.h  –  Münzzähler per Interrupt
//  Kein Rückgeld · Überzahlung verfällt
//  Guthaben verfällt nach TIMEOUT_PAYMENT_MS
// ─────────────────────────────────────────────

class CoinService {
public:
    void begin() {
        pinMode(COIN_INTERRUPT_PIN, INPUT_PULLUP);
        // Fallende Flanke = Impuls des Münzprüfers
        attachInterrupt(
            digitalPinToInterrupt(COIN_INTERRUPT_PIN),
            CoinService::onPulse,
            FALLING
        );
        reset();
    }

    // Muss in loop() aufgerufen werden – prüft Timeout
    void update() {
        if (_cents > 0 && _active) {
            if (millis() - _lastPulseTime > TIMEOUT_PAYMENT_MS) {
                DBG_PRINTLN(F("CoinService: Guthaben verfallen (Timeout)"));
                reset();
            }
        }
    }

    // Startet neue Zahlungssitzung
    void startSession() {
        reset();
        _active = true;
        DBG_PRINTLN(F("CoinService: Session gestartet"));
    }

    // Beendet die Sitzung und gibt eingezahlten Betrag zurück
    uint16_t endSession() {
        uint16_t paid = _cents;
        reset();
        return paid;
    }

    uint16_t getInsertedCents() const { return _cents; }

    bool hasEnough(uint16_t priceCents) const {
        return _cents >= priceCents;
    }

    bool isActive() const { return _active; }

    // Gibt true zurück, wenn seit letztem Update eine neue Münze erkannt wurde
    bool newCoinSinceLastCheck() {
        if (_newCoin) {
            _newCoin = false;
            return true;
        }
        return false;
    }

    void reset() {
        // Interrupts kurz sperren für atomaren Zugriff
        noInterrupts();
        _cents         = 0;
        _pulseCount    = 0;
        _newCoin       = false;
        _active        = false;
        _lastPulseTime = 0;
        interrupts();
    }

private:
    // ISR-Variablen: volatile
    static volatile uint16_t _cents;
    static volatile uint32_t _lastPulseTime;
    static volatile bool     _newCoin;
    static volatile uint16_t _pulseCount;

    bool _active = false;

    static void onPulse() {
        _pulseCount++;
        _cents += COIN_VALUE_PER_PULSE_CENT;
        _lastPulseTime = millis();
        _newCoin = true;
    }
};

// Statische Member definieren (genau einmal, in der .h da Header-only)
volatile uint16_t CoinService::_cents         = 0;
volatile uint32_t CoinService::_lastPulseTime = 0;
volatile bool     CoinService::_newCoin       = false;
volatile uint16_t CoinService::_pulseCount    = 0;
