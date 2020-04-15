import pandas as pd
import numpy as np
import matplotlib

matplotlib.use("TkAgg")
from matplotlib import pyplot as plt

csv_in_file = "nc-vs-ecdh/Simulations/ecdh_DataRate64Kbps_delay3s_Refr20/" +\
              "PacketTxRx_ecdh_DataRate64Kbps_delay3s_Refr20_Seed1585498466.csv"  # input("Path to the CSV Data File: ")

# Load the data
df = pd.read_csv(csv_in_file)

# extract if NC or ECDH
packetType = df["PacketType"][0]

# Remove uninteresting data
del df["NodeID"]
del df["EventType"]
del df["Size"]

update_rows = df["PacketState"] != "UPDATE"
msg_rows = df["PacketType"] != "MSG"

# remove all update and msg rows
df = df[update_rows & msg_rows]

begin_rows = df["PacketState"] == "BEGIN"
receive_rows = df["Comment"] == "receive"

# remove all rows where BEGIN is received
df = df.drop(df[begin_rows & receive_rows].index)

final_rows = df["PacketState"] == "FINAL"
transmit_rows = df["Comment"] == "transmit"

# remove all rows where FINAL is transmitted
df = df.drop(df[final_rows & transmit_rows].index)

del df["PacketType"]
del df["PacketState"]
del df["Comment"]

# We want to make a boxplot of the time from BEGIN to FINAL and one for the number of packets.
data = df.to_numpy()

npframe = np.ndarray((0, 2))

for i in range(0, data.shape[0], 2):
    npframe = np.vstack([npframe, [data[i + 1][0] - data[i][0], data[i + 1][1] - data[i][1] + 1]])

print(npframe)

fig, ax1 = plt.subplots()

color1 = 'red'
# ax1.set_xlabel('time (s)')
ax1.set_ylabel('time (s)', color=color1)

# Boxplot of times
bp1 = ax1.boxplot(npframe[:, 0], positions=[0.5], patch_artist=True)
for patch, color in zip(bp1['boxes'], color1):
    patch.set_edgecolor(color)
    patch.set_facecolor('white')
# ax1.boxplot(t, data1, color=color)
ax1.tick_params(axis='y', labelcolor=color1)

ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

color2 = 'blue'
ax2.set_ylabel('packets', color=color2)  # we already handled the x-label with ax1
# Boxplot of packets
bp2 = ax2.boxplot(npframe[:, 1], positions=[1], patch_artist=True)
for patch, color in zip(bp2['boxes'], color2):
    patch.set_edgecolor(color)
    patch.set_facecolor('white')
# ax2.plot(t, data2, color=color)
ax2.tick_params(axis='y', labelcolor=color2)

ax1.set_xlim([0, 1.5])
ax2.set_xticks([0.5, 1])
ax1.set_xticks([0.5, 1])

ax2.set_xticklabels(["Key generated", "Packets"])

fig.tight_layout()  # otherwise the right y-label is slightly clipped
plt.show()
exit()
