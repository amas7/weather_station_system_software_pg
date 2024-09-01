#!/bin/bash

# Load the database configuration
source config_db.conf

# Check if the DB variables are defined
if [[ -z "$DB_USER" || -z "$DB_PASSWORD" || -z "$DB_NAME" ]]; then
  echo "Error: Please define all DB variables in the configuration file."
  exit 1
fi

# Update the system
sudo apt update && sudo apt upgrade -y

# Install the necessary dependencies for Flask and the Python application
sudo apt install -y python3-pip python3-flask python3-matplotlib python3-pandas

# Install the MySQL connector for Python
pip3 install mysql-connector-python

# Create the app.py file for Flask
cat <<EOF > /home/pi/app.py
from flask import Flask, render_template, request
import mysql.connector
import pandas as pd
import matplotlib.pyplot as plt
import io
import base64
from datetime import datetime, timedelta

app = Flask(__name__)

# Load the configuration from the config_db.conf file
db_host = "$DB_HOST"
db_user = "$DB_USER"
db_password = "$DB_PASSWORD"
db_name = "$DB_NAME"

def get_db_connection():
    return mysql.connector.connect(
        host=db_host,
        user=db_user,
        password=db_password,
        database=db_name
    )

def get_current_data():
    conn = get_db_connection()
    cursor = conn.cursor(dictionary=True)
    cursor.execute("""
        WITH last_measure_time AS (
            SELECT MAX(time) AS last_time
            FROM meteo_data
        )
        SELECT 
            temperature, 
            humidity, 
            pressure, 
            wind_direction, 
            wind_speed, 
            (SELECT SUM(rain_fall) 
             FROM meteo_data 
             WHERE DATE(time) = DATE((SELECT last_time FROM last_measure_time))) AS rain_fall, 
            MAX(time) AS last_time
        FROM 
            meteo_data
        WHERE 
            time = (SELECT last_time FROM last_measure_time)
        GROUP BY 
            temperature, 
            humidity, 
            pressure, 
            wind_direction, 
            wind_speed
        LIMIT 1;
    """)
    data = cursor.fetchone()
    cursor.close()
    conn.close()
    if data and 'rain_fall' in data:
        data['rain_fall'] = round(data['rain_fall'], 2)
    return data

def get_historical_data(start_date, end_date):
    conn = get_db_connection()
    query = """
        SELECT * FROM meteo_data
        WHERE time BETWEEN %s AND %s
        ORDER BY time ASC;
    """
    df = pd.read_sql(query, conn, params=[start_date, end_date])
    conn.close()
    
    return df

def plot_to_img(fig):
    img = io.BytesIO()
    fig.savefig(img, format='png', bbox_inches='tight')
    img.seek(0)
    plot_url = base64.b64encode(img.getvalue()).decode('utf8')
    return plot_url

@app.route('/', methods=['GET', 'POST'])
def index():
    current_data = get_current_data()
    
    if request.method == 'POST':
        start_date = request.form['start_date']
        end_date = request.form['end_date']
    else:
        end_date = datetime.now().strftime('%Y-%m-%dT%H:%M:%S')
        start_date = (datetime.now() - timedelta(days=7)).strftime('%Y-%m-%dT%H:%M:%S')
    
    historical_data = get_historical_data(start_date, end_date)
    
    if historical_data.empty:
        temp_plot_url = humid_plot_url = pressure_plot_url = wind_speed_plot_url = wind_dir_plot_url = rain_plot_url = None
    else:
        # Temperature graph
        fig, ax = plt.subplots(figsize=(10, 6))  # Increase figure size
        ax.plot(historical_data['time'], historical_data['temperature'])
        ax.set_title('Temperature')
        temp_plot_url = plot_to_img(fig)
        plt.close(fig)

        # Humidity graph
        fig, ax = plt.subplots(figsize=(10, 6))  # Increase figure size
        ax.plot(historical_data['time'], historical_data['humidity'])
        ax.set_title('Humidity')
        humid_plot_url = plot_to_img(fig)
        plt.close(fig)

        # Pressure graph
        fig, ax = plt.subplots(figsize=(10, 6))  # Increase figure size
        ax.plot(historical_data['time'], historical_data['pressure'])
        ax.set_title('Pressure')
        pressure_plot_url = plot_to_img(fig)
        plt.close(fig)

        # Wind speed graph
        fig, ax = plt.subplots(figsize=(10, 6))  # Increase figure size
        ax.plot(historical_data['time'], historical_data['wind_speed'])
        ax.set_title('Wind Speed')
        wind_speed_plot_url = plot_to_img(fig)
        plt.close(fig)

        # Wind direction graph
        fig, ax = plt.subplots(figsize=(10, 6))  # Increase figure size
        ax.plot(historical_data['time'], historical_data['wind_direction'])
        ax.set_title('Wind Direction')
        wind_dir_plot_url = plot_to_img(fig)
        plt.close(fig)

        # Precipitation histogram
        if start_date.split('T')[0] == end_date.split('T')[0]:
            # By hour if the range is a single day
            historical_data['hour'] = historical_data['time'].dt.hour
            rain_data = historical_data.groupby('hour').sum()['rain_fall']
            xlabel = 'Hours'
        else:
            # By day if the range is more than one day
            historical_data['date'] = historical_data['time'].dt.date
            rain_data = historical_data.groupby('date').sum()['rain_fall']
            xlabel = 'Days'
        
        fig, ax = plt.subplots(figsize=(10, 6))  # Increase figure size
        rain_data.plot(kind='bar', ax=ax)
        ax.set_title('Precipitation Total')
        ax.set_xlabel(xlabel)
        ax.set_ylabel('Precipitation (mm)')
        rain_plot_url = plot_to_img(fig)
        plt.close(fig)

    return render_template('index.html', current_data=current_data, 
                           temp_plot_url=temp_plot_url, humid_plot_url=humid_plot_url, 
                           pressure_plot_url=pressure_plot_url, wind_speed_plot_url=wind_speed_plot_url, 
                           wind_dir_plot_url=wind_dir_plot_url, rain_plot_url=rain_plot_url, 
                           start_date=start_date, end_date=end_date)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
