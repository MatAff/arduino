import os

import pandas as pd
import plotly.express as px

os.listdir()

filename = "220619A.TXT"

with open(filename) as file:
    lines = file.readlines()

lines = [e for e in lines if '\x00' not in e]

split_lines = [e.replace("\n", "").split(",") for e in lines]

cols = ["ts", "segment", "roll", "pitch", "servoPosLeft", "servoPosRight", "servoPosTail", "esc"]
flight_df = pd.DataFrame(split_lines, columns=cols)

flight_df = flight_df[flight_df['ts'].str.len() > 0]

flight_df['ts'] = flight_df['ts'].astype('int')
for col in flight_df.columns[2:]:
    flight_df[col] = flight_df[col].astype('float')

flight_df = flight_df.reset_index()

flight_df["servoPosTail_scaled"] = flight_df["servoPosTail"] * 20
flight_df["servoPosLeft_scaled"] = flight_df["servoPosLeft"] * 20
flight_df["servoPosRight_scaled"] = flight_df["servoPosRight"] * 20

flight_df["ts_scaled"] = flight_df["ts"] / 10**6



px.line(flight_df, x="index", y=["ts_scaled", "pitch", "servoPosTail_scaled"])

px.line(flight_df, x="index", y=["ts_scaled", "roll", "servoPosLeft_scaled", "servoPosRight_scaled"])


flight_df