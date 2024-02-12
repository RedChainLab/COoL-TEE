import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from alive_progress import alive_bar
import math

pd.options.mode.chained_assignment = None  # default='warn'

import poisson

import request_lifetimes as rl
import run2param_mapper as r2p

from matplotlib.patches import Patch
from matplotlib.lines import Line2D
from matplotlib.ticker import FuncFormatter
import pickle as pkl
from scipy.stats import mannwhitneyu

import platform
import sys
import os

def is_def(x:str):
    return x in vars() or x in globals()

def print_somewhere(msg):
    if is_def("bar"):
        bar.text(msg)
    else:
        print(msg)

def find_closest_asset(t_serv, assets):
    a = None
    left=0
    right=len(assets)-1
    # closest asset smaller than t_serv
    while left<=right:
        mid=(left+right)//2
        if assets[mid]<t_serv:
            a=assets[mid]
            left=mid+1
        else:
            right=mid-1
    return a

def find_closest_asset2(dfs, assets):
    #serv_times and assets are already sorted
    #merge sort of both lists
    serv_times=dfs["SERVE"].values
    merged_list=[]
    i=0
    j=0
    while i<len(serv_times) and j<len(assets):
        if serv_times[i]<assets[j]:
            merged_list.append((True,serv_times[i]))
            i+=1
        else:
            merged_list.append((False,assets[j]))
            j+=1
    while i<len(serv_times):
        merged_list.append((True,serv_times[i]))
        i+=1
    while j<len(assets):
        merged_list.append((False,assets[j]))
        j+=1
    first_asset_list=[]
    current_asset=None
    for i in range(len(merged_list)):
        if merged_list[i][0]==False:
            current_asset=merged_list[i][1]
        else:
            first_asset_list.append(current_asset)
    assert(len(first_asset_list)==len(dfs))
    dfs["ASSET"]=first_asset_list

def find_closest_asset3(dfs, assets):
    rq_times=dfs.sort_values(["RECV"], ascending=True)
    #rq_times["ASSET"]=np.array([],len(rq_times))
    remaining_assets=assets
    rq_times['ASSET'] = pd.Series([[]] * len(rq_times), index=rq_times.index)
    for id, serv in rq_times["SERVE"].items():
        #print(id, serv, recv)
        #dichotomic search in assets for index i such that assets[i]<serv and assets[i+1]>=serv
        left=0
        right=len(remaining_assets)-1
        while left<=right:
            mid=(left+right)//2
            if remaining_assets[mid]<serv:
                left=mid+1
            else:
                right=mid-1
        #put the sublist remaining_assets[:i] in rq_times["ASSET"]
        #print("ID",id,"ASSET",remaining_assets[:left],"RQ_time",rq_times.index)
        rq_times.at[id,"ASSET"]=remaining_assets[:left]
        remaining_assets=remaining_assets[left:]
    dfs["ASSET"]=rq_times["ASSET"]
    dfs["FASTEST"]=dfs["ASSET"].apply(len)
    #print(dfs[["RECV","ASSET","FASTEST"]].values)

def find_closest_asset4(dfs, assets):
    rq_times=dfs.where(~((dfs["CONSUMER_BEHAVIOUR"]==True) & (dfs["PROVIDER_BEHAVIOUR"]==False))).sort_values(["RECV"], ascending=True)
    remaining_assets=assets
    rq_times['ASSET'] = pd.Series([[]] * len(rq_times), index=rq_times.index)
    for id, serv in rq_times["SERVE"].items():
        if np.isnan(serv):
            break
        #print(id, serv, recv)
        #dichotomic search in assets for index i such that assets[i]<serv and assets[i+1]>=serv
        left=0
        right=len(remaining_assets)-1
        while left<=right:
            mid=(left+right)//2
            if remaining_assets[mid]<serv:
                left=mid+1
            else:
                right=mid-1
        #put the sublist remaining_assets[:i] in rq_times["ASSET"]
        rq_times.at[id,"ASSET"]=remaining_assets[:left]
        remaining_assets=remaining_assets[left:]
        #print_somewhere(f'Remaining assets to assign: {len(remaining_assets)}')
    dfs["ASSET"]=rq_times["ASSET"]
    dfs["FASTEST"]=dfs["ASSET"].apply(len)
    #print(dfs.where((dfs["CONSUMER_BEHAVIOUR"]==True) & (dfs["PROVIDER_BEHAVIOUR"]==False)))
    #assert(dfs.where((dfs["CONSUMER_BEHAVIOUR"]==True) & (dfs["PROVIDER_BEHAVIOUR"]==False)).agg({"FASTEST":np.sum})["FASTEST"].sum()==0)

def dicho_search(l, val):
    left=0
    right=len(l)-1
    while left<=right:
        mid=(left+right)//2
        if l[mid]<val:
            left=mid+1
        else:
            right=mid-1
    return left

