
//for reading from sensors and processing sensor readings
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <BH1750.h>
#include <Wire.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
//for publishing on MQTT broker
#include <WiFi.h>
#include <PubSubClient.h>
const char* ssid = "Hollar";
const char* password = "123456789";
const char* mqttServer = "192.168.43.174";
const int mqttPort = 1883;
const char* mqttUser = "firedetection";
const char* mqttPassword = "firedetection";
const char* clientid = "esp1"; //client id changes with data points index
const char* topicName = "/data/1"; //topicName changes with data points index
WiFiClient espClient;
PubSubClient client(espClient);
BH1750 lightMeter;
Adafruit_BME680 bme; // I2C
int MQ2_input = 4; 
int MQ7_input = 36;
int Flame_input = 39;
int ledBlink = 14; //for blinking led when message is sent to central system
void setup() {
  Serial.begin(9600);
  pinMode(ledBlink, OUTPUT);
  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin();
  bme.begin();
  lightMeter.begin();
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  //set-up connection with wifi and MQTT broker
  WiFi.begin((char *)ssid, (char *)password);
  //wifi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  //MQTT connection
  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect(clientid, mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}
String space = "  "; //used to separate sensors values in a single string message to be sent by data points
// message[] = ""; //holds the final message to be sent as a string having all sensor values in order 
                   //Temperature - Humidity - Smoke(MQ2) - Gas(MQ7) - Flame - Light
                   //All values are separated with double space characters
void loop() {
  delay(950); //wait 1seconds to conform with publishing every 2 seconds. the remaining 50ms delay is in the codeTell BME680 to begin measurement
  String message = "";
  bme.beginReading();
  delay(50);
  bme.endReading();
  //Temperature and humidity sensor (BME680) //Bme sensors returns float values
  float t = bme.temperature;
  String ts =  String(t,2);
  message = message + ts + space; //add temperature information to message
  //Humidity
  float h = bme.humidity;
  String hs =  String(h,2);
  message = message + hs + space; //add humidity information to message
  //Smoke Sensor
  float MQ2_Aout = analogRead(MQ2_input);  /*Analog value read function*/
  MQ2_Aout = constrain(MQ2_Aout, 0, 4095); //constrain the output to be minimum of zero and maximum of 4095
  String ss =  String(MQ2_Aout,2);
  message = message + ss + space; //add smoke information to message
  //Gas Sensor
  float MQ7_Aout = analogRead(MQ7_input);  /*Analog value read function*/
  MQ7_Aout = constrain(MQ7_Aout, 0, 4095); //constrain the output to be minimum of zero and maximum of 4095
  String gs =  String(MQ7_Aout,2);
  message = message + gs + space; //add gas information to message
  //Flame Sensor
  float Flame_Aout = analogRead(Flame_input);  /*Analog value read function*/
  //calibration of the flame sensor (changes the logic to positive logic
  Flame_Aout = map(Flame_Aout, 4095, 0, 0, 4095);
  //constrain min sensor value to 0 and max sensor value to 4095
  Flame_Aout = constrain(Flame_Aout, 0, 4095);
  String fs =  String(Flame_Aout,2);
  message = message + fs + space; //add flame information to message
  //Light Sensor
  float lux = lightMeter.readLightLevel();
  lux = constrain(lux, 0, 4095); //constrain the output to be minimum of zero and maximum of 4095
  String ls =  String(lux,2);
  message = message + ls + space; //add light information to message
  // Now message holds all sensor values as a string separated with space to be published on MQTT broker
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //message pattern: temperature, humidity, smoke(Mq2), Gas(Mq7), flame, light
  client.publish(topicName, message.c_str());
  Serial.println("Message sent!");
  Serial.println(message);
  
  //blink a led after message is sent
  digitalWrite(ledBlink, HIGH);
  delay(1000);  // Publish every 2 seconds there is also a 1second delay at the start of the loop to make it a total of 2seconds delay
  digitalWrite(ledBlink, LOW);
}
//A function that is called to reconnect to MQTT if disconnected at any point
void reconnect() {
  while (!client.connected()) {
    Serial.println("Reconnecting to MQTT...");
    if (client.connect(clientid, mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}













APPENDIX 3
Data Cleaning Program

data_list = df.values.tolist()
data_cleaned = []
for i in data_list:
    if i[-1] == 1.0:
        if i[0] >= 35.0 or i[6] >= 35.0 or i[12] >= 35.0 or i[18] >= 35.0:
            if i[1] <= 60 or i[7] <= 60  or i[13] <= 60  or i[19] <= 60:
                data_cleaned.append(i)
    else:
        if i[0] < 35.0 and i[6] < 35.0 and i[12] < 35.0 and i[18] < 35.0:
            if i[1] > 60 and i[7] > 60  and i[13] > 60  and i[19] > 60:
                data_cleaned.append(i)
                
print(len(data_cleaned))
cleaned_data =pd.DataFrame(data_cleaned,columns= columns)
                



APPENDIX 4
Model Development Program
import pandas as pd
import numpy as np
import seaborn as sns
import matplotlib.pylab as plt
from scipy import stats
import joblib

data = pd.read_csv('shuffled.csv')
class_label = data.select_dtypes(include = ['float64','int64'])

# Define a mapping of column names to new names
column_mapping = {
    'T1': 'Temperature1', 'L1': 'Light1', 'G1': 'Gas1', 'S1': 'Smoke1', 'H1': 'Humidity1', 'F1': 'Flame1',
    'T2': 'Temperature2', 'L2': 'Light2', 'G2': 'Gas2', 'S2': 'Smoke2', 'H2': 'Humidity2', 'F2': 'Flame2',
    'T3': 'Temperature3', 'L3': 'Light3', 'G3': 'Gas3', 'S3': 'Smoke3', 'H3': 'Humidity3', 'F3': 'Flame3',
    'T4': 'Temperature4', 'L4': 'Light4', 'G4': 'Gas4', 'S4': 'Smoke4', 'H4': 'Humidity4', 'F4': 'Flame4'
}

# Rename the columns in the DataFrame
class_label = class_label.rename(columns=column_mapping)

# renaming the data to avoid confusion:
RF_df  = class_label
target = 'CLASS_LABEL'
RF_df[target].value_counts()


from sklearn.model_selection import StratifiedShuffleSplit

# Split the data with 1300 test points in the test data
#This creates a generator

feature_cols =[x for x in RF_df.columns if x != target ]


# next is used to iterate
strat_shuffle_split = StratifiedShuffleSplit(n_splits = 1, test_size=0.30, random_state= 42 )
train_idx,test_idx = next(strat_shuffle_split.split(RF_df[feature_cols],RF_df[target]))

#Get the index value from the genarator
x_train = RF_df.loc[train_idx, feature_cols]
y_train = RF_df.loc[train_idx, target]

x_test = RF_df.loc[test_idx, feature_cols]
y_test = RF_df.loc[test_idx, target]


from sklearn.ensemble import RandomForestClassifier

# Initialize the random forest estimator
# Number of trees is not setup here

RF = RandomForestClassifier(oob_score= True,
                           random_state= 42,
                           warm_start = True,
                           n_jobs= -1)

oob_list = list()

#Iterate through all the possibilities for the number of trees 

for n_trees in [1,2,3,8,10,14,18,20,25,28,30]:
    
    #Set the number of trees
    RF.set_params(n_estimators =n_trees)

    #Fit the model
    RF.fit(x_train, y_train)
    
    #get the oob error
    oob_error= 1- RF.oob_score_
    
    #Store it
    oob_list.append(pd.Series({'n_trees': n_trees,'oob':oob_error}))
    
rf_oob_df  = pd.concat(oob_list,axis= 1).T.set_index('n_trees')
rf_oob_df = pd.DataFrame(rf_oob_df)
rf_oob_df
model = RF.set_params(n_estimators = 18)
y_pred = RF.predict(x_test)
joblib.dump(model,'random_forest_model.pkl')
