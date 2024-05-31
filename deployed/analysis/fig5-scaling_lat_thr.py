#!/usr/bin/env python
# coding: utf-8

# In[ ]:

import glob
import pandas as pd
import numpy as np
from datetime import datetime
# Set the CSV separator
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
RES_DIR_PREFIX = "../out/results"
RES_DIR_LIST = [
        f"{RES_DIR_PREFIX}_RDM_SIM/",
        f"{RES_DIR_PREFIX}_RDM_SGX_NO_ORAM/",
        f"{RES_DIR_PREFIX}_RDM_SGX_ORAM/",
        f"{RES_DIR_PREFIX}_COoL_SIM/",
        f"{RES_DIR_PREFIX}_COoL_SGX_NO_ORAM/",
        f"{RES_DIR_PREFIX}_COoL_SGX_ORAM/",
    ]
colors = ["tab:blue", "tab:orange", "tab:green"]*2
linestyles = ["dashed","dashed","dashed","solid","solid","solid"]
for i, (RES_DIR, color, linestyle) in enumerate(zip(RES_DIR_LIST,colors, linestyles)):
    # Read latency files
    latency_files = glob.glob(f'{RES_DIR}/latency/[0-9]*.log')
    latency_df = pd.concat((pd.read_csv(file, sep=csv_separator, names=['CLIENT_ID', 'RQ_ID', "SERVER_ID", 'SEND_TIME', 'LAT']) for file in latency_files), ignore_index=True)
    latency_df.set_index(['CLIENT_ID', 'RQ_ID', "SERVER_ID"], inplace=True)

    # Convert milliseconds since epoch to datetime
    latency_df['SEND_TIME'] = pd.to_datetime(latency_df['SEND_TIME'])
    latency_df['RECV_TIME'] = latency_df['SEND_TIME'] + pd.to_timedelta(latency_df['LAT'], unit='s')
    # Keep only the record with the lowest RECV_TIME for each <CLIENT_ID;RQ_ID> pair
    if not TOTAL_THROUGHPUT:
        latency_df = latency_df.loc[latency_df.groupby(['CLIENT_ID', 'RQ_ID'])['RECV_TIME'].idxmin()]

    # Bucket latencies in equal-size time windows
    window_size = pd.Timedelta(minutes=1)
    latency_df['WINDOW'] = latency_df['SEND_TIME'].dt.floor(window_size)

    # Count latencies in each window
    latency_percentiles = latency_df[['WINDOW',"LAT"]].groupby('WINDOW').quantile([i/20 for i in range(1,20)]).unstack(level=1)
    latency_counts = latency_df[['WINDOW',"LAT"]].groupby('WINDOW').agg(['count', 'mean', 'std'])
    latency_counts["THROUGHPUT"]=latency_counts["LAT"]["count"]/window_size.total_seconds()
    latency_counts=pd.merge(latency_counts, latency_percentiles, on='WINDOW', how='inner')
    print(latency_counts)

    lim=-1 #if i<=1 else 8
    ax.plot(latency_counts["THROUGHPUT"].head(lim), latency_counts["LAT"]["mean"].head(lim), marker='+', linestyle=linestyle, linewidth=1, markersize=6, color=color)
    #plt.fill_between(latency_counts["THROUGHPUT"].head(lim),latency_counts["LAT"][0.10].head(lim),latency_counts["LAT"][0.90].head(lim), alpha=0.2)
    #plt.plot(latency_counts["THROUGHPUT"].head(lim),latency_counts["LAT"][0.90].head(lim), marker='+', linestyle='dashed', linewidth=1, markersize=6, color=color)
ax.set_xlabel("Throughput (req/s)")
ax.set_ylabel("Mean latency (ms)")
ax.grid(which='both', axis='both')
ax.set_xticks(np.arange(0, 801, 100))
ax.set_xticks(np.arange(0, 801, 20), minor=True)
ax.set_yticks(np.arange(0, 0.061, 0.01))
ax.set_yticks(np.arange(0, 0.061, 0.002), minor=True)
ax.set_yticklabels([f"{i*1000:.0f}" for i in np.arange(0, 0.061, 0.01)])
ax.grid(which='minor', axis='both', alpha=0.2)
#plt.title("Mean Latency vs Throughput")
current_time=datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
ax.legend([
        f"DeSearch-like $\\vert$ SGX-SIM",
        f"DeSearch-like $\\vert$ SGX-HW",
        f"DeSearch-like $\\vert$ SGX-HW + ORAM",
        f"COoL $\\vert$ SGX-SIM",
        f"COoL $\\vert$ SGX-HW",
        f"COoL $\\vert$ SGX-HW + ORAM",
            ], labelspacing=0.2, fontsize="small")
fig.savefig(f'../out/mean_latency_vs_{"total_" if TOTAL_THROUGHPUT else "effective_"}throughput_{current_time}.pdf', bbox_inches='tight',dpi=1000)

# %%
