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
        "wait100noLatByzRho100",
        # "wait100k2r75noLatByzRhoPoT",
        "wait100k1r75noLatByzRhoCuckoo",
        "wait100k1r75noLatByzRhoCuckooContent",
        "wait100noLatByzRho100",
    ]
CONFIG_FILENAME_LIST=[
        "configs_8SP_wait100noLatByzRho100",
        # "configs_8SP_wait100k2r75noLatByzRhoPoT",
        "configs_8SP_wait100k1r75noLatByzRhoCuckoo",
        "configs_8SP_wait100k1r75noLatByzRhoCuckooContent",
        "configs_8SP_wait100noLatByzRho100",
    ]

BEHAVIOURS={"HON_CONS":1,"MAL_CONS":1,"HON_PROV":1,"MAL_PROV":0}

TIME_OFFSET=2250
EXP_SPECS=[
        [
            [
                "WtA",
                "sameDC",
                f"+{TIME_OFFSET}s"
            ],
            [
                "noTEE",
                "WtA",
                "sameDC",
                f"+{TIME_OFFSET}s"
            ],

        ],
        [
            [
                "WtA",
                "sameDC",
                f"+{TIME_OFFSET}s"
            ]
        ],
        [
            [
                "noTEE",
                "WtA",
                "sameDC",
                f"+{TIME_OFFSET}s"
            ]
        ],
        [
            [
                "WtA",
                "sameDC",
                f"+{TIME_OFFSET}s"
            ],
        ],
    ]

RECOMPUTE=1


# In[ ]:


conditions_list=[
        [
            [    
                # [("hW","50ms"),("kErr","0"),("sHM","0.875"),("rho","75")],
                # [("hW","50ms"),("kErr","0"),("sHM","0.75"),("rho","75")],
                # [("hW","50ms"),("kErr","0"),("sHM","0.625"),("rho","75")],
                # [("hW","50ms"),("kErr","0"),("sHM","0.5"),("rho","75")],
                # [("hW","50ms"),("kErr","0"),("sHM","0.375"),("rho","75")],
                # [("hW","50ms"),("kErr","0"),("sHM","0.25"),("rho","75")],
                # [("hW","50ms"),("kErr","0"),("sHM","0.125"),("rho","75")],
                # [("hW","50ms"),("kErr","0"),("sHM","0"),("rho","75")],
                [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.875"),("rho","75")],
                [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.75"),("rho","75")],
                [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.625"),("rho","75")],
                [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","75")],
                [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.375"),("rho","75")],
                [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.25"),("rho","75")],
                [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.125"),("rho","75")],
                [("hW","50ms"),("kErr","0.00001*100"),("sHM","0"),("rho","75")],

                # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.875"),("rho","75")],
                # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.75"),("rho","75")],
                # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.625"),("rho","75")],
                # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","75")],
                # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.375"),("rho","75")],
                # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.25"),("rho","75")],
                # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.125"),("rho","75")],
                # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0"),("rho","75")],
            ],
            [    
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.875"),("rho","75")],
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.75"),("rho","75")],
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.625"),("rho","75")],
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","75")],
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.375"),("rho","75")],
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.25"),("rho","75")],
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.125"),("rho","75")],
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0"),("rho","75")],
            ],
        ],
        [
            [
                [("nReqs","100"),("hW","50ms"),("kErr","0.00001*100"),("sHM","0.875"),("rho","75")],
                [("nReqs","100"),("hW","50ms"),("kErr","0.00001*100"),("sHM","0.75"),("rho","75")],
                [("nReqs","100"),("hW","50ms"),("kErr","0.00001*100"),("sHM","0.625"),("rho","75")],
                [("nReqs","100"),("hW","50ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","75")],
                [("nReqs","100"),("hW","50ms"),("kErr","0.00001*100"),("sHM","0.375"),("rho","75")],
                [("nReqs","100"),("hW","50ms"),("kErr","0.00001*100"),("sHM","0.25"),("rho","75")],
                [("nReqs","100"),("hW","50ms"),("kErr","0.00001*100"),("sHM","0.125"),("rho","75")],
                [("nReqs","100"),("hW","50ms"),("kErr","0.00001*100"),("sHM","0"),("rho","75")],
            ]
        ],
        [
            [
                [("nReqs","100"),("hW","0ms"),("kErr","0.00001*100"),("sHM","0.875"),("rho","75")],
                [("nReqs","100"),("hW","0ms"),("kErr","0.00001*100"),("sHM","0.75"),("rho","75")],
                [("nReqs","100"),("hW","0ms"),("kErr","0.00001*100"),("sHM","0.625"),("rho","75")],
                [("nReqs","100"),("hW","0ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","75")],
                [("nReqs","100"),("hW","0ms"),("kErr","0.00001*100"),("sHM","0.375"),("rho","75")],
                [("nReqs","100"),("hW","0ms"),("kErr","0.00001*100"),("sHM","0.25"),("rho","75")],
                [("nReqs","100"),("hW","0ms"),("kErr","0.00001*100"),("sHM","0.125"),("rho","75")],
                [("nReqs","100"),("hW","0ms"),("kErr","0.00001*100"),("sHM","0"),("rho","75")],
            ]
        ],
        [
            [
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.875"),("rho","75")],
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.75"),("rho","75")],
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.625"),("rho","75")],
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","75")],
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.375"),("rho","75")],
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.25"),("rho","75")],
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.125"),("rho","75")],
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0"),("rho","75")],
            ]
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
