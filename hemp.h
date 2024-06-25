void showLogo() {
  tft.fillScreen(ST77XX_BLACK); // Bildschirm mit Schwarz füllen
  tft.drawRGBBitmap(10, 0, hemp, 128, 128); // Bild anzeigen
  int greenhemp = 180;
  int colorhemp = 100;

  tft.setTextColor(tft.color565(colorhemp, greenhemp, colorhemp)); // Textfarbe auf dunkleres Grün setzen (RGB: 0, 128, 0)
  tft.setTextSize(3);             // Textgröße einstellen
  tft.setCursor(160, 25);         // Position für den Text setzen
  tft.println(F("NUF"));          // Erste Zeile Text
  tft.setCursor(165, 60);         // Position für die zweite Zeile Text setzen
  tft.println(F("SI"));           // Zweite Zeile Text
  tft.setCursor(160, 95);         // Position für die dritte Zeile Text setzen
  tft.println(F("TOP"));          // Dritte Zeile Text

  delay(1500);
  tft.setTextColor(ST77XX_BLACK); // Textfarbe auf Schwarz setzen
  tft.setCursor(165, 60);         // Position für die zweite Zeile Text setzen
  tft.println(F("SI"));           // Zweite Zeile Text
  delay(100);
  tft.setTextColor(tft.color565(colorhemp, greenhemp, colorhemp)); // Textfarbe auf dunkleres Grün setzen (RGB: 0, 128, 0)
  tft.setCursor(165, 60);         // Position für die zweite Zeile Text setzen
  tft.println(F("IS"));           // Zweite Zeile Text
  delay(300);
  tft.setTextColor(ST77XX_BLACK); // Textfarbe auf Schwarz setzen
  tft.setCursor(160, 95);         // Position für die dritte Zeile Text setzen
  tft.println(F("TOP"));          // Dritte Zeile Text
  delay(100);
  tft.setTextColor(tft.color565(colorhemp, greenhemp, colorhemp)); // Textfarbe auf dunkleres Grün setzen (RGB: 0, 128, 0)
  tft.setCursor(160, 95);         // Position für die dritte Zeile Text setzen
  tft.println(F("FUN"));          // Dritte Zeile Text
  delay(300);
  tft.setTextColor(ST77XX_BLACK); // Textfarbe auf Schwarz setzen
  tft.setCursor(160, 25);         // Position für den Text setzen
  tft.println(F("NUF"));          // Erste Zeile Text
  delay(100);
  tft.setTextColor(tft.color565(colorhemp, greenhemp, colorhemp)); // Textfarbe auf dunkleres Grün setzen (RGB: 0, 128, 0)
  tft.setCursor(160, 25);         // Position für den Text setzen
  tft.println(F("POT"));          // Erste Zeile Text
  delay(2000);
  tft.fillScreen(ST77XX_BLACK); // Bildschirm mit Schwarz füllen
}