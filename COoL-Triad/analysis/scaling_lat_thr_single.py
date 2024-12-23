import glob
import pandas as pd
import numpy as np
from datetime import datetime

csv_separator = ';'

RES_DIR = "../out/results/"

latency_files = glob.glob(f'{RES_DIR}/latency/[0-9]*.log')
latency_df = pd.concat((pd.read_csv(file, sep=csv_separator, names=['CLIENT_ID', 'RQ_ID', "SERVER_ID", 'SEND_TIME', 'LAT']) for file in latency_files), ignore_index=True)
latency_df.set_index(['CLIENT_ID', 'RQ_ID', "SERVER_ID"], inplace=True)

latency_df['SEND_TIME'] = pd.to_datetime(latency_df['SEND_TIME'])
latency_df['RECV_TIME'] = latency_df['SEND_TIME'] + pd.to_timedelta(latency_df['LAT'], unit='s')
latency_df = latency_df.loc[latency_df.groupby(['CLIENT_ID', 'RQ_ID'])['RECV_TIME'].idxmin()]

window_size = pd.Timedelta(minutes=.1)
latency_df['WINDOW'] = latency_df['SEND_TIME'].dt.floor(window_size)

latency_percentiles = latency_df[['WINDOW',"LAT"]].groupby('WINDOW').quantile([i/20 for i in range(1,20)]).unstack(level=1)
latency_counts = latency_df[['WINDOW',"LAT"]].groupby('WINDOW').agg(['count', 'mean', 'std'])
latency_counts["THROUGHPUT"]=latency_counts["LAT"]["count"]/window_size.total_seconds()
latency_counts=pd.merge(latency_counts, latency_percentiles, on='WINDOW', how='inner')
print(latency_counts)
import matplotlib.pyplot as plt
lim=-1
plt.plot(latency_counts["THROUGHPUT"].head(lim), latency_counts["LAT"]["mean"].head(lim), marker='+', linestyle='-', linewidth=1, markersize=6)

plt.xlabel("Throughput (req/s)")
plt.ylabel("Mean Latency (s)")
plt.grid(which='both', axis='both')

current_time=datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

plt.savefig(f'{RES_DIR}/mean_latency_vs_throughput_single_{current_time}.png')
plt.show()
