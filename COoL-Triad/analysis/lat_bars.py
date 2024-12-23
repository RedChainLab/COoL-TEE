#!/usr/bin/env python
# coding: utf-8

# In[ ]:

import glob
import pandas as pd
import numpy as np
from datetime import datetime
from matplotlib.ticker import LogLocator

csv_separator = ','

import matplotlib.pyplot as plt

params = {'text.usetex' : True,
          'font.size' : 10,
            'font.family' : 'serif',
            'font.serif' : 'Computer Modern Roman',
          }
plt.rcParams.update(params) 

fig, ax = plt.subplots(figsize=(4.5, 2))

RES_DIR_PREFIX = "../out/results/"
RES_DIR_LIST = [
  f"{RES_DIR_PREFIX}NT_lats.csv",
  f"{RES_DIR_PREFIX}UT_lats.csv",
  f"{RES_DIR_PREFIX}TT_lats.csv",
    ]

column_labels = ['Get timestamp', 'Serve request', 'Format response', 'Send response']
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']
hatch_patterns = ['', '//', '..', '\\\\']

TSC_rate = 2899.998

bar_width = 0.2
for idx, file_path in enumerate(RES_DIR_LIST):
  data = pd.read_csv(file_path, sep=csv_separator, header=None)
  data.columns = column_labels
  for col_idx, col_label in enumerate(column_labels):
    data[col_label] = data[col_label] / TSC_rate
    quantiles = data[col_label].quantile([0.25, 0.5, 0.75])
    ax.bar(idx + col_idx * bar_width-1.5*bar_width, quantiles[0.5], bar_width, label=col_label if idx == 0 else "", color=colors[col_idx], zorder=3, hatch=hatch_patterns[col_idx], edgecolor='black', linewidth=0.5)
    ax.errorbar(idx + col_idx * bar_width-1.5*bar_width, quantiles[0.5], 
          yerr=[[quantiles[0.5] - quantiles[0.25]], [quantiles[0.75] - quantiles[0.5]]], 
          fmt='none', ecolor='black', capsize=4, zorder=4, linewidth=0.5)
    quantiles = data[col_label].quantile([0.01, 0.5,  0.99])
    ax.errorbar(idx + col_idx * bar_width-1.5*bar_width, quantiles[0.5],
          yerr=[[quantiles[0.5] - quantiles[0.01]], [quantiles[0.99] - quantiles[0.5]]],
          fmt='none', ecolor='black', capsize=2, zorder=4, linewidth=0.5)
    print(f"{col_label} mean: {data[col_label].mean()} std: {data[col_label].std()} min: {data[col_label].min()} max: {data[col_label].max()}")

ax.set_xticks(range(len(RES_DIR_LIST)))
ax.set_xticklabels(['No time\nsource', 'Untrusted\nOS time', 'Trusted\ntime'])
ax.set_yscale('log')
ax.yaxis.set_minor_locator(LogLocator(base=10.0, subs='auto', numticks=10))
ax.set_ylabel('Median delay ($\\mu$s)')
ax.set_yticks([0.1, 1, 10, 100, 1000, 10000])

ax.yaxis.grid(True, which='major', zorder=0, linewidth=0.5, color="black", alpha=0.5)
ax.yaxis.grid(True, which='minor', zorder=0, alpha=0.2, color="black", linewidth=0.5)
ax.xaxis.grid(False)

ax.legend(handletextpad=0.25, handlelength=1, bbox_to_anchor=(1.01, 0.75), loc='upper left', fontsize="small")

fig.tight_layout()

current_time=datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
fig.savefig(f'../out/lats_hist_{current_time}.pdf', bbox_inches='tight',dpi=1000)
