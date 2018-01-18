static const int GSM7CHARS[]  = {
  0x0040, 0x00A3, 0x0024, 0x00A5, 0x00E8, 0x00E9, 0x00F9, 0x00EC,
  0x00F2, 0x00E7, 0x000A, 0x00D8, 0x00F8, 0x000D, 0x00C5, 0x00E5,
  0x0394, 0x005F, 0x03A6, 0x0393, 0x039B, 0x03A9, 0x03A0, 0x03A8,
  0x03A3, 0x0398, 0x039E, 0x00A0, 0x00C6, 0x00E6, 0x00DF, 0x00C9,
  0x0020, 0x0021, 0x0022, 0x0023, 0x00A4, 0x0025, 0x0026, 0x0027,
  0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
  0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
  0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
  0x00A1, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
  0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
  0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
  0x0058, 0x0059, 0x005A, 0x00C4, 0x00D6, 0x00D1, 0x00DC, 0x00A7,
  0x00BF, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
  0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
  0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
  0x0078, 0x0079, 0x007A, 0x00E4, 0x00F6, 0x00F1, 0x00FC, 0x00E0,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
};

String decodeUSSD(String hex) {
  String binary = "";
  String decoded = "";
  int laenge = hex.length();

  for (int i = laenge; i >= 2; i -= 2) {
    String twos = hex.substring((i - 2), i);
    char *tc = (char *)twos.c_str();
    binary += fromHexTo8BitBinary(tc);
  }

  int gsm7Length = sizeof(GSM7CHARS);

  for (int i = binary.length(); i >= 7; i -= 7) {
    String seven = binary.substring((i - 7), i);
    char *c = (char *)seven.c_str();
    int decimalOfSeven = readBinaryString(c);
    for (int j = 0; j < gsm7Length; j++) {
      if (GSM7CHARS[j] == decimalOfSeven) {
        decoded += (char)GSM7CHARS[j];
      }
    }
  }
  return decoded;
}

int readBinaryString(char *s) {
  int result = 0;
  while (*s) {
    result <<= 1;
    if (*s++ == '1') result |= 1;
  }
  return result;
}

String fromHexTo8BitBinary(char *hex) {
  String ret = "";
  String binary =  int2bin(hex2int(hex, 2));
  int length = 8 - binary.length();
  for (int i = 0; i < length; i++) {
    ret = ret + "0";
  }
  ret = ret + binary;
  return ret;
}

unsigned long hex2int(char* a, unsigned int len) {
  int i;
  unsigned long val = 0;
  for (i = 0; i < len; i++)
    if (a[i] <= 57)
      val += (a[i] - 48) * (1 << (4 * (len - 1 - i)));
    else
      val += (a[i] - 55) * (1 << (4 * (len - 1 - i)));
  return val;
}

char* int2bin(unsigned int x) {
  static char buffer[9];
  for (int i = 0; i < 8; i++) buffer[7 - i] = '0' + ((x & (1 << i)) > 0);
  buffer[8] = '\0';
  return buffer;
}
