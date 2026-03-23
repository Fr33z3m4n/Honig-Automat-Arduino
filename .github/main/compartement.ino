/**
* Fachverwaltung / compartement
*/
/* Pin Belegung */
//const int relays[10] = {3, 4, 5, 6, 7, 8, 9, 10 , 11, 12}; //uno: 10 boxes
const int relays[3] = { 22, 24, 26 };  // mega: 15 boxes
const size_t RELAY_COUNT = sizeof(relays) / sizeof(relays[0]);
//const int poweroffrelais = 4;  // mega: 4; uno: only possible if less 10 compartments; in this case pin 12 recommended; remove this pin from relais list
// poweroffrelais replaced by powersave_relais_pin


void initCompartements() {
  // Relais initialisieren
  for (int index = 0; index < RELAY_COUNT; index++) {
    // Serial.print(index);
    digitalWrite(relays[index], HIGH);  // Turn OFF
    pinMode(relays[index], OUTPUT);
  }
}

// Fach Öffnen
void CompartementOpen(int j) {

  // detachInterrupt(coinInt); // Münzprüfer Observable deaktivieren
  if (debug) {
    Serial.print("Fachnummer ");
    Serial.print(j + 1);
    Serial.println(" oeffnen:");
  }
  //hier relais ansteuern

  digitalWrite(relays[j], LOW);  // Fach öffnen
  delay(1000);
  digitalWrite(relays[j], HIGH);  // Relais wieder aus
  delay(1000);
  
  // attachInterrupt(coinInt, coinInserted, RISING); // Münzprüfer Observable aktivieren
}

// Alle Fächer 
void CompartementOpenAll() {
  for (int index = 0; index < RELAY_COUNT; index++) {
    // CompartementOpen(index);
    digitalWrite(relays[index], LOW);
  }

  delay(1000);

  for (int index = 0; index < RELAY_COUNT; index++) {
    // CompartementOpen(index);
    digitalWrite(relays[index], HIGH);
  }
}

// Leere Fächer öffnen
void CompartementEmptyOpenAll() {
  // Not implemented
}