#%% dependencies
import re

import pandas as pd
import plotly.express as px
import numpy as np

filename = "220619A.TXT"
headers = ["ts", "segment", "roll", "pitch", "servoPosLeft", "servoPosRight", "servoPosTail", "esc"]

#%% preprocess
with open(filename) as file:
    data = file.read()

# replace any number of \x00 with one newline
data = re.sub("\0+", "\n", data)

lines = data.split('\n')

split_lines = []
for e in lines:
    split_line = e.split(",")
    # correct number of columns and every column has data
    if len(split_line) == len(headers) and all([len(e) > 0 for e in split_line]):
        split_lines.append(split_line)

#%% break into distinct flights
starts = [0]
flights = []
for i, e in enumerate(split_lines):
    if i < len(split_lines)-1 and int(split_lines[i][0]) > int(split_lines[i+1][0]):
        flights.append(split_lines[starts[-1]:i])
        starts.append(i+1)

#%% remove boring data

# TODO detect end of flight and remove long tails
# naive approach doesn't work with small changes in sensor data

# for flight in flights:
#     last_change = len(flight) - 1
#     sensor_data = flight[-1][1:]
#     for i in range(last_change - 1, 0, -1):
#         if flight[i][1:] != sensor_data:
#             last_change = i
#             break
#     flight = flight[:last_change]

#%% build dataframe for each flight
def flight_to_df(flight):
    df = pd.DataFrame(flight, columns=headers)

    df['ts'] = df['ts'].astype('int')
    for col in df.columns[2:]:
        df[col] = df[col].astype('float')

    def rescale(col_name, from_range=None, to_range=(-1, 1)):
        if from_range is None:
            from_range = [min(df[col_name]), max(df[col_name])]
        df[col_name + "_scaled"] = np.interp(df[col_name], from_range, to_range)

    rescale("ts")
    df["ts"] = (df["ts"] - df["ts"].min()) / 10**6
    rescale("roll", [-45, 45])
    rescale("pitch", [-45, 45])
    rescale("servoPosTail", [-1, 1])
    rescale("servoPosLeft", [-1, 1])
    rescale("servoPosRight", [-1, 1])
    rescale("esc", [-1, 1], [0, 1])

    return df


flight_dfs = [flight_to_df(flight) for flight in flights]


def plot_flight(flight_df):
    sens_cols = ["roll_scaled", "pitch_scaled"]
    act_cols = ["servoPosTail_scaled", "servoPosLeft_scaled", "servoPosRight_scaled"]
    fig = px.line(flight_df, x="ts", y=[*sens_cols, *act_cols])
    fig.show()


#%% view all flights
for flight_num, flight_df in enumerate(flight_dfs):
    print(f"Flight number: {flight_num}")
    plot_flight(flight_df)

#%% analyze specific flight
flight_num = 4
plot_flight(flight_dfs[flight_num])

#%%
