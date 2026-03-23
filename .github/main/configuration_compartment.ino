#define TXT_FACH_1          F("1 Alle Fächer öffnen")
#define TXT_FACH_2          F("2 Leere öffnen")
#define TXT_FACH_3          F("3 Fachnummer öffnen")
#define TXT_FACH_4          F("# Zurück")
#define TXT_FACH_OPEN_1     F("Fachnummer eingeben")
#define TXT_FACH_OPEN_2     F("mit \"D\" Bestätigen")
#define TXT_FACH_OPEN_3     F("# zurück")

// 1 - Fachverwaltung (Menü)
void enterCompartmentMenu() {
  Serial.println("-- enterCompartmentMenu --");
  actConfigMenu = 1;
  inputBuf = "";

  display(0, TXT_FACH_1, true);
  display(1, TXT_FACH_2);
  display(2, TXT_FACH_3);
  display(3, TXT_FACH_4);
}

// 11 - Fachverwaltung - Bestimmtes Fach öffnen (Menü)
void enterCompartmentOpenByNumberMenu() {
  Serial.println("-- enterCompartmentOpenByNumberMenu --");
  actConfigMenu = 11;
  inputBuf = "";

  display(0, TXT_FACH_OPEN_1, true);
  display(1, TXT_FACH_OPEN_2);
  display(2, TXT_FACH_OPEN_3);
  display(3, String("Fach: "));
}

// 1 - Fachverwaltung
void handleCompartementMenu(char key) {
  Serial.println("-- handleCompartementMenu --");
  switch (key) {
    case '1':
      Serial.println(F("Alle Fächer öffnen"));
      CompartementOpenAll();
      break;
    case '2':
      Serial.println(F("Leere Fächer öffnen"));
      CompartementEmptyOpenAll();
      break;   
    case '3':
      Serial.println(F("Gewünschtes Fach öffnen"));
      enterCompartmentOpenByNumberMenu();
      break;         
    case '#':
      enterConfigMode();
      break;
    default:
      break;
  }
}

// 11 - Fachverwaltung - Bestimmtes Fach öffnen
void handleCompartementOpenByNumberMenu(char key) {
  Serial.println("-- handleCompartementOpenByNumberMenu --");
  switch (key) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      displayClearLine(3);
      display(3, String("Fach: ") + String(inputBuf));
      break;
    case '#':
      enterCompartmentMenu(); // Zurück zum Fachverwaltungsmenü
      break;       
    case 'D':
      const int fach = inputBuf.toInt();
      display(3, String("Fach: ") + fach + " geöffnet");
      CompartementOpen(fach - 1);
      inputBuf = "";
      displayClearLine(3);
      display(3, String("Fach: ") + inputBuf);
      break;
    default:
      break;
  }
}