import pandas as pd
from matplotlib import pyplot as plt
import os

DENSITY = False if len(os.sys.argv) > 1 and os.sys.argv[1]=="False" else True
CUMUL = False if len(os.sys.argv) > 2 and os.sys.argv[2]=="False" else True
print(DENSITY, CUMUL)

DATASET_DIR="../Datasets/cloud-edge-latency"
FIG_DIR="fig"
# Load the dataset
df = pd.read_csv(f'{DATASET_DIR}/edge.csv', sep=',')
print(df.head())

df_mat=pd.DataFrame()
for i in range(1, len(df.columns), 2):
    sub_df = df.iloc[:, [0, i, i+1]]
    sub_df.columns = ["PROBE", 'LAT', 'NODE']
    sub_df.loc[:, 'NODE'] = sub_df["NODE"]
    sub_df.set_index(['PROBE', 'NODE'], inplace=True)
    df_mat = pd.concat([df_mat, sub_df], axis=0)
#print(df_mat)
#df_mat.where(df_mat['LAT'] <250, inplace=True)
df_mat.dropna(inplace=True)

print(df_mat)

# for each probe find the closest node and its latency
df_mat_closest=pd.DataFrame()
for probe in df_mat.index.get_level_values('PROBE').unique():
    sub_df = df_mat.loc[probe]
    closest_node = sub_df['LAT'].idxmin()
    closest_lat = sub_df.loc[closest_node, 'LAT']
    df_mat_closest = pd.concat([df_mat_closest, pd.DataFrame({'PROBE': [probe], 'NODE': [closest_node], 'LAT': [closest_lat]})], axis=0)

print(df_mat_closest)
fig, ax = plt.subplots()
ax.hist(df_mat_closest['LAT'], bins=range(0,int(df_mat_closest["LAT"].max()+1)), density=DENSITY, cumulative=CUMUL, histtype='step')

ax.set_xticks(range(0, int(df_mat_closest["LAT"].max())+1, 10), minor=True)
ax.set_xticks(range(0, int(df_mat_closest["LAT"].max())+1, 50))
if DENSITY:
    ax.set_yticks([x/100 for x in range(0, 101, 10)])
    ax.set_yticks([x/100 for x in range(0, 101, 2)], minor=True)
    ax.set_yticklabels([f'{x:.0%}' for x in ax.get_yticks()])
else:
    if CUMUL:
        ax.set_yticks(range(0, df_mat_closest.shape[0]+101, 100), minor=True)
        ax.set_yticks(range(0, df_mat_closest.shape[0]+501, 500))

ax.grid(which='minor', linestyle=':', linewidth='0.5', color='black', alpha=0.5)
ax.grid(which='major', linestyle='-', linewidth='0.5', color='black', alpha=0.7)
ax.set_xlabel('Latency (ms)')
if CUMUL and not DENSITY:
    ax.set_ylabel('Users cumulative count')
elif not CUMUL and DENSITY:
    ax.set_ylabel('Users PDF (%)')
elif CUMUL and DENSITY:
    ax.set_ylabel('Users CDF (%)')
else:
    ax.set_ylabel('Users count')

fig.savefig(f"{FIG_DIR}/edge-closest-latency-hist{'-dens' if DENSITY else ''}{'-cumul' if CUMUL else ''}.png")
ax.set_xscale('log')
fig.savefig(f"{FIG_DIR}/edge-closest-latency-hist{'-dens' if DENSITY else ''}{'-cumul' if CUMUL else ''}-log.png")

# closest_node = df_mat.groupby('PROBE')['LAT'].idxmin().apply(lambda x: x[1])
# closest_node.name='NODE'
# print(closest_node)
# cloud_closest=closest_node.value_counts()

# fig, ax = plt.subplots()
# fig.set_size_inches(15, 10)
# cloud_closest.plot(kind='bar', ax=ax)
# fig.savefig(f"{FIG_DIR}/cloud-closest-node.png")

print(df_mat_closest.where(df_mat_closest['LAT'] >240).dropna()["PROBE"].unique())