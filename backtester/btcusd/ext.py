import pandas as pd
from datetime import datetime, timedelta


infile = "btcusd_1-min_data.csv"
outfile = "btcusd_1-min_last_week.csv"

df = pd.read_csv(
    infile,
    names=["ts", "open", "high", "low", "close", "volume"],
    header=None,
    skiprows=1,
)

df["datetime"] = pd.to_datetime(df["ts"], unit="s")
latest_dt = df["datetime"].max()
cutoff = latest_dt - timedelta(days=7)
df_week = df[df["datetime"] >= cutoff]
df_week.to_csv(outfile, index=False)

print(f"Extracted {len(df_week)} rows from {cutoff} to {latest_dt}")
