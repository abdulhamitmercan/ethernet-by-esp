#include <SPI.h>
#include <EthernetENC.h>
#include "PubSubClient.h"

// SPI pin tanımlamaları (ENC28J60 için)
#define SPI_SCK  12    // Clock pin
#define SPI_MISO 13    // MISO pin
#define SPI_MOSI 11    // MOSI pin
#define SPI_CS   10    // Chip Select (CS) pin

// MAC adresi (değiştirilebilir)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Public MQTT Broker (Eclipse Mosquitto) kullanıyoruz
char server[] = "test.mosquitto.org";  // Mosquitto'nun public server'ı
#define CLIENT_ID "JEZIEL_78233"
#define INTERVAL 3000  // 3 saniye arayla veri gönder

// IP adresi ve DNS sunucu
IPAddress ip(192, 168, 1, 177);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress myDns(192, 168, 1, 1);

EthernetClient client;
PubSubClient mqttClient(client);

long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Serial port'un bağlanmasını bekler
  }

  Serial.println("Ethernet başlatılıyor...");
  
  // SPI bağlantısını başlat (manuel pin tanımlamaları ile)
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);

  // Ethernet bağlantısını başlat
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Ethernet DHCP ile yapılandırılamadı.");
    // Ethernet donanımının olup olmadığını kontrol et
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet donanımı bulunamadı!");
      while (true) {
        delay(1); // Donanım yoksa sonsuza kadar bekle
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet kablosu bağlı değil.");
    }
    // DHCP yerine sabit IP ile yapılandırma yap
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP ile IP alındı: ");
    Serial.println(Ethernet.localIP());
  }

  // MQTT sunucusunu belirle
  mqttClient.setServer(server, 1883); // Mosquitto public sunucusu
  Serial.println(F("MQTT istemcisi yapılandırıldı"));

  // İlk başlatma için geçen süreyi kaydet
  previousMillis = millis();
}

void loop() {
  // Bağlantı durumunu kontrol et
  auto link = Ethernet.linkStatus();
  Serial.print("Link durumu: ");
  switch (link) {
    case Unknown:
      Serial.println("Bilinmiyor");
      break;
    case LinkON:
      Serial.println("Bağlantı Açık");
      break;
    case LinkOFF:
      Serial.println("Bağlantı Kapalı");
      break;
  }
  delay(1000);

  // Belirli bir aralıkla veri gönder
  if (millis() - previousMillis > INTERVAL) {
    sendData();
    previousMillis = millis();
  }

  // MQTT istemcisi ile loop işlemi
  mqttClient.loop();
}

// MQTT sunucusuna veri gönderme fonksiyonu
void sendData() {
  // MQTT'ye bağlan
  if (mqttClient.connect(CLIENT_ID)) {
    mqttClient.publish("/MYTEST", "hello_from_public_mqtt");
    Serial.println("Mesaj gönderildi: hello_from_public_mqtt");
  } else {
    Serial.print("MQTT bağlantı hatası: ");
    Serial.println(mqttClient.state());
  }
}
