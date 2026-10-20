#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// [TR] 1.3 inç SH1106 I2C OLED ekran tanımlaması (Reset pini kullanılmıyor)
// [EN] 1.3-inch SH1106 I2C OLED display initialization (No reset pin used)
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// [TR] Pin Tanımlamaları
// [EN] Pin Definitions
const int BUTTON_PIN = 2; // [TR] Sayfa geçiş butonu / [EN] Page toggle button
const int BUZZER_PIN = 8; // [TR] Uyarı ses üreteci / [EN] Alarm buzzer
const int POT_PIN    = A0; // [TR] Ses kontrol potansiyometresi / [EN] Volume control potentiometer
const int LDR_PIN    = A1; // [TR] 4 Pinli LDR Modülünün AO (Analog Out) pini / [EN] AO pin of 4-pin LDR module

// [TR] Durum ve sistem değişkenleri
// [EN] State and system variables
int currentPage = 1;          // [TR] Aktif sayfa (1: CPU/RAM, 2: GPU) / [EN] Active page (1: CPU/RAM, 2: GPU)
bool lastButtonState = HIGH;  // [TR] Butonun önceki durumu / [EN] Previous state of the button

// [TR] PC'den alınacak ve işlenecek sistem verileri
// [EN] System metrics received from PC and processed
int cpuVal = 0, ramVal = 0, gpuLoad = 0, gpuTemp = 0;
int lastVol = -1;              // [TR] Son gönderilen ses seviyesi / [EN] Last transmitted volume level
unsigned long lastAlarmTime = 0; // [TR] Son buzzer alarmı zaman damgası / [EN] Timestamp of the last buzzer alarm

void setup() {
  // [TR] Seri haberleşmeyi başlat ve Python tarafındaki veri aktarımı için zaman aşımını ayarla
  // [EN] Start serial communication and set timeout for Python data transfer
  Serial.begin(115200);
  Serial.setTimeout(50); // [TR] Takılmaları ve gecikmeleri önleyen ayar / [EN] Prevents delays and blocking

  // [TR] Pin modlarını ayarla (Dahili pull-up direnci ile buton kullanımı)
  // [EN] Configure pin modes (Button uses internal pull-up resistor)
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  // [TR] Ekranı başlat ve I2C saat hızını yüksek moda getir
  // [EN] Initialize display and set I2C clock speed to fast mode
  u8g2.begin();
  Wire.setClock(400000); // [TR] Fast I2C modu (400kHz) / [EN] Fast I2C mode (400kHz)
}

