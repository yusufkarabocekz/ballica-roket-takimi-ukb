/*
 * Ballica Roket Takimi KTR Iletisim Testi 2025 Verici
 * 
 * Author: Ballica Roket Takimi
 * Version: 1.0
 * Date: 2025
 * License: MIT
 * Hardware: Arduino Mega 2560, MPU6050, MS5611, M8N GPS, LoRa E22
 * Project: Teknofest Roket Yarismasi
 * 
 * Bu kodda verici adresi 44 kanal 23 ||| alici adresi 63 kanal 23
 * kanallar sabit olmak zorunda!!!
 */

#include <Wire.h>
#include "MS5611.h"
#include <MPU6050_light.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include "LoRa_E22.h"

#define M0 7
#define M1 6
#define FUNYE_1 54  // PC1 - İrtifa 3000m ve açı 30° için
#define FUNYE_2 53  // PC0 - İrtifa 500m altı ve düşüş için

SoftwareSerial mySerial(10, 11);  // LoRa TX-10 pinine bagla RX-11 pine bagla
LoRa_E22 e22ttl(&mySerial);

// MS5611 sensör nesnesi (I2C adresi: 0x77)
MS5611 MS5611(0x77);

MPU6050 mpu(Wire);

TinyGPS gps;
SoftwareSerial ss(4, 3);

float lat, lon, altitude;
float accX, accY, accZ;
float gyroX, gyroY, gyroZ;
float angleX, angleY, angleZ;
float temperature, pressure;
float paket_sayisi = 0;

// Fünye kontrol değişkenleri
float onceki_altitude = 0;
bool funye_1_tetiklendi = false;
bool funye_2_tetiklendi = false;

struct Signal {
  byte paket_sayisi[4];
  byte lat[4];
  byte lon[4];
  byte altitude[4];
  byte accX[4];
  byte accY[4];
  byte accZ[4];
  byte gyroX[4];
  byte gyroY[4];
  byte gyroZ[4];
  byte angleX[4];
  byte angleY[4];
  byte angleZ[4];
  byte temperature[4];
  byte pressure[4];
} data;

void setup() {
  // Serial haberleşmeyi başlat
  Serial.begin(115200);
  while (!Serial) {
    ;  // Serial port hazır olana kadar bekle
  }

  ss.begin(4800);

  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);
  digitalWrite(M1, LOW);

  // Fünye pinlerini çıkış olarak ayarla
  pinMode(FUNYE_1, OUTPUT);
  pinMode(FUNYE_2, OUTPUT);
  digitalWrite(FUNYE_1, LOW);
  digitalWrite(FUNYE_2, LOW);

  Serial.println("╔══════════════════════════════════════════════════════════════╗");
  Serial.println("║                    SENSÖR TEST SİSTEMİ                      ║");
  Serial.println("║        MS5611 + MPU6050 + GPS + LoRa E22 Sensörleri        ║");
  Serial.println("╚══════════════════════════════════════════════════════════════╝");
  Serial.println();

  // I2C haberleşmeyi başlat
  Wire.begin();

  // LoRa E22 modülünü başlat
  Serial.println("LoRa E22 modülü başlatılıyor...");
  e22ttl.begin();
  Serial.println("LoRa E22 modülü başlatıldı (Verici: 44, Alıcı: 63, Kanal: 23)");

  // MS5611 sensörünü başlat
  Serial.println("MS5611 Barometrik Basınç Sensörü kontrol ediliyor...");
  if (MS5611.begin() == true) {
    Serial.print("MS5611 sensörü bulundu (I2C: 0x");
    Serial.print(MS5611.getAddress(), HEX);
    Serial.println(")");
  } else {
    Serial.println("HATA: MS5611 sensörü bulunamadı!");
    Serial.println("   Bağlantıları kontrol edin ve tekrar deneyin.");
  }

  // MPU6050 sensörünü başlat
  Serial.println("MPU6050 İvmeölçer/Jiroskop sensörü kontrol ediliyor...");
  byte status = mpu.begin();
  Serial.print("MPU6050 durumu: ");
  Serial.println(status);

  if (status == 0) {
    Serial.println("MPU6050 sensörü başarıyla başlatıldı");
    Serial.println("Kalibrasyon yapılıyor... (MPU6050'yi hareket ettirmeyin!)");
    delay(1000);
    mpu.calcOffsets(true, true);  // gyro ve accelero kalibrasyonu
    Serial.println("Kalibrasyon tamamlandı!");
  } else {
    Serial.println("HATA: MPU6050 sensörü başlatılamadı!");
  }

  // GPS sensörünü başlat
  Serial.println("GPS sensörü kontrol ediliyor...");
  Serial.println("GPS başlatıldı (RX: 4, TX: 3)");
  Serial.println("GPS sinyali bekleniyor...");

  Serial.println();
  Serial.println("Ölçümler başlıyor...");
  Serial.println("╔══════════════════════════════════════════════════════════════╗");
  delay(1000);
}