def generate_dfs(file_list, s, behaviours, time_offset=0, exp_specs=[], legacy_data_gen=False):
    if legacy_data_gen:
        dfs=rl.gen_dfs_legacy(file_list)
    else:
        dfs=rl.gen_dfs(file_list)
    dfs=dfs.where(dfs["SEND"]>time_offset).dropna()

    dfs.sort_index(inplace=True)

    dfs["CONSUMER_BEHAVIOUR"]=dfs.index.get_level_values(level=0).isin(dfs.index.get_level_values(level=0).unique()[:behaviours["HON_CONS"]])
    dfs["PROVIDER_BEHAVIOUR"]=dfs.index.get_level_values(level=1).isin(dfs.index.get_level_values(level=1).unique()[:behaviours["HON_PROV"]])

    #print(dfs)

    s=s[dicho_search(s,time_offset):]
    if "WtA" in exp_specs:
        if "noTEE" in exp_specs:
            find_closest_asset4(dfs, s)
        else:
            find_closest_asset3(dfs, s)
    elif "rank" in exp_specs:
        find_race_rankings(dfs, s)
    else:
        find_closest_asset2(dfs, s)
    return dfs

def plot_acq_cons_abs(dfs, str_desc):
    fig1,ax=plt.subplots(1,1)
    count_fast_req=dfs.groupby(level="CONSUMER_ID").agg({"FASTEST":np.sum})
    cons_behaviours=[dfs[dfs.index.get_level_values("CONSUMER_ID")==cons].index.get_level_values("CONSUMER_BEHAVIOUR").unique()[0] for cons in dfs.index.get_level_values("CONSUMER_ID").unique()]
    colours=["tab:blue" if behav else "tab:red" for behav in cons_behaviours]
    ax.bar(count_fast_req.index, count_fast_req["FASTEST"], color=colours)

    mean_hon=dfs.groupby("CONSUMER_BEHAVIOUR").agg({"FASTEST":np.sum})["FASTEST"][True]/dfs[dfs.index.get_level_values("CONSUMER_BEHAVIOUR")==True].dropna().index.get_level_values("CONSUMER_ID").nunique()
    mean_mal=dfs.groupby("CONSUMER_BEHAVIOUR").agg({"FASTEST":np.sum})["FASTEST"][False]/dfs[dfs.index.get_level_values("CONSUMER_BEHAVIOUR")==False].dropna().index.get_level_values("CONSUMER_ID").nunique()

    print(mean_hon, mean_mal)
    ax.axhline(y=mean_hon, xmin=0, xmax=1, linewidth=2, color="darkblue", linestyle="--", label="Mean honest consumers")
    ax.axhline(y=mean_mal, xmin=0, xmax=1, linewidth=2, color="darkred", linestyle="--", label="Mean malicious consumers")

    ax.set_xlabel("Consumer ID")
    ax.set_ylabel("Number of acquired assets")
    ax.set_title("Number of acquired assets per consumer")
    ax.set_xticks(count_fast_req.index)
    ax.set_xticklabels(count_fast_req.index)
    legend_elements = [
        Patch(facecolor='tab:red',label='Malicious consumer'),
        Line2D([0], [0], color='darkred', lw=2, label=f'Malicious mean: {mean_mal:.1f}'),
        Patch(facecolor='tab:blue',label='Honest consumer'),
        Line2D([0], [0], color='darkblue', lw=2, label=f'Honest mean: {mean_hon:.1f}'),
        Patch(facecolor='white'),
        Patch(facecolor='white', label=f'Malicious gain: {"+" if mean_mal>mean_hon else ""}{100*(mean_mal-mean_hon)/mean_hon:.1f}%'),
                    ]
    # Shrink current axis by 20%
    box = ax.get_position()
    ax.set_position([box.x0, box.y0, box.width * 0.8, box.height])

    # Put a legend to the right of the current axis
    ax.legend(loc='center left', bbox_to_anchor=(1, 0.5),handles=legend_elements)
    ax.grid(axis='y',alpha=0.5)

    fig1.set_figheight(7)
    fig1.set_figwidth(18)
    fig1.savefig(f"figs/acq-cons-abs-{str_desc}.png", transparent=True)
    print(f"Saved figs/acq-cons-abs-{str_desc}.png")

