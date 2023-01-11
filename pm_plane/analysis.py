#%% dependencies
import re

import numpy as np
import pandas as pd
import plotly.express as px
from plotly.subplots import make_subplots
import plotly.graph_objects as go

filename = "230110B.TXT"
# headers = ["ts", "segment", "roll", "pitch", "servoPosLeft", "servoPosRight", "servoPosTail", "esc"]
headers = ["ts", "segment", "roll","pitch","servoPosLeft","servoPosRight",
  "servoPosTail","esc","roll_target","pitch_target","esc_target","accX",
  "accY","accZ","rollRate"]


#%% preprocess
with open(filename) as file:
    data = file.read()

# replace any number of \x00 with one newline
data = re.sub("\0+", "\n", data)
print('data', data[0:100])

# split lines
lines = data.split('\n')
print(len(lines))

split_lines = []
for e in lines:
    split_line = e.split(",")
    # correct number of columns and every column has data
    if (len(split_line) == len(headers)): #and
    #    (all([len(e) > 0 for e in split_line]):
        split_lines.append(split_line)
    else:
        print('nope - should show once')
print(len(split_lines))

#%% break into distinct flights
starts = [0]
flights = []
for i, e in enumerate(split_lines):
    if i < len(split_lines) - 1 and int(split_lines[i][0]) > int(split_lines[i+1][0]):
        flights.append(split_lines[starts[-1]:i])
        starts.append(i+1)
flights.append(split_lines[starts[-1]:i])
print(len(flights))

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
    # flight = flights[0] # Debug only REMOVE!!
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
    rescale("roll", [-45, 45], [-45, 45])
    rescale("pitch", [-45, 45], [-45, 45])
    rescale("servoPosTail", [-1, 1], [-45, 45])
    rescale("servoPosLeft", [-1, 1], [-45, 45])
    rescale("servoPosRight", [-1, 1], [-45, 45])
    rescale("esc", [-1, 1], [0, 1])
    df['rollRateExp'] = (df['roll_scaled'] - df['roll_scaled'].shift(1)) / (df['ts'] - df['ts'].shift(1))

    return df


def trim_end(flight_df):
    """Trims the end of the data frame based on pitch sd"""
    flight_df["pitch_scaled_sd"] = flight_df["pitch_scaled"].rolling(20).std()

    # Fill first points with max value
    max_sd = flight_df["pitch_scaled_sd"].max(skipna=True)
    flight_df["pitch_scaled_sd"] = flight_df["pitch_scaled_sd"].fillna(max_sd)
    flight_df["has_pitch_sd"] = flight_df["pitch_scaled_sd"] > 0.001

    # All point have sd
    if all(flight_df["has_pitch_sd"]):
        return flight_df

    end_pos = flight_df["has_pitch_sd"].argmin()
    return flight_df.iloc[:end_pos]


def plot_flight(flight_df):
    sens_cols = ["roll_scaled", "pitch_scaled"]
    act_cols = ["servoPosTail_scaled", "servoPosLeft_scaled", "servoPosRight_scaled"]
    acc_cols = ["accX", "accY", "accZ"]
    fig = px.line(flight_df, x="ts", y=[*sens_cols, *act_cols, *acc_cols])
    fig.show()


def plot_flight_split(flight_df):

    # m/s2 > m/s > m

    flight_df['vert_speed'] = (((flight_df.accZ) - 9.8) * -1).cumsum() # / 5 + 2
    flight_df['height'] = (flight_df.vert_speed).cumsum()
    flight_df['speed'] = (((flight_df.accX) - 0.0) * 1).cumsum() * -0.1

    fig = make_subplots(rows=4, cols=1)

    fig.add_trace(go.Line(name="pitch", x=flight_df["ts"], y=flight_df["pitch_scaled"]), row=1, col=1)
    fig.add_trace(go.Line(name="tail servo", x=flight_df["ts"], y=flight_df["servoPosTail_scaled"]), row=1, col=1)

    fig.add_trace(go.Line(name="roll", x=flight_df["ts"], y=flight_df["roll_scaled"]), row=2, col=1)
    fig.add_trace(go.Line(name="roll servo", x=flight_df["ts"], y=flight_df["servoPosLeft_scaled"]), row=2, col=1)
    fig.add_trace(go.Line(name="roll rate", x=flight_df["ts"], y=flight_df["rollRate"]), row=2, col=1)
    fig.add_trace(go.Line(name="roll rate exp", x=flight_df["ts"], y=flight_df["rollRateExp"]), row=2, col=1)

    fig.add_trace(go.Line(name="accX", x=flight_df["ts"], y=flight_df["accX"]), row=3, col=1)
    fig.add_trace(go.Line(name="accY", x=flight_df["ts"], y=flight_df["accY"]), row=3, col=1)
    fig.add_trace(go.Line(name="accZ", x=flight_df["ts"], y=flight_df["accZ"]), row=3, col=1)

    fig.add_trace(go.Line(name="vert_speed", x=flight_df["ts"], y=flight_df["vert_speed"]), row=4, col=1)
    fig.add_trace(go.Line(name="speed", x=flight_df["ts"], y=flight_df["speed"]), row=4, col=1)

    fig.update_layout(height=1200, width=800, title_text="Pitch and Roll")
    fig.show()


#%% view all flights

len(flights)
len(flights[0])



flight_dfs = [flight_to_df(flight) for flight in flights]
len(flight_dfs[0])
flight_dfs = [trim_end(flight_df) for flight_df in flight_dfs]
len(flight_dfs[0])

print(len(flight_dfs))
len(flight_dfs[0])

for flight_num, flight_df in enumerate(flight_dfs):
    print(f"Flight number: {flight_num}")
    plot_flight_split(flight_df)

# #%% analyze specific flight
# flight_num = 4
# plot_flight_split(flight_dfs[flight_num])

# %%

# flight_df = flight_dfs[-3]
# flight_df['height'] = (((flight_df.accZ) - 9.8) * -1).cumsum() / 5 + 2

# px.line(flight_df, x='ts', y='height')

# (((flight_dfs[-1].accZ) - 9.75) * -1).cumsum().plot.line()
# %%

# %%
