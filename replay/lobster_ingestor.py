import csv
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parent.parent

INPUT_FILE = (
    PROJECT_ROOT
    / "data"
    / "AAPL_2012-06-21_34200000_57600000_message_10.csv"
)

OUTPUT_FILE = PROJECT_ROOT / "data" / "AAPL_replay.txt"

def main():
    event_counts = {
        1: 0,
        2: 0,
        3: 0,
        4: 0,
        5: 0,
        7: 0,
    }

    output_lines = []

    with INPUT_FILE.open("r", newline="") as file:
        reader = csv.reader(file)

        for row in reader:
            if not row:
                continue

            time = float(row[0])
            event_type = int(row[1])
            order_id = int(row[2])
            size = int(row[3])
            price = int(row[4])
            direction = int(row[5])

            event_counts[event_type] = (
                event_counts.get(event_type, 0) + 1
            )

            if event_type == 1:
                # New limit order
                is_buy = 1 if direction == 1 else 0

                output_lines.append(
                    f"AddLimit {order_id} {is_buy} {size} {price}"
                )

            elif event_type == 2:
                # Partial cancellation
                output_lines.append(
                    f"ReduceOrder {order_id} {size}"
                )

            elif event_type == 3:
                # Full cancellation
                output_lines.append(
                    f"CancelLimit {order_id}"
                )

            elif event_type == 4:
                # Visible execution
                output_lines.append(
                    f"ExecuteOrder {order_id} {size}"
                )

            elif event_type == 5:
                # Hidden execution.
                # Ignore for the visible LOB reconstruction.
                pass

            elif event_type == 7:
                # Trading halt.
                pass

    with OUTPUT_FILE.open("w", newline="\n") as file:
        file.write("\n".join(output_lines))
        file.write("\n")

    print("========================================")
    print("        LOBSTER INGESTION")
    print("========================================")
    print(f"Input:  {INPUT_FILE}")
    print(f"Output: {OUTPUT_FILE}")
    print()
    print("Event counts:")

    for event_type, count in sorted(event_counts.items()):
        print(f"Event {event_type}: {count}")

    print()
    print(f"Commands generated: {len(output_lines)}")


if __name__ == "__main__":
    main()