```mermaid
graph TD
    subgraph Cloud_AI_Layer [CLOUD / AI LAYER]
        Hub[Cloud IoT Hub <br> AWS/Azure/GCP] --> Ingest[Data Ingestion]
        Ingest --> DB[(Time-Series Database)]
        DB --> AI[AI/LSTM Model]
    end

    subgraph Microcontroller_Gateway [MICROCONTROLLER / GATEWAY]
        ESP[ESP32 DevKit v1]
    end

    subgraph Sensor_Layer [SENSOR LAYER]
        NEO[NEO-6M <br> GPS/Location]
        MH[MH-Z19B <br> CO2/Occupancy]
        BH[BH1750 <br> Light]
        DHT[DHT22 <br> Temp/Humidity]
    end

    %% Connections
    ESP -- Wi-Fi 802.11 b/g/n <br> MQTT / HTTPS --> Hub
    
    NEO -- HW UART 1 --> ESP
    MH -- HW UART 2 --> ESP
    BH -- I2C --> ESP
    DHT -- GPIO --> ESP
