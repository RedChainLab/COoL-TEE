import pandas as pd
from matplotlib import pyplot as plt
import os

colors = plt.rcParams['axes.prop_cycle'].by_key()['color']

CLOUD = os.sys.argv[1] if len(os.sys.argv) > 1 else "ALL"
USER = os.sys.argv[2] if len(os.sys.argv) > 2 else "ALL"

DENSITY=True
CUMUL=True

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

location_dict = {
    "EU": europe_locations,
    "NA": north_america_locations,
    "SA": south_america_locations,
    "As": asia_locations,
    "Oc": oceania_locations,
    "ALL": all_locations
}

df_mat_closest=df_mat_closest.loc[df_mat_closest['NODE'].isin(location_dict[USER])].dropna()
df_mat=df_mat.loc[df_mat.index.get_level_values('PROBE').isin(df_mat_closest['PROBE'])].dropna()

df_cloud_mat=df_mat.pivot_table(index='PROBE', columns='NODE', values='LAT').dropna()
df_cloud_mat=df_cloud_mat[cloud_dict[CLOUD]]

print(df_cloud_mat)

df_cloud_mat.to_csv(f'{DATASET_DIR}/cloud-{CLOUD}-{USER}.csv', sep='\t')