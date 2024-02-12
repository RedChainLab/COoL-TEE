import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os
import seaborn as sns
from enum import Enum
import re
import platform
import sys
Event= Enum('Event', ["SEND","IN","QUEUE","SERVE","OUT","RECV","START"])
RQ_types= Enum('RQ_Type', ["First","All"])

# Return the sublist of filepaths that match the string regex
def restrict_runs(filepaths, regex):
    return [file for file in filepaths if re.search(regex,file)]

def get_runs_in_dir(dir_path):
    dirs = []
    for file in os.listdir(dir_path):
        dirs.append(os.path.join(dir_path, file))
    return dirs

def get_files_in_dir(dir_path):
    files = []
    for file in os.listdir(dir_path):
        if file.endswith(".csv"):
            files.append(os.path.join(dir_path, file))
    return files

def gen_dfs_legacy(files):
    dfs = []
    keys =[]
    for file in files:
        df = pd.read_csv(file, header=None, names=["SEND","IN","QUEUE","SERVE","OUT","RECV"])
        # Extract the 3rd term in-between '-' characters in the filename
        if platform.system()=="Windows":
            filename = file.split('\\')[-1]
        else:
            filename = file.split('/')[-1]
        nReqs = filename.split('-')[2]
        nProvs = filename.split('-')[4].split('.')[0]
        # Put df in a 2D dictionary at position (nReqs, nProvs)
        dfs.append(df)
        keys.append((nReqs, nProvs))
    ndf=pd.concat(dfs, keys=keys)
    #print(ndf)
    return ndf

def gen_dfs(file):
    df=pd.read_csv(file, header=None, names=["CONSUMER_ID","PROVIDER_ID","SEND","IN","QUEUE","SERVE","OUT","RECV"])
    df=df.set_index(["CONSUMER_ID","PROVIDER_ID"], append=True)
    df=df.swaplevel(0,1)
    df=df.swaplevel(1,2)
    #print(df)
    return df

def plot_subplots(df,ax,column,level,HORIZONTAL=True):
    # for i, idx in enumerate(col.index.levels[level]):
    #     grp=col.groupby(level=level).get_group(idx).values
    #     ax.plot(grp, -i*np.ones(len(grp)) if HORIZONTAL else np.arange(len(grp)),linestyle='None',marker='+')
    #     ax.set_xlim(0)
    #     ax.set_xlabel('Time')
    #     ax.set_ylabel('Timeline #')
    #     #ax.set_title(f'Plot of {end.name} - {start.name} times')
    #     #show legend
    #     ax.legend(list(col.index.levels[level]))
    sns.violinplot(data=df,x=column, y=df.index.get_level_values(level), hue="FASTEST", split=True, cut=0, ax=ax)#, orient="h" if HORIZONTAL else "v")

def plot_histograms_subplots(df,ax, column, level):
    col=df[column]
    max_val=0
    for idx in col.index.levels[level]:
        grp=col.groupby(level=level).get_group(idx).values
        counts,_=np.histogram(grp, bins=1000)
        ax.hist(grp, bins=1000, cumulative=True, histtype='step')
        max_val=max(max_val,sum(counts))

    ax.set_xlim(0)
    ax.set_xlabel('Waittime')
    ax.set_ylabel('# of requests')
    #ax.set_title(f'Histogram of {end.name} - {start.name} times')
    #show legend
    ax.legend(list(col.index.levels[level]))

    ax.set_ylim(bottom=0)
    ax.set_ylim(top=1.05*max_val)
    ax.set_yticks(np.arange(0,1.05*max_val,step=10000))
    ax.set_yticks(np.arange(0,1.05*max_val,step=1000),minor=True)
    ax.minorticks_on()
    ax.grid(axis='y',alpha=0.5, which='minor')
    ax.grid(axis='y',alpha=0.75, which='major')

if __name__ == "__main__":
    SUFFIX="5%"
    TIME_INTERVAL=600
    ASSET_RATE=1

    START=Event.SEND
    END=Event.RECV

    HORIZONTAL=True
    RELATIVE=True

    RQ_type=RQ_types.All

    COLUMN=("DIFF" if RELATIVE else END.name)

    if RQ_type==RQ_types.First:
        file_list=get_files_in_dir("example_waittimes/first"+SUFFIX)
    else:
        file_list=get_files_in_dir("example_waittimes/all"+SUFFIX)

    dfs=gen_dfs(file_list)
    dfs["DIFF"]=dfs[END.name]-(dfs[START.name] if RELATIVE else 0)

    _,ax=plt.subplots(2,2, sharex=True)
    plot_subplots(dfs,ax[0][0],"DIFF",0, HORIZONTAL)
    plot_subplots(dfs,ax[1][0],"DIFF",1, HORIZONTAL)
    plot_histograms_subplots(dfs,ax[0][1],"DIFF",0)
    plot_histograms_subplots(dfs,ax[1][1],"DIFF",1)

    plt.show()