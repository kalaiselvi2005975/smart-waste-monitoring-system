♻️ Smart IoT Waste Classification & Monitoring System

An intelligent waste management system integrating IoT sensors, automated segregation, cloud monitoring, and machine learning analytics to improve waste management efficiency.

📌 Overview

The Smart IoT Waste Classification and Monitoring System is designed to automate waste segregation and monitor waste generation patterns in real time.

The system uses sensors connected to a microcontroller to:

Detect waste

Classify it as Wet Waste or Dry Waste

Automatically route waste into the correct bin using a servo motor

Sensor data is uploaded to the cloud using ThingSpeak, enabling real-time monitoring and dashboards.

Collected data is also used to train a machine learning model using Scikit-learn to analyze waste patterns and determine peak waste generation time.

🚀 Features

Automatic waste detection using IR Sensor

Waste classification using Moisture Sensor

Servo motor based automatic segregation

Real-time IoT monitoring

Cloud data storage

Waste statistics dashboard

Machine learning prediction model

Peak waste generation time analysis

🏗 System Architecture
Workflow

1️⃣ User places waste in the smart bin
2️⃣ IR sensor detects waste
3️⃣ Moisture sensor reads moisture level
4️⃣ Waste is classified as Wet / Dry
5️⃣ Servo motor directs waste to correct bin
6️⃣ Sensor data is uploaded to cloud
7️⃣ Dashboard displays waste statistics
8️⃣ Dataset is used for machine learning
9️⃣ Model identifies peak waste disposal time

🔌 Hardware Components
Component	Purpose
NodeMCU ESP8266	Microcontroller with WiFi
IR Sensor	Detects waste
Moisture Sensor	Measures moisture level
Servo Motor	Directs waste into bins

💻 Software Stack
Technology	Purpose
Arduino IDE	Programming NodeMCU
ThingSpeak	IoT cloud monitoring
Figma	Dashboard UI design
Scikit-learn	Machine learning model
🧠 Waste Classification Logic

Waste type is determined using moisture sensor values.

Example Threshold
Threshold Value = 550
Moisture Value	Waste Type
< 550	Wet Waste
≥ 550	Dry Waste
📊 Dataset & Machine Learning Model

Sensor data collected from the IoT system is stored in a dataset and used to train a Random Forest Machine Learning Model.

Dataset Fields
Column	Description
Time	Waste detection time
Hour	Extracted hour
Moisture	Moisture sensor value
Waste_Type	Wet / Dry classification
Example Dataset
Time	Moisture	Waste_Type
08:00	320	     Wet
09:00	680	    Dry
12:00	450   	Wet
18:00	720   	Dry
📈 Peak Waste Time Analysis

The trained machine learning model analyzes historical data to determine peak waste disposal hours.

Example
Hour	Waste Count
08:00	 `5
12:00	14
18:00	21
21:00	9
Result
Peak Waste Time: 6 PM – 7 PM

This helps optimize garbage collection schedules.

📊 Dashboard & Data Visualization

The dashboard shows:

Moisture sensor values

Waste classification

Wet waste count

Dry waste count

Total waste collected

Waste distribution charts

Waste trend graphs

Peak waste insights

⚙ Installation & Setup
Hardware Setup

Connect IR Sensor to NodeMCU digital pin

Connect Moisture Sensor to analog pin

Connect Servo Motor to PWM pin

Software Setup

Install Arduino IDE

Install ESP8266 board package

Upload Arduino code to NodeMCU

Configure WiFi credentials

Connect device to ThingSpeak

📁 Project Structure
smart-waste-monitoring-system
│
├── arduino_code
│   └── smart_waste_system.ino
│
├── dataset
│   └── waste_data.csv
│
├── machine_learning
│   └── waste_model.py
│
├── dashboard
│   └── dashboard_ui.fig
│
├── images
│   ├── system_architecture.png
│   ├── hardware_setup.jpg
│   └── dashboard_preview.png
│
└── README.md
🌍 Applications

Smart city waste management

Public garbage bins

College campuses

Hospitals

Restaurants

Residential waste systems

🔮 Future Improvements

AI image-based waste detection

Mobile application integration

Multiple waste category sorting

Smart bin fill-level monitoring

Predictive waste analytics
