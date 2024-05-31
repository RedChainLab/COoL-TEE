#!/usr/bin/env python
# coding: utf-8

# In[ ]:

from analysis.fig8_config_attacked_cool12 import *

# In[ ]:

count=0
# Attacked COoL
d={ 
    'CONSUMER_BEHAVIOUR': ['Malicious consumers', 'Malicious consumers', 'Malicious consumers', 'Malicious consumers', 'Malicious consumers',
                            'Honest consumers', 'Honest consumers', 'Honest consumers', 'Honest consumers', 'Honest consumers'],
    'ATTACK_TYPE': ['COoL Timing attack', 'COoL Content attack', 'COoL Cuckoo-T attack', 'COoL Cuckoo-C attack', 'COoL Fault-free',
                    'COoL Timing attack', 'COoL Content attack', 'COoL Cuckoo-T attack', 'COoL Cuckoo-C attack', 'COoL Fault-free'],
    **{(i,"hon"):np.ones(2*nb_cond//N_SP)*2 for i in range(1,N_SP+1)},
    **{(i,"mal"):np.ones(2*nb_cond//N_SP)*2 for i in range(1,N_SP+1)},
    **{(i,"err"):np.ones(2*nb_cond//N_SP)*2 for i in range(1,N_SP+1)},
    }

for k, exp_specs in enumerate(EXP_SPECS):
    for i, exp_spec in enumerate(exp_specs):
        for j, conditions in enumerate(conditions_list[k][i]):
            str_desc="-".join([EXP_LIST[k],*exp_spec,*CONFIG_FILENAME_LIST[k].split("_")[1:],"nS="+str(NB_POISSON_SAMPLES),"rS="+str(ASSET_RATE),",".join([a+"="+b for a,b in conditions])]).replace("*","x")
            str_cond=",".join([a+"="+b for a,b in conditions])+"-"+",".join(exp_spec)
            readable_cond=str_cond.split("-")[0].replace("kErr=0.00001*100","LOoL").replace("kErr=0","rdm").replace("hW=","")+","+("noTEE" if str_cond.split("-")[1].split(",")[0]=="noTEE" else "TEE")
            #nbByz=N_SP-int(float(re.compile("sHM=[0-9.]+").search(readable_cond).group(0).split("=")[1])*N_SP if "sHM=" in readable_cond else 0)
            nbByz=N_SP-int(float(re.compile("sHM=[0-9.]+").search(readable_cond).group(0).split("=")[1].split("I")[0]) if "sHM=" in readable_cond else 0)
            readable_cond=re.sub("sHM=[0-9.]+",str(nbByz)+"MalProv",readable_cond)
            full_dfs=pd.read_csv(f"{OUTPUT_DIR}/{str_desc}.csv", usecols=cols, index_col=idx_cols, low_memory=True)
            full_dfs["DIFF"]=full_dfs[END]-full_dfs[BEGIN]

            acq_behav=pd.DataFrame({True:[],False:[]})
            acq_behav.columns=pd.MultiIndex.from_tuples([("DIFF",True),("DIFF",False)])
            acq_behav=pd.concat([acq_behav,full_dfs.groupby(level=["CONSUMER_BEHAVIOUR","PROVIDER_BEHAVIOUR","REPETITION"]).agg({"DIFF":np.size}).unstack(1)]).fillna(0)
            acq_behav.set_index(pd.MultiIndex.from_tuples([(b,a) for a,b in acq_behav.index]), inplace=True)
            acq_behav.index.names=["REPETITION","CONSUMER_BEHAVIOUR"]
            acq_behav.columns=["HON_PROV","MAL_PROV"]

            width=0.9/nb_cond # /number of conditions, ie 6
            index=np.arange(2)
            offset=-width*(nb_cond/2-0.5)+width*multiplier

            rat_acq_behav=acq_behav["HON_PROV"]/(acq_behav["HON_PROV"]+acq_behav["MAL_PROV"])

            mean_acq_behav=rat_acq_behav.groupby(level="CONSUMER_BEHAVIOUR").mean()
            std_acq_behav=rat_acq_behav.groupby(level="CONSUMER_BEHAVIOUR").std()

            print(mean_acq_behav)
            print(std_acq_behav)
            
            plt.bar(index+offset,mean_acq_behav, width=width, color='tab:blue', label='Honest providers', edgecolor='black')
            plt.bar(index+offset,1-mean_acq_behav, bottom=mean_acq_behav, width=width, color='tab:red', label='Malicious providers', edgecolor='black')

            plt.errorbar(index+offset,mean_acq_behav, yerr=std_acq_behav, fmt='none', ecolor='black', capsize=3)

            d[(nbByz,"hon")][count//N_SP]=mean_acq_behav[False]
            d[(nbByz,"hon")][count//N_SP+len(d[(nbByz,"hon")])//2]=mean_acq_behav[True]
            d[(nbByz,"mal")][count//N_SP]=1-mean_acq_behav[False]
            d[(nbByz,"mal")][count//N_SP+len(d[(nbByz,"mal")])//2]=1-mean_acq_behav[True]
            d[(nbByz,"err")][count//N_SP]=std_acq_behav[False]
            d[(nbByz,"err")][count//N_SP+len(d[(nbByz,"err")])//2]=std_acq_behav[True]
            count+=1

            multiplier+=1

            legend_elements.append(Patch(facecolor="none",edgecolor='black', label=f'{readable_cond}'))
            print(f"Finished {readable_cond}")


# In[ ]:


df=pd.DataFrame(d)

df.set_index(['CONSUMER_BEHAVIOUR', 'ATTACK_TYPE'], inplace=True)
df=df[[df.columns[x//3+(x%3)*N_SP] for x in range(N_SP*3)]]
df.columns = pd.MultiIndex.from_product([["MalProv"+str(i) for i in range(1,N_SP+1)],["hon","mal","err"]])

df=df.rename(index={'COoL Cuckoo-C attack':'COoL $\\vert$ Cuckoo-Content (non-TEE)'})
df=df.rename(index={'COoL Cuckoo-T attack':'COoL $\\vert$ Cuckoo-Timing (TEE)'})
df=df.rename(index={'COoL Timing attack':'COoL $\\vert$ Timing (TEE)'})
df=df.rename(index={'COoL Content attack':'COoL $\\vert$ Content (non-TEE)'})
df=df.rename(index={'Fault-free':'COoL $\\vert$ Fault-free'})

# Create figure with a subplot for each factory zone with a relative width
# proportionate to the number of factories
zones = df.index.levels[0]
nplots = zones.size
plots_width_ratios = [df.xs(zone).index.size for zone in zones]
fig, axes = plt.subplots(nrows=1, ncols=nplots, sharey=True, figsize=(9, 3),
                         gridspec_kw = dict(width_ratios=plots_width_ratios, wspace=0))

# Loop through array of axes to create grouped bar chart for each factory zone
alpha = 0.3 # used for grid lines, bottom spine and separation lines between zones
firstCol = True # used to draw left spine only for first subplot

for zone, ax in zip(zones, axes):
    mal_cols=df.xs(zone).columns.levels[0]
    width=0.9/len(mal_cols)
    multiplier=0
    index=np.arange(df.xs(zone).index.size)
    for col in mal_cols:
        offset=-width*(len(mal_cols)/2-0.5) + width*multiplier

        ax.bar(index+offset, df.xs(zone)[col]['hon'], yerr=df.xs(zone)[col]['err'], width=width, color="lightskyblue", edgecolor="black", ecolor='black', capsize=3)
        bars=ax.bar(index+offset, df.xs(zone)[col]['mal'], bottom=df.xs(zone)[col]['hon'], width=width, color="crimson", edgecolor="black")
        multiplier+=1

    for spine in ['top','right']:
        ax.spines[spine].set_visible(False)
    #ax.spines['bottom'].set_alpha(alpha)

    # Set and place x labels for factory zones
    ax.set_xlabel(zone)
    ax.xaxis.set_label_coords(x=0.5, y=-0.475)
    
    # Format major tick labels for factory names: note that because this figure is
    # only about 10 inches wide, I choose to rewrite the long names on two lines.
    ticklabels = [name.replace(' ', '\n') if len(name) > 10 else name
                  for name in df.xs(zone).index]
    ax.set_xticks(np.arange(df.xs(zone).index.size))
    ax.set_xticklabels(ticklabels, rotation=0, ha='center')
    ax.tick_params(axis='x', length=0, pad=18)
    
    # Set and format minor tick marks for separation lines between zones: note
    # that except for the first subplot, only the right tick mark is drawn to avoid
    # duplicate overlapping lines so that when an alpha different from 1 is chosen
    # (like in this example) all the lines look the same
    if firstCol:
        ax.set_ylabel("Share of requests")
        ax.yaxis.set_label_coords(x=-.1, y=0.5)
        #ax.set_xticks([*ax.get_xlim()], minor=True)
        ax.set_yticklabels([f"{x}\%" for x in range(0,101,10)])
        firstCol = False
    #else:
        ax.set_xticks([ax.get_xlim()[1]], minor=True)
    ax.tick_params(axis="x", which='minor', length=55, width=0.8, color=[0, 0, 0, alpha])

    ax.set_yticks(np.arange(0,1.01,0.1))
    ax.set_yticks(np.arange(0,1.01,0.02),minor=True)
    ax.grid(axis="y", which="major", alpha=1)
    ax.grid(axis="y", which="minor", alpha=0.3)

legend_elements=[
    Patch(facecolor="lightskyblue",edgecolor='black', label='Sent to honest providers'),
    Patch(facecolor="crimson",edgecolor='black', label='Sent to malicious providers'),
]

pM_xlabel_height=-0.15
nb_exps=6
for j in range(0,nb_exps):
    for i in range(0,N_SP):
        ax.text(-0.4+i*0.114+j, pM_xlabel_height, f"$\\frac{i+1}{8}$", ha="center", va="bottom")
        ax.text(-(0.9+nb_exps)+i*0.114+j, pM_xlabel_height, f"$\\frac{i+1}{8}$", ha="center", va="bottom")
ax.text(-(nb_exps+1.1), pM_xlabel_height, "$p_M$:", ha="right", va="bottom")
ax.text(-(nb_exps+1.1), pM_xlabel_height-0.1, "Prov. selection:", ha="right", va="bottom")
ax.text(-(nb_exps+1.1), pM_xlabel_height-0.2, "Attack type:", ha="right", va="bottom")
ax.text(-(nb_exps+1.1), pM_xlabel_height-0.4, "Sent by:", ha="right", va="bottom")

# Add legend using the labels and handles from the last subplot
fig.legend(handles=legend_elements, loc=(0.165, 0.3575))
fig.tight_layout()
#filename=f"{FIGS_DIR}/{','.join(EXP_LIST)}-{str_specs}-sentrq-cons-behav-prov-behav-share-werr_{step}-{str_vals}.pdf"
filename=f"{FIGS_DIR}/{','.join(EXP_LIST)}-sentrq-cons-behav-prov-behav-share-werr_{step}-{str_vals}.pdf"
plt.savefig(filename, transparent=True, dpi=1000, bbox_inches='tight')
print(f"Saved {filename}")
#fig.suptitle('Production Quantity by Zone and Factory on both days', y=1.02, size=14)


# %%
