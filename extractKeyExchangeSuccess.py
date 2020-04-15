import pandas as pd
import numpy as np

csv_in_file = "nc-vs-ecdh/Simulations/nc_DataRate3Mbps_delay10ms_Refr20/" + \
              "KeyExchange_nc_DataRate3Mbps_delay10ms_Refr20_Seed1585498320.csv"  # input("Path to the CSV Data File: ")

# Load the data
df = pd.read_csv(csv_in_file)

# remove uninteresting data
del df["Time"]
del df["NodeID"]

data = df.to_numpy()

npframe = np.ndarray((0, 1))

for i in range(0, data.shape[0], 2):
    if data[i] == data[i + 1]:
        npframe = np.vstack([npframe, 1])
    else:
        npframe = np.vstack([npframe, 0])

print("Of {} exchanges {} yielded the same key".format(npframe.shape[0], int(npframe.sum())))

exit()
