String indexHtml =  "";
String htmlHeader = "<!DOCTYPE html>"
                    "<head>"
                    "<meta charset=\"utf-8\">"
                    "<title>GSM Spannungsw&auml;chter Konfiguration</title>"
                    "<style>"
                    ".c{text-align: center;}"
                    "div,input{padding:5px;font-size:1em;}"
                    "input{width:95%;}"
                    "body{text-align: left;font-family:verdana;font-size: 24px; margin: 15px 15px 15px 15px ; padding: 5px;}"
                    "button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;}"
                    ".q{float: right;width: 64px;text-align: right;}"
                    "</style>"
                    "</head>"
                    "<body>";

void startAPmode() {
  generateIndexHTML();
  display.drawBitmap(32, 0,  toolBMP, 64, 66, WHITE);
  display.display();
  WiFi.mode(WIFI_AP);
  WiFi.softAP("GSMGateway");
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());
  Serial.println("AP Modus gestartet");
  webServer.on("/set", []() {
    if (webServer.arg("action") == "1") {
      byte webACPowerDownMins = webServer.arg("ACPowerDownMins").toInt();
      byte webACPowerUpMins = webServer.arg("ACPowerUpMins").toInt();
      byte webcostPerSMS = webServer.arg("costPerSMS").toInt();
      String webSMSreceiver[maxSMSReceiver];
      for (int i = 0; i < maxSMSReceiver; i++) {
        webSMSreceiver[i] = webServer.arg("SMSReceiver" + String(i));
        SMSreceiver[i]=webSMSreceiver[i];
      }
      
      String webACPowerDownMessage = webServer.arg("ACPowerDownMessage");
      String webACPowerUpMessage = webServer.arg("ACPowerUpMessage");

      ACPowerDownMins = webACPowerDownMins;
      ACPowerUpMins = webACPowerUpMins;
      costPerSMS = webcostPerSMS;

      webACPowerDownMessage.replace("&nbsp;", " ");
      webACPowerUpMessage.replace("&nbsp;", " ");

      ACPowerDownMessage = webACPowerDownMessage;
      ACPowerUpMessage = webACPowerUpMessage;

      saveConfig();
      display.clearDisplay();
      display.drawBitmap(32, 0,  checkBMP, 64, 66, WHITE);
    } else {
      display.clearDisplay();
      display.drawBitmap(32, 0,  cancelBMP, 64, 66, WHITE);
    }

    webServer.send(200, "text/html", htmlHeader + "Ger&auml;t startet neu.</body></html>");
    display.display();
    delay(2000);
    ESP.restart();
  });

  webServer.onNotFound([]() {
    webServer.send(200, "text/html", indexHtml);
    Serial.println("Index HTML gesendet");
  });
  webServer.begin();
}

void generateIndexHTML() {
  indexHtml = htmlHeader;

  String webACPowerDownMessage = ACPowerDownMessage;
  String webACPowerUpMessage = ACPowerUpMessage;
  webACPowerDownMessage.replace(" ", "&nbsp;");
  webACPowerUpMessage.replace(" ", "&nbsp;");

  indexHtml += "<form action=\"/set\" method=\"post\" autocomplete=\"off\">";
  indexHtml += "<label for=\"ACPowerDownMins\">SMS nach x Minuten bei Stromausfall:</label><input id=\"ACPowerDownMins\" name=\"ACPowerDownMins\" value=" + String(ACPowerDownMins) + " maxlength=\"2\"><br><br>";
  indexHtml += "<label for=\"ACPowerUpMins\">SMS nach x Minuten bei Stromr&uuml;ckkehr:</label><input id=\"ACPowerUpMins\" name=\"ACPowerUpMins\" value=" + String(ACPowerUpMins) + " maxlength=\"2\"><br><br>";
  indexHtml += "<label for=\"ACPowerDownMessage\">Nachricht bei Stromausfall:</label><input id=\"ACPowerDownMessage\" name=\"ACPowerDownMessage\" value=" + String(webACPowerDownMessage) + " maxlength=\"160\"><br><br>";
  indexHtml += "<label for=\"ACPowerUpMessage\">Nachricht bei Stromr&uuml;ckkehr:</label><input id=\"ACPowerUpMessage\" name=\"ACPowerUpMessage\" value=" + String(webACPowerUpMessage) + " maxlength=\"160\"><br><br>";
  for (int i = 0; i < maxSMSReceiver; i++)
    indexHtml += "<label for=\"SMSReceiver" + String(i) + "\">SMS Empf&auml;nger " + String(i + 1) + ":</label><input id=\"SMSReceiver" + String(i) + "\" name=\"SMSReceiver" + String(i) + "\" value=\"" + SMSreceiver[i] + "\" maxlength=\"20\"><br><br>";
  indexHtml += "<label for=\"costPerSMS\">Preis je SMS (Cent):</label><input id=\"costPerSMS\" name=\"costPerSMS\" value=" + String(costPerSMS) + " maxlength=\"2\"><br><br>";
  indexHtml += "<button type=\"submit\" name=\"action\" value=\"1\">OK</button><br>";
  indexHtml += "<button type=\"submit\" name=\"action\" value=\"0\">Abbruch</button>";
  indexHtml += "</form>";
  indexHtml += "</body>";
  indexHtml += "</html>";
}

