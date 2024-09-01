#!/bin/bash

# Charger la configuration MQTT
source config_mqtt.conf

# Vérifier que les variables MQTT sont définies
if [[ -z "$MQTT_USERNAME_PUBLISHER" || -z "$MQTT_PASSWORD_PUBLISHER" || -z "$MQTT_USERNAME_SUBSCRIBER" || -z "$MQTT_PASSWORD_SUBSCRIBER" || -z "$MQTT_TOPIC" ]]; then
  echo "Erreur : Veuillez définir toutes les variables MQTT dans le fichier de configuration."
  exit 1
fi

# Mise à jour du système
sudo apt update && sudo apt upgrade -y

# Installation de Mosquitto et des clients Mosquitto
sudo apt install -y mosquitto mosquitto-clients

# Création des utilisateurs MQTT avec mot de passe
sudo mosquitto_passwd -c /etc/mosquitto/passwd $MQTT_USERNAME_PUBLISHER
sudo mosquitto_passwd -b /etc/mosquitto/passwd $MQTT_USERNAME_PUBLISHER $MQTT_PASSWORD_PUBLISHER
sudo mosquitto_passwd -b /etc/mosquitto/passwd $MQTT_USERNAME_SUBSCRIBER $MQTT_PASSWORD_SUBSCRIBER

# Ajout de la configuration à Mosquitto
sudo bash -c "cat <<EOF > /etc/mosquitto/mosquitto.conf
allow_anonymous false
password_file /etc/mosquitto/passwd
listener 1883
EOF"

# Redémarrage du service Mosquitto
sudo systemctl restart mosquitto

echo "Configuration MQTT terminée."
