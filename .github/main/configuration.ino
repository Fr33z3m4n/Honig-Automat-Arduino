#include <Keypad.h>

#define TXT_CONFIG_MODE       F("=== KONFIG-MODUS ===")
#define TXT_CONFIG_MAIN_1     F("1 Fachverwaltung")
#define TXT_CONFIG_MAIN_2     F("2 Preise")
#define TXT_CONFIG_MAIN_3     F("# Beenden")
#define TXT_EXIT              F("Konfig-Modus beendet")

/**
* Aktuelles Konfigurationsmenu
* 0 = Hauptmenü
* 1 = Fachverwaltung (configuration_compartment.ino)
* 11 = Fachverwaltung -> Fach mit Nummer öffnen (configuration_compartment.ino)
**/
int actConfigMenu = 0;

// --- Sequenz zum Eintritt ins Konfig-Menü ---
const char SECRET[] = "A";        // deine Kombination (z.B. C#1234#C)
const uint8_t SECRET_LEN = sizeof(SECRET) - 1;
String inputBuf;                         // Rolling Buffer
bool configMode = false;

void handleKeyPadConfigurationMenu() {
  char key = keypad.getKey();
  const unsigned long now = millis();

  // Timeout für Eingabesequenz im Normalmodus
  if (!configMode && inputBuf.length() > 0 && (now - lastKeyTs > INPUT_TIMEOUT)) {
    inputBuf = "";
  }

  if (key) {
    lastKeyTs = now;

    if (configMode) {
      handleKeypad(key);
      return;
    }

    // --- Normalmodus: Sequenz-Detektor ---
    // Taste an Buffer anhängen, aber nur die letzten SECRET_LEN Zeichen halten
    inputBuf += key;
    if (inputBuf.length() > SECRET_LEN) {
      inputBuf.remove(0, inputBuf.length() - SECRET_LEN);
    }

    // Prüfen, ob die Sequenz exakt passt
    if (inputBuf.equals(SECRET)) {
      enterConfigMode();
    }
  }

}

void handleKeypad(char key) {
  Serial.println("-- handleKeypad --");
  if (key >= '0' && key <= '9') {   // nur Ziffern puffern
    inputBuf += key;
  }
  handleConfigMenu(key);
}

/**
* Hauptmenü
*/
void enterConfigMode() {
  Serial.println("-- enterConfigMode --");
  configMode = true;
  inputBuf = "";

  actConfigMenu = 0;

  display(0, TXT_CONFIG_MODE, true);
  display(1, TXT_CONFIG_MAIN_1);
  display(2, TXT_CONFIG_MAIN_2);
  display(3, TXT_CONFIG_MAIN_3);
}

// Konfiguration verlassen
void exitConfigMode() {
  Serial.println("-- exitConfigMode --");
  configMode = false;
  Serial.println(TXT_EXIT);
  display(0, "Bereit", true);
}

// Eingaben an das aktuelle Menü weiterleiten
void handleConfigMenu(char key) {
  Serial.println("-- handleConfigMenu --");
   switch (actConfigMenu) {
    case 0:
      handleMainMenu(key);
      break;
    case 1:
      handleCompartementMenu(key);
      break;
    case 11:
      handleCompartementOpenByNumberMenu(key);      
      break;
    default:
      break;
   }
}

// Eingaben Hauptmenü
void handleMainMenu(char key) {
  Serial.println("-- handleMainMenu --");
  switch (key) {
    case '1':
      Serial.println(F("-> Fachverwaltung"));
      enterCompartmentMenu();
      break;
    case '2':
      Serial.println(F("-> Weitere Funktionen (Dummy)"));
      break;
    case '#':
      exitConfigMode();
      break;
    default:
      // ignorieren oder Hilfe anzeigen
      break;
  }
}