def plot_acq_cons_prov_abs(dfs, str_desc):
    fig1,ax=plt.subplots(1,1)

    users=dfs.index.get_level_values("CONSUMER_ID").unique()
    providers=dfs.index.get_level_values("PROVIDER_ID").unique()

    behaviours={"HON_CONS":dfs[dfs.index.get_level_values("CONSUMER_BEHAVIOUR")==True].index.get_level_values("CONSUMER_ID").nunique(),
                "MAL_CONS":dfs[dfs.index.get_level_values("CONSUMER_BEHAVIOUR")==False].index.get_level_values("CONSUMER_ID").nunique(),
                "HON_PROV":dfs[dfs.index.get_level_values("PROVIDER_BEHAVIOUR")==True].index.get_level_values("PROVIDER_ID").nunique(),
                "MAL_PROV":dfs[dfs.index.get_level_values("PROVIDER_BEHAVIOUR")==False].index.get_level_values("PROVIDER_ID").nunique()}

    width=0.9/len(users)
    multiplier=0
    dual_group=dfs.groupby(level=["CONSUMER_BEHAVIOUR","CONSUMER_ID","PROVIDER_ID"]).agg({"FASTEST":np.sum})
    index=np.array([int(x) for x in dual_group.index.get_level_values(level="PROVIDER_ID").unique()])
    for user in users:
        offset=-width*(len(users)/2-1.5)+width*multiplier
        values=dual_group.loc[(slice(None),user),:]["FASTEST"].unstack(level=0)
        values=[b for [b] in values.values]

        colours=['tab:blue' if user in users[:behaviours["HON_CONS"]] else 'tab:red' for _ in range(behaviours["HON_PROV"])]+['tab:orange' if user in users[:behaviours["MAL_CONS"]] else 'tab:green' for _ in range(behaviours["MAL_PROV"])]

        ax.bar(index+offset,values, width, color=colours, label=f'Consumer {user}')
        multiplier+=1
    ax.set_xlabel('Provider ID')
    ax.set_ylabel('Number of acquired assets')
    #ax.set_yscale('log')
    ax.set_xticks(index + width, index)
    legend_elements = [
        Patch(facecolor='tab:blue',label='honest consumer & provider'),
        Line2D([0], [0], color='darkblue', lw=2, label=f'honest consumer & provider means'),
        Patch(facecolor='tab:red',label='malicious consumer & honest provider'),
        Line2D([0], [0], color='darkred', lw=2, label=f'malicious consumer & honest provider means'),
        Patch(facecolor='tab:orange',label='honest consumer & malicious provider'),
        Line2D([0], [0], color='brown', lw=2, label=f'honest consumer & malicious means'),
        Patch(facecolor='tab:green',label='malicious consumer & provider'),
        Line2D([0], [0], color='darkgreen', lw=2, label=f'malicious consumer & provider means'),
                    ]
    # Shrink current axis by 20%
    box = ax.get_position()
    ax.set_position([box.x0, box.y0, box.width * 0.8, box.height])

    # Put a legend to the right of the current axis
    ax.legend(loc='center left', bbox_to_anchor=(1, 0.5),handles=legend_elements)

    mean_cons_prov=2*dual_group.groupby(["PROVIDER_ID","CONSUMER_BEHAVIOUR"]).agg({"FASTEST":np.sum}).unstack(1)["FASTEST"]/len(users)
    mean_cons_prov=mean_cons_prov.astype(float)
    mean_cons_prov_err=2*dual_group.groupby(["PROVIDER_ID","CONSUMER_BEHAVIOUR"]).agg({"FASTEST":np.std}).unstack(1)["FASTEST"]/len(users)
    mean_cons_prov_err=mean_cons_prov_err.astype(float)

    width=0.9/len(users)
    length=len(users)/2
    for hon in [True, False]:
        multiplier=0
        for idx in providers:
            offset=-width*length+width
            colour='darkblue' if hon and idx<behaviours["HON_PROV"] \
                else 'darkred' if not hon and idx<behaviours["HON_PROV"] \
                else 'brown' if hon and idx>=behaviours["HON_PROV"] \
                else 'darkgreen'
            y=mean_cons_prov[hon].loc[int(idx)]
            xmin=offset-0.05*(1-hon)+0.5*(1-hon+2*multiplier)
            xmax=offset-0.05*(1-hon)+0.9/2+(1-hon+2*multiplier)/2
            xmid=(xmin+xmax)/2
            ax.hlines(y=y, xmin=xmin,xmax=xmax, linewidth=2, color=colour)
            multiplier+=1
            err=mean_cons_prov_err[hon].loc[int(idx)]
            ax.errorbar(xmid, y, err, linestyle='None', capsize=5, color='black')

    ax.grid(axis='y',alpha=0.5)
    #ax.yaxis.set_major_formatter(FuncFormatter('{0:.1%}'.format))

    ax.set_title("Acquired assets per consumer per provider wrt. their resp. behaviour")
    fig1.savefig(f"figs/acq-cons-prov-abs-{str_desc}.png", transparent=True)
    print(f"Saved figs/acq-cons-prov-abs-{str_desc}.png")

