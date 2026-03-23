#pragma once
#include <Arduino.h>
#include "Config.h"

// ─────────────────────────────────────────────
//  KeypadService.h  –  Nicht-blockierendes Keypad-Polling
//  Matrix 4×4: (1,2,3,A),(4,5,6,B),(7,8,9,C),(*,0,#,D)
// ─────────────────────────────────────────────

class KeypadService {
public:
    static constexpr char NO_KEY = '\0';

    void begin() {
        for (uint8_t r = 0; r < KEYPAD_ROWS; r++) {
            pinMode(KEYPAD_ROW_PINS[r], OUTPUT);
            digitalWrite(KEYPAD_ROW_PINS[r], HIGH);
        }
        for (uint8_t c = 0; c < KEYPAD_COLS; c++) {
            pinMode(KEYPAD_COL_PINS[c], INPUT_PULLUP);
        }
    }

    // Muss in loop() aufgerufen werden.
    // Gibt gedrückte Taste zurück (einmalig beim Drücken), sonst NO_KEY.
    char getKey() {
        char key = scanMatrix();

        if (key != NO_KEY) {
            if (_lastKey == key) {
                // Taste gehalten – ignorieren (bereits gemeldet)
                return NO_KEY;
            }
            _lastKey = key;
            _pressTime = millis();
            return key;
        }

        // Taste losgelassen
        if (_lastKey != NO_KEY) {
            _lastKey = NO_KEY;
        }
        return NO_KEY;
    }

    // Gibt true zurück, wenn IRGENDEINE Taste gerade gedrückt ist
    bool isAnyKeyPressed() {
        return scanMatrix() != NO_KEY;
    }

private:
    static constexpr char KEYMAP[KEYPAD_ROWS][KEYPAD_COLS] = {
        {'1','2','3','A'},
        {'4','5','6','B'},
        {'7','8','9','C'},
        {'*','0','#','D'}
    };

    char    _lastKey   = NO_KEY;
    uint32_t _pressTime = 0;

    char scanMatrix() {
        for (uint8_t row = 0; row < KEYPAD_ROWS; row++) {
            // Zeile aktiv ziehen
            digitalWrite(KEYPAD_ROW_PINS[row], LOW);
            delayMicroseconds(10); // RC-Entprellung

            for (uint8_t col = 0; col < KEYPAD_COLS; col++) {
                if (digitalRead(KEYPAD_COL_PINS[col]) == LOW) {
                    digitalWrite(KEYPAD_ROW_PINS[row], HIGH);
                    return KEYMAP[row][col];
                }
            }
            digitalWrite(KEYPAD_ROW_PINS[row], HIGH);
        }
        return NO_KEY;
    }
};

// Konstante außerhalb der Klasse definieren (ODR-Regel)
constexpr char KeypadService::KEYMAP[KEYPAD_ROWS][KEYPAD_COLS];
