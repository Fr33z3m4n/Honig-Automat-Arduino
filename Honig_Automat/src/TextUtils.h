#pragma once
#include <Arduino.h>
#include "Config.h"

// ─────────────────────────────────────────────
//  TextUtils.h  –  LCD-Hilfsklasse
//  Zentrieren, linksbündig auffüllen, Cent→String
// ─────────────────────────────────────────────

class TextUtils {
public:
    // Gibt str zentriert auf LCD_COLS Zeichen zurück (mit führenden Leerzeichen)
    static String center(const String& str, uint8_t width = LCD_COLS) {
        if (str.length() >= width) return str.substring(0, width);
        uint8_t padding = (width - str.length()) / 2;
        String result = "";
        for (uint8_t i = 0; i < padding; i++) result += ' ';
        result += str;
        while (result.length() < width) result += ' ';
        return result;
    }

    // Füllt str rechts mit Leerzeichen auf 'width' Zeichen auf
    static String padRight(const String& str, uint8_t width = LCD_COLS) {
        String result = str.substring(0, width);
        while (result.length() < width) result += ' ';
        return result;
    }

    // Konvertiert Cent-Betrag in lesbaren String: 500 → "5,00 EUR"
    static String centsToString(uint16_t cents) {
        String result = String(cents / 100);
        result += ',';
        uint8_t frac = cents % 100;
        if (frac < 10) result += '0';
        result += String(frac);
        result += " EUR";
        return result;
    }

    // Wiederholt ein Zeichen n-mal
    static String repeat(char c, uint8_t n) {
        String result = "";
        for (uint8_t i = 0; i < n; i++) result += c;
        return result;
    }
};
