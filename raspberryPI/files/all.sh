#!/bin/bash

# List of scripts to make executable and run
scripts=("configure_firewall.sh" "setup_mqtt.sh" "setup_db.sh" "run_db.sh" "visualisation.sh")

# Loop through each script
for script in "${scripts[@]}"; do
  # Check if the script exists
  if [[ -f "$script" ]]; then
    # Make the script executable
    chmod +x "$script"
    echo "Made $script executable."

    # Run the script
    ./"$script"
    if [[ $? -ne 0 ]]; then
      echo "Error occurred while running $script. Exiting."
      exit 1
    fi
  else
    echo "Error: $script not found. Exiting."
    exit 1
  fi
done

echo "All scripts have been executed successfully."
