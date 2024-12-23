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
print(df_mat)

fig, ax = plt.subplots()

ax.hist(df_mat['LAT'], bins=range(0,int(df_mat["LAT"].max()+1)), density=DENSITY, cumulative=CUMUL, histtype='step')

if DENSITY:
    ax.set_yticks([x/100 for x in range(0, 101, 10)])
    ax.set_yticks([x/100 for x in range(0, 101, 2)], minor=True)
    ax.set_yticklabels([f'{x:.0%}' for x in ax.get_yticks()])
else:
    if CUMUL:
        ax.set_yticks(range(0, df_mat.shape[0]+1001, 1000), minor=True)
        ax.set_yticks(range(0, df_mat.shape[0]+5001, 5000))

ax.grid(which='minor', linestyle=':', linewidth='0.5', color='black', alpha=0.5)
ax.grid(which='major', linestyle='-', linewidth='0.5', color='black', alpha=0.7)
ax.set_xlabel('Latency (ms)')
if CUMUL and not DENSITY:
    ax.set_ylabel('User-Node pair cumulative count')
elif not CUMUL and DENSITY:
    ax.set_ylabel('User-Node pair PDF (%)')
elif CUMUL and DENSITY:
    ax.set_ylabel('User-Node pair CDF (%)')
else:
    ax.set_ylabel('User-Node pair count')

# quant_keys=[0.25, 0.5, 0.75, 0.9, 0.999, 0.999999]
# quantiles=df_mat['LAT'].quantile(quant_keys)
# print(quantiles)
# for k, q in zip(quant_keys, quantiles):
#     ax.axvline(x=q, color='r', linestyle='--')
#     ax.text(q, 100, f'{k:%} : {q:.2f}', rotation=90)
fig.savefig(f"{FIG_DIR}/edge-latency-hist{'-dens' if DENSITY else ''}{'-cumul' if CUMUL else ''}.png")
ax.set_xscale('log')
#ax.set_yscale('log')
fig.savefig(f"{FIG_DIR}/edge-latency-hist{'-dens' if DENSITY else ''}{'-cumul' if CUMUL else ''}-log.png")
