#!/bin/bash

# Load the database configuration
source config_db.conf

# Check if the DB variables are defined
if [[ -z "$DB_USER" || -z "$DB_PASSWORD" || -z "$DB_NAME" ]]; then
  echo "Error: Please define all the DB variables in the configuration file."
  exit 1
fi

# Install MySQL/MariaDB
sudo apt update && sudo apt install -y mariadb-server

# Secure MySQL
sudo mysql_secure_installation <<EOF

Y
$DB_PASSWORD
$DB_PASSWORD
Y
Y
Y
Y
EOF

# Create the database and the user
sudo mysql -u root -p$DB_PASSWORD -e "CREATE DATABASE IF NOT EXISTS $DB_NAME;"
sudo mysql -u root -p$DB_PASSWORD -e "CREATE USER IF NOT EXISTS '$DB_USER'@'localhost' IDENTIFIED BY '$DB_PASSWORD';"
sudo mysql -u root -p$DB_PASSWORD -e "GRANT ALL PRIVILEGES ON $DB_NAME.* TO '$DB_USER'@'localhost';"
sudo mysql -u root -p$DB_PASSWORD -e "FLUSH PRIVILEGES;"

# Create the table for weather data
sudo mysql -u $DB_USER -p$DB_PASSWORD $DB_NAME -e "CREATE TABLE IF NOT EXISTS meteo_data (
    id INT AUTO_INCREMENT PRIMARY KEY,
    temperature FLOAT,
    humidity FLOAT,
    pressure FLOAT,
    wind_direction INT,
    wind_speed FLOAT,
    rain_fall FLOAT,
    time DATETIME
);"

echo "Database configuration completed."

