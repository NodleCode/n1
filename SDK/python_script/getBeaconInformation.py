# This script downloads beacon records using the nodle APIs.
# A bearer token needs to be generated first using iot.nodle.com. A login attempt will return a magic link with the token. That token needs to be used
# in the authorization for the Nodle APIs to create an apiKey and to get the Organization Id. 
# The above mentioned token may need to be recreated since it expires after some time.
# This script gets the beacon records and displays the following beacon details
# Accuracy
# Altitude
# Scan Time
# Receive Time
# Latitude
# Longitude
# iBeacon UUID
# iBeacon Major No
# iBeacon Minor No
# iBeacon Signal Strength

import requests
import json
import time

# The following token after Bearer needs to be generated using iot.nodle.com and copied below
auth = "Bearer eyJhbGciOiJSUzI1NiIsImtpZCI6ImRjOGViMDM1Y2ZjNzQzMmJlMmZiOGJlN2U3YjAxNDhiIiwidHlwIjoiSldUIn0.eyJpc3MiOiJsaW5rLmdvLm5vZGxlLmlvIiwic3ViIjoic3VicmFtYW55YW4ua3Jpc2huYW5AY2Vsc3RyZWFtLmNvbSIsImV4cCI6MTY3OTQ5MDAzOCwiaWF0IjoxNjc4ODg1MjM4fQ.WM8hVkdaZ6IW0ne0O1cX-UdrjIvct1MuiF-KPSjwihsCn80NrAAgPBOAFk7_H4FHG3NHibidCoqToDuCx3kXtleNnD2uJE74ZF8JLWyQWQqO43-z-6napXKMSaltPMy_u2mpUhNd-Shd-0GUoSd8l2PqkdREXH6r1ho2ewS5l-SZs0hjXJWCKC2Cge2vpYbfU6QQymk4kC1U9HMa2OsTQXbLboFHf1IPffEIc5ko57Q9ZxjEq-Re41SiRzZ4d9SUoqxsKvYId7Ph_IUghZPM7TmhoEsZkXV7EmQIPUGp4NH9AKhHbSYMEIA-8_4_UzVbpgpuMPNG8UqqNJodF-UKRw"

url = "https://api.sdk.nodle.io/api/v1/currentuser/apikeys"
payload = {"displayName": "string"}
# Adding empty header as parameters are being sent in payload
headers = {
    "accept": "application/json",
    "Authorization": auth,
    "Content-Type": "application/json"}
r = requests.post(url, data=json.dumps(payload), headers=headers)
yjson = json.loads(r.content)
apiKey=yjson['apikey']

url = "https://api.sdk.nodle.io/api/v1/currentuser/organizations"
# Adding empty header as parameters are being sent in payload
headers = {
    "accept": "application/json",
    "Authorization": auth,
    "Content-Type": "application/json"}
r = requests.get(url, headers=headers)

#Get organization id
yjson = json.loads(r.content)
orgId=yjson[0]['id']

count=0
while (True):
    url = "https://iot.nodle.com/api/v1/organizations/" + str(orgId) + "/pings?page_size=100"

    # Adding empty header as parameters are being sent in payload
    headers = {
        "accept": "application/json",
        "x-api-key": apiKey}
    r = requests.get(url, headers=headers)
    djson = json.loads(r.content)
    for element in djson['data']:
        accuracy = element['accuracy']
        altitude = element['altitude']
        scantime = element['scantime']
        receivetime = element['receivetime']
        latitude = element['latitude']
        longitude = element['longitude']
        payload = element['last_payload']
        pload_json = json.loads(payload)
        for l in pload_json:
            if (l['field']=="ib_uuid"):
                uuid=l['value']
            elif (l['field']=="ib_maj"):
                majorno=l['value']
            elif (l['field']=="ib_min"):
                minorno=l['value']
            elif (l['field']=="ib_sig"):
                power=l['value']
                
        print ("Accuracy =",accuracy,",Altitude =",altitude,",Latitude =",latitude, ",Longitude =", longitude, ",Scantime =", scantime,",Receive Time =",receivetime, ",UUID =",uuid, ",Major No =",majorno, ",Minor No =",minorno, ",Power =", power)
    count += 1
    print ("-------------------------------------------------------------------------------------------------------------------------------------------------------")
    time.sleep (10)
