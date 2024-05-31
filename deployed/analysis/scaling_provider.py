import glob
import pandas as pd
import numpy as np
# Set the CSV separator
csv_separator = ';'

RES_DIR = "../out/results/"

# Read latency files
latency_files = glob.glob(f'{RES_DIR}/latency/[0-9]*.log')
latency_df = pd.concat((pd.read_csv(file, sep=csv_separator, names=['CLIENT_ID', 'RQ_ID', "SERVER_ID", 'SEND_TIME', 'LAT']) for file in latency_files), ignore_index=True)
latency_df.set_index(['CLIENT_ID', 'RQ_ID', "SERVER_ID"], inplace=True)

# Convert milliseconds since epoch to datetime
latency_df['SEND_TIME'] = pd.to_datetime(latency_df['SEND_TIME'])
latency_df['RECV_TIME'] = latency_df['SEND_TIME'] + pd.to_timedelta(latency_df['LAT'], unit='s')
# Keep only the record with the lowest RECV_TIME for each <CLIENT_ID;RQ_ID> pair
latency_df = latency_df.loc[latency_df.groupby(['CLIENT_ID', 'RQ_ID'])['RECV_TIME'].idxmin()]

# Bucket latencies in equal-size time windows
window_size = pd.Timedelta(minutes=.02)
latency_df['WINDOW'] = latency_df['SEND_TIME'].dt.floor(window_size)

# Count latencies in each window
latency_percentiles = latency_df[['WINDOW',"LAT"]].groupby('WINDOW').quantile([i/20 for i in range(1,20)]).unstack(level=1)
latency_counts = latency_df[['WINDOW',"LAT"]].groupby('WINDOW').agg(['count', 'mean', 'std'])
latency_counts["THROUGHPUT"]=latency_counts["LAT"]["count"]/window_size.total_seconds()
latency_counts=pd.merge(latency_counts, latency_percentiles, on='WINDOW', how='inner')
print(latency_counts)

import matplotlib.pyplot as plt

# Print the dataframes
print("Latency DataFrame:")
print(latency_df)

# Plot latency_count
fig, ax1 = plt.subplots()
# Plot latency_count for each provider
providers = latency_df.index.get_level_values('SERVER_ID').unique()
colors = ['blue', 'green', 'red', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive', 'cyan']

for i, provider in enumerate(providers):
    print(f"Plotting latency for provider {provider}")
    provider_latency_counts = latency_df[latency_df.index.get_level_values('SERVER_ID') == provider][['WINDOW',"LAT"]].groupby('WINDOW').agg(['count', 'mean', 'std'])
    print(provider_latency_counts["LAT"])
    ax1.plot(provider_latency_counts.index, provider_latency_counts["LAT"]["mean"], label=f'{provider} Latency', linestyle="dashed", color=colors[i])

ax1.set_xlabel('Time')
ax1.set_ylabel('Latency')
ax1.set_title('Latency and Throughput over Time')
# ax1.set_yscale('log')

ax2 = ax1.twinx()

for i, provider in enumerate(providers):
    print(f"Plotting throughput for provider {provider}")
    provider_latency_counts = latency_df[latency_df.index.get_level_values('SERVER_ID') == provider][['WINDOW',"LAT"]].groupby('WINDOW').agg(['count', 'mean', 'std'])
    print(provider_latency_counts["LAT"])
    provider_latency_counts["THROUGHPUT"]=provider_latency_counts["LAT"]["count"]/window_size.total_seconds()
    ax2.plot(provider_latency_counts.index, provider_latency_counts["THROUGHPUT"], label=f'{provider} Throughput', color=colors[i])

ax2.set_ylabel('Throughput')

ax1.plot(latency_counts.index, latency_counts["LAT"]["mean"], label='Avg. Latency', linestyle='dashed', color='black', linewidth=2)
ax2.plot(latency_counts.index, latency_counts["THROUGHPUT"], label='Throughput', color='black', linewidth=2)

fig.legend(loc='upper left')

plt.savefig(f'{RES_DIR}/provider_latency_throughput_over_time.png')
plt.show()
