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
        "wait100k2r75noLatByzRhoPoT",
    ]
CONFIG_FILENAME_LIST=[
        "configs_8SP_wait100noLatByzRho100",
        "configs_8SP_wait100k2r75noLatByzRhoPoT"
    ]

BEHAVIOURS={"HON_CONS":1,"MAL_CONS":1,"HON_PROV":1,"MAL_PROV":0}

TIME_OFFSET=2250
EXP_SPECS=[
        [
            [
                "noTEE",
                "WtA",
                "sameDC",
                f"+{TIME_OFFSET}s"
            ],
            [
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
    ]

RECOMPUTE=1


# In[ ]:


conditions_list=[
        [
            [    
                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","75")],
            ],
            [    
                [("hW","50ms"),("kErr","0"),("sHM","0.5"),("rho","75")],

                [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","75")],

                [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","75")],
            ],
        ],
        [
            [
                [("nReqs","100"),("hW","50ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","75")],
            ]
        ]
    ]


# In[ ]:


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

count=0

d={ 
    'CONSUMER_BEHAVIOUR': ['Malicious consumers', 'Malicious consumers', 'Malicious consumers', 'Malicious consumers', 'Malicious consumers'],
    'ATTACK_TYPE': ['without TEEs', 'random selection with~TEEs', 'COoL -TEE', 'Fault-free', 'PoT'],
    **{(i,"hon"):np.ones(nb_cond)*2 for i in range(4,5)},
    **{(i,"mal"):np.ones(nb_cond)*2 for i in range(4,5)},
    **{(i,"err"):np.ones(nb_cond)*2 for i in range(4,5)},
    }
for k, exp_specs in enumerate(EXP_SPECS):
    for i, exp_spec in enumerate(exp_specs):
        for j, conditions in enumerate(conditions_list[k][i]):
            str_desc="-".join([EXP_LIST[k],*exp_spec,*CONFIG_FILENAME_LIST[k].split("_")[1:],"nS="+str(NB_POISSON_SAMPLES),"rS="+str(ASSET_RATE),",".join([a+"="+b for a,b in conditions])]).replace("*","x")
            str_cond=",".join([a+"="+b for a,b in conditions])+"-"+",".join(exp_spec)
            readable_cond=str_cond.split("-")[0].replace("kErr=0.00001*10","LOoL").replace("kErr=0","rdm").replace("hW=","")+","+("noTEE" if str_cond.split("-")[1].split(",")[0]=="noTEE" else "TEE")
            nbByz=8-int(float(re.compile("sHM=[0-9.]+").search(readable_cond).group(0).split("=")[1])*8 if "sHM=" in readable_cond else 0)
            readable_cond=re.sub("sHM=[0-9.]+",str(nbByz)+"MalProv",readable_cond)
            full_dfs=pd.read_csv(f"{OUTPUT_DIR}/{str_desc}.csv", usecols=cols, index_col=idx_cols, low_memory=True)
            full_dfs["DIFF"]=full_dfs[END]-full_dfs[BEGIN]

            acq_behav=pd.DataFrame({True:[],False:[]})
            acq_behav.columns=pd.MultiIndex.from_tuples([("FASTEST",True),("FASTEST",False)])
            acq_behav=pd.concat([acq_behav,full_dfs.groupby(level=["CONSUMER_BEHAVIOUR","PROVIDER_BEHAVIOUR","REPETITION"]).agg({"FASTEST":np.sum}).unstack(1)]).fillna(0)
            acq_behav.set_index(pd.MultiIndex.from_tuples([(b,a) for a,b in acq_behav.index]), inplace=True)
            acq_behav.index.names=["REPETITION","CONSUMER_BEHAVIOUR"]
            acq_behav.columns=["HON_PROV","MAL_PROV"]
            print(acq_behav)
            width=0.9/nb_cond # /number of conditions, ie 6
            index=np.arange(2)
            offset=-width*(nb_cond/2-0.5)+width*multiplier

            mean_acq_behav=acq_behav.groupby(level="CONSUMER_BEHAVIOUR").agg({"HON_PROV":np.mean,"MAL_PROV":np.mean})
            total_mean=mean_acq_behav.sum().sum()
            mean_acq_behav=mean_acq_behav/total_mean
            std_acq_behav=(acq_behav["HON_PROV"]+acq_behav["MAL_PROV"]).groupby(level="CONSUMER_BEHAVIOUR").std()
            std_acq_behav=std_acq_behav/total_mean
            print(mean_acq_behav)
            print(std_acq_behav)

            plt.bar(index+offset,mean_acq_behav["HON_PROV"], width=width, color='tab:blue', label='Honest providers', edgecolor='black')
            plt.bar(index+offset,mean_acq_behav["MAL_PROV"], bottom=mean_acq_behav["HON_PROV"], width=width, color='tab:red', label='Malicious providers', edgecolor='black')

            #plt.errorbar(index+offset,mean_acq_behav["HON_PROV"], yerr=std_acq_behav["HON_PROV"], fmt='none', ecolor='darkblue', capsize=3)
            plt.errorbar(index+offset,mean_acq_behav["MAL_PROV"]+mean_acq_behav["HON_PROV"], yerr=std_acq_behav, fmt='none', ecolor='black', capsize=3)

            d[(nbByz,"hon")][count]=mean_acq_behav["HON_PROV"][False]
            #d[(nbByz,"hon")][count//total_nb_byz+len(d[(nbByz,"hon")])//2]=mean_acq_behav["HON_PROV"][True]
            d[(nbByz,"mal")][count]=mean_acq_behav["MAL_PROV"][False]
            #d[(nbByz,"mal")][count//total_nb_byz+len(d[(nbByz,"mal")])//2]=mean_acq_behav["MAL_PROV"][True]
            d[(nbByz,"err")][count]=std_acq_behav[False]
            #d[(nbByz,"err")][count//total_nb_byz+len(d[(nbByz,"err")])//2]=std_acq_behav[True]
            count+=1
            multiplier+=1

            legend_elements.append(Patch(facecolor="none",edgecolor='black', label=f'{readable_cond}'))
            print(f"Finished {readable_cond}")


# In[ ]:


df=pd.DataFrame(d)

df.set_index(['CONSUMER_BEHAVIOUR', 'ATTACK_TYPE'], inplace=True)
print(df)
#df=df[[df.columns[x//3+(x%3)*8] for x in range(24)]]
df.columns = pd.MultiIndex.from_product([["MalProv"+str(i) for i in range(4,5)],["hon","mal","err"]])
print(df)
# Create figure with a subplot for each factory zone with a relative width
# proportionate to the number of factories
zones = df.index.levels[0]
nplots = zones.size
plots_width_ratios = [df.xs(zone).index.size for zone in zones]
fig, axes = plt.subplots(nrows=1, ncols=nplots, sharey=True, figsize=(3.5, 2.5),
                         gridspec_kw = dict(width_ratios=plots_width_ratios, wspace=0))

# Loop through array of axes to create grouped bar chart for each factory zone
alpha = 0.3 # used for grid lines, bottom spine and separation lines between zones
firstCol = True # used to draw left spine only for first subplot

for zone, ax in zip(zones, [axes]):
    mal_cols=df.xs(zone).columns.levels[0]
    width=0.9/len(mal_cols)
    multiplier=0
    index=np.arange(df.xs(zone).index.size)
    for col in mal_cols:
        offset=-width*(len(mal_cols)/2-0.5) + width*multiplier

        #ax.bar(index+offset, df.xs(zone)[col]['hon'], width=width, color="lightskyblue", edgecolor="black")
        ax.bar(index+offset, df.xs(zone)[col]['mal']+df.xs(zone)[col]['hon'], yerr=df.xs(zone)[col]['err'], width=width, color="lightgrey", edgecolor="black", ecolor='black', capsize=3)
        multiplier+=1

    for spine in ['top','right']:
        ax.spines[spine].set_visible(False)
    #ax.spines['bottom'].set_alpha(alpha)

    # Set and place x labels for factory zones
    #ax.set_xlabel(zone)
    ax.xaxis.set_label_coords(x=0.5, y=-0.215)
    ax.yaxis.set_label_coords(x=-0.15, y=0)
    # Format major tick labels for factory names: note that because this figure is
    # only about 10 inches wide, I choose to rewrite the long names on two lines.
    ticklabels = [name.replace(' ', '\n') if len(name) > 8 else name
                  for name in df.xs(zone).index]
    ax.set_xticks(np.arange(df.xs(zone).index.size))
    ax.set_xticklabels(ticklabels, rotation=0, ha='center')
    ax.tick_params(axis='x', length=0, pad=7)
    
    # Set and format minor tick marks for separation lines between zones: note
    # that except for the first subplot, only the right tick mark is drawn to avoid
    # duplicate overlapping lines so that when an alpha different from 1 is chosen
    # (like in this example) all the lines look the same
    if firstCol:
        ax.set_ylabel("Share of never-before-seen assets")
        #ax.set_xticks([*ax.get_xlim()], minor=True)
        ax.set_yticklabels([f"{x}\%" for x in range(0,101,10)])
        firstCol = False
    #else:
        #ax.set_xticks([ax.get_xlim()[1]], minor=True)
    ax.tick_params(axis="x", which='minor', length=55, width=0.8, color=[0, 0, 0, alpha])

    ax.set_yticks(np.arange(0,1.01,0.1))
    ax.set_yticks(np.arange(0,1.01,0.02),minor=True)
    ax.grid(axis="y", which="major", alpha=1)
    ax.grid(axis="y", which="minor", alpha=0.25)
    ax.hlines(0.5,-1,3.75,linestyles="dashed",colors="black")
    ax.arrow(-.625,0.5,0,0.25,head_width=0.05,head_length=0.05,fc='k',ec='k')
    ax.arrow(-.625,0.5,0,-0.25,head_width=0.05,head_length=0.05,fc='k',ec='k')
    ax.text(-.75,0.75,"Advantage",ha="center",va="center", rotation=90)
    ax.text(-.75,0.25,"Disadvantage",ha="center",va="center", rotation=90)
    ax.set_ylim(0,1)
    ax.set_xlim(-1,3.5)
    ax.text(1.1,0.225,"DeSearch-like\nsystems",ha="center",va="center",rotation=65)
    ax.text(2,0.25,"proposed \n solution",ha="center",va="center",rotation=65)
legend_elements=[
    Patch(facecolor="lightskyblue",edgecolor='black', label='Discovered through honest providers'),
    Patch(facecolor="crimson",edgecolor='black', label='Discovered through by malicious providers'),
]
# Add legend using the labels and handles from the last subplot
#fig.legend(handles=legend_elements, loc=(0.2, 0.8))
#fig.tight_layout()
filename=f"{FIGS_DIR}/{','.join(EXP_LIST)}-{str_specs}-mal-acqshare-cons-behav-prov-behav-werr_{step}-{BEGIN}-{END}-{str_vals}.pdf"
plt.savefig(filename, transparent=True, dpi=1000, bbox_inches='tight')
print(f"Saved {filename}")
#fig.suptitle('Production Quantity by Zone and Factory on both days', y=1.02, size=14)


# %%
