void initDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.clearDisplay();
  display.display();
}

void drawGSMRXLevel() {
  byte AntennaLeft = 103;
  for (int b = 0; b <= GSMRXLevel; b++) {
    display.fillRect(112 + (b * 3), 15 - (b * 3), 2, b * 3, WHITE);
  }

  for (int c = GSMRXLevel + 1; c < 5; c++) {
    display.fillRect(112 + (c * 3), 15 - (c * 3), 2, c * 3, BLACK);
  }

  Serial.println("GSMRXLevel = " + String(GSMRXLevel));

  display.drawLine(AntennaLeft, 0, AntennaLeft + 10 , 0, WHITE);
  display.drawLine(AntennaLeft + 5, 0, AntennaLeft + 5, 13, WHITE);
  display.drawLine(AntennaLeft, 0, AntennaLeft + 5, 6, WHITE);
  display.drawLine(AntennaLeft + 5, 6, AntennaLeft + 10, 0, WHITE);
  display.display();
}

void drawBalance() {
  int balance = getBalance();
  String gh = String(balance, DEC);
  String tmp = "";
  for (int i = 0; i < gh.length(); i++) {
    if (i == gh.length()  - 2) tmp += ".";
    tmp += gh[i];
  }

  byte factor = (ACPowerUpMins > 0) ? 2 : 1;
  
  if (balance > (getSMSreceiverCount() * costPerSMS * factor)) {
    drawWarningSign(BLACK);
  } else {
    drawWarningSign(WHITE);
  }
  printDisplayLine(0, false, "$=" + tmp + " EUR");
}

void drawWarningSign(uint8_t color) {
  display.drawTriangle(86, 13, 93, 2, 100, 13, color);
  display.drawLine(93, 5, 93, 9, color);
  display.drawPixel(93, 11, color);
  display.display();
}

void printDisplayLine(byte LineNumber, byte CLEAR) {
  if (CLEAR == 1)
    printDisplayLine(LineNumber, false, "                     ", LEFT);
}

void printDisplayLine(byte LineNumber, boolean inverted, String text) {
  printDisplayLine(LineNumber, inverted, text, LEFT);
}

void printDisplayLine(byte LineNumber, boolean inverted, String text, byte align) {
  if (text != oldDisplayLine[LineNumber]) {
    String spaces = "";

    //Alten Text löschen
    if (oldDisplayLine[LineNumber] != "") {
      display.setCursor(0, LineNumber * 8);
      display.setTextColor(BLACK, BLACK);
      if (align == CENTER) {
        int spaceCount = (21 - oldDisplayLine[LineNumber].length()) / 2;
        for (int i = 0; i < spaceCount; i++) {
          spaces += " ";
        }
      }
      display.print(spaces + oldDisplayLine[LineNumber]);
    }

    //Neuen Text schreiben
    display.setCursor(0, LineNumber * 8);
    display.setTextColor(WHITE, BLACK);
    if (inverted)
      display.setTextColor(BLACK, WHITE);

    spaces = "";
    if (align == CENTER) {
      int spaceCount = (21 - text.length()) / 2;
      for (int i = 0; i < spaceCount; i++) {
        spaces += " ";
      }
    }
    display.print(spaces + text);


    display.display();
    oldDisplayLine[LineNumber] = text;
  }
}
