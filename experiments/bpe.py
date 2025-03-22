from time import time
from collections import Counter


def preprocess_merge_rules(merge_rules: list[tuple[tuple[str, str], str]]) -> list[tuple[str, int]]:
    """Preprocess merge rules into a sorted list of (value, length) tuples."""
    return sorted([(v, len(v)) for k, v in merge_rules], key=lambda x: x[1], reverse=True)


def find_largest_slice_possible_faster(s: str, i: int, sorted_merge_rules: list[tuple[str, int]]):
    """Find the largest slice that meets the merge rules using a sorted list."""
    for v, length in sorted_merge_rules:
        if s.startswith(v, i):
            return length
    return 1


def iter_tokens_faster(s: str, sorted_merge_rules: list[tuple[str, int]]):
    """Iterate over tokens using preprocessed and sorted merge rules."""
    i = 0
    while i < len(s):
        slice_len = find_largest_slice_possible_faster(s, i, sorted_merge_rules)
        yield i, i + slice_len
        i += slice_len


class Time:
    def __init__(self, type="s", round=None):
        if type == "s":
            self.time_scalar = 1
        elif type == "ms":
            self.time_scalar = 1e3
        elif type == "μs":
            self.time_scalar = 1e6
        elif type == "ns":
            self.time_scalar = 1e9
        else:
            raise ValueError(f"No type {type}")

        self.type = type
        self.round = round

    def __enter__(self):
        self.t0 = time()

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.t1 = time()
        t = (self.t1 - self.t0) * self.time_scalar
        print(f"{round(t, self.round)}{self.type}")


class BPE:
    def __init__(self, data: list[str], init_vocab: list[str], n_vocab=20):
        assert len(data) > 0, "data needs data! duh"
        assert n_vocab > 0, "n_vocab must be greater than 0"
        assert len(init_vocab) > 0, "init_vocab must be populated"

        self.data = data
        self.init_vocab = init_vocab
        self.n_vocab = n_vocab

    def train(self):
        iters = self.n_vocab - len(self.init_vocab)
        if iters <= 0:
            return  # don't need to do any training, already have all the n_vocab we need

        merge_rules = []
        for i in range(iters):
            c = Counter()
            # for all strings, count the most frequent pair of characters
            for s in self.data:
                token_slices = list(iter_tokens_faster(s, merge_rules))
                for s1, s2 in zip(token_slices, token_slices[1:]):
                    str1 = s[s1[0] : s1[1]]
                    str2 = s[s2[0] : s2[1]]
                    c[str1 + str2] += 1
            most_freq, _ = c.most_common(1)[0]
            merge_rules.insert(0, (most_freq, len(most_freq)))
        return merge_rules

    def encode(self, x: str, rules: list[str]):
        for i, j in iter_tokens_faster(x, rules):
            yield x[i:j]


if __name__ == "__main__":
    ds = ["hi there my name is donnydonny", *100] * 2_000
    bpe = BPE(ds, init_vocab=list("abcdefghijklmnopqrstuvqxyz"), n_vocab=32)
    rules = bpe.train()
    print(list(bpe.encode(ds[0], rules)))
