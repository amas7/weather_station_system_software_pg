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

information about the files:
1. config_mqtt.conf

This configuration file stores the credentials and settings required for the MQTT (Message Queuing Telemetry Transport) service. It contains the following variables:

    MQTT_USERNAME_PUBLISHER: Username for the MQTT publisher, which is likely the ESP32 device that sends weather data.
    MQTT_PASSWORD_PUBLISHER: Password for the MQTT publisher.
    MQTT_TOPIC: The topic on which the ESP32 publishes weather data.
    MQTT_USERNAME_SUBSCRIBER: Username for the MQTT subscriber, which is the Raspberry Pi in this case, subscribing to the weather data.
    MQTT_PASSWORD_SUBSCRIBER: Password for the MQTT subscriber.

2. config_db.conf

This configuration file contains the necessary database settings:

    DB_HOST: Hostname of the database, typically "localhost" for a local database.
    DB_USER: Database username with privileges to create tables and manage data.
    DB_PASSWORD: Password for the database user.
    DB_NAME: Name of the database where weather data will be stored.

3. setup_mqtt.sh

This script is responsible for setting up the MQTT broker on the Raspberry Pi:

    It sources (source config_mqtt.conf) the MQTT configuration from config_mqtt.conf.
    Verifies that all necessary MQTT variables are defined.
    Installs Mosquitto, an MQTT broker, and its client utilities.
    Creates user accounts for the publisher and subscriber within Mosquitto using the credentials specified in config_mqtt.conf.
    Configures Mosquitto to disable anonymous access and use the created user accounts.
    Restarts the Mosquitto service to apply the new configuration.

4. setup_db.sh

This script sets up the MySQL/MariaDB database:

    It sources (source config_db.conf) the database configuration from config_db.conf.
    Ensures that all necessary database variables are defined.
    Installs MariaDB, a MySQL-compatible database server.
    Runs mysql_secure_installation to secure the database server.
    Creates the weather station database and the user as specified in config_db.conf.
    Creates a table called meteo_data where weather data (temperature, humidity, etc.) will be stored.

5. run_db.sh

This script integrates the MQTT broker with the database:

    It sources both config_mqtt.conf and config_db.conf.
    Installs Python dependencies required to interface with MQTT and MySQL.
    Creates a Python script (mqtt_to_db.py) that subscribes to the MQTT topic and inserts the received weather data into the MySQL database.
    Sets up a systemd service to automatically run the Python script on startup, ensuring continuous data collection even after reboots.

6. configure_firewall.sh

This script configures the Raspberry Pi's firewall using UFW (Uncomplicated Firewall):

    It checks the status of UFW and enables it if it's not already active.
    Allows SSH traffic to ensure continued access to the Raspberry Pi.
    Opens port 5000, which is used by the Flask web application for data visualization.
    Reloads the UFW rules to apply these changes.

7. visualisation.sh

This script sets up a Flask web application for visualizing the weather data:

    It sources the database configuration from config_db.conf.
    Installs necessary dependencies for Flask, Python, and MySQL.
    Creates a Python Flask application (app.py) that connects to the MySQL database, retrieves weather data, and generates visualizations (graphs) of the data using Matplotlib and Pandas.
    Sets up the necessary HTML template (index.html) to display the data and graphs.
    Configures a systemd service to start the Flask web application automatically on boot.

8. all.sh

This is a master script that automates the execution of all the setup steps:

    It makes each of the scripts (configure_firewall.sh, setup_mqtt.sh, setup_db.sh, run_db.sh, visualisation.sh) executable.
    Sequentially runs each of these scripts, checking for errors after each execution.
    If any script fails, it stops the process and reports the error.
    Ensures that all necessary services are set up and running by the end of its execution.

9. mqtt_to_db.py

This is a Python script created by run_db.sh:

    It connects to the MQTT broker as a subscriber.
    Listens for incoming weather data on the specified MQTT topic.
    Parses the incoming JSON data and inserts it into the MySQL database.

10. app.py

This Python script is the core of the Flask web application:

    It connects to the MySQL database and retrieves both current and historical weather data.
    Generates visualizations for temperature, humidity, pressure, wind speed, wind direction, and precipitation.
    Renders these visualizations in an HTML template (index.html) for display in a web browser.

11. index.html

This is the HTML template used by the Flask web application:

    It displays the latest weather data and the generated graphs.
    Includes a form for selecting a date range to view historical data.
    Uses Bootstrap for basic styling and layout.
