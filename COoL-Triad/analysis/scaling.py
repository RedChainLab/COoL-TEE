import glob
import pandas as pd
import numpy as np

csv_separator = ';'

latency_files = glob.glob('../out/results/latency/[0-9]*.log')
latency_df = pd.concat((pd.read_csv(file, sep=csv_separator, names=['CLIENT_ID', 'RQ_ID', "SERVER_ID", 'SEND_TIME', 'LAT']) for file in latency_files), ignore_index=True)
latency_df.set_index(['CLIENT_ID', 'RQ_ID', "SERVER_ID"], inplace=True)

latency_df['SEND_TIME'] = pd.to_datetime(latency_df['SEND_TIME'])
latency_df['RECV_TIME'] = latency_df['SEND_TIME'] + pd.to_timedelta(latency_df['LAT'], unit='s')
latency_df = latency_df.loc[latency_df.groupby(['CLIENT_ID', 'RQ_ID'])['RECV_TIME'].idxmin()]

window_size = pd.Timedelta(minutes=.5)
latency_df['WINDOW'] = latency_df['SEND_TIME'].dt.floor(window_size)

latency_percentiles = latency_df[['WINDOW',"LAT"]].groupby('WINDOW').quantile([i/20 for i in range(1,20)]).unstack(level=1)
latency_counts = latency_df[['WINDOW',"LAT"]].groupby('WINDOW').agg(['count', 'mean', 'std'])
latency_counts["THROUGHPUT"]=latency_counts["LAT"]["count"]/window_size.total_seconds()
latency_counts=pd.merge(latency_counts, latency_percentiles, on='WINDOW', how='inner')
print(latency_counts)

import matplotlib.pyplot as plt

print("Latency DataFrame:")
print(latency_df)

fig, ax1 = plt.subplots()

ax1.plot(latency_counts.index, latency_counts["LAT"]["mean"], label='Avg. Latency', linestyle='dashed', color='black')
ax1.fill_between(latency_counts.index, latency_percentiles["LAT"][0.05], latency_percentiles["LAT"][0.10], alpha=0.3, color="tab:blue", label='5%-10% Percentiles')
ax1.fill_between(latency_counts.index, latency_percentiles["LAT"][0.10], latency_percentiles["LAT"][0.25], alpha=0.5, color="tab:blue", label='10%-25% Percentiles')
ax1.fill_between(latency_counts.index, latency_percentiles["LAT"][0.25], latency_percentiles["LAT"][0.50], alpha=0.7, color="tab:blue", label='25%-50% Percentiles')
ax1.plot(latency_counts.index, latency_percentiles["LAT"][0.5], label='Median Latency', color='tab:blue')
ax1.fill_between(latency_counts.index, latency_percentiles["LAT"][0.50], latency_percentiles["LAT"][0.75], alpha=0.7, color="tab:blue", label='50%-75% Percentiles')
ax1.fill_between(latency_counts.index, latency_percentiles["LAT"][0.75], latency_percentiles["LAT"][0.90], alpha=0.5, color="tab:blue", label='75%-90% Percentiles')
ax1.fill_between(latency_counts.index, latency_percentiles["LAT"][0.90], latency_percentiles["LAT"][0.95], alpha=0.3, color="tab:blue", label='90%-95% Percentiles')

ax1.set_xlabel('Time')
ax1.set_ylabel('Latency')
ax1.set_title('Latency and Throughput over Time')

ax2 = ax1.twinx()
ax2.plot(latency_counts.index, latency_counts["THROUGHPUT"], label='Throughput', color='red')
ax2.set_ylabel('Throughput')

fig.legend(loc='upper left')

plt.savefig('../out/results/latency_throughput_over_time.png')
plt.show()
