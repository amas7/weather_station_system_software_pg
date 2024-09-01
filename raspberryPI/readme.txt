1. Connect the Raspberry Pi to Your Local Network

    Action: Connect the Raspberry Pi to your local network.

2. Find the IP Address

    Action: Find the IP address of the Raspberry Pi by running the following command: "hostname -I", note that ip adress for esp32 configuration

3. Download and Place the Weather Station Files

    Action: Download the directory "files" and place it in a directory on your Raspberry Pi with appropriate permissions.

4. Edit the Configuration Files

   Action: Edit and save the configuration files config_mqtt.conf and config_db.conf with your desired username and password. Note the publisher password, publisher username, and topic for the ESP32 configuration.

5. Make the Script Executable

    Action: Run the following command to make the script executable: chmod +x all.sh.

6. Run the Script

    Action: Execute the script by running: ./all.sh.

7. Automatic Service Restart

    Note: If the Raspberry Pi shuts down and restarts, the functionalities (MQTT broker, database integration, and visualization) are services that will restart automatically.

To access the visualization of weather data on devices connected to the same local network, go to http://<Raspberry_Pi_IP>:5000.