def plot_acq_cons_behav_prov_behav_abs(dfs, str_desc):
    fig1,ax=plt.subplots(1,1)
    acq_behav=pd.DataFrame({True:[],False:[]})
    acq_behav.columns=pd.MultiIndex.from_tuples([("FASTEST",True),("FASTEST",False)])
    acq_behav=pd.concat([acq_behav,dfs.groupby(level=["CONSUMER_BEHAVIOUR","PROVIDER_BEHAVIOUR"]).agg({"FASTEST":np.sum}).unstack(1)]).fillna(0)
    print(acq_behav)
    behav=["Honest" if behav else "Malicious" for behav in acq_behav.index]
    ax.bar(behav, acq_behav["FASTEST"][True], color='tab:blue', label='honest consumer')
    ax.bar(behav, acq_behav["FASTEST"][False], bottom=acq_behav["FASTEST"][True], color='tab:red', label='malicious consumer')
    ax.set_xlabel('Consumer behaviour')
    ax.set_xticks(behav)
    ax.set_ylabel('Number of acquired assets')
    #ax.set_yscale('log')
    legend_elements = [
        Patch(facecolor='tab:blue',label='honest provider'),
        Patch(facecolor='tab:red',label='malicious provider'),
                    ]
    # Shrink current axis by 20%
    box = ax.get_position()
    ax.set_position([box.x0, box.y0, box.width * 0.8, box.height])

    # Put a legend to the right of the current axis
    ax.legend(loc='center left', bbox_to_anchor=(1, 0.5),handles=legend_elements)

    ax.grid(axis='y',alpha=0.5)
    #ax.yaxis.set_major_formatter(FuncFormatter('{0:.1%}'.format))

    ax.set_title("Acquired assets wrt. consumer and provider behaviours")
    filename=f"figs/acq-cons-behav-prov-behav-abs-{str_desc}.png"
    fig1.savefig(filename, transparent=True)
    print(f"Saved {filename}")

def plot_merit_acq(dfs, str_desc):
    fig,ax=plt.subplots(1,1)

    #print(dfs)
    dfs["DIFF"]=dfs["RECV"]-dfs["SEND"]
    #print(dfs[["DIFF","FASTEST"]])
    groups=dfs.groupby("CONSUMER_ID").groups

    for group in groups:
        n,bins,_=ax.hist(dfs.loc[groups[group]]["DIFF"], weights=dfs.loc[groups[group]]["FASTEST"], bins=[x/1000 for x in range(0,int(1000*dfs["DIFF"].max()+1),1)], label=f"Consumer {group}")
    ax.set_xlabel("Time to acquire asset (s)")
    ax.set_ylabel("Number of acquired assets")
    ax.set_title("Number of acquired assets per acquisition time")
    ax.grid(axis='y',alpha=0.5)
    ax.set_yscale('log')
    fig.set_figheight(7)
    fig.set_figwidth(18)
    fig.savefig(f"figs/acq-merit-cons-{str_desc}.png", transparent=True)
    print(f"Saved figs/acq-merit-cons-{str_desc}.png")

def plot_merit_acq_mem_opt(grp:pd.DataFrame, bins, ax):
    ax.hist(grp["DIFF"], weights=grp["FASTEST"], bins=bins, label=f"Consumer {grp.index.get_level_values('CONSUMER_ID').unique()[0]}")

def plot_data(dfs, str_desc):
    plot_acq_cons_abs(dfs, str_desc)
    plot_acq_cons_prov_abs(dfs, str_desc)
    plot_acq_cons_behav_prov_behav_abs(dfs, str_desc)
    #plot_merit_acq(dfs, str_desc)

def countBehaviourAssets(dfs, agg_funct, behaviour):
    return dfs.groupby(level=["CONSUMER_BEHAVIOUR","CONSUMER_ID"]).agg({"FASTEST":np.sum}).groupby(level="CONSUMER_BEHAVIOUR").agg({"FASTEST":agg_funct})["FASTEST"][behaviour]

def behaviourAssetSum(dfs, behaviour):
    return countBehaviourAssets(dfs, np.sum, behaviour)

def behaviourAssetMax(dfs, behaviour):
    return countBehaviourAssets(dfs, np.max, behaviour)

def scoreRuns(df, cond_func):
    return sorted([(grp,cond_func(df.loc[grp])) for grp in df.groupby(level=["MARKET_REPETITION","REPETITION"]).groups.keys()], key=lambda x: x[1], reverse=True)

def selectTopNRuns(df, N, cond_func):
    return scoreRuns(df, cond_func)[:N]

def giniScore(dfs):
    sorted_dfs=dfs.sort_values("FASTEST", ascending=True)
    n=len(sorted_dfs)
    total_assets=sorted_dfs["FASTEST"].sum()
    gini=2*sum([(i+1)*sorted_dfs.iloc[i]["FASTEST"] for i in range(n)])/(n*total_assets)-(n+1)/n
    print(gini)
    return gini

