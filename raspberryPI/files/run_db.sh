#!/bin/bash

# Load the configurations
source config_mqtt.conf
source config_db.conf

# Check that all necessary variables are defined
if [[ -z "$MQTT_USERNAME_SUBSCRIBER" || -z "$MQTT_PASSWORD_SUBSCRIBER" || -z "$MQTT_TOPIC" || -z "$DB_USER" || -z "$DB_PASSWORD" || -z "$DB_NAME" ]]; then
  echo "Error: Please define all variables in the configuration files."
  exit 1
fi

# Install Python dependencies
sudo apt update && sudo apt install -y python3-pip
pip3 install paho-mqtt mysql-connector-python

# Create the Python script to insert data into the database
cat <<EOF > /home/pi/mqtt_to_db.py
import paho.mqtt.client as mqtt
import mysql.connector
import json

# Load the configuration
mqtt_username = "$MQTT_USERNAME_SUBSCRIBER"
mqtt_password = "$MQTT_PASSWORD_SUBSCRIBER"
mqtt_topic = "$MQTT_TOPIC"
db_host = "$DB_HOST"
db_user = "$DB_USER"
db_password = "$DB_PASSWORD"
db_name = "$DB_NAME"

# Callback function for MQTT connection
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(mqtt_topic)

# Callback function for receiving MQTT messages
def on_message(client, userdata, msg):
    print("Message received on topic " + msg.topic)
    payload = json.loads(msg.payload.decode())

    # Connect to the database
    db = mysql.connector.connect(
        host=db_host,
        user=db_user,
        password=db_password,
        database=db_name
    )
    cursor = db.cursor()

    # Insert data into the database
    sql = "INSERT INTO meteo_data (temperature, humidity, pressure, wind_direction, wind_speed, rain_fall, time) VALUES (%s, %s, %s, %s, %s, %s, %s)"
    val = (
        float(payload["temperature"]),
        float(payload["humidity"]),
        float(payload["pressure"]),
        int(payload["wind_direction"]),
        float(payload["wind_speed"]),
        float(payload["rain_fall"]),
        payload["time"]
    )
    cursor.execute(sql, val)
    db.commit()
    cursor.close()
    db.close()

# Configure the MQTT client
client = mqtt.Client()
client.username_pw_set(mqtt_username, mqtt_password)
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)
client.loop_forever()
EOF

# Create a systemd service to automatically start the Python script
sudo bash -c "cat <<EOF > /etc/systemd/system/mqtt_to_db.service
[Unit]
Description=MQTT to MySQL Service
After=multi-user.target

[Service]
Type=idle
ExecStart=/usr/bin/python3 /home/pi/mqtt_to_db.py

[Install]
WantedBy=multi-user.target
EOF"

# Enable and start the service
sudo systemctl daemon-reload
sudo systemctl enable mqtt_to_db.service
sudo systemctl start mqtt_to_db.service

echo "MQTT to MySQL service successfully started."
