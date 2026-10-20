# 🖥️ Arduino & Python PC Hardware Monitor & Control Station

[English](#english) | [Türkçe](#türkçe)

---

<a name="english"></a>
## 🇬🇧 English

Arduino Uno, Python, and a 1.3" OLED display based hardware monitoring and control station that tracks real-time PC metrics (CPU, RAM, GPU, Temp) and allows hardware-based control over Windows audio and screen brightness.

![Arduino](https://img.shields.io/badge/Arduino-Uno-00979D?style=for-the-badge&logo=Arduino&logoColor=white)
![Python](https://img.shields.io/badge/Python-3.13-3776AB?style=for-the-badge&logo=python&logoColor=white)
![Display](https://img.shields.io/badge/Display-SH1106_OLED-blue?style=for-the-badge)

### ✨ Features
- 📊 **Real-Time Hardware Tracking:** Display CPU/RAM usage, NVIDIA GPU load, and GPU temperature with dynamic progress bars.
- 📑 **Multi-Page Interface:** Easily toggle between "CPU/RAM" and "GPU/Temp" screens via a push button.
- 🔊 **Hardware Volume Control:** Adjust Windows master volume directly using a potentiometer.
- 💡 **Auto Brightness:** Automatically adjusts OLED brightness based on ambient light using a 4-pin LDR module.
- 🚨 **Overheat Alert:** Passive buzzer alerts with a sound when GPU temperature exceeds 80°C.
- ⚡ **Zero Lag:** Optimized with 400kHz Fast I2C and non-blocking serial communication.

### 🛠️ Hardware Requirements
* Arduino Uno (or compatible board)
* 1.3" I2C OLED Display (SH1106 Controller)
* 1x Push Button
* 1x Potentiometer (10kΩ)
* 1x 4-Pin LDR Sensor Module
* 1x Passive Buzzer
* Jumper Wires & Breadboard

### 🔌 Wiring Diagram

| Component | Pin / Leg | Arduino Uno Pin |
| :--- | :--- | :--- |
| **OLED Display** | VCC / GND | 5V / GND |
| | SCL / SDA | A5 / A4 |
| **Push Button** | Signal / GND | Pin 2 / GND |
| **Potentiometer**| Left / Right / Middle | 5V / GND / **A0** |
| **4-Pin LDR** | VCC / GND / AO | 5V / GND / **A1** |
| **Buzzer** | + (Long) / - (Short)| **Pin 8** / GND |

### 💻 Installation & Usage

1. **Python Setup:**
   Install required Python packages:
   ```
   pip install psutil pynvml pycaw comtypes pyserial
   ```
Update the SERIAL_PORT variable in pc_monitor.py (e.g., "COM3") and run:
 ```
  python pc_monitor.py
 ```
1. **Arduino Setup:**
   ```
   Install U8g2 library via Arduino IDE Library Manager.
   Flash arduino_monitor.ino to your Arduino Uno.
   ```
<a name="turkce"></a>
## 🇹🇷 Türkçe

# 🖥️ Arduino & Python PC Hardware Monitor & Control Station

Arduino Uno, Python ve 1.3" OLED ekran kullanılarak geliştirilmiş, bilgisayarın anlık sistem verilerini (CPU, RAM, GPU, Sıcaklık) gösteren ve üzerindeki potansiyometre/LDR ile Windows'u yöneten çift yönlü bir masaüstü kontrol istasyonu.

![Project Banner](https://img.shields.io/badge/Arduino-Uno-00979D?style=for-the-badge&logo=Arduino&logoColor=white)
![Python](https://img.shields.io/badge/Python-3.13-3776AB?style=for-the-badge&logo=python&logoColor=white)
![OLED](https://img.shields.io/badge/Display-SH1106_OLED-blue?style=for-the-badge)

---

## ✨ Özellikler

- 📊 **Anlık Donanım Takibi:** CPU ve RAM kullanımı, NVIDIA GPU yükü ve sıcaklık değerlerinin OLED ekranda grafik barlarla gösterimi.
- 📑 **Sayfalamalı Arayüz:** Butona basarak "CPU/RAM" ve "GPU/Sıcaklık" sayfaları arasında geçiş yapabilme.
- 🔊 **Fiziksel Windows Ses Kontrolü:** Potansiyometre çevrilerek Windows ana ses seviyesinin doğrudan donanım üzerinden ayarlanması.
- 💡 **Otomatik Ekran Parlaklığı:** 4 pinli LDR modülü ile ortam ışığına göre OLED ekran parlaklığının dinamik olarak ayarlanması.
- 🚨 **Aşırı Isınma İkazı:** GPU sıcaklığı kritik eşiği (80°C) aştığında aktifleşen buzzer sesli uyarısı.
- ⚡ **Hızlı ve Akıcı:** 400kHz Fast I2C ve optimize edilmiş seri haberleşme protokolü ile sıfır takılma/gecikme.

---

## 🛠️ Gerekli Donanımlar

* Arduino Uno (veya uyumlu klon)
* 1.3 inç I2C OLED Ekran (SH1106 Sürücülü)
* 1x Push Buton
* 1x Potansiyometre (10kΩ)
* 1x 4 Pinli LDR Sensör Modülü
* 1x Pasif Buzzer
* Jumper Kablolar & Breadboard

---

## 🔌 Devre Bağlantı Şeması

| Bileşen | Pin / Bacak | Arduino Uno Pini |
| :--- | :--- | :--- |
| **OLED Ekran** | VCC / GND | 5V / GND |
| | SCL / SDA | A5 / A4 |
| **Push Buton** | Sinyal / GND | Pin 2 / GND |
| **Potansiyometre**| Sol / Sağ / Orta | 5V / GND / **A0** |
| **4 Pin LDR** | VCC / GND / AO | 5V / GND / **A1** |
| **Buzzer** | + (Uzun) / - (Kısa)| **Pin 8** / GND |

---

## 💻 Yazılım ve Kurulum

### 1. Python Tarafı (Bilgisayar)

Gerekli Python kütüphanelerini yükleyin:

```
pip install psutil pynvml pycaw comtypes pyserial
```
pc_monitor.py dosyasındaki SERIAL_PORT değişkenini kendi Arduino portunuzla (örneğin "COM3") güncelleyin ve script'i çalıştırın:
```
  python pc_monitor.py
```
### 1. Arduino Tarafı
```
Arduino IDE üzerinden U8g2 kütüphanesini (U8g2 by olikraus) yükleyin.
arduino_monitor.ino kodunu Arduino Uno kartınıza yükleyin.
```
Not: Python script'ini çalıştırmadan önce Arduino IDE'deki Seri Port Ekranı'nın (Serial Monitor) kapalı olduğundan emin olun.

---


## 📩 İletişim/Contact

Proje hakkında sorularınız, geri bildirimleriniz veya iş birliği önerileriniz için bana ulaşabilirsiniz.

Feel free to reach out for any questions, feedback, or collaboration opportunities.

* **Instagram:** [@TechByAysenur](https://www.instagram.com/techbyaysenur/) — DIY teknoloji ve robotik içeriklerim/ DIY tech & robotics content
* **LinkedIn:** [Ayşenur Karaküçük](www.linkedin.com/in/ayşenur-karaküçük-918ba2248) — Profesyonel ağım/ Professional network
* **GitHub:** [aysenurkarakucuk](https://github.com/AysenurKrkck) — Diğer projelerim/ My other projects

---
