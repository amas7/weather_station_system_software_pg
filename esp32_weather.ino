#include <Wire.h> // I2C
#include <SPIFFS.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <AS5600.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <RtcDS1302.h> // Rtc by Makuna
#include <Ticker.h>	   
#include <WiFiManager.h>

// wifi connection and input parameters
	// config wifi parameters
	const char config_wifi_ssid[] = "Weather_station_config ";

	const char config_wifi_password[] = "3615codePereNoel";

	// JSON configuration file
	#define JSON_CONFIG_FILE "/test_config.json"

	// Flag for saving data
	bool shouldSaveConfig = false;

	// Variables to hold data from custom textboxes
	char mqtt_adress[50] = "default";
	char mqtt_username[50] = "default";
	char mqtt_password[50] = "default";
	char station_name[50] = "default";
	char time_str[15] = "20020505200001";
	float anemometer_radius = 0; //in cm
	float rainfall_coef = 0; //in mm
	int delta_angle = 999;

	// Define WiFiManager Object
	WiFiManager wm;

	void saveConfigFile()
	// Save Config in JSON format
	{
		Serial.println(F("Saving configuration..."));

		// Create a JSON document
		StaticJsonDocument<512> json;

		json["mqtt_adress"] = mqtt_adress;
		json["mqtt_username"] = mqtt_username;
		json["mqtt_password"] = mqtt_password;
		json["station_name"] = station_name;
		json["time_str"] = time_str;
		json["delta_angle"] = delta_angle;
		json["rainfall_coef"] = rainfall_coef; 
		json["anemometer_radius"] = anemometer_radius;

		// Open config file
		File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
		if (!configFile)
		{
			// Error, file did not open
			Serial.println("failed to open config file for writing");
		}

		// Serialize JSON data to write to file
		serializeJsonPretty(json, Serial);
		if (serializeJson(json, configFile) == 0)
		{
			// Error writing file
			Serial.println(F("Failed to write to file"));
		}
		// Close file
		configFile.close();
	}

	bool loadConfigFile()
	// Load existing configuration file
	{
		// Uncomment if we need to format filesystem
		// SPIFFS.format();

		// Read configuration from FS json
		Serial.println("Mounting File System...");

		// May need to make it begin(true) first time you are using SPIFFS
		if (SPIFFS.begin(false) || SPIFFS.begin(true))
		{
			Serial.println("mounted file system");
			if (SPIFFS.exists(JSON_CONFIG_FILE))
			{
				// The file exists, reading and loading
				Serial.println("reading config file");
				File configFile = SPIFFS.open(JSON_CONFIG_FILE, "r");
				if (configFile)
				{
					Serial.println("Opened configuration file");
					StaticJsonDocument<512> json;
					DeserializationError error = deserializeJson(json, configFile);
					serializeJsonPretty(json, Serial);
					if (!error)
					{
						Serial.println("Parsing JSON");

						strcpy(mqtt_adress, json["mqtt_adress"]);
						strcpy(mqtt_username, json["mqtt_username"]);
						strcpy(mqtt_password, json["mqtt_password"]);
						strcpy(station_name, json["station_name"]);
						strcpy(time_str, json["time_str"]);
						delta_angle = json["delta_angle"].as<int>();
						rainfall_coef = json["rainfall_coef"].as<float>(); 
						anemometer_radius = json["anemometer_radius"].as<float>(); 
						
						return true;
					}
					else
					{
						// Error loading JSON data
						Serial.println("Failed to load json config");
					}
				}
			}
		}
		else
		{
			// Error mounting file system
			Serial.println("Failed to mount FS");
		}

		return false;
	}

	void saveConfigCallback()
	// Callback notifying us of the need to save configuration
	{
		Serial.println("Should save config");
		shouldSaveConfig = true;
	}

	void configModeCallback(WiFiManager *myWiFiManager)
// Called when config mode launched
{
	Serial.println("Entered Configuration Mode");

	Serial.print("Config SSID: ");
	Serial.println(myWiFiManager->getConfigPortalSSID());

	Serial.print("Config IP Address: ");
	Serial.println(WiFi.softAPIP());
}

	WiFiClient espClient;
	PubSubClient mqttClient(espClient);

// reset button
	#define PushButton_PIN 0
	volatile bool reset_bool = false;
	unsigned long timeStartPush = 0;
	const unsigned long delayPushed = 5000;

	void IRAM_ATTR interruptPushButton()
{
	if (digitalRead(PushButton_PIN) == LOW) //if button press
	{							  
		timeStartPush = millis(); // Store current time
		reset_bool = false;
	}
	else
	{ // if button unpress
		if (millis() - timeStartPush >= delayPushed)
		{
			reset_bool = true; // button hold for delayPushed, 5 s
		}
	}
}

//I2C sensors
	AMS_5600 ams5600;
	Adafruit_BME280 bme;

