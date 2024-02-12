#!/usr/bin/env python
# coding: utf-8

# In[ ]:


OUTPUT_DIR="/mnt/g/data"
FIGS_DIR="/mnt/g/figs"


# In[ ]:


import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

import math

pd.options.mode.chained_assignment = None  # default='warn'

import sys


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
EXP_DIR="wait500ByzRho100"
CONFIG_FILENAME="configs_8SP_wait500ByzRho100"

BEHAVIOURS={"HON_CONS":1,"MAL_CONS":1,"HON_PROV":1,"MAL_PROV":0}

TIME_OFFSET=2250
EXP_SPECS=[
    [
        "WtA",
        f"+{TIME_OFFSET}s"
    ],
]

RECOMPUTE=1


# In[ ]:


conditions_list=[
        [    
            [("hW","0ms"),("kErr","0"),("sHM","0.5"),("rho","25")],

            [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","25")],
        ],
    ]


# In[ ]:


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

plt.figure(figsize=(3.5,3))

xstep=0.01
ymax=0
xmax=0
BEGIN="SEND"
END="RECV"
colours=["cornflowerblue","crimson",]
colour_idx=0
for i, exp_spec in enumerate(EXP_SPECS):
    distribs=pd.DataFrame()
    for conditions in conditions_list[i]:
        str_desc="-".join([EXP_DIR,*exp_spec,*CONFIG_FILENAME.split("_")[1:],"nS="+str(NB_POISSON_SAMPLES),"rS="+str(ASSET_RATE),",".join([a+"="+b for a,b in conditions])]).replace("*","x")
        str_cond=",".join([a+"="+b for a,b in conditions])+"-"+",".join(exp_spec)
        readable_cond=str_cond.split("-")[0].replace("kErr=0.00001*100","LOoL").replace("kErr=0","rdm").replace("hW=","")+","+("noTEE" if str_cond.split("-")[1].split(",")[0]=="noTEE" else "TEE")
        full_dfs=pd.read_csv(f"{OUTPUT_DIR}/{str_desc}.csv", usecols=cols, index_col=idx_cols, low_memory=True)
        full_dfs["DIFF"]=full_dfs[END]-full_dfs[BEGIN]

        grps=full_dfs.groupby(level="REPETITION")
        for grp in grps.groups.keys():
            bins=np.arange(0,grps.get_group(grp)["DIFF"].max()+xstep,xstep)
            n,_=np.histogram(grps.get_group(grp)["DIFF"], bins=bins)
            n=np.cumsum(n)
            #ymax=max(ymax,n.max())
            #xmax=max(xmax,bins.max())
            bin_dict={bins[i]:n[i] for i in range(len(bins)-1)}
            distribs = pd.concat([distribs,pd.DataFrame(bin_dict,index=[grps.get_group(grp)])])
            print(f"Finished {readable_cond} repetition {grp}")
        quantiles=distribs.quantile([0.1,0.25,0.5,0.75,0.9])
        print(quantiles)
        print(f"Finished {readable_cond}")
        th_str="$^{th}$"
        p=plt.plot(quantiles.loc[0.1],label=f"{'COoL' if readable_cond.split(',')[1]=='LOoL' else 'random'}: 10{th_str} pctl", linewidth=1, linestyle="dotted", color=colours[colour_idx])
        plt.plot(quantiles.loc[0.25],label=f"{'COoL' if readable_cond.split(',')[1]=='LOoL' else 'random'}: 25{th_str} pctl", linewidth=1, color=colours[colour_idx], linestyle="dashed")
        plt.plot(quantiles.loc[0.5],label=f"{'COoL' if readable_cond.split(',')[1]=='LOoL' else 'random'}: median", linewidth=1, color=colours[colour_idx])
        plt.plot(quantiles.loc[0.75],label=f"{'COoL' if readable_cond.split(',')[1]=='LOoL' else 'random'}: 75{th_str} pctl", linewidth=1, color=colours[colour_idx], linestyle="dashed")
        plt.plot(quantiles.loc[0.9],label=f"{'COoL' if readable_cond.split(',')[1]=='LOoL' else 'random'}: 90{th_str} pctl", linewidth=1, color=colours[colour_idx], linestyle="dotted")
        #plt.fill_between(quantiles.columns, quantiles.loc[0.1], quantiles.loc[0.9], alpha=0.2, color=p[0].get_color())
        #plt.fill_between(quantiles.columns, quantiles.loc[0.25], quantiles.loc[0.75], alpha=0.4, color=p[0].get_color())
        xmax=max(xmax,quantiles.columns.max())
        ymax=max(ymax,quantiles.max().max())
        colour_idx+=1
plt.xlim(0,0.1)
ypow=math.ceil(math.log(ymax,10))-1
xpow=math.ceil(math.log(xmax,10))-2
ylabels=[f"{int(y/1000)}k" for y in np.arange(0,ymax+2*10**(ypow-1),10**ypow)]
plt.yticks(np.arange(0,ymax+2*10**(ypow-1),10**ypow), labels=ylabels)
plt.yticks(np.arange(0,ymax+2*10**(ypow-1),(10**ypow)/5),minor=True)
plt.xticks(np.arange(0,xmax+10**xpow,10**xpow), labels=[f"{x:.1f}" if (int(10*x))%2==0 else "" for x in np.arange(0,xmax+10**xpow,10**xpow)])
plt.xticks(np.arange(0,xmax+10**xpow,10**xpow/5),minor=True)
plt.grid(axis="y")
plt.grid(axis="x")
plt.grid(which="minor", axis="y", alpha=0.5)
plt.grid(which="minor", axis="x", alpha=0.5)
plt.xlabel("Roundtrip response time $\Delta$ (s)")
plt.ylabel("Number of requests")
#plt.title(f"Response time distribution")
plt.legend(reverse=True, loc="lower right", handletextpad=0.25, labelspacing=0.1)
filename=f"{FIGS_DIR}/{EXP_DIR}-{str_specs}-lat-dist-cumul-quantiles-{BEGIN}-{END}-{str_vals}.pdf"
plt.savefig(filename, transparent=True, dpi=1000, bbox_inches='tight')
print(f"Saved {filename}")
plt.show()

