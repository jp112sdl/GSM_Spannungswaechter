void checkMillis() {
  if (lastMillis > millis())
    lastMillis = 0;
  if (oldMillis > millis())
    oldMillis = 0;
  if (debugMillis > millis())
    debugMillis = 0;
  if (hasACPowerSince > millis())
    hasACPowerSince = 0;
  if (noACPowerSince > millis())
    noACPowerSince = 0;
  if (DisplaySMSsentTimeoutMillis > millis())
    DisplaySMSsentTimeoutMillis = 0;
}

byte getSMSreceiverCount() {
  byte SMSreceiverCount = 0;
  for (int i = 0; i < maxSMSReceiver; i++) {
    if (SMSreceiver[i].length() > 1) SMSreceiverCount++;
  }
  return SMSreceiverCount;
}

