void processLocalConfig(String serread) {
  serread.replace("\r", ""); serread.replace("\n", "");
  if (serread.startsWith("SET:ACPOWERDOWNMINS=")) {
    Serial.println("ACPOWERDOWNMINS: " + serread.substring(20, serread.length()));
    ACPowerDownMins = (byte)(serread.substring(20, serread.length())).toInt();
  } else if (serread.startsWith("SET:ACPOWERUPMINS=")) {
    Serial.println("ACPOWERUPMINS: " + serread.substring(18, serread.length()));
    ACPowerUpMins = (byte)(serread.substring(18, serread.length())).toInt();
  } else if (serread.startsWith("SET:COSTPERSMS=")) {
    Serial.println("COSTPERSMS: " + serread.substring(15, serread.length()));
    costPerSMS = (byte)(serread.substring(15, serread.length())).toInt();
  } else if (serread.startsWith("SET:ACPOWERUPMESSAGE=")) {
    Serial.println("ACPOWERUPMESSAGE: " + serread.substring(21, serread.length()));
    ACPowerUpMessage = (serread.substring(21, serread.length()));
  } else if (serread.startsWith("SET:ACPOWERDOWNMESSAGE=")) {
    Serial.println("ACPOWERDOWNMESSAGE: " + serread.substring(23, serread.length()));
    ACPowerDownMessage = (serread.substring(23, serread.length()));
  } else if (serread.startsWith("SET:SMSRECEIVER")) {
    String alles = serread;
    alles.replace("SET:SMSRECEIVER", "");
    String number = alles.substring(alles.indexOf("=") + 1, alles.length());
    int index = alles.substring(0, alles.indexOf("=")).toInt();
    if (number == "CLEAR") {
      SMSreceiver[index - 1] = "";
    } else {
      SMSreceiver[index - 1] = number;
    }
  } else if (serread.startsWith("SET:CLEAR")) {
    Serial.println("Leere SMS Empfänger, setze Zeiten auf 10 Min. und SMS Kosten auf 9 Cent");
    for (int i = 0; i < maxSMSReceiver; i++) {
      SMSreceiver[i] = "";
    }
    ACPowerDownMins = 10;
    ACPowerUpMins =   10;
    costPerSMS = 9;
  } else if (serread.startsWith("SET:?")) {
    Serial.println("");
    Serial.println("Commands: ");
    Serial.println("SET:ACPOWERUPMINS=<minutes>");
    Serial.println("SET:ACPOWERDOWNMINS=<minutes>");
    Serial.println("SET:COSTPERSMS=<cents>");
    Serial.println("SET:ACPOWERUPMESSAGE=<Nachricht>");
    Serial.println("SET:ACPOWERDOWNMESSAGE=<Nachricht>");
    Serial.println("SET:SMSRECEIVER<n>=<number> im Format +49...");
    Serial.println("SET:SMSRECEIVER<n>=CLEAR zum Entfernen");
  } else if (serread.startsWith("GET:?")) {
    Serial.println("");
    Serial.println("Commands: ");
    Serial.println("GET:BALANCE");
  } else if (serread.startsWith("GET:BALANCE")) {
    float gh = (float)getBalance() / 100.0;
    Serial.println("Guthaben: " + String(gh, 2) + " EUR");
  }

  if (serread.startsWith("SET:")) {
    Serial.println("");
    saveConfig();
    Serial.println("");
  }
}

bool loadConfig() {
  File configFile = SPIFFS.open("/" + configFilename, "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  String serialOutput;
  json.printTo(serialOutput);
  serialOutput.replace(",\"", ",\n\"");
  Serial.print(serialOutput);
  Serial.println("");  Serial.println("");


  ACPowerDownMins = json["ACPowerDownMins"];
  ACPowerUpMins = json["ACPowerUpMins"];
  costPerSMS = json["costPerSMS"];
  ACPowerUpAlertSent = json["ACPowerUpAlertSent"];

  const char* acpum = json["ACPowerUpMessage"];
  ACPowerUpMessage = acpum;
  const char* acpdm = json["ACPowerDownMessage"];
  ACPowerDownMessage = acpdm;

  for (int i = 0; i < maxSMSReceiver; i++) {
    const char* temp = json["SMSreceiver" + String(i)];
    SMSreceiver[i] = temp;
    //Serial.println("SMSreceiver" + String(i) + "=" + SMSreceiver[i]);
  }
  return true;
}

bool saveConfig() {
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["ACPowerDownMins"] = ACPowerDownMins;
  json["ACPowerUpMins"] = ACPowerUpMins;
  json["costPerSMS"] = costPerSMS;
  json["ACPowerUpAlertSent"] = ACPowerUpAlertSent;
  json["ACPowerUpMessage"] = ACPowerUpMessage;
  json["ACPowerDownMessage"] = ACPowerDownMessage;

  for (int i = 0; i < maxSMSReceiver ; i++) {
    Serial.println("SMSreceiver" + String(i) + "=" + SMSreceiver[i]);
    json["SMSreceiver" + String(i)] = SMSreceiver[i];
  }

  File configFile = SPIFFS.open("/" + configFilename, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  String serialOutput;
  json.printTo(serialOutput);
  serialOutput.replace(",\"", ",\n\"");
  Serial.print(serialOutput);
  json.printTo(configFile);
  Serial.println(""); Serial.println("");
  return true;
}
