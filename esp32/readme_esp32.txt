   Installation

  Install Arduino IDE and Required Libraries: Make sure to install the required libraries listed above using the Arduino Library Manager or by downloading them manually.

    Upload the Code:
        Open the weather_station.ino file in the Arduino IDE.
        Connect your ESP32 board and upload the code.

   First-Time Setup:

    Start the ESP32 in Configuration Mode:
        After uploading the code, the ESP32 will automatically start in configuration mode.

    Connect to the Configuration Network:
        Connect your device to the Wi-Fi network named Weather_station_config using the password 3615codePereNoel.

    Configure Wi-Fi and MQTT Settings:
        Follow the on-screen instructions to connect the ESP32 to your local Wi-Fi network.
        For the MQTT settings:
            MQTT Address: Enter the IP address of your Raspberry Pi.
            MQTT Username: Use the publisher username.
            MQTT Password: Use the publisher password.
            Station Name: Enter the topic name.

    Set Anemometer Radius and Rainfall Coefficient:
        Radius of the Anemometer: Input the radius in centimeters.
        Rainfall Coefficient: Calculate the rainfall coefficient by multiplying the volume (in mL) of one bucket from the tipping bucket gauge by 10, then divide by the rain collection area (in cm²).

Configuration

    Wi-Fi Configuration: The ESP32 will automatically enter configuration mode if it fails to connect to a previously known network. Use the web interface to input Wi-Fi credentials. In this case the previous MQTT settings, radius of the anemometer, and rainfall coefficient will be displayed by default.
   
   If configuring the ESP32 following a reset, the process is the same as the first-time setup.

   
Usage
Data Collection and Transmission

    The ESP32 collects data from connected sensors at regular intervals (600 seconds by default) and attempts to send it to the configured MQTT server.
    If the device loses connection to the MQTT server, data is stored locally and will be sent once the connection is reestablished.

Resetting Configuration

    Press and hold the reset button for 5 seconds to reset the configuration and restart the device. This will erase all settings and stored data, forcing the device to enter configuration mode upon restart.

Data Storage

    Configuration data and unsent sensor data are stored in the SPIFFS file system on the ESP32.
    The configuration is saved in a JSON file (/test_config.json), and unsent sensor data is stored in /data.json.

Maintenance

    Resetting the Device: Use the reset button to clear all settings and data.
    Updating Firmware: Connect the ESP32 to your computer and upload the new firmware using the Arduino IDE.

Troubleshooting

    Device Not Connecting to Wi-Fi:
        Ensure the correct Wi-Fi credentials are entered during configuration.
        If the problem persists, reset the device and reconfigure the Wi-Fi settings.
    No Data on MQTT Server:
        Check if the ESP32 is connected to the network and MQTT server.
        Ensure the MQTT credentials and server address are correct.
        Verify the MQTT broker is running and accessible.


