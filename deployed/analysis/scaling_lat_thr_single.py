import glob
import pandas as pd
import numpy as np
from datetime import datetime
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
window_size = pd.Timedelta(minutes=.5)
latency_df['WINDOW'] = latency_df['SEND_TIME'].dt.floor(window_size)

# Count latencies in each window
latency_percentiles = latency_df[['WINDOW',"LAT"]].groupby('WINDOW').quantile([i/20 for i in range(1,20)]).unstack(level=1)
latency_counts = latency_df[['WINDOW',"LAT"]].groupby('WINDOW').agg(['count', 'mean', 'std'])
latency_counts["THROUGHPUT"]=latency_counts["LAT"]["count"]/window_size.total_seconds()
latency_counts=pd.merge(latency_counts, latency_percentiles, on='WINDOW', how='inner')
print(latency_counts)
import matplotlib.pyplot as plt
lim=-1 #if i<=1 else 8
plt.plot(latency_counts["THROUGHPUT"].head(lim), latency_counts["LAT"]["mean"].head(lim), marker='+', linestyle='-', linewidth=1, markersize=6)
#plt.fill_between(latency_counts["THROUGHPUT"].head(lim),latency_counts["LAT"][0.10].head(lim),latency_counts["LAT"][0.90].head(lim), alpha=0.2)
#plt.plot(latency_counts["THROUGHPUT"].head(lim),latency_counts["LAT"][0.90].head(lim), marker='+', linestyle='dashed', linewidth=1, markersize=6, color=color)
plt.xlabel("Throughput (req/s)")
plt.ylabel("Mean Latency (s)")
plt.grid(which='both', axis='both')
#plt.title("Mean Latency vs Throughput")
current_time=datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

plt.savefig(f'{RES_DIR}/mean_latency_vs_throughput_single_{current_time}.png')
plt.show()
