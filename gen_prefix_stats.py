import csv
import sys
from collections import defaultdict


def compute_prefix_stats(input_tsv_path: str, output_csv_path: str) -> None:
    prefix_to_widths = defaultdict(list)

    with open(input_tsv_path, "r", newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t")
        for row in reader:
            file_name = row["file"].strip()
            if not file_name:
                continue
            prefix3 = file_name[:3]
            try:
                min_width = float(row["min_width"])  # numeric width
            except Exception:
                # Skip malformed rows
                continue
            prefix_to_widths[prefix3].append(min_width)

    # Write output CSV with the same header format as 222.csv
    with open(output_csv_path, "w", newline="", encoding="utf-8") as out:
        writer = csv.writer(out)
        writer.writerow(["prefix", "num_cells", "avg_width"])
        for prefix in sorted(prefix_to_widths.keys()):
            widths = prefix_to_widths[prefix]
            count = len(widths)
            avg = sum(widths) / count if count else 0.0
            writer.writerow([prefix, count, f"{avg:.2f}"])


def main():
    if len(sys.argv) not in (2, 3):
        print("Usage: python gen_prefix_stats.py <input_tsv> [output_csv]")
        sys.exit(1)
    input_tsv = sys.argv[1]
    output_csv = sys.argv[2] if len(sys.argv) == 3 else "222_from_widths.csv"
    compute_prefix_stats(input_tsv, output_csv)


if __name__ == "__main__":
    main()


