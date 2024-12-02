#!/usr/bin/env python
# coding: utf-8

# In[ ]:


OUTPUT_DIR="G:/data"
FIGS_DIR="G:/figs"


# In[ ]:


import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

pd.options.mode.chained_assignment = None  # default='warn'

from matplotlib.patches import Patch

import sys
import re

from datetime import datetime

# In[ ]:


params = {'text.usetex' : True,
          'font.size' : 10,
            'font.family' : 'serif',
            'font.serif' : 'Computer Modern Roman',
          }
plt.rcParams.update(params) 


# In[ ]:


sys.path.insert(1, '../sim/market')
import request_lifetimes as rl


# In[ ]:


TIME_INTERVAL=2400
ASSET_RATE=100

START=rl.Event.SEND
END=rl.Event.RECV

HORIZONTAL=True
RELATIVE=True

NB_POISSON_SAMPLES=1

RQ_type=rl.RQ_types.All

COLUMN=("DIFF" if RELATIVE else END.name)
INPUT_DIR="E:/wt"
EXP_LIST=[
        "wait2000r25Atlas",
    ]
CONFIG_FILENAME_LIST=[
        "configs_44SP_wait2000r25Atlas",
    ]

TIME_OFFSET=2250
EXP_SPECS=[
        [
            [
                "WtA",
                f"+{2250}s"
            ],
        ],
    ]

RECOMPUTE=1


# In[ ]:


conditions_list=[
        [
            [
                [("nReqs","2000"),("hW","50ms"),("kErr","0.00001*2000"),("sHM","0.875"),("rho","25"),("t4ct","true")],
                [("nReqs","2000"),("hW","50ms"),("kErr","0.00001*2000"),("sHM","0.75"),("rho","25"),("t4ct","true")],
                [("nReqs","2000"),("hW","50ms"),("kErr","0.00001*2000"),("sHM","0.625"),("rho","25"),("t4ct","true")],
                [("nReqs","2000"),("hW","50ms"),("kErr","0.00001*2000"),("sHM","0.5"),("rho","25"),("t4ct","true")],
                [("nReqs","2000"),("hW","50ms"),("kErr","0.00001*2000"),("sHM","0.375"),("rho","25"),("t4ct","true")],
                [("nReqs","2000"),("hW","50ms"),("kErr","0.00001*2000"),("sHM","0.25"),("rho","25"),("t4ct","true")],
                [("nReqs","2000"),("hW","50ms"),("kErr","0.00001*2000"),("sHM","0.125"),("rho","25"),("t4ct","true")],
                [("nReqs","2000"),("hW","50ms"),("kErr","0.00001*2000"),("sHM","0"),("rho","25"),("t4ct","true")],
            ],
        ]
    ]

idx_cols=["MARKET_REPETITION","REPETITION","CONSUMER_BEHAVIOUR","CONSUMER_ID","PROVIDER_BEHAVIOUR","PROVIDER_ID","REQUEST_ID"]
cols=idx_cols+["SEND","SERVE","RECV","FASTEST"]

values_per_key={}
for j, exp_spec_list in enumerate(EXP_SPECS):
    for i, exp_spec in enumerate(exp_spec_list):
        for conditions in conditions_list[j][i]:
            for (k, v) in conditions:
                if k not in values_per_key:
                    values_per_key[k]=set()
                values_per_key[k].add(v)
print(values_per_key)

str_vals=';'.join([','.join(v).replace('*','x') for v in values_per_key.values()])
str_specs=';'.join([','.join("("+','.join(v)+")" for v in w) for w in EXP_SPECS])

step=500
ymax=0
BEGIN="SEND"
END="RECV"
multiplier=0
hatches=["//",".","\\\\","oo","xx","OO","XX","*"]
hatchCount=0
legend_elements=[
    Patch(facecolor="tab:blue",edgecolor='black', label='Honest providers'),
    Patch(facecolor="tab:red",edgecolor='black', label='Malicious providers'),
]
semi_flat_cond=[item for sublist in conditions_list for item in sublist]
nb_cond=len([item for sublist in semi_flat_cond for item in sublist])
total_nb_byz=8
