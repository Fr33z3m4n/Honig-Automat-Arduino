#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include "Config.h"

// ─────────────────────────────────────────────
//  CompartmentService.h  –  Fachverwaltung
//  Status: BEFUELLT, LEER, DEFEKT, GESPERRT
//  Persistenz: EEPROM (Preise + Status)
// ─────────────────────────────────────────────

enum class CompartmentStatus : uint8_t {
    BEFUELLT  = 0,
    LEER      = 1,
    DEFEKT    = 2,
    GESPERRT  = 3
};

struct Compartment {
    uint8_t           pin;
    uint16_t          priceCents;
    CompartmentStatus status;
};

class CompartmentService {
public:
    void begin() {
        for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
            _compartments[i].pin = COMPARTMENT_PINS[i];
            pinMode(_compartments[i].pin, OUTPUT);
            digitalWrite(_compartments[i].pin, LOW);
        }
        loadFromEEPROM();
    }

    // ── Status ────────────────────────────────

    CompartmentStatus getStatus(uint8_t idx) const {
        return _compartments[idx].status;
    }

    String getStatusString(uint8_t idx) const {
        switch (_compartments[idx].status) {
            case CompartmentStatus::BEFUELLT: return "VOLL";
            case CompartmentStatus::LEER:     return "LEER";
            case CompartmentStatus::DEFEKT:   return "DFKT";
            case CompartmentStatus::GESPERRT: return "GSPR";
        }
        return "????";
    }

    bool isAvailable(uint8_t idx) const {
        return _compartments[idx].status == CompartmentStatus::BEFUELLT;
    }

    void setStatus(uint8_t idx, CompartmentStatus st) {
        _compartments[idx].status = st;
        saveStatusToEEPROM();
    }

    // Markiert nach Öffnung als LEER
    void markEmpty(uint8_t idx) {
        setStatus(idx, CompartmentStatus::LEER);
        DBG_PRINT(F("Fach "));
        DBG_PRINT(idx + 1);
        DBG_PRINTLN(F(" → LEER"));
    }

    // Alle Fächer befüllen
    void refillAll() {
        for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
            if (_compartments[i].status != CompartmentStatus::DEFEKT &&
                _compartments[i].status != CompartmentStatus::GESPERRT) {
                _compartments[i].status = CompartmentStatus::BEFUELLT;
            }
        }
        saveStatusToEEPROM();
        DBG_PRINTLN(F("Alle Faecher befuellt"));
    }

    // Einzelnes Fach befüllen
    void refill(uint8_t idx) {
        _compartments[idx].status = CompartmentStatus::BEFUELLT;
        saveStatusToEEPROM();
    }

    // ── Preise ────────────────────────────────

    uint16_t getPrice(uint8_t idx) const {
        return _compartments[idx].priceCents;
    }

    void setPrice(uint8_t idx, uint16_t cents) {
        _compartments[idx].priceCents = cents;
        savePricesToEEPROM();
        DBG_PRINT(F("Preis Fach "));
        DBG_PRINT(idx + 1);
        DBG_PRINT(F(" → "));
        DBG_PRINTLN(cents);
    }

    // ── Aktor ─────────────────────────────────

    // Öffnet Fach (Aktor kurz HIGH)
    void open(uint8_t idx) {
        DBG_PRINT(F("Oeffne Fach "));
        DBG_PRINTLN(idx + 1);
        digitalWrite(_compartments[idx].pin, HIGH);
        _openStart  = millis();
        _openIdx    = idx;
        _isOpening  = true;
    }

    // Muss in loop() aufgerufen werden – schaltet Aktor nach Ablauf ab
    void update() {
        if (_isOpening && (millis() - _openStart >= COMPARTMENT_OPEN_DURATION_MS)) {
            digitalWrite(_compartments[_openIdx].pin, LOW);
            _isOpening = false;
            markEmpty(_openIdx);
        }
    }

    bool isOpening() const { return _isOpening; }

    // ── Hilfsmethoden ─────────────────────────

    uint8_t count() const { return NUM_COMPARTMENTS; }

    // Gibt true zurück wenn mind. ein Fach verfügbar ist
    bool anyAvailable() const {
        for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
            if (isAvailable(i)) return true;
        }
        return false;
    }

private:
    Compartment _compartments[NUM_COMPARTMENTS];
    bool        _isOpening  = false;
    uint8_t     _openIdx    = 0;
    uint32_t    _openStart  = 0;

    // ── EEPROM ────────────────────────────────

    void loadFromEEPROM() {
        uint8_t magic;
        EEPROM.get(EEPROM_ADDR_MAGIC, magic);

        if (magic != EEPROM_MAGIC_VALUE) {
            // Erste Initialisierung – Defaults schreiben
            DBG_PRINTLN(F("EEPROM: Erstinitialisierung"));
            for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
                _compartments[i].priceCents = DEFAULT_PRICES[i];
                _compartments[i].status     = CompartmentStatus::BEFUELLT;
            }
            savePricesToEEPROM();
            saveStatusToEEPROM();
            EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC_VALUE);
        } else {
            // Werte laden
            for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
                uint16_t price;
                EEPROM.get(EEPROM_ADDR_PRICES + i * 2, price);
                _compartments[i].priceCents = price;

                uint8_t st;
                EEPROM.get(EEPROM_ADDR_STATUS + i, st);
                _compartments[i].status = static_cast<CompartmentStatus>(st);
            }
            DBG_PRINTLN(F("EEPROM: Daten geladen"));
        }
    }

    void savePricesToEEPROM() {
        for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
            EEPROM.put(EEPROM_ADDR_PRICES + i * 2, _compartments[i].priceCents);
        }
    }

    void saveStatusToEEPROM() {
        for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
            EEPROM.put(EEPROM_ADDR_STATUS + i,
                       static_cast<uint8_t>(_compartments[i].status));
        }
    }
};
