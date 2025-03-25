from datasets import load_dataset, Dataset
import re
import multiprocessing
import os
from tqdm import tqdm


KEEP_CHARS = set(
    [
        " ",
        "a",
        "b",
        "c",
        "d",
        "e",
        "f",
        "g",
        "h",
        "i",
        "j",
        "k",
        "l",
        "m",
        "n",
        "o",
        "p",
        "q",
        "r",
        "s",
        "t",
        "u",
        "v",
        "w",
        "x",
        "y",
        "z",
        "?",
        ".",
        ",",
        ":",
        "&",
        "%",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "0",
    ]
)


def normalize(x: str, keep_chars: set[str] = KEEP_CHARS):
    x = re.sub(r"\s", " ", x)  # make sure using the same space char (no special chars)
    x = re.sub(r"–", "-", x)  # use the - instead of –
    x = x.lower()  # only consider lower case chars

    # Keep chars we define and throw away others
    res = ""
    for c in x:
        if c in keep_chars:
            res += c
    return res


ds = load_dataset("laion/biorXiv_metadata")


def parse(x):
    x["title_parsed"] = normalize(x["title"])
    return x


df = ds["train"].to_pandas()
df = df.drop_duplicates("doi")
ds = Dataset.from_pandas(df)

title_parsed = ds.map(parse, num_proc=multiprocessing.cpu_count())

FILE_OUT = "titles.txt"
if os.path.exists(FILE_OUT):
    os.remove(FILE_OUT)
with open(FILE_OUT, "a") as f:
    for t in tqdm(title_parsed):
        f.write(t["title_parsed"] + "\n")