EOF

# Create the index.html file for the template
mkdir -p /home/pi/templates
cat <<EOF > /home/pi/templates/index.html
<!doctype html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Weather Data</title>
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css">
</head>
<body>
<div class="container">
    <h1 class="mt-5">Latest Measurement</h1>
    <div class="row">
        <div class="col-md-2"><strong>Temperature:</strong> {{ current_data.temperature }}°C</div>
        <div class="col-md-2"><strong>Humidity:</strong> {{ current_data.humidity }}%</div>
        <div class="col-md-2"><strong>Pressure:</strong> {{ current_data.pressure }} hPa</div>
        <div class="col-md-3"><strong>Wind Speed:</strong> {{ current_data.wind_speed }} km/h</div>
        <div class="col-md-3"><strong>Wind Direction:</strong> {{ current_data.wind_direction }}°</div>
    </div>
    <div class="row mt-3">
        <div class="col-md-4"><strong>Precipitation Total Today:</strong> {{ current_data.rain_fall }} mm</div>
        <div class="col-md-4"><strong>Date and Time of Measurement:</strong> {{ current_data.last_time }}</div>
    </div>
    
    <h2 class="mt-5">Data History</h2>
    <form method="post">
        <div class="form-group">
            <label for="start_date">Start Date:</label>
            <input type="datetime-local" class="form-control" id="start_date" name="start_date" value="{{ start_date }}">
        </div>
        <div class="form-group">
            <label for="end_date">End Date:</label>
            <input type="datetime-local" class="form-control" id="end_date" name="end_date" value="{{ end_date }}">
        </div>
        <button type="submit" class="btn btn-primary">Update</button>
    </form>

    {% if temp_plot_url %}
    <div class="mt-5">
        <h3>Temperature</h3>
        <img src="data:image/png;base64,{{ temp_plot_url }}" class="img-fluid">
    </div>
    <div class="mt-5">
        <h3>Humidity</h3>
        <img src="data:image/png;base64,{{ humid_plot_url }}" class="img-fluid">
    </div>
    <div class="mt-5">
        <h3>Pressure</h3>
        <img src="data:image/png;base64,{{ pressure_plot_url }}" class="img-fluid">
    </div>
    <div class="mt-5">
        <h3>Wind Speed</h3>
        <img src="data:image/png;base64,{{ wind_speed_plot_url }}" class="img-fluid">
    </div>
    <div class="mt-5">
        <h3>Wind Direction</h3>
        <img src="data:image/png;base64,{{ wind_dir_plot_url }}" class="img-fluid">
    </div>
    <div class="mt-5">
        <h3>Precipitation Total</h3>
        <img src="data:image/png;base64,{{ rain_plot_url }}" class="img-fluid">
    </div>
    {% endif %}
</div>
</body>
</html>
EOF

# Create a systemd service to automatically start the Flask application
sudo bash -c "cat <<EOF > /etc/systemd/system/flask_app.service
[Unit]
Description=Flask App Service
After=network.target

[Service]
User=pi
WorkingDirectory=/home/pi
ExecStart=/usr/bin/python3 /home/pi/app.py
Restart=always

[Install]
WantedBy=multi-user.target
EOF"

# Enable and start the Flask service
sudo systemctl daemon-reload
sudo systemctl enable flask_app.service
sudo systemctl start flask_app.service

echo "Flask application successfully configured and started. You can access the data at http://<IP_Raspberry_Pi>:5000"
