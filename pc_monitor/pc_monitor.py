import time
import warnings
import psutil
from pycaw.pycaw import AudioUtilities, ISimpleAudioVolume
import pynvml
import serial

# [TR] pycaw'ın yeni sürümünde sesi kontrol eden temiz metotlar ve COM arabirimleri
# [EN] COM interfaces and clean methods for volume control in newer pycaw versions
from comtypes import CLSCTX_ALL
from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume

# [TR] Gereksiz sistem/kütüphane uyarılarını gizle
# [EN] Suppress unnecessary system/library warnings
warnings.filterwarnings("ignore")

# -------------------------------------------------------------------
# SERİ PORT VE İLETİŞİM AYARLARI / SERIAL PORT & COMMUNICATION CONFIG
# -------------------------------------------------------------------
SERIAL_PORT = "COM3"  # [TR] Arduino'nun bağlı olduğu COM portu / [EN] COM port attached to Arduino
BAUD_RATE = 115200    # [TR] Arduino setup'ındaki baud rate ile eşleşmeli / [EN] Must match Arduino setup baud rate

# [TR] Seri port bağlantısını başlat ve Arduino'nun sıfırlanması için bekle
# [EN] Initialize serial port connection and wait for Arduino to reset
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
time.sleep(2)

# -------------------------------------------------------------------
# SES YÖNETİMİ BAŞLATMA / AUDIO MANAGEMENT INITIALIZATION
# -------------------------------------------------------------------
# [TR] Windows ana ses seviyesini değiştirmek için pycaw arabirimini al
# [EN] Acquire pycaw interface to control Windows master volume
try:
    devices = AudioUtilities.GetSpeakers()
    interface = devices.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
    volume = interface.QueryInterface(IAudioEndpointVolume)
except AttributeError:
    # [TR] Üstteki yöntem başarısız olursa alternatif modern pycaw çağrısı
    # [EN] Fallback modern pycaw call if the method above fails
    device = AudioUtilities.GetSpeakers()
    volume = device.EndpointVolume

# -------------------------------------------------------------------
# GPU YÖNETİMİ BAŞLATMA (NVIDIA NVML) / GPU MANAGEMENT INITIALIZATION
# -------------------------------------------------------------------
has_gpu = False
gpu_handle = None
try:
    pynvml.nvmlInit()  # [TR] NVML sürücüsünü başlat / [EN] Initialize NVML driver
    gpu_handle = pynvml.nvmlDeviceGetHandleByIndex(0)  # [TR] İlk GPU'yu seç / [EN] Select primary GPU
    has_gpu = True
    print("NVIDIA GPU Algılandı.")
except Exception as e:
    print(f"GPU Hatası: {e}")

# [TR] Son bilinen varsayılan GPU değerleri
# [EN] Fallback default GPU metrics
last_gpu_load = 0
last_gpu_temp = 45

print("Sistem İzleme Başlatıldı...")

# -------------------------------------------------------------------
# ANA DÖNGÜ / MAIN MONITORING LOOP
# -------------------------------------------------------------------
try:
    while True:
        # ---------------------------------------------------------------
        # 1. ARDUINO'DAN GELEN SES KOMUTUNU DİNLE / READ VOLUME COMMAND FROM ARDUINO
        # ---------------------------------------------------------------
        # [TR] Örn: Arduino "VOL:75\n" gönderdiğinde bu bloğa girer
        # [EN] Triggers when Arduino sends e.g. "VOL:75\n"
        if ser.in_waiting > 0:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if line.startswith("VOL:"):
                try:
                    target_vol = int(line.split(":")[1])
                    # [TR] Windows sesini %0-100 değerinden 0.0-1.0 aralığına dönüştürerek ayarla
                    # [EN] Convert 0-100% value to 0.0-1.0 scalar and set Windows master volume
                    volume.SetMasterVolumeLevelScalar(target_vol / 100.0, None)
                    print(f"Windows Sesi Ayarlandı: %{target_vol}")
                except Exception as err:
                    print(f"Ses Ayar Hatası: {err}")

        # ---------------------------------------------------------------
        # 2. SİSTEM BİLGİLERİNİ OKU / FETCH SYSTEM METRICS
        # ---------------------------------------------------------------
        # [TR] İşlemci ve RAM kullanım yüzdelerini psutil ile al
        # [EN] Fetch CPU and RAM usage percentages via psutil
        cpu = int(psutil.cpu_percent(interval=0.1))
        ram = int(psutil.virtual_memory().percent)

        # [TR] Eğer NVIDIA GPU mevcutsa yük ve sıcaklık verilerini oku
        # [EN] Fetch GPU load and temperature if NVIDIA GPU is available
        if has_gpu and gpu_handle:
            try:
                last_gpu_load = int(
                    pynvml.nvmlDeviceGetUtilizationRates(gpu_handle).gpu
                )
            except Exception:
                pass

            try:
                t = pynvml.nvmlDeviceGetTemperature(
                    gpu_handle, pynvml.NVML_TEMPERATURE_GPU
                )
                if t > 0:
                    last_gpu_temp = int(t)
            except Exception:
                pass

        # ---------------------------------------------------------------
        # 3. VERİYİ ARDUINO'YA GÖNDER / TRANSMIT DATA TO ARDUINO
        # ---------------------------------------------------------------
        # [TR] "CPU,RAM,GPU_LOAD,GPU_TEMP\n" formatında Seri Port üzerinden gönder
        # [EN] Send comma-separated values via Serial in "CPU,RAM,GPU_LOAD,GPU_TEMP\n" format
        data_string = f"{cpu},{ram},{last_gpu_load},{last_gpu_temp}\n"
        ser.write(data_string.encode("utf-8"))

        time.sleep(0.2)  # [TR] Döngü tazeleme süresi (~5Hz) / [EN] Loop refresh interval (~5Hz)

# -------------------------------------------------------------------
# GÜVENLİ KAPATMA / SAFE SHUTDOWN ON INTERRUPT
# -------------------------------------------------------------------
except KeyboardInterrupt:
    # [TR] Kullanıcı Ctrl+C ile programı durdurduğunda kaynakları serbest bırak
    # [EN] Release resources when user terminates the script with Ctrl+C
    if has_gpu:
        try:
            pynvml.nvmlShutdown()
        except Exception:
            pass
    ser.close()
    print("Kapatıldı.")