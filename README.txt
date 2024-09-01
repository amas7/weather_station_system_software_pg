Overview

This project is a connected weather station that measures and records various meteorological data every 10 minutes, including temperature, humidity, pressure, wind direction, average wind speed, and precipitation. The collected data is visualized online within your local network, allowing for real-time monitoring and analysis.
Features

    Real-Time Data Visualization: View the latest weather data recorded by the station directly through a web interface on your local network.
    Historical Data Analysis: Select specific date ranges to display graphs showing the evolution of weather parameters over time.
    Precipitation Data Grouping:
        If the selected date range is less than a day, precipitation data is grouped by hour as depicted in Weather Data_hours.html
        If the selected date range is more than a day, precipitation data is grouped by day as depicted in Weather Data_days.html
    Date and Time in UTC: All recorded data timestamps, as well as the timestamps used in the visualization, are in Coordinated Universal Time (UTC).

Setup Instructions

    Read the Raspberry Pi README:
        Begin by reading the Raspberry Pi-specific README. This will guide you through setting up the Raspberry Pi, including connecting to your network, configuring the MQTT broker, setting up the database, and launching the visualization service.

    Read the ESP32 README:
        After setting up the Raspberry Pi, proceed with the ESP32 README. This document will walk you through configuring the ESP32 to connect to your network and communicate with the MQTT broker.

    Installation Process:
        Raspberry Pi Setup: Follow the instructions provided in the Raspberry Pi README to install and configure all necessary services on your Raspberry Pi.
        ESP32 Setup: Once the Raspberry Pi is ready, follow the ESP32 README to set up the ESP32 module, ensuring it can connect to the network and start sending data to the Raspberry Pi.

    Visualize Your Data:
        Access the weather station's web interface from any device connected to the same local network as the Raspberry Pi. Here, you can view the latest recorded data and explore historical trends through customizable graphs.

License

This project is licensed under a MIT free license, allowing you to freely use, modify, and distribute the software.