def getDnetRanking(dfs):
    dfs["DIFF"]=(dfs["RECV"]-dfs["SERVE"])
    #Compute median and size on DIFF column in aggregated dataframe
    cons_prov_dnet=dfs.groupby(level=["CONSUMER_ID","PROVIDER_ID"]).agg({"DIFF":[np.median,np.size]}).sort_values(("DIFF","median"), ascending=True)
    dfs.drop(["DIFF"], axis=1, inplace=True)
    cons_prov_dnet.columns=["dnet","nReqs"]
    #print([(cons_prov_dnet.index[i], cons_prov_dnet.index[i-1], cons_prov_dnet['dnet'].iloc[i], cons_prov_dnet['dnet'].iloc[i-1]) for i in range(1,len(cons_prov_dnet)) if cons_prov_dnet['dnet'].iloc[i]==cons_prov_dnet['dnet'].iloc[i-1]])
    ranking=cons_prov_dnet.groupby(["dnet"]).agg({"nReqs":np.sum})

    ranking["rank"]=0
    ranking["rank"].iloc[0]=(ranking["nReqs"].iloc[0]+1)/2
    for i in range(1,len(ranking)):
        ranking["rank"].iloc[i]=ranking.iloc[i-1]["rank"]+(1+ranking.iloc[i]["nReqs"])/2
    ranking=ranking.reset_index()[["dnet","rank"]]
    cons_prov_dnet=cons_prov_dnet.join(ranking.set_index("dnet"), on="dnet", how="left")
    cons_prov_dnet["score"]=cons_prov_dnet["nReqs"]*cons_prov_dnet["rank"]
    cons_prov_dnet=cons_prov_dnet.groupby(level=["CONSUMER_ID"]).agg({"score":np.sum, "nReqs":np.sum})
    cons_prov_dnet["score"]=cons_prov_dnet["score"]/cons_prov_dnet["nReqs"]
    cons_prov_dnet.sort_values("score", ascending=True, inplace=True)
    #print(cons_prov_dnet)
    return cons_prov_dnet

def getAssetRanking(dfs):
    cons_asset=dfs.groupby(level=["CONSUMER_ID"]).agg({"FASTEST":np.sum}).sort_values("FASTEST", ascending=False)
    ranking=cons_asset.groupby(["FASTEST"]).agg({"FASTEST":np.size})
    ranking.columns=["count_fastest"]
    ranking.sort_values("FASTEST", ascending=False, inplace=True)
    ranking["rank"]=0
    ranking["rank"].iloc[0]=(ranking["count_fastest"].iloc[0]+1)/2
    for i in range(1,len(ranking)):
        ranking["rank"].iloc[i]=ranking.iloc[i-1]["rank"]+(1+ranking.iloc[i]["count_fastest"])/2

    ranking=ranking.reset_index()[["FASTEST","rank"]]
    cons_asset=cons_asset.join(ranking.set_index("FASTEST"), on="FASTEST", how="left")
    cons_asset.sort_values("rank", ascending=True, inplace=True)
    #print(cons_asset)
    return cons_asset

def getSuccessRanking(dfs, begin="SERVE", end="RECV"):
    dfs["DIFF"]=(dfs[end]-dfs[begin])
    #successExp=getSuccessExpectancy(dfs)

    consumerLats=dfs["DIFF"].groupby(level=["CONSUMER_ID"])

    rules=[]
    for consumer1 in consumerLats.groups:
        for consumer2 in consumerLats.groups:
            if consumer1!=consumer2:
                _, pv1sup2 = mannwhitneyu(consumerLats.get_group(consumer1), consumerLats.get_group(consumer2), alternative="greater")
                if pv1sup2<0.05:
                    rules.append((consumer1,consumer2))
    countOutrankings={consumer:0 for consumer in consumerLats.groups}
    for rule in rules:
        countOutrankings[rule[0]]+=1
    ranking=pd.DataFrame.from_dict(countOutrankings, orient="index", columns=["outrank_count"])
    ranking.index.name="CONSUMER_ID"
    #print(ranking)
    return ranking.sort_values("outrank_count", ascending=True)

def getExpectancyPerWT(full_dfs, begin="SERVE", end="RECV"):
    full_dfs["DIFF"]=full_dfs[end]-full_dfs[begin]
    group_per_Na=full_dfs.groupby("FASTEST")
    print(full_dfs["DIFF"].max())
    bins=np.arange(0,int(1000*(full_dfs["DIFF"].max()))+2)/1000
    p_Na=pd.DataFrame(index=bins[:-1])
    for grp in list(group_per_Na.groups.keys()):
        n,_=np.histogram(group_per_Na.get_group(grp)["DIFF"], bins=bins)
        p_Na[grp]=n

    pTotal=p_Na.agg("sum", axis="columns")
    esp_na=0*p_Na[0]
    for na in p_Na.columns[1:]:
        esp_na+=na*p_Na[na]
    esp_na=esp_na/pTotal
    return esp_na

def getExpectRanking(dfs, expectancies, begin="SERVE", end="RECV"):
    dfs["DIFF"]=(dfs[end]-dfs[begin])
    dfs["expect_acq"]=dfs["DIFF"].apply(lambda x: expectancies[math.trunc(x*1000)/1000])
    #print(dfs["expect_acq"])
    ranking=dfs.groupby(level=["CONSUMER_ID"]).agg({"expect_acq":np.sum}).sort_values("expect_acq", ascending=False)
    #print(ranking)
    return ranking