// DS1302 RTC
	#define CLK_PIN 27
	#define DAT_PIN 26
	#define RST_PIN 25

	ThreeWire myWire = ThreeWire(DAT_PIN, CLK_PIN, RST_PIN);
	RtcDS1302<ThreeWire> rtc(myWire);

// Hall effect sensor pins
	#define H1_PIN 12
	#define H2_PIN 14

	volatile unsigned long hall1Counter = 0;
	volatile unsigned long hall2Counter = 0;

	void IRAM_ATTR hall1ISR()
	{
		hall1Counter++;
	}

	void IRAM_ATTR hall2ISR()
	{
		hall2Counter++;
	}


// records data
	#define MAX_RECORDS 1000
	#define DATA_FILE "/data.json"
	int count_localy_stored_records = 0;
	StaticJsonDocument<300> doc;
	

// Sensor data variables
	float temperature, humidity, pressure;
	unsigned long hall1Count, hall2Count;
	int wind_direction;
	RtcDateTime currentTime;
// Tickers
	Ticker dataTicker; 
	Ticker check_connectedTicker;
	volatile bool readAndSendData_bool = false;
	volatile bool check_connexion_bool = false;



int countStoredrecords()
{
	int lineCount = 0;
	File file = SPIFFS.open(DATA_FILE, "r");
	if (!file)
	{
		Serial.println("Failed to open data file for reading");
		return 0;
	}

	// Compter les lignes en lisant les caractères un par un
	while (file.available())
	{
		if (file.read() == '\n')
		{
			lineCount++;
		}
	}

	file.close();
	return lineCount;
}

