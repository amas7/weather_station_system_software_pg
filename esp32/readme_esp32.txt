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
Initialize Timestamp

    The system requires a timestamp for the real-time clock (RTC). During configuration, you will be prompted to enter the initial timestamp in the format YYYYMMDDHHMMSS in UTC.

RTC Module with Battery Backup

    Note that the RTC module is powered by a battery, which ensures that the time is maintained even when the main power is off.
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


Functions explanation
1. saveConfigFile()
This function saves the current configuration parameters in a JSON file on the SPIFFS file system. It creates a JSON document, inserts the configuration parameters into it, and then writes this document to a file called /test_config.json. If writing fails, it outputs an error message to the console.

2. loadConfigFile()
This function loads the configuration from a JSON file located on the SPIFFS file system. It checks if the configuration file exists, then reads and deserializes its contents into a JSON document. The configuration parameters are then extracted from the JSON document and used to initialize the corresponding variables in the program. If the file or deserialization fails, the function returns false; otherwise, it returns true.

3. saveConfigCallback()
This function is a callback used to notify the system that the configuration parameters need to be saved. When called, it sets the shouldSaveConfig flag to true, which will trigger the saving of parameters at the next opportunity.

4. configModeCallback(WiFiManager *myWiFiManager)
This function is called when the module enters Wi-Fi configuration mode. It displays the SSID of the configuration access point and the current IP address of the access point, helping the user to connect to the module for setting up Wi-Fi parameters.

5. interruptPushButton()
This function is an interrupt handler for a push button. It measures the time the button is held down. If the button is pressed for more than 5 seconds (delayPushed), it triggers a reset_bool flag to indicate that a reset is requested.

6. countStoredrecords()
This function counts the number of lines in a stored data file (/data.json). Each line represents a locally stored data record. The counting is done by reading characters one by one and counting newline characters.

7. wifimanager_setup()
This function sets up the Wi-Fi connection using the WiFiManager library. It loads configuration parameters from the file system, initializes WiFiManager, and launches the configuration portal if necessary. The configured parameters are then saved if the shouldSaveConfig flag is set.

8. setup()
The setup() function initializes the module at startup. It configures serial communication, initializes sensors (RTC, AS5600, BME280), sets up interrupts for the Hall effect sensors, initializes the reset button, and sets up Tickers for data reading and connection checking. It also configures Wi-Fi using the wifimanager_setup() function.

9. loop()
The loop() function is called continuously and manages the main operations of the program. It checks the status of the Wi-Fi and MQTT connection and handles a reset if the button is held down. It also triggers the reading, sending, and storing of data based on the flags set by the Tickers.

10. readAndSendData()
This function is called periodically by a Ticker to read sensor data and send it. It updates a readAndSendData_bool flag, which triggers processing in the main loop.

11. storeDataLocally()
This function stores the sensor data locally on the SPIFFS file system. If the number of stored records is below a defined maximum (MAX_RECORDS), it opens the data file (/data.json) in append mode and writes the JSON data to it.

12. sendStoredData()
This function sends the locally stored data via MQTT. It reads each line from the data file, publishes the content to the MQTT server, and deletes the file after sending all the data.

13. check_connexion()
This function is called periodically to check the status of the Wi-Fi and MQTT connection. If the connection is lost, it attempts to reconnect.

14. readData()
This function reads data from the sensors (temperature, humidity, pressure, wind direction, and Hall effect sensor counters) and stores these values in global variables. It also retrieves the current time from the RTC.

15. prepareData()
This function prepares the data for sending by converting the sensor readings into formatted strings and inserting them into a JSON document. It also includes a timestamp based on the current time from the RTC.

16. sendData()
This function sends the data prepared by prepareData() to the MQTT server by serializing it into JSON and publishing it under the station name.

17. reset()
This function resets the configuration parameters to their default values, clears the locally stored data, and restarts the ESP module. It is called when a long press on the push button is detected.
