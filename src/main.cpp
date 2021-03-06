#include "HoneyNode.h"
#include "Ticker.h"
#include "custom_types.h"
#include "sensors.h"
#include "power.h"
#include "anomaly.h"
#include "EEPROM.h"

HoneyNode honey(RF_CE, RF_CS);

void reportReadouts() {
    auto data = getSensorReadouts();
    auto result = honey.Send(CH_READOUTS, data);
    if (result != 0) Serial.println("[READOUTS] Report Failed");
}

void reportPower() {
    auto data = getPowerReadouts();
    auto result = honey.Send(CH_POWER, data);
    if (result != 0) Serial.println("[POWER] Report Failed");
}

void reportAnomaly() {
    if (hasAnyAnomaly()) {
        honey.Send(CH_ANOMALY, anomaly_flags);
        clearAnomaly(AN_INTRUSION); // Intrusion is a one-off alarm
    }
}

Ticker readoutTimer(reportReadouts, 2000);
Ticker powerTimer(reportPower, 10000);
Ticker anomalyTimer(reportAnomaly, 5000);

void setup() {
    // Setting up everything
    Serial.begin(115200);
    Serial.setTimeout(5000);
    pinMode(BTN_RESET, INPUT_PULLUP);
    #ifndef NOSENSOR
        initSensors();
    #endif
    analogReference(INTERNAL);
    // Clear NodeID
    if (!digitalRead(BTN_RESET)) {
        EEPROM.write(0, 0);
        Serial.println("Node ID is cleared");
    }
    // Configure Net
    honey.begin();
    honey.AddChannel(CH_ANOMALY, uint32_t);
    #ifndef NOPOWER
        honey.AddChannel(CH_POWER, power_t);
    #endif
    #ifndef NOSENSOR
        honey.AddChannel(CH_READOUTS, readouts_t);
    #endif

    // Begin Operations
    #ifndef NOPOWER
        powerTimer.start();
    #endif
    anomalyTimer.start();
    #ifndef NOSENSOR
        readoutTimer.start();
    #endif
}

void loop() {
    honey.update();
    #ifndef NOPOWER
        powerTimer.update();
    #endif
    anomalyTimer.update();
    #ifndef NOSENSOR
        readoutTimer.update();
    #endif
}
