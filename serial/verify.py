#!/usr/bin/python3

import sys
import ijson
from collections import defaultdict

def count_models(path):
    model_counts = defaultdict(int)

    with open(path, "rb") as f:
        for obj in ijson.items(f, "item"):
            model = obj.get("model")
            if model is not None:
                model_counts[model] += 1

    return model_counts

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <json_file>")
        sys.exit(1)

    counts = count_models(sys.argv[1])

    print(f"Total distinct models: {len(counts)}")
    for model, count in counts.items():
        print(f"{count}: {model}")

