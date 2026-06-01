#include <LoRa.h>
#include <SPI.h>

// Pinii modulului LoRa
#define ss 5
#define rst 14
#define dio0 2

// Variabila globala pentru dimensiunea pachetului
int packetSize;

void setup()
{
    // Initializeaza comunicatia seriala
    Serial.begin(115200);
    while (!Serial);
    Serial.println("LoRa Receiver");

    // Configureaza pinii si initializeaza modulul LoRa
    LoRa.setPins(ss, rst, dio0);
    while (!LoRa.begin(433E6))  // 433E6 - Asia, 866E6 - Europa, 915E6 - America de Nord
    {
        Serial.println(".");
        delay(500);
    }

    // Configureaza parametrii modulului LoRa
    LoRa.setSyncWord(0xA5);
    LoRa.setTxPower(20);
    LoRa.setSpreadingFactor(12);
    LoRa.setCodingRate4(5);
    Serial.println("LoRa Initializing OK!");
}

void loop()
{
    // Verifica daca a fost receptionat un pachet LoRa
    packetSize = LoRa.parsePacket();
    if (packetSize)
    {
        // Citeste octetii primiti intr-un buffer
        uint8_t buf[64];
        int len = 0;
        while (LoRa.available() && len < (int)sizeof(buf))
            buf[len++] = (uint8_t)LoRa.read();

        // Verifica daca pachetul este suficient de lung
        if (len < 8)
        {
            Serial.printf("\n[WARN] Pachet scurt: doar %d byte(s), asteptat 8. Se ignora.\n", len);
            return;
        }

        // Decodeza campurile senzorilor din buffer
        int   temperature  = (int)buf[0];
        int   pressure     = (int)buf[1] + 900;
        int   humidity_air = map(buf[2], 0, 255, 0, 100);
        int   humidity_soil= map(buf[3], 0, 255, 0, 100);
        int   uv_raw       = buf[4];
        int   air_dir      = map(buf[5], 0, 255, 0, 360);
        int   air_speed    = buf[6];
        int   water_level  = map(buf[7], 0, 255, 0, 4095);

        // Afiseaza datele primite
        Serial.println("\n---- Pachet LoRa Receptionat ----");
        Serial.printf("  RSSI        : %d dBm\n",  LoRa.packetRssi());
        Serial.printf("  Octeti recv : %d\n",       len);
        Serial.printf("  Temperatura : %d oC\n",    temperature);
        Serial.printf("  Presiune    : %d hPa\n",   pressure);
        Serial.printf("  Umiditate Aer:%d %%RH\n",  humidity_air);
        Serial.printf("  Umiditate Sol:%d %%RH\n",  humidity_soil);
        Serial.printf("  Index UV    : %d (raw)\n", uv_raw);
        Serial.printf("  Dir aer     : %d gr\n",    air_dir);
        Serial.printf("  Viteza aer  : %d (raw)\n", air_speed);
        Serial.printf("  Nivel apa   : %d (raw)\n", water_level);
        Serial.println("-----------------------------------");
    }

    // Proceseaza comenzile trimise prin monitorul serial
    if (Serial.available() > 0)
    {
        String input = Serial.readStringUntil('\n');
        input.trim();

        // Cauta spatiul dintre comanda si parametrul numeric
        int spaceIndex = input.indexOf(' ');
        if (spaceIndex > 0)
        {
            String textPart  = input.substring(0, spaceIndex);
            int    numberPart = input.substring(spaceIndex + 1).toInt();

            // Aplica modificari pe baza comenzii primite
            if (textPart == "SF") { LoRa.setSpreadingFactor(numberPart); Serial.printf("SF setat la %d\n", numberPart); }
            if (textPart == "CR") { LoRa.setCodingRate4(numberPart);     Serial.printf("CR setat la %d\n", numberPart); }
        }
    }
}
