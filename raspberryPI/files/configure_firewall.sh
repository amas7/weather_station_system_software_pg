#!/bin/bash

# Check the status of UFW
echo "Checking the status of UFW..."
sudo ufw status

# Check if UFW is active
ufw_active=$(sudo ufw status | grep -i "Status: active")

if [[ -z "$ufw_active" ]]; then
    echo "UFW is inactive. Activating UFW and allowing SSH traffic..."
    # Allow SSH to avoid losing access
    sudo ufw allow ssh
    
    # Enable UFW
    sudo ufw enable
else
    echo "UFW is already active."
fi

# Allow traffic on port 5000
echo "Allowing traffic on port 5000..."
sudo ufw allow 5000/tcp

# Reload UFW to apply the new rules
echo "Reloading UFW rules..."
sudo ufw reload

# Check the applied UFW rules
echo "Checking UFW rules..."
sudo ufw status

echo "Firewall configuration completed. Port 5000 is now open."
