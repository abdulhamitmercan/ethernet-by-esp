#include <Arduino.h>
#include <EthernetENC.h>
#include <MicroOcpp.h>

// Ethernet için MAC adresi (değiştirilebilir)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// OCPP Backend URL ve Charge Box ID
#define OCPP_BACKEND_URL   "ws://cyberia.com.tr:8080"
#define OCPP_CHARGE_BOX_ID "steve/websocket/CentralSystemService/BT11028002"

EthernetClient client;

void setup() {
    Serial.begin(115200);

    Serial.print(F("[main] Wait for Ethernet: "));

    // Ethernet bağlantısını başlat
    if (Ethernet.begin(mac) == 0) {
        Serial.println(F("Ethernet DHCP ile yapılandırılamadı."));
        // Ethernet donanımının olup olmadığını kontrol et
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
            Serial.println(F("Ethernet donanımı bulunamadı!"));
            while (true) {
                delay(1); // Donanım yoksa sonsuza kadar bekle
            }
        }
        if (Ethernet.linkStatus() == LinkOFF) {
            Serial.println(F("Ethernet kablosu bağlı değil."));
        }
    } else {
        Serial.print(F("  DHCP ile IP alındı: "));
        Serial.println(Ethernet.localIP());
    }

    Serial.println(F(" connected!"));

    /*
     * Initialize the OCPP library
     */
    mocpp_initialize(OCPP_BACKEND_URL, OCPP_CHARGE_BOX_ID, "My Charging Station", "My company name");

    /*
     * Integrate OCPP functionality. You can leave out the following part if your EVSE doesn't need it.
     */
    setEnergyMeterInput([]() {
        //take the energy register of the main electricity meter and return the value in watt-hours
        return 0.f;
    });

    setSmartChargingCurrentOutput([](float limit) {
        //set the SAE J1772 Control Pilot value here
        Serial.printf("[main] Smart Charging allows maximum charge rate: %.0f\n", limit);
    });

    setConnectorPluggedInput([]() {
        //return true if an EV is plugged to this EVSE
        return false;
    });

    //... see MicroOcpp.h for more settings
}

void loop() {

    /*
     * Do all OCPP stuff (process WebSocket input, send recorded meter values to Central System, etc.)
     */
    mocpp_loop();
    delay(5);
}
  