void wifimanager_setup(){
	// wifi connection and input parameters (wifimanager)
		//  Change to true when testing to force configuration every time we run
		bool forceConfig = false;

		bool spiffsSetup = loadConfigFile();
		if (!spiffsSetup)
		{
			Serial.println(F("Forcing config mode as there is no saved config"));
			forceConfig = true;
		}

		// Explicitly set WiFi mode
		WiFi.mode(WIFI_STA);

		// Reset settings (only for development)
		// wm.resetSettings();

		// Set config save notify callback
		wm.setSaveConfigCallback(saveConfigCallback);

		// Set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
		wm.setAPCallback(configModeCallback);

		// Custom elements

		// Text box (String) - 50 characters maximum
		WiFiManagerParameter custom_text_box_mqtt_adress("mqtt_adress_key", "Enter the address of mqtt server", mqtt_adress, 50);
		WiFiManagerParameter custom_text_box_mqtt_username("mqtt_username_key", "Enter the mqtt ID", mqtt_username, 50);
		WiFiManagerParameter custom_text_box_mqtt_password("mqtt_password_key", "Enter the mqtt password", mqtt_password, 50);
		WiFiManagerParameter custom_text_box_station_name("station_name_key", "Enter the station name", station_name, 50);
		WiFiManagerParameter custom_text_box_time_str("time_str_key", "Enter the GMT time format YYYYMMDDHHMMSS ", time_str, 50);
		WiFiManagerParameter custom_text_box_rainfall_coef("rainfall_coef_key", "Enter the rainfall coefficient(mm)", String(rainfall_coef).c_str(), 10);
		WiFiManagerParameter custom_text_box_anemometer_radius("anemometer_radius_key", "Enter the anemometer radius (cm)", String(anemometer_radius).c_str(), 10);

		// Add all defined parameters
		wm.addParameter(&custom_text_box_mqtt_adress);
		wm.addParameter(&custom_text_box_mqtt_username);
		wm.addParameter(&custom_text_box_mqtt_password);
		wm.addParameter(&custom_text_box_station_name);
		wm.addParameter(&custom_text_box_time_str);
		wm.addParameter(&custom_text_box_rainfall_coef); 
		wm.addParameter(&custom_text_box_anemometer_radius);

		if (forceConfig)
		// Run if we need a configuration
		{
			if (!wm.startConfigPortal(config_wifi_ssid, config_wifi_password))
			{
				Serial.println("failed to connect and hit timeout");
				delay(3000);
				// reset and try again, or maybe put it to deep sleep
				ESP.restart();
				delay(5000);
			}
		}
		else
		{
			if (!wm.autoConnect(config_wifi_ssid, config_wifi_password))
			{
				Serial.println("failed to connect and hit timeout");
				delay(3000);
				// if we still have not connected restart and try all over again
				ESP.restart();
				delay(5000);
			}
		}

		// If we get here, we are connected to the WiFi

		Serial.println("");
		Serial.println("WiFi connected");
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());

		// Lets deal with the user config values

		// Copy the entered values to their strings
		strncpy(mqtt_adress, custom_text_box_mqtt_adress.getValue(), sizeof(mqtt_adress));
		Serial.print("mqtt_adress: ");
		Serial.println(mqtt_adress);

		strncpy(mqtt_username, custom_text_box_mqtt_username.getValue(), sizeof(mqtt_username));
		Serial.print("mqtt_username: ");
		Serial.println(mqtt_username);

		strncpy(mqtt_password, custom_text_box_mqtt_password.getValue(), sizeof(mqtt_password));
		Serial.print("mqtt_password: ");
		Serial.println(mqtt_password);

		strncpy(station_name, custom_text_box_station_name.getValue(), sizeof(station_name));
		Serial.print("station_name: ");
		Serial.println(station_name);

		strncpy(time_str, custom_text_box_time_str.getValue(), sizeof(time_str));
		Serial.print("time_str: ");
		Serial.println(time_str);
		
		rainfall_coef = atof(custom_text_box_rainfall_coef.getValue());
		Serial.print("rainfall_coef: ");
		Serial.println(rainfall_coef);

		anemometer_radius = atof(custom_text_box_anemometer_radius.getValue());
		Serial.print("anemometer_radius: ");
		Serial.println(anemometer_radius);

		// Save the custom parameters to FS
		if (shouldSaveConfig)
		{
			saveConfigFile();
		}


}
void setup()
{	
	Serial.begin(115200);
	Wire.begin();
	delay(10);

	wifimanager_setup();

	// Initialize RTC
		// extract the time data from the input time_str
		int year = (time_str[0] - '0') * 1000 + (time_str[1] - '0') * 100 + (time_str[2] - '0') * 10 + (time_str[3] - '0');
		int month = (time_str[4] - '0') * 10 + (time_str[5] - '0');
		int day = (time_str[6] - '0') * 10 + (time_str[7] - '0');
		int hour = (time_str[8] - '0') * 10 + (time_str[9] - '0');
		int minute = (time_str[10] - '0') * 10 + (time_str[11] - '0');
		int second = (time_str[12] - '0') * 10 + (time_str[13] - '0');

		rtc.Begin();
		RtcDateTime input_time = RtcDateTime(year, month, day, hour, minute, second);

		if (rtc.GetIsWriteProtected())
		{
			Serial.println("RTC was write protected, enabling writing now");
			rtc.SetIsWriteProtected(false);
		}

		if (!rtc.GetIsRunning())
		{
			Serial.println("RTC was not actively running, starting now");
			rtc.SetIsRunning(true);
		}

		if (!rtc.IsDateTimeValid())
		{
			Serial.println("RTC lost confidence in the DateTime!");
			rtc.SetDateTime(input_time);
		}

		RtcDateTime now = rtc.GetDateTime();
		if (now < input_time)
		{
			Serial.println("RTC is older than compile time! Updating DateTime");
			rtc.SetDateTime(input_time);
		}
		else if (now > input_time)
		{
			Serial.println("RTC is newer than compile time. (this is expected)");
		}
		else
		{
			Serial.println("RTC is the same as compile time! (not expected but all is fine)");
		}
		rtc.SetIsWriteProtected(true);

	// Initialize AS5600
		if (!ams5600.detectMagnet())
		{
			Serial.println("No magnet detected for AS5600 sensor!");
		}
		
		if (delta_angle == 999) {
			delta_angle = map(ams5600.getRawAngle(), 0, 4095, 0, 360);
			saveConfigFile();
			
		}

	// Initialize BME280
		bool status = bme.begin(0x76);
		if (!status)
		{
			Serial.println("Could not find a valid BME280 sensor, check wiring!");
		}

	//  Initialize hall effect sensors 
		pinMode(H2_PIN, INPUT);
		

		// Attach interrupts
		attachInterrupt(digitalPinToInterrupt(H1_PIN), hall1ISR, FALLING);
		attachInterrupt(digitalPinToInterrupt(H2_PIN), hall2ISR, FALLING);

    //Initialize push button 
		pinMode(PushButton_PIN, INPUT);
		attachInterrupt(digitalPinToInterrupt(PushButton_PIN), interruptPushButton, CHANGE);

	//trickers
		
		dataTicker.attach(600, readAndSendData);

		
		check_connectedTicker.attach(32, check_connexion);

	// stored records
		count_localy_stored_records = countStoredrecords();
}

void loop()
{
	if (check_connexion_bool)
	{
		check_connexion_bool = false;
		if (WiFi.status() != WL_CONNECTED)
		{
			WiFi.begin();
		}
		if (!mqttClient.connected())
		{
			if (mqttClient.connect(station_name, mqtt_username, mqtt_password))
			{
				Serial.println("mqtt reconnected");
			}
		}
		mqttClient.loop();
	}
	if (reset_bool)
	{
		reset_bool = false;
		reset();
		
	}
	if (readAndSendData_bool)
	{
		readAndSendData_bool = false;
		prepareData();

		// Check if WiFi and MQTT are connected
		if (WiFi.status() == WL_CONNECTED && mqttClient.connected())
		{
			sendData();
			// After sending, check if there is any stored data and send it
			sendStoredData();
		}
		else
		{
			Serial.println("MQTT or wifi not connected. Storing data locally.");
			// Store data locally if the connection is lost
			storeDataLocally();
		}
	}
}

