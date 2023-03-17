# The getBeaconInformation.py script downloads beacon records from the Nodle Cloud.
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