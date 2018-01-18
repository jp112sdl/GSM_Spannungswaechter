byte A6waitFor(String response1, String response2, int timeOut) {
  unsigned long entry = millis();
  int count = 0;
  String reply = A6read();
  byte retVal = 99;
  do {
    reply = A6read();
    delay(5);
    if (reply != "") {
      if (A6DEBUG) {
        Serial.print((millis() - entry));
        Serial.print(" ms ");
        Serial.println("Reply: " + reply);
      }
      lastReply = reply;
    }
  } while ((reply.indexOf(response1) + reply.indexOf(response2) == -2) && millis() - entry < timeOut );

  if ((millis() - entry) >= timeOut) {
    retVal = TIMEOUT;
  } else {
    if (reply.indexOf(response1) + reply.indexOf(response2) > -2) retVal = OK;
    else retVal = NOTOK;
  }
  return retVal;
}

byte A6command(String command, String response1, String response2, int timeOut, int repetitions) {
  byte returnValue = NOTOK;
  byte count = 0;
  while (count < repetitions && returnValue != OK) {
    A6board.println(command);
    if (A6DEBUG) {
      Serial.print("Command: ");
      Serial.println(command);
    }
    if (A6waitFor(response1, response2, timeOut) == OK) {
      returnValue = OK;
    } else returnValue = NOTOK;
    count++;
  }
  return returnValue;
}

void A6input() {
  String hh;
  char buffer[100];
  while (1 == 1) {
    if (Serial.available()) {
      hh = Serial.readStringUntil('\n');
      hh.toCharArray(buffer, hh.length() + 1);
      if (hh.indexOf("ende") == 0) {
        A6board.write(end_c);
        Serial.println("ende");
      } else {
        A6board.write(buffer);
        A6board.write('\n');
      }
    }
    if (A6board.available()) {
      Serial.write(A6board.read());
    }
  }
}

bool A6begin() {
  printDisplayLine(2, true, " ");

  while  (A6command("AT+CPIN?", "OK", "yy", 2000, 1) != OK) {
    Serial.println("SIM CARD WAITING");
    if (lastReply.indexOf("ERROR:10") > 0) {
      display.clearDisplay();
      display.drawBitmap(32, 0,  simErrorBMP, 64, 66, WHITE);
      display.display();
    }
  }

  if (lastReply.indexOf("READY") > 0) {
    Serial.println("SIM CARD READY");
  } else {
    display.clearDisplay();
    display.drawBitmap(32, 0,  simErrorBMP, 64, 66, WHITE);
    display.display();
  }


  A6board.println("AT+CREG?");
  byte hi = A6waitFor("1,", "5,", 2000);  // 1: registered, home network ; 5: registered, roaming
  Serial.println("A6board Wait for AT+CREG?");
  printDisplayLine(2, true, "  ");
  while ( hi != OK) {
    A6board.println("AT+CREG?");
    hi = A6waitFor("1,", "5,", 2000);
    Serial.println("A6board Wait for AT+CREG?");
  }
  printDisplayLine(2, true, "      ");

  if (A6command("AT&F0", "OK", "yy", 5000, 2) == OK) {   // Reset to factory settings
    printDisplayLine(2, true, "          ");
    if (A6command("ATE0", "OK", "yy", 5000, 2) == OK) {  // disable Echo
      printDisplayLine(2, true, "              ");
      if (A6command("AT+CSCS=\"HEX\"", "OK", "yy", 5000, 2) == OK) {
        printDisplayLine(2, true, "                  ");
        if (A6command("AT+CMEE=2", "OK", "yy", 5000, 2) == OK) {
          return OK;
          printDisplayLine(2, true, "                     ");
        }
        else {
          return NOTOK;
          printDisplayLine(2, false, "                     ");

        }
      }
    }
  }
}

String A6read() {
  String reply = "";
  if (A6board.available())  {
    reply = A6board.readString();
  }
  return reply;
}
