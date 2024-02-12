import pandas as pd

def get_configs(filename):
    df = pd.read_csv(filename, header=None, sep=", |: ", engine="python")
    columns=["runID"]
    for i in range(1, len(df.loc[0].values)):
        columns.append(df.loc[0].values[i].split("=")[0][1:])
    df.columns=columns
    df["runID"]=df["runID"].apply(lambda x: int(x.split(" ")[1]))
    df[columns[1:]]=df[columns[1:]].applymap(lambda x: x.split("=")[1])
    df.index=df["runID"]
    df=df.drop(columns=["runID"])
    return df

if __name__ == "__main__":
    df=get_configs("configs")
    print(df)