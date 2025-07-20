# Ballica Roket Takimi KTR Iletisim Testi 2025

**Author:** Ballica Roket Takimi  
**Version:** 1.0  
**Date:** 2025  
**License:** MIT  
**Hardware:** Arduino Mega 2560, MPU9250, MS5611, M8N GPS, LoRa E22
**Project:** Teknofest Roket Yarismasi  

## Genel Bakis

Bu kod, roket ucus verilerini toplayan ve LoRa ile iletisim kuran kapsamli bir sensör sistemidir.

## Sensörler

- **MS5611**: Sicaklik ve basinç olcumu
- **MPU6050**: Ivme, acisal hiz ve aci olcumu  
- **GPS**: Konum ve irtifa bilgisi
- **LoRa E22**: Kablosuz veri iletimi

## Pin Baglantilari

### LoRa E22
- TX: Pin 10
- RX: Pin 11
- M0: Pin 7
- M1: Pin 6

### GPS
- RX: Pin 4
- TX: Pin 3

### Funyeler
- FUNYE_1: Pin 54 (PC1)
- FUNYE_2: Pin 53 (PC0)

## Funye Tetikleme Kosullari

### FUNYE_1
- Irtifa >= 3000 metre
- Aci >= 30 derece
- Sadece bir kez tetiklenir

### FUNYE_2  
- Irtifa <= 500 metre
- Dusus tespit edildi
- Sadece bir kez tetiklenir

## LoRa Iletisimi

- **Verici Adresi**: 44
- **Alici Adresi**: 63
- **Kanal**: 23
- **Veri Boyutu**: 60 byte

## Kurulum

1. Arduino IDE'yi acin
2. Gerekli kutuphaneleri yukleyin:
   - MS5611
   - MPU6050_light
   - TinyGPS
   - LoRa_E22
3. Kodu yukleyin
4. Serial Monitor'u acin (115200 baud)

## Cikti Format

```
Zaman: 500 ms
MS5611 Barometrik Sensor Verileri:
   Sicaklik: 25.30 C
   Basinç: 1013.25 hPa
MPU6050 Ivmeolcer/Jiroskop Verileri:
   Ivme (m/s²): X=0.12 Y=-0.05 Z=9.81
   Acisal Hiz (/s): X=0.00 Y=0.00 Z=0.00
   Aci (): X=0.00 Y=0.00 Z=0.00
GPS Konum Verileri:
   Enlem: 41.008200
   Boylam: 28.978400
   GPS Irtifa: 100.5 m
Funye Durumu - FUNYE_1: PASIF | FUNYE_2: PASIF | Irtifa: 100.5m | Aci: 0.0 derece
LoRa: Veri gonderildi (Paket #1)
```

## Notlar

- GPS sinyali yoksa barometrik irtifa kullanilir
- Her 500ms'de bir olcum alinir
- Funyeler sadece bir kez tetiklenir
- LoRa iletisimi hata kontrolu yapilir

## Iletisim

**Email:** yusufkarabocekz@gmail.com


## Lisans

Bu proje MIT lisansi altinda lisanslanmistir. Detaylar icin LICENSE dosyasina bakiniz. 
