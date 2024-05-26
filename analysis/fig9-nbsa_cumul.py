

# In[ ]:


import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib as mpl
import seaborn as sns
from alive_progress import alive_bar
from scipy.stats import mannwhitneyu
from scipy.stats import ttest_rel
import graphviz as gv
import math

import csv

pd.options.mode.chained_assignment = None  # default='warn'

from matplotlib.patches import Patch
from matplotlib.lines import Line2D
from matplotlib.ticker import FuncFormatter
import pickle as pkl

from datetime import datetime

import platform
import sys
import os

params = {'text.usetex' : True,
          'font.size' : 10,
            'font.family' : 'serif',
            'font.serif' : 'Computer Modern Roman',
          }
plt.rcParams.update(params) 

sys.path.insert(1, '../sim/market')
import request_lifetimes as rl

TIME_INTERVAL=2400
ASSET_RATE=100

START=rl.Event.SERVE
END=rl.Event.RECV

HORIZONTAL=True
RELATIVE=True

NB_POISSON_SAMPLES=1

RQ_type=rl.RQ_types.All

COLUMN=("DIFF" if RELATIVE else END.name)
INPUT_DIR="E:/wt"
OUTPUT_DIR="G:/data"
FIGS_DIR="G:/figs"
EXP_DIR="wait500ByzRho100"#"wait500Byz"#"wait100noLat"#
CONFIG_FILENAME="configs_8SP_wait500ByzRho100"#"configs_8SP_wait500Byz"#"configs_8SP_wait100noLat"#

BEHAVIOURS={"HON_CONS":5,"MAL_CONS":5,"HON_PROV":4,"MAL_PROV":4}

TIME_OFFSET=2250
EXP_SPECS=[
    # ["WtA",f"+{TIME_OFFSET}s"],
    # ["noTEE","WtA",f"+{TIME_OFFSET}s"],
    # [
    #     "noTEE",
    #     "WtA",
    #     #"sameDC",
    #     f"+{TIME_OFFSET}s"
    # ],
    [
        "WtA",
        #"sameDC",
        f"+{TIME_OFFSET}s"
    ],
]

