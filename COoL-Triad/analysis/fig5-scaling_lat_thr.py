#!/usr/bin/env python
# coding: utf-8

# In[ ]:

import glob
import pandas as pd
import numpy as np
from datetime import datetime

csv_separator = ';'

import matplotlib.pyplot as plt

params = {'text.usetex' : True,
          'font.size' : 10,
            'font.family' : 'serif',
            'font.serif' : 'Computer Modern Roman',
          }
plt.rcParams.update(params) 

fig, ax = plt.subplots(figsize=(4.5,2))

TOTAL_THROUGHPUT = True
RES_DIR_PREFIX = "../out/results/latency"
RES_DIR_LIST = [
        f"{RES_DIR_PREFIX}_NTr_3000/",
        f"{RES_DIR_PREFIX}_UTr_3000/",
        f"{RES_DIR_PREFIX}_TTr_3000/",
    ]
colors = ["tab:blue", "tab:orange", "tab:green"]*2
linestyles = ["dashed","dashed","dashed","solid","solid","solid"]
for i, (RES_DIR, color, linestyle) in enumerate(zip(RES_DIR_LIST,colors, linestyles)):

    latency_files = glob.glob(f'{RES_DIR}/[0-9]*.log')
    latency_df = pd.concat((pd.read_csv(file, sep=csv_separator, names=['CLIENT_ID', 'RQ_ID', "SERVER_ID", 'SEND_TIME', 'LAT']) for file in latency_files), ignore_index=True)
    latency_df.set_index(['CLIENT_ID', 'RQ_ID', "SERVER_ID"], inplace=True)


    latency_df['SEND_TIME'] = pd.to_datetime(latency_df['SEND_TIME'])
    latency_df['RECV_TIME'] = latency_df['SEND_TIME'] + pd.to_timedelta(latency_df['LAT'], unit='s')
    if not TOTAL_THROUGHPUT:
        latency_df = latency_df.loc[latency_df.groupby(['CLIENT_ID', 'RQ_ID'])['RECV_TIME'].idxmin()]

    window_size = pd.Timedelta(minutes=0.2)
    latency_df['WINDOW'] = latency_df['SEND_TIME'].dt.floor(window_size)

    latency_percentiles = latency_df[['WINDOW',"LAT"]].groupby('WINDOW').quantile([i/20 for i in range(1,20)]).unstack(level=1)
    latency_counts = latency_df[['WINDOW',"LAT"]].groupby('WINDOW').agg(['count', 'mean', 'std'])
    latency_counts["THROUGHPUT"]=latency_counts["LAT"]["count"]/window_size.total_seconds()
    latency_counts=pd.merge(latency_counts, latency_percentiles, on='WINDOW', how='inner')
    print(latency_counts)

    head_lim=20
    tail_lim=20
    ax.plot(latency_counts["THROUGHPUT"].head(head_lim).tail(tail_lim), latency_counts["LAT"]["mean"].head(head_lim).tail(tail_lim), marker='+', linestyle=linestyle, linewidth=1, markersize=6, color=color)

ax.set_xlabel("Throughput (req/s)")
ax.set_ylabel("Mean latency (ms)")
ax.grid(which='both', axis='both')
ax.set_xticks(np.arange(0, 801, 100))
ax.set_xticks(np.arange(0, 801, 20), minor=True)
ax.set_yticks(np.arange(0, 0.061, 0.01))
ax.set_yticks(np.arange(0, 0.061, 0.002), minor=True)
ax.set_yticklabels([f"{i*1000:.0f}" for i in np.arange(0, 0.061, 0.01)])
ax.grid(which='minor', axis='both', alpha=0.2)

current_time=datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
ax.legend([
        f"COoL $\\vert$ no time",
        f"COoL $\\vert$ untrst. time",
        f"COoL $\\vert$ trusted time",
            ], labelspacing=0.2, fontsize="small", handletextpad=0.25, loc="upper left")
fig.savefig(f'../out/mean_latency_vs_{"total_" if TOTAL_THROUGHPUT else "effective_"}throughput_{current_time}.pdf', bbox_inches='tight',dpi=1000)
