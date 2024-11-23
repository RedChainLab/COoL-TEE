import pandas as pd
from matplotlib import pyplot as plt
import os

colors = plt.rcParams['axes.prop_cycle'].by_key()['color']

DENSITY = False if len(os.sys.argv) > 1 and os.sys.argv[1]=="False" else True
CUMUL = False if len(os.sys.argv) > 2 and os.sys.argv[2]=="False" else True
CLOUD = os.sys.argv[3] if len(os.sys.argv) > 3 else "ALL"
print(DENSITY, CUMUL)

DATASET_DIR="../Datasets/cloud-edge-latency"
FIG_DIR="fig"
# Load the dataset
df = pd.read_csv(f'{DATASET_DIR}/cloud.csv', sep=',')
print(df.head())

df_mat=pd.DataFrame()
for i in range(1, len(df.columns), 2):
    sub_df = df.iloc[:, [0, i, i+1]]
    sub_df.columns = ["PROBE", 'LAT', 'NODE']
    sub_df.loc[:, 'NODE'] = sub_df["NODE"].apply(lambda x: str(x).split('.')[0])
    sub_df.set_index(['PROBE', 'NODE'], inplace=True)
    df_mat = pd.concat([df_mat, sub_df], axis=0)
#print(df_mat)
#df_mat.where(df_mat['LAT'] <250, inplace=True)
df_mat.dropna(inplace=True)

gcp_locations = ['europe-west1', 'europe-west3', 'europe-north1', 'europe-west2', 'europe-west4',
'us-east4', 'northamerica-northeast1', 'us-east1', 'us-central1', 'us-west1',
'southamerica-east1',
'asia-northeast1', 'asia-south1', 'asia-east1', 'asia-southeast1',
'australia-southeast1'
]
aws_locations = [
    'EUFrankfurt', 'EUParis', 'EULondon', 'EUIreland',
    'USEastNViginia', 'CanadaCentral', 'USWestNCalifornia', 'USWestOregon', 'USEastOhio',
    'SouthAmericaSaoPaola',
    'AsiaPacificSingapure', 'AsiaPacificMumbai', 'AsiaPacificTokyo', 'AsiaPacificSeoul',
    'AsiaPacificSydney'
]
ibm_locations = [
    'Amsterdam1-AMS01', 'Frankfurt5-FRA05', 'Paris-PAR01', 'Oslo-OSL01', 'London-Lon02', 'Milan-MIL01', 
    'Dallas-DAL01', 'Seattle-SEA01', 'SanJose-SJC01', 'Toronto-TOR01', 'Washington-WDC01', 'Houston-HOU2', 'Montreal-MON01', 'Queretaro-MEX01',
    'SaoPaola-SAO01', 
    'Singapore-SNG01', 'Chennai-CHE01', 'Tokyo-TOK02', 'HongKong-HKG02', 'Seoul-SEO01',
    'Melbourne-MEL01', 'Sydney-SYD01'
]
oracle_locations = [
    'eu-frankfurt', 'uk-london', 'EuropeAmsterdam', 'EuropeSlough',
    'Us-central1 2', 'Us-east1 2', 'us-ashburn', 'us-phoenix',
    'AmericaSao_Paulo', 
    'AustraliaSydney'
]
rackspace_locations = [
    'xLondon', 
    'NorthVirginia', 'Chicago', 'Dallas',
    'HongKong', 
    'Sydney'
]

all_locations=gcp_locations+aws_locations+ibm_locations+oracle_locations+rackspace_locations

cloud_dict = {
    'GCP': gcp_locations,
    'AWS': aws_locations,
    'IBM': ibm_locations,
    'Oracle': oracle_locations,
    'Rackspace': rackspace_locations,
    'ALL': all_locations
}

df_mat=df_mat.loc[df_mat.index.get_level_values('NODE').isin(cloud_dict[CLOUD])]

