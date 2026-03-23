#pragma once
#include <Arduino.h>
#include "Config.h"
#include "DisplayService.h"

// ─────────────────────────────────────────────
//  PowerSaveService.h  –  LCD-Energiesparmodus
//  Dimmt / schaltet LCD nach Inaktivität ab
// ─────────────────────────────────────────────

enum class PowerState : uint8_t {
    ACTIVE,
    DIMMED,
    OFF
};

class PowerSaveService {
public:
    explicit PowerSaveService(DisplayService& display)
        : _display(display), _state(PowerState::ACTIVE) {}

    void begin() {
        _lastActivity = millis();
        _state        = PowerState::ACTIVE;
    }

    // Aktivität registrieren (Taste, Münze, etc.)
    void registerActivity() {
        _lastActivity = millis();
        if (_state != PowerState::ACTIVE) {
            _display.setBacklight(LCD_BRIGHTNESS_NORMAL);
            _state = PowerState::ACTIVE;
            DBG_PRINTLN(F("PowerSave: ACTIVE"));
        }
    }

    // Muss in loop() aufgerufen werden
    void update() {
        uint32_t idle = millis() - _lastActivity;

        if (_state == PowerState::ACTIVE && idle >= TIMEOUT_POWERSAVE_MS) {
            _display.setBacklight(LCD_BRIGHTNESS_DIM);
            _state = PowerState::DIMMED;
            DBG_PRINTLN(F("PowerSave: DIMMED"));
        } else if (_state == PowerState::DIMMED && idle >= TIMEOUT_POWEROFF_MS) {
            _display.setBacklight(LCD_BRIGHTNESS_OFF);
            _state = PowerState::OFF;
            DBG_PRINTLN(F("PowerSave: OFF"));
        }
    }

    PowerState getState() const { return _state; }
    bool       isOff()    const { return _state == PowerState::OFF; }

private:
    DisplayService& _display;
    PowerState      _state;
    uint32_t        _lastActivity = 0;
};
