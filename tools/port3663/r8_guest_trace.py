"""Print the bounded, payload-free 3663 guest packet trace as CSV."""
import argparse
import csv
from datetime import datetime, timedelta, timezone
from pathlib import Path
import struct
import sys


HEADER = struct.Struct("<8sIIII")
PACKET = struct.Struct("<QHHBBH")
FILETIME_EPOCH = datetime(1601, 1, 1, tzinfo=timezone.utc)


def parse_time(value):
    time = datetime.fromisoformat(value)
    if time.tzinfo is None:
        raise argparse.ArgumentTypeError("Supply an explicit UTC offset")
    return time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--after", type=parse_time)
    args = parser.parse_args()
    with args.trace.open("rb") as handle:
        header = handle.read(HEADER.size)
        if len(header) != HEADER.size:
            parser.error("Incomplete trace header")
        magic, capacity, count, truncated, pid = HEADER.unpack(header)
        if (magic != b"CDMPPKT1" or not 0 <= count <= capacity == 131072 or truncated not in (0, 1)
                or args.trace.stat().st_size != HEADER.size + capacity * PACKET.size):
            parser.error("Invalid trace header or size")
        if truncated:
            print("Trace reached capacity. Later packets were omitted.", file=sys.stderr)
        writer = csv.writer(sys.stdout, lineterminator="\n")
        writer.writerow(("pid", "index", "utc", "direction", "opcode", "frame_bytes", "status"))
        for index in range(count):
            tick, op, size, direction, done, reserved = PACKET.unpack(handle.read(PACKET.size))
            if direction not in (1, 2) or done not in (0, 1) or reserved or not tick:
                parser.error(f"Incomplete packet row {index}")
            time = FILETIME_EPOCH + timedelta(microseconds=tick // 10)
            if args.after and time < args.after:
                continue
            label = "host-to-guest" if direction == 1 else "guest-to-host"
            status = ("returned" if direction == 1 else "queued") if done else "pending"
            writer.writerow((pid, index, time.isoformat(), label, op, size, status))


if __name__ == "__main__":
    main()