void loop() {
  // Her 500ms'de bir ölçüm al
  delay(500);

  // Zaman damgası
  Serial.print("Zaman: ");
  Serial.print(millis());
  Serial.println(" ms");

  // MS5611 verilerini oku ve göster
  readMS5611();

  // MPU6050 verilerini oku ve göster
  readMPU6050();

  // GPS verilerini oku ve göster
  readGPS();

  Serial.println("╔══════════════════════════════════════════════════════════════╗");

  // Verileri LoRa paketine hazırla
  prepareLoRaData();
  
  // LoRa ile veri gönder
  sendLoRaData();
  
  // Fünye kontrolü yap
  checkFunyaControl();
}

void readMS5611() {
  // Sensörden veri oku
  int result = MS5611.read();

  // Ölçüm sonucunu kontrol et
  if (result != MS5611_READ_OK) {
    Serial.println("MS5611 ölçüm hatası!");
    return;
  }

  // Verileri global değişkenlere ata
  pressure = MS5611.getPressure();
  temperature = MS5611.getTemperature();

  // Ölçüm değerlerini güzel formatta yazdır
  Serial.println("MS5611 Barometrik Sensör Verileri:");
  Serial.print("   Sıcaklık: ");
  Serial.print(temperature, 2);
  Serial.println(" °C");

  Serial.print("   Basınç: ");
  Serial.print(pressure, 2);
  Serial.println(" hPa");
}

void readMPU6050() {
  // Sensör verilerini güncelle
  mpu.update();

  // Verileri global değişkenlere ata
  accX = mpu.getAccX();
  accY = mpu.getAccY();
  accZ = mpu.getAccZ();
  gyroX = mpu.getGyroX();
  gyroY = mpu.getGyroY();
  gyroZ = mpu.getGyroZ();
  angleX = mpu.getAngleX();
  angleY = mpu.getAngleY();
  angleZ = mpu.getAngleZ();

  // İvmeölçer verilerini güzel formatta yazdır
  Serial.println("MPU6050 İvmeölçer/Jiroskop Verileri:");

  Serial.print("   İvme (m/s²): X=");
  Serial.print(accX, 2);
  Serial.print(" Y=");
  Serial.print(accY, 2);
  Serial.print(" Z=");
  Serial.println(accZ, 2);

  Serial.print("   Açısal Hız (°/s): X=");
  Serial.print(gyroX, 2);
  Serial.print(" Y=");
  Serial.print(gyroY, 2);
  Serial.print(" Z=");
  Serial.println(gyroZ, 2);

  Serial.print("   Açı (°): X=");
  Serial.print(angleX, 2);
  Serial.print(" Y=");
  Serial.print(angleY, 2);
  Serial.print(" Z=");
  Serial.println(angleZ, 2);
}