def meritScoreV1(dfs):
    asset_ranking=getAssetRanking(dfs)
    dnet_ranking=getDnetRanking(dfs)
    cons_ranking=dnet_ranking.merge(asset_ranking, on="CONSUMER_ID", how="left")
    print(cons_ranking)

    truncPos=np.vectorize(lambda x: x if x>0 else float(0))
    diff=np.vectorize(lambda x,y: x-y)

    ar_a_sort=np.array(asset_ranking["rank"], dtype=np.float64)
    #print("ar_a_sort\t",ar_a_sort)
    ar_rq_sort=np.array(cons_ranking["rank"], dtype=np.float64)
    #print("ar_rq_sort\t",ar_rq_sort)
    ar_rev_sort=np.array(ar_a_sort[::-1], dtype=np.float64)
    #print("ar_rev_sort\t",ar_rev_sort)
    ar_diff=diff(ar_a_sort,ar_rq_sort)
    ar_diff=truncPos(ar_diff)
    #print("ar_diff\t",sum(ar_diff), ar_diff)
    ar_max_diff=diff(ar_a_sort,ar_rev_sort)
    ar_max_diff=truncPos(ar_max_diff)
    #print("ar_max_diff\t",sum(ar_max_diff), ar_max_diff)
    #print(ar_diff.sum()/ar_max_diff.sum())
    return 1-(ar_diff.sum()/ar_max_diff.sum())

def meritScore(dfs, begin="SERVE", end="RECV"):
    success_ranking=getSuccessRanking(dfs, begin=begin, end=end)
    asset_ranking=getAssetRanking(dfs)
    cons_ranking=success_ranking.merge(asset_ranking, on="CONSUMER_ID", how="left").sort_values(by=["outrank_count","rank"], ascending=[True,True])
    #print("a",cons_ranking)

    truncPos=np.vectorize(lambda x: x if x>0 else float(0))
    diff=np.vectorize(lambda x,y: x-y)

    ar_a_sort=np.array(asset_ranking["rank"], dtype=np.float64)
    #print("ar_a_sort\t",ar_a_sort)
    ar_rq_sort=np.array(cons_ranking["rank"], dtype=np.float64)
    #print("ar_rq_sort\t",ar_rq_sort)
    ar_rev_sort=np.array(ar_a_sort[::-1], dtype=np.float64)
    #print("ar_rev_sort\t",ar_rev_sort)
    ar_diff=diff(ar_a_sort,ar_rq_sort)
    ar_diff=truncPos(ar_diff)
    #print("ar_diff\t",sum(ar_diff), ar_diff)
    ar_max_diff=diff(ar_a_sort,ar_rev_sort)
    ar_max_diff=truncPos(ar_max_diff)
    #print("ar_max_diff\t",sum(ar_max_diff), ar_max_diff)
    print(1-(ar_diff.sum()/ar_max_diff.sum()))
    return 1-(ar_diff.sum()/ar_max_diff.sum())

def expectScore(dfs,expectancies, begin="SERVE", end="RECV"):
    expect_ranking=getExpectRanking(dfs,expectancies,begin=begin,end=end)
    asset_ranking=getAssetRanking(dfs)
    cons_ranking=expect_ranking.merge(asset_ranking, on="CONSUMER_ID", how="left").sort_values(by=["expect_acq","rank"], ascending=[False,True])
    #print("a",cons_ranking)

    truncPos=np.vectorize(lambda x: x if x>0 else float(0))
    diff=np.vectorize(lambda x,y: x-y)

    ar_a_sort=np.array(asset_ranking["rank"], dtype=np.float64)
    #print("ar_a_sort\t",ar_a_sort)
    ar_rq_sort=np.array(cons_ranking["rank"], dtype=np.float64)
    #print("ar_rq_sort\t",ar_rq_sort)
    ar_rev_sort=np.array(ar_a_sort[::-1], dtype=np.float64)
    #print("ar_rev_sort\t",ar_rev_sort)
    ar_diff=diff(ar_a_sort,ar_rq_sort)
    ar_diff=truncPos(ar_diff)
    #print("ar_diff\t",sum(ar_diff), ar_diff)
    ar_max_diff=diff(ar_a_sort,ar_rev_sort)
    ar_max_diff=truncPos(ar_max_diff)
    #print("ar_max_diff\t",sum(ar_max_diff), ar_max_diff)
    print(1-(ar_diff.sum()/ar_max_diff.sum()))
    return 1-(ar_diff.sum()/ar_max_diff.sum())

