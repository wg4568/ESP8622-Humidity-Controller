# graph.py
# Reads data generated by log.py and creates a graph

import os
import plotly.graph_objs as go
import pandas as pd
import json

# Used for visual aid on plot
min_humidity = 55
max_humidity = 58

data = {"log": [], "humidity": [], "temperature": [], "status": []}

files = len(os.listdir("logs/"))
for file in range(files):
    with open("logs/log_%s.json" % file, "r") as f:
        log_data = json.load(f)

    data["log"].append(file / 60)
    data["humidity"].append(log_data["humidity"])
    data["status"].append(log_data["humidifier"])
    data["temperature"].append(log_data["temperature"])

df = pd.DataFrame(data)

fig = go.Figure()

fig.add_shape(
    type="rect",
    x0=0,
    y0=max_humidity,
    x1=files / 60,
    y1=min_humidity,
    line=dict(
        width=0,
    ),
    fillcolor="rgba(135,206,250,0.5)",
)

fig.add_scattergl(x=df.log, y=df.humidity, line={"color": "black"})
fig.add_scattergl(x=df.log, y=df.humidity.where(df.status), line={"color": "red"})

# Optional temperature graph
# fig.add_scattergl(x=df.log, y=df.temperature, line={'color': 'blue'})

fig.show()