# for each probe find the closest node and its latency
df_mat_closest=pd.DataFrame()
for probe in df_mat.index.get_level_values('PROBE').unique():
    sub_df = df_mat.loc[probe]
    closest_node = sub_df['LAT'].idxmin()
    closest_lat = sub_df.loc[closest_node, 'LAT']
    df_mat_closest = pd.concat([df_mat_closest, pd.DataFrame({'PROBE': [probe], 'NODE': [closest_node], 'LAT': [closest_lat]})], axis=0)

europe_locations =  ['Amsterdam1-AMS01', 'eu-frankfurt', 'europe-west1', 'uk-london', 'europe-west3', 'Frankfurt5-FRA05', 'Paris-PAR01', 'Oslo-OSL01', 'EUFrankfurt', 'EUParis', 'europe-north1', 'europe-west2', 'EULondon', 'EUIreland', 'europe-west4', 'London-Lon02', 'EuropeAmsterdam', 'Milan-MIL01', 'xLondon', 'EuropeSlough']
north_america_locations =  ['us-east4', 'NorthVirginia', 'USEastNViginia', 'Dallas-DAL01', 'Us-central1 2', 'Chicago', 'Us-east1 2', 'CanadaCentral', 'northamerica-northeast1', 'Seattle-SEA01', 'SanJose-SJC01', 'us-ashburn', 'Dallas', 'us-east1', 'Toronto-TOR01', 'Washington-WDC01', 'Houston-HOU2', 'us-phoenix', 'USWestNCalifornia', 'USWestOregon', 'USEastOhio', 'us-central1', 'us-west1', 'Montreal-MON01', 'Queretaro-MEX01']
south_america_locations = ['southamerica-east1', 'SouthAmericaSaoPaola', 'SaoPaola-SAO01', 'AmericaSao_Paulo']
asia_locations = ['Singapore-SNG01', 'HongKong', 'Chennai-CHE01', 'asia-northeast1', 'asia-south1', 'asia-east1', 'Tokyo-TOK02', 'asia-southeast1', 'AsiaPacificSingapure', 'AsiaPacificMumbai', 'HongKong-HKG02', 'Seoul-SEO01', 'AsiaPacificTokyo', 'AsiaPacificSeoul']
oceania_locations = ['australia-southeast1', 'Melbourne-MEL01', 'Sydney', 'Sydney-SYD01', 'AsiaPacificSydney', 'AustraliaSydney']

fig, ax = plt.subplots()

if CLOUD == 'ALL':
    for idx, loc_grp in enumerate([europe_locations, north_america_locations, south_america_locations, asia_locations, oceania_locations]):
        ax.hist(df_mat_closest.loc[df_mat_closest['NODE'].isin(loc_grp), 'LAT'], bins=range(0,int(df_mat_closest["LAT"].max()+1)), label=loc_grp[0], density=DENSITY, cumulative=CUMUL, histtype='step', linestyle = '-' if idx//len(colors)==0 else '--' if idx//len(colors)==1 else ':')
else:
    for idx, node in enumerate(cloud_dict[CLOUD]):
        ax.hist(df_mat_closest.loc[df_mat_closest['NODE']==node, 'LAT'], bins=range(0,int(df_mat_closest["LAT"].max()+1)), label=node, density=DENSITY, cumulative=CUMUL, histtype='step', linestyle = '-' if idx//len(colors)==0 else '--' if idx//len(colors)==1 else ':')

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

ax.set_xlim(0, 300)
ax.legend(loc='lower right', labels=['Europe', 'North America', 'South America', 'Asia', 'Oceania'] if CLOUD == 'ALL' else cloud_dict[CLOUD], fontsize='x-small')
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
fig.savefig(f"{FIG_DIR}/cloud-closest-latency-hist{'-dens' if DENSITY else ''}{'-cumul' if CUMUL else ''}-{CLOUD}.png")
ax.set_xscale('log')
ax.set_xlim()
fig.savefig(f"{FIG_DIR}/cloud-closest-latency-hist{'-dens' if DENSITY else ''}{'-cumul' if CUMUL else ''}-{CLOUD}-log.png")
