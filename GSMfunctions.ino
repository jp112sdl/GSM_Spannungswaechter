void sendSMStoAll(String Nachricht) {
  for (int i = 0; i < maxSMSReceiver; i++) {
    if (SMSreceiver[i].length() > 10 && SMSreceiver[i].startsWith("+491")) sendSMS(SMSreceiver[i], Nachricht);
  }
}

bool sendSMS(String toNumber, String Nachricht) {
  Serial.println("Starte sendSMS");
  if (!gsmActive) {
    gsmActive = true;
    if (!DEBUG) {
      if (A6command("AT+CMGF=1", "OK", "yy", 2000, 2) == OK) {
        if (A6command("AT+CMGS = \"" + toNumber + "\"", ">", "yy", 20000, 2) == OK) {
          delay(100);
          A6board.println(Nachricht);
          A6board.println(end_c);
          A6board.println();
        }
        Serial.println("SMS: " + Nachricht + " sent to " + toNumber);
      }
    } else Serial.println("DEBUG ist aktiv. SMS " + Nachricht + " an " + toNumber + " wird nicht versandt!");
    gsmActive = false;
    return true;
  } else {
    return false;
  }
}

int getBalance() {
  int Guthaben = -1;
  if (!gsmActive) {
    gsmActive = true;
    if (A6command("AT+CUSD=1,\"*100#\",15", "OK", "yy", 8000, 1) == OK) {
      String last = lastReply;
      if (last.indexOf("\"")  > 0) {
        last = last.substring(last.indexOf("\"") + 1, last.length());
        last = last.substring(0, last.lastIndexOf("\""));
        String decodedGuthaben = decodeUSSD(last);
        String tmp = "";
        for (int i = 0; i < decodedGuthaben.length(); i++) {
          if ((int)decodedGuthaben[i] >= 48 && (int)decodedGuthaben[i] <= 57) {
            tmp += decodedGuthaben[i];
          }
        }
        Guthaben = tmp.toInt();
      }
    }
    gsmActive = false;
  }
  return Guthaben;
}

void checkSignalStrength() {
  if (!gsmActive && hasACPower && !ACPowerDownAlertSent) {
    Serial.println("Signal strength...");
    gsmActive = true;
    if (A6command("AT+CSQ", "OK", "yy", 5000, 2) == OK) {
      if (lastReply.indexOf("+CSQ: ") != -1) {
        String rest = lastReply.substring(7, lastReply.length());
        String rest1 = rest.substring(0, rest.indexOf(","));
        rest1 .trim();

        if (rest1.toInt() < 32 && rest1.toInt() >= 20) {
          Serial.println("Signal strength... EXCELLENT");
          GSMRXLevel = 4;
        } else if (rest1.toInt() < 20 && rest1.toInt() >= 15) {
          Serial.println("Signal strength... GOOD");
          GSMRXLevel = 3;
        } else if (rest1.toInt() < 15 && rest1.toInt() >= 10) {
          Serial.println("Signal strength... OK");
          GSMRXLevel = 2;
        } else if (rest1.toInt() < 10 && rest1.toInt() >= 2) {
          Serial.println("Signal strength... Marginal");
          GSMRXLevel = 1;
        } else {
          Serial.println("Signal strength... UNKNOWN");
          GSMRXLevel = 0;
        }
      }
      drawGSMRXLevel();
    }
    gsmActive = false;
  }
}
