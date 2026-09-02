flowchart TB

    subgraph S["SENSOR LAYER"]
        SCD["SCD41<br/>CO₂ / Temperature / Humidity"]
        DHT["DHT11<br/>Temperature / Humidity"]
        LDR["LDR<br/>Relative Light Level"]
    end

    subgraph E["EDGE / GATEWAY LAYER"]
        ESP["DFRobot FireBeetle ESP32<br/>Sensor Acquisition & JSON Formatting"]
    end

    subgraph C["CLOUD / DIGITAL REPRESENTATION LAYER"]
        MQTT["EMQX Cloud<br/>MQTT Broker"]
        PY["Python MQTT-to-InfluxDB Bridge<br/>JSON Decoding & Data Transfer"]
        DB[("InfluxDB Cloud<br/>Time-Series Storage")]
        DASH["Live Dashboard<br/>Prototype-Level Digital Representation"]

        MQTT --> PY
        PY --> DB
        DB --> DASH
    end

    SCD -->|"I²C"| ESP
    DHT -->|"Digital GPIO"| ESP
    LDR -->|"Analogue ADC"| ESP

    ESP -->|"Wi-Fi / MQTT<br/>smartbuilding/sensors"| MQTT
