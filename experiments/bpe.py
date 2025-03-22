from time import time


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


if __name__ == "__main__":
    example = "Hi there my name is donny!" * 100_000
    merges = [
        (("H", "i"), "Hi"),
        (("Hi", " "), "Hi "),
        (("Hi ", "t"), "Hi t"),
        (("d ", "o"), "do"),
    ]
    sorted_merge_rules = preprocess_merge_rules(merges)
    with Time("ms"):
        for i, j in iter_tokens_faster(example, sorted_merge_rules):
            pass