conditions_list=[
    # [
    #     [("rho","12.5")],
    #     [("rho","25")],
    #     [("rho","50")],
    #     [("rho","80")],
    #     [("rho","95")],
    #     [("rho","95")],
    #     [("rho","100")],
    # ]
        # [    
        #     # [("hW","0ms"),("kErr","0"),("sHM","0.875"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0"),("sHM","0.75"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0"),("sHM","0.625"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0"),("sHM","0.5"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0"),("sHM","0.375"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0"),("sHM","0.25"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0"),("sHM","0.125"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0"),("sHM","0"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.875"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.75"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.625"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.375"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.25"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.125"),("rho","25")],
        #     # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0"),("rho","25")],
        # ],
        [    
            # [("hW","50ms"),("kErr","0"),("sHM","0.875"),("rho","25")],
            # [("hW","50ms"),("kErr","0"),("sHM","0.75"),("rho","25")],
            # [("hW","50ms"),("kErr","0"),("sHM","0.625"),("rho","25")],
            [("hW","50ms"),("kErr","0"),("sHM","0.5"),("rho","25")],
            # [("hW","50ms"),("kErr","0"),("sHM","0.375"),("rho","25")],
            # [("hW","50ms"),("kErr","0"),("sHM","0.25"),("rho","25")],
            # [("hW","50ms"),("kErr","0"),("sHM","0.125"),("rho","25")],
            # [("hW","50ms"),("kErr","0"),("sHM","0"),("rho","25")],
            # [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.875"),("rho","25")],
            # [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.75"),("rho","25")],
            # [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.625"),("rho","25")],
            [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","25")],
            # [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.375"),("rho","25")],
            # [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.25"),("rho","25")],
            # [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.125"),("rho","25")],
            # [("hW","50ms"),("kErr","0.00001*100"),("sHM","0"),("rho","25")],

            # [("hW","0ms"),("kErr","0"),("sHM","0.875"),("rho","25")],
            # [("hW","0ms"),("kErr","0"),("sHM","0.75"),("rho","25")],
            # [("hW","0ms"),("kErr","0"),("sHM","0.625"),("rho","25")],
            # [("hW","0ms"),("kErr","0"),("sHM","0.5"),("rho","25")],
            # [("hW","0ms"),("kErr","0"),("sHM","0.375"),("rho","25")],
            # [("hW","0ms"),("kErr","0"),("sHM","0.25"),("rho","25")],
            # [("hW","0ms"),("kErr","0"),("sHM","0.125"),("rho","25")],
            # [("hW","0ms"),("kErr","0"),("sHM","0"),("rho","25")],
            # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.875"),("rho","25")],
            # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.75"),("rho","25")],
            # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.625"),("rho","25")],
            # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","25")],
            # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.375"),("rho","25")],
            # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.25"),("rho","25")],
            # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.125"),("rho","25")],
            # [("hW","0ms"),("kErr","0.00001*100"),("sHM","0"),("rho","25")],
        ],
    ]

RECOMPUTE=1

idx_cols=["MARKET_REPETITION","REPETITION","CONSUMER_BEHAVIOUR","CONSUMER_ID","PROVIDER_BEHAVIOUR","PROVIDER_ID","REQUEST_ID"]
cols=idx_cols+["SEND","SERVE","RECV","FASTEST"]

values_per_key={}
for i, exp_spec in enumerate(EXP_SPECS):
    for conditions in conditions_list[i]:
        for (k, v) in conditions:
            if k not in values_per_key:
                values_per_key[k]=set()
            values_per_key[k].add(v)
print(values_per_key)

str_vals=';'.join([','.join(v).replace('*','x') for v in values_per_key.values()])
str_specs=';'.join([','.join(v) for v in EXP_SPECS])

xstep=0.01
ymax=0
xmax=0
BEGIN="SERVE"
END="RECV"

CUMUL=True
LOG=False

colours=["cornflowerblue","crimson",]
colour_idx=0

plt.figure(figsize=(4.5,2.25))

legend_handles=[
    Line2D([0], [0], color="black", linestyle="solid", label="Honest Consumer - Honest Provider"),
    Line2D([0], [0], color="black", linestyle="dashdot", label="Malicious Consumer - Honest Provider"),
    Line2D([0], [0], color="black", linestyle="dashed", label="Malicious Consumer - Malicious Provider"),
    Line2D([0], [0], color="black", linestyle="dotted", label="Honest Consumer - Malicious Provider"),
    Patch(facecolor="crimson", edgecolor="black", label="COoL"),
    Patch(facecolor="cornflowerblue", edgecolor="black", label="DeSearch-like"),
]

for i, exp_spec in enumerate(EXP_SPECS):

    for conditions in conditions_list[i]:
        str_desc="-".join([EXP_DIR,*exp_spec,*CONFIG_FILENAME.split("_")[1:],"nS="+str(NB_POISSON_SAMPLES),"rS="+str(ASSET_RATE),",".join([a+"="+b for a,b in conditions])]).replace("*","x")
        str_cond=",".join([a+"="+b for a,b in conditions])+"-"+",".join(exp_spec)
        readable_cond=str_cond.split("-")[0].replace("kErr=0.00001*10","LOoL").replace("kErr=0","rdm").replace("hW=","")+","+("noTEE" if str_cond.split("-")[1].split(",")[0]=="noTEE" else "TEE")
        full_dfs=pd.read_csv(f"{OUTPUT_DIR}/{str_desc}.csv", usecols=cols, index_col=idx_cols, low_memory=True)
        full_dfs["DIFF"]=full_dfs[END]-full_dfs[BEGIN]

        distribs=pd.DataFrame()
        behav_grps=full_dfs.groupby(level=["CONSUMER_BEHAVIOUR","PROVIDER_BEHAVIOUR"])
        p=None
        behav_strs={(True,True):"hon-hon",(False,True):"mal-hon",(False,False):"mal-mal",(True,False):"hon-mal"}
        behav_lines={(True,True):"solid",(False,True):"dashdot",(False,False):"dashed",(True,False):"dotted"}
        for behav in behav_grps.groups.keys():
            bins=np.arange(0,behav_grps.get_group(behav)["DIFF"].max()+xstep,xstep)
            n,_=np.histogram(behav_grps.get_group(behav)["DIFF"],weights=behav_grps.get_group(behav)["FASTEST"], bins=bins)
            if CUMUL:
                n=n.cumsum()
            if p is None:
                p=plt.plot(bins[:-1],n,label=f"{behav_strs[behav]}-{readable_cond}", linewidth=1, linestyle=behav_lines[behav], color=colours[colour_idx], drawstyle="steps-post" if CUMUL else "default")
            else:
                plt.plot(bins[:-1],n,label=f"{behav_strs[behav]}-{readable_cond}", linewidth=1, color=p[0].get_color(), linestyle=behav_lines[behav], drawstyle="steps-post" if CUMUL else "default")
            ymax=max(ymax,n.max())
            xmax=max(xmax,bins.max())
            print(f"Finished {behav_strs[behav]}-{readable_cond}")
        colour_idx+=1

plt.xlim(0,0.1)
ypow=math.ceil(math.log(ymax,10))-1
xpow=math.ceil(math.log(xmax,10))-1
plt.yticks(np.arange(0,ymax+2*10**(ypow-1),10**ypow))
plt.yticks(np.arange(0,ymax+2*10**(ypow-1),(10**ypow)/5),minor=True)
plt.xticks(np.arange(0,0.21,0.05),labels=["0","50","100","150","200"])#xmax+10**xpow,10**xpow))#
plt.xticks(np.arange(0,0.21,0.01), minor=True)#xmax+10**xpow,(10**xpow)/5),minor=True)#
plt.grid(axis="y")
plt.grid(axis="x")
plt.grid(which="minor", axis="y", alpha=0.5)
plt.grid(which="minor", axis="x", alpha=0.5)
plt.xlabel("Service-to-reception time $\Delta$ (ms)")
plt.ylabel("Never-before-seen assets")
plt.tight_layout()
if LOG:
    plt.yscale("log")

plt.legend(handles=legend_handles, loc="center right" if CUMUL and not LOG else "upper right", handletextpad=0.25, labelspacing=0.1, fontsize="small")
current_time=datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
filename=f"{FIGS_DIR}/{EXP_DIR}-{str_specs}-nbsa-{'cumul-' if CUMUL else ''}{'log-' if LOG else ''}wt-consprov-behav-{BEGIN}-{END}-{str_vals}-{current_time}.pdf"
plt.tight_layout()
plt.savefig(filename, transparent=True)
print(f"Saved {filename}")
plt.show()


# In[ ]:
