#!/usr/bin/env python
# coding: utf-8

# In[ ]:

from fig6_config_attacked_kX import *

# In[ ]:

count=0
# Attacked COoL
d={ 
    'CONSUMER_BEHAVIOUR': ['Malicious consumers', 'Malicious consumers', 'Malicious consumers', 'Malicious consumers','Malicious consumers', 'Malicious consumers', 'Malicious consumers', 'Malicious consumers',
                            'Honest consumers', 'Honest consumers', 'Honest consumers', 'Honest consumers', 'Honest consumers', 'Honest consumers', 'Honest consumers', 'Honest consumers'],
    'ATTACK_TYPE': ['rdm-k8 Timing attack', 'COoL-k8 Timing attack', 'rdm-k4 Timing attack', 'COoL-k4 Timing attack', 'rdm-k2 Timing attack', 'COoL-k2 Timing attack', 'rdm-k1 Timing attack', 'COoL-k1 Timing attack',
                    'rdm-k8 Timing attack', 'COoL-k8 Timing attack', 'rdm-k4 Timing attack', 'COoL-k4 Timing attack', 'rdm-k2 Timing attack', 'COoL-k2 Timing attack', 'rdm-k1 Timing attack', 'COoL-k1 Timing attack',],
    **{(i,"hon"):np.ones(2*nb_cond//8)*2 for i in range(1,9)},
    **{(i,"mal"):np.ones(2*nb_cond//8)*2 for i in range(1,9)},
    **{(i,"err"):np.ones(2*nb_cond//8)*2 for i in range(1,9)},
    }

for k, exp_specs in enumerate(EXP_SPECS):
    for i, exp_spec in enumerate(exp_specs):
        for j, conditions in enumerate(conditions_list[k][i]):
            str_desc="-".join([EXP_LIST[k],*exp_spec,*CONFIG_FILENAME_LIST[k].split("_")[1:],"nS="+str(NB_POISSON_SAMPLES),"rS="+str(ASSET_RATE),",".join([a+"="+b for a,b in conditions])]).replace("*","x")
            str_cond=",".join([a+"="+b for a,b in conditions])+"-"+",".join(exp_spec)
            readable_cond=str_cond.split("-")[0].replace("kErr=0.00001*100","LOoL").replace("kErr=0","rdm").replace("hW=","")+","+("noTEE" if str_cond.split("-")[1].split(",")[0]=="noTEE" else "TEE")
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

            d[(nbByz,"hon")][count//total_nb_byz]=mean_acq_behav["HON_PROV"][False]
            d[(nbByz,"hon")][count//total_nb_byz+len(d[(nbByz,"hon")])//2]=mean_acq_behav["HON_PROV"][True]
            d[(nbByz,"mal")][count//total_nb_byz]=mean_acq_behav["MAL_PROV"][False]
            d[(nbByz,"mal")][count//total_nb_byz+len(d[(nbByz,"mal")])//2]=mean_acq_behav["MAL_PROV"][True]
            d[(nbByz,"err")][count//total_nb_byz]=std_acq_behav[False]
            d[(nbByz,"err")][count//total_nb_byz+len(d[(nbByz,"err")])//2]=std_acq_behav[True]
            count+=1
            multiplier+=1

            legend_elements.append(Patch(facecolor="none",edgecolor='black', label=f'{readable_cond}'))
            print(f"Finished {readable_cond}")


# In[ ]:


df=pd.DataFrame(d)

df.set_index(['CONSUMER_BEHAVIOUR', 'ATTACK_TYPE'], inplace=True)
df=df[[df.columns[x//3+(x%3)*8] for x in range(24)]]
df.columns = pd.MultiIndex.from_product([["MalProv"+str(i) for i in range(1,9)],["hon","mal","err"]])

fig, ax = plt.subplots(figsize=(3, 4))

df.columns=df.columns.swaplevel(0,1)
#print(df)
#print(df.loc["Malicious consumers"]["hon"]+df.loc["Malicious consumers"]["mal"])
df2=df.loc["Malicious consumers"]["hon"]+df.loc["Malicious consumers"]["mal"]
dfErr=df.loc["Malicious consumers"]["err"]

linestyles=["solid","dotted","solid","dotted","solid","solid","dotted","dotted"]
colors=["navy","crimson","blue","orangered","dodgerblue","coral","skyblue","lightsalmon"]
for ls, cl, (idx, row), (_, err) in zip(linestyles,colors, df2.iterrows(), dfErr.iterrows()):
    print(row)
    ax.errorbar([f"$\\frac{i+1}{8}$" for i in range(0,8)], row, yerr=err, linestyle=ls, label=idx, color=cl, ecolor='black', capsize=3)

ax.vlines(4,0,1, color="black")
ax.text(4.1,0.95,"$p^{exodus}_{2}$", ha="left")

ax.set_ylim(0,1)
ax.set_yticks(np.arange(0,1.1,0.1))
ax.set_yticks(np.arange(0,1.01,0.02),minor=True)
ax.set_yticklabels([f"{i}\%" for i in np.arange(0,101,10)])
ax.grid(axis="y", which="major", alpha=1)
ax.grid(axis="y", which="minor", alpha=0.3)
ax.grid(axis="x", which="major", alpha=1)
ax.set_xlabel(f"Fraction of malicious providers $p_M$")
ax.set_ylabel(f"Share of dNBS-assets by malicious consumers")

fig.legend( bbox_to_anchor=(0.45, 0.025, 0.5, 0.5), labelspacing=0.25)
fig.tight_layout()
#filename=f"{FIGS_DIR}/{','.join(EXP_LIST)}-{str_specs}-acqshare-cons-behav-prov-behav-werr_{step}-{BEGIN}-{END}-{str_vals}.pdf"
filename=f"{FIGS_DIR}/{','.join(EXP_LIST)}-acqshare-cons-behav-prov-behav-plot-werr_{step}-{BEGIN}-{END}-{str_vals}.pdf"
plt.savefig(filename, transparent=True, dpi=1000, bbox_inches='tight')
print(f"Saved {filename}")
#fig.suptitle('Production Quantity by Zone and Factory on both days', y=1.02, size=14)

# %%

