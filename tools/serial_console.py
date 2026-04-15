import argparse
import queue
import sys
import threading
import time

import serial


def read_serial(port: serial.Serial, stop_event: threading.Event, output_queue: queue.Queue[str]) -> None:
    while not stop_event.is_set():
        try:
            data = port.read(4096)
        except serial.SerialException as exc:
            output_queue.put(f"\nSerial read error: {exc}\n")
            stop_event.set()
            return

        if data:
            output_queue.put(data.decode("utf-8", errors="replace"))


def print_serial_output(output_queue: queue.Queue[str], stop_event: threading.Event) -> None:
    while not stop_event.is_set():
        try:
            text = output_queue.get(timeout=0.1)
        except queue.Empty:
            continue

        print(text, end="", flush=True)


def main() -> int:
    parser = argparse.ArgumentParser(description="Line-oriented ESP32 serial console.")
    parser.add_argument("--port", default="COM7", help="Serial port, for example COM7.")
    parser.add_argument("--baud", type=int, default=115200, help="Serial baud rate.")
    parser.add_argument("--reset", action="store_true", help="Pulse RTS to reset the ESP32 on connect.")
    args = parser.parse_args()

    try:
        port = serial.Serial(args.port, args.baud, timeout=0.1)
    except serial.SerialException as exc:
        print(f"Could not open {args.port}: {exc}")
        print("Close Arduino/PlatformIO serial monitors and try again.")
        return 1

    with port:
        port.setDTR(False)
        if args.reset:
            port.setRTS(True)
            time.sleep(0.1)
            port.setRTS(False)
        else:
            port.setRTS(False)

        stop_event = threading.Event()
        output_queue: queue.Queue[str] = queue.Queue()
        threads = [
            threading.Thread(target=read_serial, args=(port, stop_event, output_queue), daemon=True),
            threading.Thread(target=print_serial_output, args=(output_queue, stop_event), daemon=True),
        ]

        for thread in threads:
            thread.start()

        print(f"Connected to {args.port} at {args.baud}.")
        print("Type ESP32 commands and press Enter. Type quit to close.")
        print("Try: status, scan, test, off, all 4095, m 0 4095 2000")

        try:
            while not stop_event.is_set():
                command = input("esp32> ").strip()
                if command.lower() in {"quit", "exit"}:
                    break
                if command:
                    port.write((command + "\n").encode("utf-8"))
                    port.flush()
        except (EOFError, KeyboardInterrupt):
            pass
        finally:
            stop_event.set()

    print("\nSerial console closed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