void readAndSendData()
{
	readData();
	readAndSendData_bool = true;
}
void storeDataLocally()
{
	// Open file for appending
	if (count_localy_stored_records < MAX_RECORDS)
	{
		File file = SPIFFS.open(DATA_FILE, "a");
		if (!file)
		{
			Serial.println("Failed to open data file for writing");
			return;
		}

		// Write the JSON data to the file
		if (serializeJson(doc, file) == 0)
		{
			// Error writing file
			Serial.println(F("Failed to write to DATA_FILE"));
		}
		else
		{
			file.println(); // Add a new line after each JSON object
			count_localy_stored_records++;
		}

		file.close();
	}
}
void sendStoredData()
{
	// Open the file in read mode
	File file = SPIFFS.open(DATA_FILE, "r");
	if (!file)
	{
		Serial.println("No stored data found.");
		return;
	}

	// Read each line from the file and publish it to MQTT
	while (file.available())
	{
		String data = file.readStringUntil('\n');
		if (!mqttClient.publish(station_name, data.c_str()))
		{
			Serial.println("Failed to send data via MQTT");
			return;
		}
		else
		{
			if (count_localy_stored_records > 0)
			{
				count_localy_stored_records--;
			}
			else
			{
				Serial.println("count_localy_stored_records<=0");
			}
		}
	}

	// Close the file and delete it after sending
	file.close();
	SPIFFS.remove(DATA_FILE);
}

void check_connexion()
{
	check_connexion_bool = true;
}
void readData()
{
	temperature = bme.readTemperature();
	humidity = bme.readHumidity();
	pressure = bme.readPressure() / 100.0F; // Convert to hPa

	// Convert raw angle to degrees and apply delta_angle to have wind direction
	wind_direction = (map(ams5600.getRawAngle(), 0, 4095, 0, 360)+ delta_angle) % 360;

	// Make a copy of the hall sensor counters
	hall1Count = hall1Counter;
	hall2Count = hall2Counter;

	// Reset hall sensor counters
	hall1Counter = 0;
	hall2Counter = 0;

	// Get the current time from the RTC
	currentTime = rtc.GetDateTime();
}

void prepareData()
{
    char tempBuffer[10];
    char humBuffer[10];
    char presBuffer[10];
    char windSpeedBuffer[10];
    char rainFallBuffer[10];

    // Convert each float to a string with 3 decimal places
    dtostrf(temperature, 6, 2, tempBuffer);
    dtostrf(humidity, 6, 2, humBuffer);
    dtostrf(pressure, 6, 2, presBuffer);
    dtostrf(hall2Count/600.0*anemometer_radius*71.102325*0.01, 6, 3, windSpeedBuffer); // wind_speed calculation
    dtostrf(hall1Count * rainfall_coef, 6, 3, rainFallBuffer); // rain_fall calculation

    // Store these formatted strings in the JSON document
    doc["temperature"] = tempBuffer;
    doc["humidity"] = humBuffer;
    doc["pressure"] = presBuffer;
    doc["wind_direction"] = wind_direction;
    doc["wind_speed"] = windSpeedBuffer;
    doc["rain_fall"] = rainFallBuffer;

    char timestamp[20];
    sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02d", currentTime.Year(), currentTime.Month(), currentTime.Day(), currentTime.Hour(), currentTime.Minute(), currentTime.Second());
    doc["time"] = timestamp;
}

void sendData()
{
	char jsonBuffer[512];
	if (serializeJson(doc, jsonBuffer) == 0)
	{
		// Error writing file
		Serial.println(F("Failed to write to jsonBuffer"));
	}
	else
	{
		if (!mqttClient.publish(station_name, jsonBuffer))
		{
			Serial.println("Failed to send data via MQTT");
		}
	}
}

void reset(){
	wm.resetSettings();
	rtc.SetIsWriteProtected(false);
	rtc.SetDateTime(RtcDateTime(2002, 5, 5, 20, 00, 01));
	rtc.SetIsWriteProtected(true);
	strcpy(mqtt_adress, "default");
	strcpy(mqtt_username, "default");
	strcpy(mqtt_password, "default");
	strcpy(station_name, "default");
	strcpy(time_str, "20020505200001");
	delta_angle = 999;
	rainfall_coef = 0F; 
    anemometer_radius = 0F; 
	saveConfigFile();
	SPIFFS.remove(DATA_FILE);
	delay(1000);
	ESP.restart();
	delay(5000);

}