#!/usr/bin/env python3
"""Receive VSCP UDP debug log messages and print them to stdout."""

import argparse
import socket
import sys
from typing import Tuple


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Listen for UDP debug messages from vscp-udp-log.h.",
    )
    parser.add_argument(
        "--host",
        default="127.0.0.1",
        help="IP address to bind to for the UDP socket (default: 127.0.0.1)",
    )
    parser.add_argument(
        "--port",
        type=int,
        default=9999,
        help="UDP port to listen on (default: 9999)",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=1.0,
        help="Socket timeout in seconds while waiting for packets (default: 1.0)",
    )
    parser.add_argument(
        "--buffer-size",
        type=int,
        default=512,
        help="Maximum packet size in bytes (default: 512)",
    )
    return parser.parse_args()


def make_socket(host: str, port: int, timeout: float) -> socket.socket:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind((host, port))
    sock.settimeout(timeout)
    return sock


def receive_loop(host: str, port: int, timeout: float, buffer_size: int) -> None:
    sock = make_socket(host, port, timeout)
    print(f"Listening for VSCP UDP debug messages on {host}:{port}...")
    print("Press Ctrl+C to stop.")

    try:
        while True:
            try:
                data, addr = sock.recvfrom(buffer_size)
            except socket.timeout:
                continue

            message = data.decode("utf-8", errors="replace").rstrip("\r\n")
            print(f"[{addr[0]}:{addr[1]}] {message}")
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        sock.close()


def main() -> int:
    args = parse_args()
    try:
        receive_loop(args.host, args.port, args.timeout, args.buffer_size)
    except OSError as exc:
        print(f"Failed to bind UDP socket: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