void loop() {
  // -------------------------------------------------------------------
  // 1. BUTON İLE SAYFA GEÇİŞİ / PAGE SWITCHING WITH BUTTON
  // -------------------------------------------------------------------
  bool reading = digitalRead(BUTTON_PIN);
  // [TR] Butona basıldığını algıla (Düşen kenar tetiklemesi: HIGH -> LOW)
  // [EN] Detect button press (Falling edge trigger: HIGH -> LOW)
  if (reading == LOW && lastButtonState == HIGH) {
    currentPage = (currentPage == 1) ? 2 : 1; // [TR] Sayfayı değiştir / [EN] Toggle page
    delay(30); // [TR] Ark önleme gecikmesi / [EN] Debounce delay
  }
  lastButtonState = reading;

  // -------------------------------------------------------------------
  // 2. POTANSİYOMETRE İLE SES KONTROLÜ / VOLUME CONTROL WITH POTENTIOMETER
  // -------------------------------------------------------------------
  int rawPot = analogRead(POT_PIN);
  int currentVol = map(rawPot, 0, 1023, 0, 100); // [TR] 0-1023 değerini %0-100 ölçeğine çevir / [EN] Scale 0-1023 reading to 0-100%
  
  // [TR] Anlık dalgalanmaları ve gereksiz Seri haberleşmeyi önlemek için en az %2 değişim kontrolü
  // [EN] Check for at least 2% change to prevent analog jitter and excessive Serial traffic
  if (abs(currentVol - lastVol) >= 2) {
    lastVol = currentVol;
    Serial.print("VOL:");
    Serial.println(currentVol); // [TR] Seri port üzerinden Python'a gönder / [EN] Send to Python via serial
  }

  // -------------------------------------------------------------------
  // 3. LDR İLE OTOMATİK PARLAKLIK / AUTOMATIC BRIGHTNESS VIA LDR
  // -------------------------------------------------------------------
  int ldrVal = analogRead(LDR_PIN);
  // [TR] LDR okumasına göre kontrastı ayarla (Karanlıkta kısık, aydınlıkta parlak)
  // [EN] Adjust contrast based on LDR reading (Dim in dark, bright in ambient light)
  uint8_t contrast = map(constrain(ldrVal, 100, 900), 100, 900, 20, 255);
  u8g2.setContrast(contrast);

  // -------------------------------------------------------------------
  // 4. KRİTİK GPU SICAKLIK UYARISI / CRITICAL GPU TEMP BUZZER ALARM
  // -------------------------------------------------------------------
  // [TR] GPU sıcaklığı 80°C üzeri olduğunda periyodik uyarı tonu çal
  // [EN] Trigger periodic warning tone if GPU temperature reaches or exceeds 80°C
  if (gpuTemp >= 80) {
    if (millis() - lastAlarmTime > 3000) { // [TR] Her 3 saniyede bir / [EN] Every 3 seconds
      tone(BUZZER_PIN, 1500, 100);        // [TR] 1.5kHz ton, 100ms süre / [EN] 1.5kHz tone for 100ms
      lastAlarmTime = millis();
    }
  }

  // -------------------------------------------------------------------
  // 5. PYTHON'DAN GELEN VERİLERİ OKUMA / READ INCOMING DATA FROM PYTHON
  // -------------------------------------------------------------------
  // [TR] Seri portta veri varsa oku ve virgüülle ayrılmış formatı değişkenlere ayrıştır
  // [EN] Check serial buffer, read line, and parse comma-separated values
  if (Serial.available() > 0) {
    char buffer[32];
    byte len = Serial.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
    buffer[len] = '\0'; // [TR] Metin dizisini sonlandır / [EN] Null-terminate string
    
    // [TR] "CPU,RAM,GPU_LOAD,GPU_TEMP" formatındaki veriyi çöz
    // [EN] Parse string format "CPU,RAM,GPU_LOAD,GPU_TEMP"
    sscanf(buffer, "%d,%d,%d,%d", &cpuVal, &ramVal, &gpuLoad, &gpuTemp);
  }

  // -------------------------------------------------------------------
  // 6. EKRAN ÇİZİMİ / DISPLAY RENDERING (U8g2 First/Next Page Loop)
  // -------------------------------------------------------------------
  u8g2.firstPage();
  do {
    // [TR] Üst Başlık Barı Çizimi
    // [EN] Render Header Bar
    u8g2.setFont(u8g2_font_6x10_tf);
    if (currentPage == 1) {
      u8g2.drawStr(20, 10, "SYSTEM: CPU/RAM");
    } else {
      u8g2.drawStr(28, 10, "SYSTEM: GPU");
    }
    u8g2.drawHLine(0, 13, 128); // [TR] Başlık altı ayırıcı çizgi / [EN] Separator line below header

    if (currentPage == 1) {
      // ---------------------------------------------------------------
      // SAYFA 1: CPU & RAM GÖSTERGELERİ / PAGE 1: CPU & RAM METRICS
      // ---------------------------------------------------------------
      u8g2.setFont(u8g2_font_5x7_tf);
      
      // [TR] CPU Değeri ve İlerleme Çubuğu
      // [EN] CPU Value and Progress Bar
      u8g2.setCursor(2, 28);
      u8g2.print("CPU: ");
      u8g2.print(cpuVal);
      u8g2.print("%");
      u8g2.drawFrame(55, 21, 70, 9); // [TR] Çerçeve / [EN] Outer frame
      u8g2.drawBox(57, 23, map(constrain(cpuVal, 0, 100), 0, 100, 0, 66), 5); // [TR] Dolgu / [EN] Fill box

      // [TR] RAM Değeri ve İlerleme Çubuğu
      // [EN] RAM Value and Progress Bar
      u8g2.setCursor(2, 46);
      u8g2.print("RAM: ");
      u8g2.print(ramVal);
      u8g2.print("%");
      u8g2.drawFrame(55, 39, 70, 9);
      u8g2.drawBox(57, 41, map(constrain(ramVal, 0, 100), 0, 100, 0, 66), 5);
    } else {
      // ---------------------------------------------------------------
      // SAYFA 2: GPU & SICAKLIK GÖSTERGELERİ / PAGE 2: GPU & TEMP METRICS
      // ---------------------------------------------------------------
      u8g2.setFont(u8g2_font_5x7_tf);
      
      // [TR] GPU Yükü ve İlerleme Çubuğu
      // [EN] GPU Load and Progress Bar
      u8g2.setCursor(2, 28);
      u8g2.print("GPU: ");
      u8g2.print(gpuLoad);
      u8g2.print("%");
      u8g2.drawFrame(55, 21, 70, 9);
      u8g2.drawBox(57, 23, map(constrain(gpuLoad, 0, 100), 0, 100, 0, 66), 5);

      // [TR] GPU Sıcaklığı ve İlerleme Çubuğu (30°C - 90°C arası ölçekli)
      // [EN] GPU Temperature and Progress Bar (Scaled between 30°C and 90°C)
      u8g2.setCursor(2, 46);
      u8g2.print("TEMP: ");
      u8g2.print(gpuTemp);
      u8g2.print(" C");
      u8g2.drawFrame(55, 39, 70, 9);
      int tempWidth = map(constrain(gpuTemp, 30, 90), 30, 90, 0, 66);
      u8g2.drawBox(57, 41, tempWidth, 5);
    }

    // -----------------------------------------------------------------
    // ALT SAYFA İNDİKATÖR NOKTALARI / BOTTOM PAGE INDICATOR DOTS
    // -----------------------------------------------------------------
    // [TR] Dolu daire (disc) aktif sayfayı, boş daire (circle) pasif sayfayı temsil eder
    // [EN] Filled circle (disc) indicates active page, outline circle indicates inactive page
    if (currentPage == 1) {
      u8g2.drawDisc(60, 58, 2);   // [TR] Sayfa 1 Aktif / [EN] Page 1 Active
      u8g2.drawCircle(68, 58, 2); // [TR] Sayfa 2 Pasif / [EN] Page 2 Inactive
    } else {
      u8g2.drawCircle(60, 58, 2); // [TR] Sayfa 1 Pasif / [EN] Page 1 Inactive
      u8g2.drawDisc(68, 58, 2);   // [TR] Sayfa 2 Aktif / [EN] Page 2 Active
    }
  } while (u8g2.nextPage());
}