if __name__ == "__main__":
    TIME_INTERVAL=2400
    ASSET_RATE=100
    MARKET_PERIOD=12

    START=rl.Event.SEND
    END=rl.Event.RECV

    HORIZONTAL=True
    RELATIVE=True

    NB_POISSON_SAMPLES=1

    RQ_type=rl.RQ_types.All

    COLUMN=("DIFF" if RELATIVE else END.name)

    TIME_OFFSET=2250

    ###
    # <HIGH-LEVEL CONFIG>
    ###
    INPUT_DIR="/mnt/e/wt"
    OUTPUT_DIR="/mnt/e/data"

    ###
    # </HIGH-LEVEL CONFIG>
    ###

    ###
    # <EXPERIMENT CONFIG>
    ###

    EXP_DIR="wait500ByzRho100"#"wait100noLatByzRho100"
    
    CONFIG_FILENAME="configs_8SP_wait500ByzRho100"#"configs_8SP_wait100noLatByzRho100"

    EXP_SPECS=[
            #"noTEE",
            "WtA",#Winner takes all (assets)
            #"sameDC",
            f"+{TIME_OFFSET}s"
        ]

    BEHAVIOURS=[
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":7,"MAL_PROV":1},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":7,"MAL_PROV":1},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":7,"MAL_PROV":1},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":7,"MAL_PROV":1},

        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":6,"MAL_PROV":2},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":6,"MAL_PROV":2},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":6,"MAL_PROV":2},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":6,"MAL_PROV":2},

        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":5,"MAL_PROV":3},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":5,"MAL_PROV":3},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":5,"MAL_PROV":3},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":5,"MAL_PROV":3},

        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":4,"MAL_PROV":4},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":4,"MAL_PROV":4},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":4,"MAL_PROV":4},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":4,"MAL_PROV":4},

        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":3,"MAL_PROV":5},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":3,"MAL_PROV":5},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":3,"MAL_PROV":5},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":3,"MAL_PROV":5},

        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":2,"MAL_PROV":6},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":2,"MAL_PROV":6},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":2,"MAL_PROV":6},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":2,"MAL_PROV":6},

        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":1,"MAL_PROV":7},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":1,"MAL_PROV":7},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":1,"MAL_PROV":7},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":1,"MAL_PROV":7},

        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":0,"MAL_PROV":8},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":0,"MAL_PROV":8},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":0,"MAL_PROV":8},
        {"HON_CONS":50,"MAL_CONS":50,"HON_PROV":0,"MAL_PROV":8},
        ]

    conditions_list=[
                      [("hW","0ms"),("kErr","0"),("sHM","0.875"),("rho","25")],
                      [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.875"),("rho","25")],
                      [("hW","50ms"),("kErr","0"),("sHM","0.875"),("rho","25")],
                      [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.875"),("rho","25")],

                      [("hW","0ms"),("kErr","0"),("sHM","0.75"),("rho","25")],
                      [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.75"),("rho","25")],
                      [("hW","50ms"),("kErr","0"),("sHM","0.75"),("rho","25")],
                      [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.75"),("rho","25")],

                      [("hW","0ms"),("kErr","0"),("sHM","0.625"),("rho","25")],
                      [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.625"),("rho","25")],
                      [("hW","50ms"),("kErr","0"),("sHM","0.625"),("rho","25")],
                      [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.625"),("rho","25")],

                      [("hW","0ms"),("kErr","0"),("sHM","0.5"),("rho","25")],
                      [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","25")],
                      [("hW","50ms"),("kErr","0"),("sHM","0.5"),("rho","25")],
                      [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.5"),("rho","25")],

                      [("hW","0ms"),("kErr","0"),("sHM","0.375"),("rho","25")],
                      [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.375"),("rho","25")],
                      [("hW","50ms"),("kErr","0"),("sHM","0.375"),("rho","25")],
                      [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.375"),("rho","25")],

                      [("hW","0ms"),("kErr","0"),("sHM","0.25"),("rho","25")],
                      [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.25"),("rho","25")],
                      [("hW","50ms"),("kErr","0"),("sHM","0.25"),("rho","25")],
                      [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.25"),("rho","25")],

                      [("hW","0ms"),("kErr","0"),("sHM","0.125"),("rho","25")],
                      [("hW","0ms"),("kErr","0.00001*100"),("sHM","0.125"),("rho","25")],
                      [("hW","50ms"),("kErr","0"),("sHM","0.125"),("rho","25")],
                      [("hW","50ms"),("kErr","0.00001*100"),("sHM","0.125"),("rho","25")],

                      [("hW","0ms"),("kErr","0"),("sHM","0"),("rho","25")],
                      [("hW","0ms"),("kErr","0.00001*100"),("sHM","0"),("rho","25")],
                      [("hW","50ms"),("kErr","0"),("sHM","0"),("rho","25")],
                      [("hW","50ms"),("kErr","0.00001*100"),("sHM","0"),("rho","25")],
                     ]
    ###
    # </EXPERIMENT CONFIG>
    ###

    RECOMPUTE=[0,1]
    LEGACY_DATA_GEN=False
    
    if RQ_type==rl.RQ_types.First:
        run_list=rl.get_runs_in_dir(f"{INPUT_DIR}/first/{EXP_DIR}")
    else:
        run_list=rl.get_runs_in_dir(f"{INPUT_DIR}/all/{EXP_DIR}")
    print(run_list)
    df=r2p.get_configs(CONFIG_FILENAME)

    runIDpos=0 if LEGACY_DATA_GEN else 1

    if platform.system()=="Windows":
        df2=pd.DataFrame([[int(run.split("\\")[-1].split("-")[runIDpos]),run] for run in run_list], columns=["runID","filepath"])
    else:
        df2=pd.DataFrame([[int(run.split("/")[-1].split("-")[runIDpos]),run] for run in run_list], columns=["runID","filepath"])
        
    df2.index=df2["runID"]
    df2=df2.drop(columns=["runID"])
    df=df.join(df2)
    print(df)

    if "PME" in EXP_SPECS: 
        samples_path=f"{OUTPUT_DIR}/{'-'.join([EXP_DIR,*EXP_SPECS,*CONFIG_FILENAME.split('_')[1:],'nS='+str(NB_POISSON_SAMPLES),'rS='+str(ASSET_RATE),'per='+str(MARKET_PERIOD),'assets'])}.csv"
    else:
        samples_path=f"{OUTPUT_DIR}/{'-'.join([EXP_DIR,*EXP_SPECS,*CONFIG_FILENAME.split('_')[1:],'nS='+str(NB_POISSON_SAMPLES),'rS='+str(ASSET_RATE),'assets'])}.csv"

    samples=[]
    if not os.path.exists(samples_path):
        for i in range(NB_POISSON_SAMPLES):
            if "PME" in EXP_SPECS:
                s=poisson.draw_poisson_periodic(TIME_INTERVAL,ASSET_RATE,MARKET_PERIOD)
            else:
                s=poisson.draw_poisson(TIME_INTERVAL,ASSET_RATE)
            s.sort()
            samples.append(s)
        pd.DataFrame(samples).to_csv(samples_path, header=None, index=None)
    else:
        print(f"Loading samples from {samples_path}")
        samples=pd.read_csv(samples_path, header=None)
        samples=samples.values.tolist()

    for cond_idx, conditions in enumerate(conditions_list):
        str_desc="-".join([EXP_DIR,*EXP_SPECS,*CONFIG_FILENAME.split("_")[1:],"nS="+str(NB_POISSON_SAMPLES),"rS="+str(ASSET_RATE),",".join([a+"="+b for a,b in conditions])]).replace("*","x")
        if 0 in RECOMPUTE:
            print("Conditions:", ",".join([a+"="+b for a,b in conditions]))
            agg_condition=None
            for col, val in conditions:
                if agg_condition is None:
                    agg_condition=(df[col]==val)
                else:
                    agg_condition=agg_condition & (df[col]==val)
            filtered_df=df.where(agg_condition).dropna()
            run_list=filtered_df["filepath"].dropna().values
            print(run_list)

            first=True
            if not os.path.exists(f"{OUTPUT_DIR}/{str_desc}.csv"):
                with alive_bar(len(run_list)*len(samples), bar='blocks', spinner='pulse', length=20, title='Processing runs x samples') as bar:
                    for sample_idx, s in enumerate(samples):
                        for i, run in enumerate(run_list):
                            if LEGACY_DATA_GEN:
                                file_list=rl.get_files_in_dir(run)
                            else:
                                file_list=run
                            dfs=generate_dfs(file_list, s, BEHAVIOURS[cond_idx], time_offset=TIME_OFFSET, exp_specs=EXP_SPECS, legacy_data_gen=LEGACY_DATA_GEN)
                            dfs.drop(["IN","QUEUE","OUT"], axis=1, inplace=True)
                            dfs["REPETITION"]=i
                            dfs["MARKET_REPETITION"]=sample_idx
                            dfs.reset_index(inplace=True)
                            dfs.columns=["CONSUMER_ID","PROVIDER_ID","REQUEST_ID",*dfs.columns[3:-2],"REPETITION","MARKET_REPETITION"]
                            dfs.set_index(["MARKET_REPETITION","REPETITION","CONSUMER_BEHAVIOUR","CONSUMER_ID","PROVIDER_BEHAVIOUR","PROVIDER_ID","REQUEST_ID"], inplace=True)
                            
                            dfs.to_csv(f"{OUTPUT_DIR}/{str_desc}.csv", mode='a', header=first)
                            
                            first=False
                            bar()
            else:
                print(f"{OUTPUT_DIR}/{str_desc}.csv already exists: skipping data generation")

        if 1 in RECOMPUTE:
            idx_cols=["MARKET_REPETITION","REPETITION","CONSUMER_BEHAVIOUR","CONSUMER_ID","PROVIDER_BEHAVIOUR","PROVIDER_ID","REQUEST_ID"]
            cols=idx_cols+["FASTEST"]
            print(f"Loading data from {OUTPUT_DIR}/{str_desc}.csv")
            full_dfs=pd.read_csv(f"{OUTPUT_DIR}/{str_desc}.csv", usecols=cols, index_col=idx_cols, low_memory=True)
            print(f"Loaded data from {OUTPUT_DIR}/{str_desc}.csv")

        #plot_data(full_dfs, str_desc)
        plt.close('all')
    #plt.show()