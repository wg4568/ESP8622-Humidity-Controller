# log.py
# Pulls data from server and saves it in logs/
# Note: logs folder must be created before running.

import requests
import time
import json

url = "http://192.168.1.69/data.json"
count = 0
delay = 60

while True:
    resp = requests.get(url)
    data = {"error": True}

    if resp.status_code == 200:
        data = resp.json()
        data["error"] = False

    with open("logs/log_%s.json" % count, "w+") as f:
        json.dump(data, f)

    print("Wrote to logs/log_%s.json" % count)
    count += 1

    time.sleep(delay)
