"""
AI Waste Model - Single File
1. Put your dataset CSV in same folder
2. Run: python ai_waste_model.py
   OR: python ai_waste_model.py your_data.csv
3. Model trains and saves, shows accuracy + predictions
"""

import pandas as pd
import sys
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score, classification_report
import os
import pickle

# ========== CONFIG ==========
DATASET_PATH = sys.argv[1] if len(sys.argv) > 1 else "waste_data.csv"
MODEL_SAVE_PATH = "waste_model.pkl"

# ========== LOAD DATASET ==========
def load_dataset(path):
    """Load CSV - needs: Time (or Hour), Moisture, Waste_Type (Wet/Dry)"""
    df = pd.read_csv(path)
    # Handle different column names
    if 'Time' in df.columns:
        df['hour'] = pd.to_datetime(df['Time']).dt.hour
    elif 'Hour' not in df.columns:
        df['hour'] = 12  # default
    if 'Hour' in df.columns and 'hour' not in df.columns:
        df['hour'] = df['Hour']
    if 'Moisture' not in df.columns and 'moisture' in df.columns:
        df['Moisture'] = df['moisture']
    return df

# ========== TRAIN MODEL ==========
def train_model(df):
    if 'Waste_Type' not in df.columns and 'waste_type' in df.columns:
        df['Waste_Type'] = df['waste_type']
    X = df[['hour', 'Moisture']]
    y = df['Waste_Type']
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    model = RandomForestClassifier(n_estimators=100, random_state=42)
    model.fit(X_train, y_train)
    y_pred = model.predict(X_test)
    acc = accuracy_score(y_test, y_pred)
    return model, acc, classification_report(y_test, y_pred)

# ========== PREDICT ==========
def predict(model, hour, moisture):
    return model.predict([[hour, moisture]])[0]

# ========== MAIN ==========
if __name__ == "__main__":
    print("=" * 50)
    print("   AI WASTE MODEL - Load, Train, Predict")
    print("=" * 50)
    
    # 1. Load dataset
    if not os.path.exists(DATASET_PATH):
        print(f"\nERROR: Dataset not found: {DATASET_PATH}")
        print("Create CSV with columns: Time, Moisture, Waste_Type")
        print("Example:\nTime,Moisture,Waste_Type\n2024-01-01 08:00:00,320,Wet\n2024-01-01 09:00:00,680,Dry")
        exit(1)
    
    print(f"\n1. Loading dataset: {DATASET_PATH}")
    df = load_dataset(DATASET_PATH)
    print(f"   Rows: {len(df)}")
    print(f"   Columns: {list(df.columns)}")
    
    # 2. Train
    print("\n2. Training model...")
    model, acc, report = train_model(df)
    print(f"   Accuracy: {acc*100:.2f}%")
    print("\n   Classification Report:")
    print(report)
    
    # 3. Save model
    with open(MODEL_SAVE_PATH, 'wb') as f:
        pickle.dump(model, f)
    print(f"\n3. Model saved: {MODEL_SAVE_PATH}")
    
    # 4. Sample predictions
    print("\n4. Sample Predictions:")
    print("-" * 30)
    for hour, moisture in [(8, 320), (12, 650), (18, 400)]:
        pred = predict(model, hour, moisture)
        print(f"   Hour={hour}, Moisture={moisture} -> {pred}")
    
    # 5. Interactive prediction
    print("\n5. Enter values for prediction (or press Enter to skip):")
    try:
        h = input("   Hour (0-23): ").strip()
        m = input("   Moisture (0-1023): ").strip()
        if h and m:
            p = predict(model, int(h), int(m))
            print(f"\n   >>> Prediction: {p}")
    except:
        pass
    
    print("\n" + "=" * 50)
    print("Done! Use waste_model.pkl in your app.")
    print("=" * 50)
