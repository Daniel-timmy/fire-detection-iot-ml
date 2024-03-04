Data logging Program 
 import paho.mqtt.client as mqtt
import datetime
import RPi.GPIO as GPIO
from time import sleep
import csv
#setup to log data in google sheet named ourDataSheet
import gspread
from google.oauth2.service_account import Credentials
scope = ['https://www.googleapis.com/auth/spreadsheets',
        'https://www.googleapis.com/auth/drive']
 
creds = Credentials.from_service_account_file("client_secret.json", scopes=scope)
client = gspread.authorize(creds)
 
google_sh = client.open_by_url("https://docs.google.com/spreadsheets/d/1QoXvlXoNVlfUBt0hRdqD87Iw5eDeZxaMvZgQG9kT7us/edit?pli=1#gid=0")
sheet1 = google_sh.get_worksheet(0)
 
 
#Grant access to mqtt broker
mqtt_user = "firedetection"
mqtt_password = "firedetection"
 
#class_label switch and leds set up
switch_input = 4
green_led = 11 #for data logging or data receiving notification (blinks)
red_led = 10 #for fire notification (blinks)
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.setup(switch_input, GPIO.IN)
GPIO.setup(green_led, GPIO.OUT, initial = GPIOLOW)
GPIO.setup(red_led, GPIO.OUT, initial = GPIO.LOW)
 
   
#initialize data_to_log to all -1
data_to_log = [-1] * 26
 
 
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("/data/1")
    client.subscribe("/data/2")
    client.subscribe("/data/3")
    client.subscribe("/data/4")
    # Add more topics as needed
 
def on_message(client, userdata, msg):
    global data_to_log
    print(f"Received message on topic {msg.topic}: {msg.payload.decode()}")
 
    #get current date time in only date and time
    date_time = ''
    full_date = str(datetime.datime.now())
    for date in full_date:
        if date != '.':
            date_time = date_time + date
        else:
            break
       
    data_to_log[0] = date_time
    data_to_log[-1] = GPIO.input(switch_input)
 
    if msg.topic == '/data/1':
        message1 = msg.payload.decode().split()
        for i in range(1,7):
            data_to_log[i] = message1[i-1]
 
    elif msg.topic == '/data/2':
        message2 = msg.payload.decode().split()
        for i in range(7,13):
            data_to_log[i] = message2[i-7]
 
    elif msg.topic == '/data/3':
        message3 = msg.payload.decode().split()
        for i in range(13,19):
            data_to_log[i] = message3[i-13]
 
    elif msg.topic == '/data/4':
        message4 = msg.payload.decode().split()
        for i in range(19,25):
            data_to_log[i] = message4[i-19]
 

    if -1 not in data_to_log: #check if it has collect all data points messages
        #log data
        # sheet1.append_rows(values = [data_to_log])
        csv_file_path = 'data.csv'
        with open(csv_file_path,mode = 'a',newline = '') as file:
            writer = csvwriter(file)
            writer.writerow(data_to_log)
        #blink led to indicate that data has been receved and logged
        GPIO.output(green_led, GPIO.HIGH)
        sleep(1) #sleep for 1 seconds
        GPIO.output(green_led, GPIO.LOW)
       
        data_to_log = [-1] * 26 #re-initialize data_to_log
        sleep(4) #sleep for 5seconds before trying to log another data (since it already slept for 1second earlier
 
client = mqtt.Client()
client.username_pw_set(mqtt_user, mqtt_password)
client.on_connect = on_connect
client.on_message = on_message
 
broker_address = "192.168.43.174"
client.connect(broker_address, 1883, 60)
client.loop_forever()