void readGPS() {

  bool newData = false;

  // GPS verilerini oku (kısa süre)
  for (unsigned long start = millis(); millis() - start < 100;) {
    while (ss.available()) {
      char c = ss.read();
      if (gps.encode(c)) {
        newData = true;
      }
    }
  }

  if (newData) {
    unsigned long age;

    // Konum bilgilerini al
    gps.f_get_position(&lat, &lon, &age);

    // İrtifa bilgisini al
    altitude = gps.f_altitude();

    // Verileri yazdır
    Serial.println("GPS Konum Verileri:");
    Serial.print("   Enlem: ");
    Serial.print(lat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : lat, 6);
    Serial.println("°");

    Serial.print("   Boylam: ");
    Serial.print(lon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : lon, 6);
    Serial.println("°");

    Serial.print("   GPS İrtifa: ");
    Serial.print(altitude == TinyGPS::GPS_INVALID_F_ALTITUDE ? 0.0 : altitude, 1);
    Serial.println(" m");
  } else {
    Serial.println("GPS: Sinyal bekleniyor...");
  }
}

void checkFunyaControl() {
  // GPS irtifa değerini kullan (eğer geçerliyse)
  float current_altitude = altitude;
  
  // Eğer GPS irtifa geçerli değilse, barometrik irtifa kullan
  if (altitude == TinyGPS::GPS_INVALID_F_ALTITUDE || altitude == 0.0) {
    // Barometrik irtifa hesaplama (yaklaşık)
    current_altitude = 44330 * (1 - pow(pressure / 1013.25, 0.1903));
  }
  
  // FUNYE_1 Kontrolu: Irtifa 3000m ve aci 30 derece uzeri
  if (!funye_1_tetiklendi && current_altitude >= 3000.0 && abs(angleX) >= 30.0) {
    digitalWrite(FUNYE_1, HIGH);
    funye_1_tetiklendi = true;
    Serial.println("FUNYE_1 TETIKLENDI! (Irtifa: 3000m+, Aci: 30 derece+)");
  }
  
  // FUNYE_2 Kontrolu: Irtifa 500m alti ve dusus
  if (!funye_2_tetiklendi && current_altitude <= 500.0 && current_altitude < onceki_altitude) {
    digitalWrite(FUNYE_2, HIGH);
    funye_2_tetiklendi = true;
    Serial.println("FUNYE_2 TETIKLENDI! (Irtifa: 500m alti, Dusus tespit edildi)");
  }
  
  // Funye durumlarini goster
  Serial.print("Funye Durumu - FUNYE_1: ");
  Serial.print(digitalRead(FUNYE_1) ? "AKTIF" : "PASIF");
  Serial.print(" | FUNYE_2: ");
  Serial.print(digitalRead(FUNYE_2) ? "AKTIF" : "PASIF");
  Serial.print(" | Irtifa: ");
  Serial.print(current_altitude, 1);
  Serial.print("m | Aci: ");
  Serial.print(angleX, 1);
  Serial.println(" derece");
  
  // Onceki irtifa degerini guncelle
  onceki_altitude = current_altitude;
}

void prepareLoRaData() {
  // Paket sayısını artır
  paket_sayisi++;
  
  // Float değerleri byte dizilerine dönüştür
  *(float*)(data.paket_sayisi) = paket_sayisi;
  *(float*)(data.lat) = lat;
  *(float*)(data.lon) = lon;
  *(float*)(data.altitude) = altitude;
  *(float*)(data.accX) = accX;
  *(float*)(data.accY) = accY;
  *(float*)(data.accZ) = accZ;
  *(float*)(data.gyroX) = gyroX;
  *(float*)(data.gyroY) = gyroY;
  *(float*)(data.gyroZ) = gyroZ;
  *(float*)(data.angleX) = angleX;
  *(float*)(data.angleY) = angleY;
  *(float*)(data.angleZ) = angleZ;
  *(float*)(data.temperature) = temperature;
  *(float*)(data.pressure) = pressure;
}

void sendLoRaData() {
  // LoRa ile veri gönder (Verici: 44, Alıcı: 63, Kanal: 23)
  ResponseStatus rs = e22ttl.sendFixedMessage(44, 63, 23, &data, sizeof(Signal));
  
  if (rs.code == 1) {
    Serial.print("LoRa: Veri gönderildi (Paket #");
    Serial.print(paket_sayisi);
    Serial.println(")");
  } else {
    Serial.print("LoRa: Gönderme hatası - ");
    Serial.println(rs.getResponseDescription());
  }
}
