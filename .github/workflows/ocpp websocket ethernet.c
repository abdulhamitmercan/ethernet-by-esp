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

    /*
     * Energize EV plug if OCPP transaction is up and running
     */
    if (ocppPermitsCharge()) {
        //OCPP set up and transaction running. Energize the EV plug here
    } else {
        //No transaction running at the moment. De-energize EV plug
    }

    /*
     * Use NFC reader to start and stop transactions
     */
    if (/* RFID chip detected? */ false) {
        String idTag = "0123456789ABCD"; //e.g. idTag = RFID.readIdTag();

        if (!getTransaction()) {
            //no transaction running or preparing. Begin a new transaction
            Serial.printf("[main] Begin Transaction with idTag %s\n", idTag.c_str());

            /*
             * Begin Transaction. The OCPP lib will prepare transaction by checking the Authorization
             * and listen to the ConnectorPlugged Input. When the Authorization succeeds and an EV
             * is plugged, the OCPP lib will send the StartTransaction
             */
            auto ret = beginTransaction(idTag.c_str());

            if (ret) {
                Serial.println(F("[main] Transaction initiated. OCPP lib will send a StartTransaction when" \
                                 "ConnectorPlugged Input becomes true and if the Authorization succeeds"));
            } else {
                Serial.println(F("[main] No transaction initiated"));
            }

        } else {
            //Transaction already initiated. Check if to stop current Tx by RFID card
            if (idTag.equals(getTransactionIdTag())) {
                //card matches -> user can stop Tx
                Serial.println(F("[main] End transaction by RFID card"));

                endTransaction(idTag.c_str());
            } else {
                Serial.println(F("[main] Cannot end transaction by RFID card (different card?)"));
            }
        }
    }

    //... see MicroOcpp.h for more possibilities
}
  
