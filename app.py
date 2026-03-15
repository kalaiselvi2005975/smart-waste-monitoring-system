"""
AI Smart Waste Prediction Dashboard
Hackathon/Expo - Flask + AI + ThingSpeak
"""

from flask import Flask, render_template, request, jsonify
import pandas as pd
import urllib.request
import json
import os
from sklearn.ensemble import RandomForestClassifier

app = Flask(__name__)

THINGSPEAK_CHANNEL_ID = 3282129
THINGSPEAK_READ_KEY = ""

model = None

def load_and_train_model():
    global model
    csv_path = os.path.join(os.path.dirname(__file__), "waste_data.csv")
    if os.path.exists(csv_path):
        data = pd.read_csv(csv_path)
        data['hour'] = pd.to_datetime(data['Time']).dt.hour
        X = data[['hour', 'Moisture']]
        y = data['Waste_Type']
        model = RandomForestClassifier(n_estimators=50, random_state=42)
        model.fit(X, y)
        return True
    model = None
    return False

def predict_waste(hour, moisture):
    if model is not None:
        return model.predict([[hour, moisture]])[0]
    return "Wet" if moisture < 550 else "Dry"

def fetch_thingspeak_data():
    try:
        url = f"https://api.thingspeak.com/channels/{THINGSPEAK_CHANNEL_ID}/feeds.json?results=50"
        if THINGSPEAK_READ_KEY:
            url += f"&api_key={THINGSPEAK_READ_KEY}"
        with urllib.request.urlopen(url, timeout=5) as response:
            return json.loads(response.read().decode()).get("feeds", [])
    except:
        return []

@app.route('/')
def index():
    return render_template('index.html', prediction="")

@app.route('/predict', methods=['POST'])
def predict():
    try:
        hour = int(request.form.get('hour', 12))
        moisture = int(request.form.get('moisture', 500))
        return jsonify({'prediction': predict_waste(hour, moisture)})
    except Exception as e:
        return jsonify({'prediction': 'Error'})

@app.route('/api/dashboard-data')
def dashboard_data():
    feeds = fetch_thingspeak_data()
    wet_count = dry_count = 0
    moisture_values, labels = [], []
    for feed in feeds:
        if feed.get('field2'):
            wet_count += 1 if int(feed['field2']) == 1 else 0
            dry_count += 1 if int(feed['field2']) == 0 else 0
        if feed.get('field1'):
            moisture_values.append(int(feed['field1']))
        if feed.get('created_at'):
            labels.append(feed['created_at'][-8:-3])
    if not feeds:
        wet_count, dry_count = 15, 22
        moisture_values = [320, 450, 380, 620, 280, 550, 410, 690, 340, 600]
        labels = [f"T-{len(moisture_values)-i}" for i in range(len(moisture_values))]
    return jsonify({
        'wet_count': wet_count, 'dry_count': dry_count,
        'total': wet_count + dry_count,
        'moisture_values': moisture_values[-20:],
        'labels': labels[-20:]
    })

if __name__ == "__main__":
    load_and_train_model()
    print("AI Waste Dashboard: http://127.0.0.1:5000")
    app.run(debug=True, host='0.0.0.0', port=5000)
