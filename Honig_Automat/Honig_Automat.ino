// ═══════════════════════════════════════════════════
//  Honig-Verkaufsautomat
//  Arduino Mega · main.ino
//
//  Architektur:
//    DisplayService, KeypadService, CoinService,
//    CompartmentService, PowerSaveService,
//    CustomerFlowController, AdminMenuController
//
//  Kein delay() im Normalbetrieb · millis()-basiert
// ═══════════════════════════════════════════════════

#include "src/Config.h"
#include "src/TextUtils.h"
#include "src/DisplayService.h"
#include "src/KeypadService.h"
#include "src/CoinService.h"
#include "src/CompartmentService.h"
#include "src/PowerSaveService.h"
#include "src/AdminMenuController.h"
#include "src/CustomerFlowController.h"

// ── Service-Instanzen ─────────────────────────────
DisplayService      display;
KeypadService       keypad;
CoinService         coins;
CompartmentService  compartments;
PowerSaveService    powerSave(display);

CustomerFlowController customerFlow(display, keypad, coins, compartments, powerSave);
AdminMenuController    adminMenu(display, keypad, compartments);

// ─────────────────────────────────────────────────
void setup() {
#ifdef DEBUG_ENABLED
    Serial.begin(9600);
    DBG_PRINTLN(F("Honig-Automat startet..."));
#endif

    display.begin();
    keypad.begin();
    coins.begin();
    compartments.begin();
    powerSave.begin();
    adminMenu.begin();
    customerFlow.begin();

    DBG_PRINTLN(F("Setup abgeschlossen."));
}

// ─────────────────────────────────────────────────
void loop() {
    // Energiesparmodus prüfen
    powerSave.update();

    // Münzzähler aktualisieren (Timeout)
    coins.update();

    if (adminMenu.isActive()) {
        // Admin-Modus hat Priorität
        adminMenu.update();
    } else {
        // Normaler Kunden-Ablauf
        customerFlow.update();

        // Admin-Anfrage aus CustomerFlow weitergeben
        if (customerFlow.adminRequested()) {
            adminMenu.activate();
        }
    }
}
