#include <LoRa.h>
#include <SPI.h>

// LoRa pins
#define ss   5
#define rst  14
#define dio0 2

// RGB LED pins (common cathode — HIGH = on)
#define LED_R 12
#define LED_G 13
#define LED_B 15

void setLED(bool r, bool g, bool b) {
    digitalWrite(LED_R, r);
    digitalWrite(LED_G, g);
    digitalWrite(LED_B, b);
}

// Packet structure (must match TX):
// [0] temperature    : uint8  -> degrees C         (decode: as-is)
// [1] pressure       : uint8  -> hPa - 900         (decode: + 900)
// [2] humidity_air   : uint8  -> % RH 0-100        (decode: as-is, from DHT11)
// [3] humidity_soil  : uint8  -> % RH 0-100        (decode: as-is, inverted FC-28)
// [4] uv_index       : uint8  -> UV index * 10     (decode: / 10.0)
// [5] air_speed      : uint8  -> rotations/sec     (decode: as-is)
// [6] water_level    : uint8  -> raw mapped 0-255  (decode: as-is)
#define PACKET_LEN 7

int packetSize;
uint32_t lastPacketTime = 0;        // millis() of last good packet
#define NO_PACKET_TIMEOUT 10000     // red after 10 s without a packet

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("MeteoRX started...");

    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    setLED(0, 0, 0);
    setLED(1, 0, 0);
    delay(300);
    setLED(0, 0, 0);
    setLED(0, 1, 0);
    delay(300);
    setLED(0, 0, 0);
    setLED(0, 0, 1);
    delay(300);
    setLED(0, 0, 0);
    LoRa.setPins(ss, rst, dio0);
    while (!LoRa.begin(433E6)) {
        Serial.println(".");
        setLED(1, 0, 0); // red: LoRa not ready
        delay(500);
    }
    setLED(0, 0, 0);

    LoRa.setSyncWord(0xA5);
    LoRa.setTxPower(20);
    LoRa.setSpreadingFactor(12);
    LoRa.setCodingRate4(5);
    Serial.println("LoRa Initializing OK!");
}

void loop() {
    packetSize = LoRa.parsePacket();
    if (packetSize) {
        uint8_t buf[64];
        int len = 0;
        while (LoRa.available() && len < (int)sizeof(buf))
            buf[len++] = (uint8_t)LoRa.read();

        if (len < PACKET_LEN) {
            Serial.printf("\n[WARN] Short packet: got %d byte(s), expected %d. Ignoring.\n", len, PACKET_LEN);
            setLED(1, 0, 0); // red: bad packet
            return;
        }

        lastPacketTime = millis();
        setLED(0, 1, 0); // green: good packet received

        // Decode fields
        int   temperature     = (int)buf[0];                  // °C
        int   pressure        = (int)buf[1] + 900;            // hPa
        int   humidity_air    = (int)buf[2];                  // % RH (DHT11, direct)
        int   humidity_soil   = (int)buf[3];                  // % RH (FC-28, inverted)
        float uv_index        = buf[4] / 10.0f;              // UV index (0.0 - 15.0)
        int   air_speed       = (int)buf[5];                  // rotations/sec
        int   water_level     = (int)buf[6];                  // raw 0-255

        Serial.println("\n---- LoRa Packet Received ----");
        Serial.printf("  RSSI         : %d dBm\n",   LoRa.packetRssi());
        Serial.printf("  SNR          : %.1f dB\n",  LoRa.packetSnr());
        Serial.printf("  Bytes recv   : %d\n",       len);
        Serial.printf("  Temperature  : %d C\n",     temperature);
        Serial.printf("  Pressure     : %d hPa\n",   pressure);
        Serial.printf("  Air humidity : %d %%RH\n",  humidity_air);
        Serial.printf("  Soil humidity: %d %%RH\n",  humidity_soil);
        Serial.printf("  UV index     : %.1f\n",     uv_index);
        Serial.printf("  Wind speed   : %d rps\n",   air_speed);
        Serial.printf("  Water level  : %d\n",       water_level);
        Serial.println("------------------------------");
    }

    // Serial commands: "SF 10", "CR 5", "TxPOW 20"
    if (Serial.available() > 0) {
        //setLED(0, 0, 1); // blue: processing serial command
        String input = Serial.readStringUntil('\n');
        input.trim();
        int spaceIndex = input.indexOf(' ');
        if (spaceIndex > 0) {
            String cmd = input.substring(0, spaceIndex);
            int    val = input.substring(spaceIndex + 1).toInt();
            if (cmd == "SF")    { LoRa.setSpreadingFactor(val); Serial.printf("SF set to %d\n", val); }
            if (cmd == "CR")    { LoRa.setCodingRate4(val);     Serial.printf("CR set to %d\n", val); }
            if (cmd == "TxPOW") { LoRa.setTxPower(val);        Serial.printf("TxPOW set to %d\n", val); }
        }
        setLED(0, 0, 0);
    }

    // Red if no packet received within timeout window
    if (lastPacketTime > 0 && (millis() - lastPacketTime > NO_PACKET_TIMEOUT))
        setLED(1, 0, 0);
}
