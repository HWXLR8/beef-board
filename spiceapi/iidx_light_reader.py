#!/usr/bin/env python3
"""
IIDX Light State Reader with USB HID Output

This script reads the IIDX tape LEDs from spiceapi,
then sends them to a USB HID device in a specific format:

- Tape LEDs: 48 bytes (16 RGB LEDs)
"""

import sys
# Set this to where your spiceapi python files are located
# These are available in spice2x releases
sys.path.append("/path/to/spice2x/api/resources/python")
import time
import argparse
try:
    from spiceapi.connection import Connection
    from spiceapi.iidx import iidx_tapeled_get
except ImportError:
    print("Error: Could not find spiceapi python files. Make sure to set the correct path above.")
    sys.exit(1)

try:
    import hid
except ImportError:
    print("Error: hidapi not installed. Install with: pip install hidapi.")
    sys.exit(1)

# Default USB settings
DEFAULT_VID = 0x1CCF
DEFAULT_PID = 0x8048
OUTPUT_INTERFACE = 4
NUM_OF_LEDS = 16

def connect_to_spiceapi(port, password):
    """Connect to the spiceapi server.

    Args:
        host (str): The spiceapi server host
        port (int): The spiceapi server port
        password (str): The spiceapi server password

    Returns:
        Connection: The connection object to the spiceapi server
    """
    host = "localhost"
    conn = Connection(host, port, password)
    print(f"Connected to spiceapi server at {host}:{port}")
    return conn

def find_usb_device(vid, pid):
    """Find the USB HID device.

    Args:
        vid (int): Vendor ID
        pid (int): Product ID

    Returns:
        device: HID device object or None if not found
    """
    try:
        # List all connected HID devices
        devices = hid.enumerate(vid, pid)

        if not devices:
            return None

        # Use the device on tape led output interface
        device_info = None
        for d in devices:
            if d['interface_number'] == OUTPUT_INTERFACE and d['product_string'] == 'BEEF BOARD':
                device_info = d
                break

        # Open the device
        device = hid.device()
        device.open_path(device_info['path'])
        print(f"Opened HID device: {device_info['manufacturer_string']} {device_info['product_string']}")
        return device
    except Exception as e:
        print(f"Error finding HID device: {e}")
        return None

def read_tape_leds(conn):
    """Read the IIDX tape LED states.

    Args:
        conn (Connection): The connection to the spiceapi server

    Returns:
        dict: List of tape LED states
    """
    return iidx_tapeled_get(conn)[0]

def format_tape_leds(tape_leds):
    """Format tape LED data into 48 integers (16 RGB LEDs).

    Args:
        tape_leds (dict): Tape LED states from iidx_tapeled_get

    Returns:
        list: 48 integers representing 16 RGB LEDs
    """
    # Get LEDs from tape_leds
    side_panel_left_leds = tape_leds.get("side_panel_left", [])

    # Create 16 RGB LEDs with values from control_panel_under
    result = []

    # If we have no LED data, return all zeros
    if not side_panel_left_leds:
        return [0, 0, 0] * NUM_OF_LEDS

    # Calculate how many RGB triplets we have
    total_triplets = len(side_panel_left_leds) // 3

    # If we have 16 or fewer LEDs, just use them directly
    if total_triplets <= NUM_OF_LEDS:
        for i in range(total_triplets):
            index = i * 3
            r = int(side_panel_left_leds[index]) & 0xFF
            g = int(side_panel_left_leds[index + 1]) & 0xFF
            b = int(side_panel_left_leds[index + 2]) & 0xFF
            result.extend([r, g, b])

        # Fill the rest with zeros if needed
        padding = NUM_OF_LEDS - total_triplets
        result.extend([0, 0, 0] * padding)
    else:
        # Downscale: We need to fit more than 16 LEDs into 16 output slots
        # Distribute the input LEDs evenly across the 16 output LEDs
        for i in range(NUM_OF_LEDS):
            # Calculate the range of source LEDs that map to this output LED
            start_idx = int((i * total_triplets) / NUM_OF_LEDS)
            end_idx = int(((i + 1) * total_triplets) / NUM_OF_LEDS)

            # Average the RGB values for all LEDs in this range
            r_sum = g_sum = b_sum = 0
            count = end_idx - start_idx

            for j in range(start_idx, end_idx):
                base_idx = j * 3
                r_sum += side_panel_left_leds[base_idx]
                g_sum += side_panel_left_leds[base_idx + 1]
                b_sum += side_panel_left_leds[base_idx + 2]

            # Calculate the average and convert to byte values
            r = int((r_sum / count)) & 0xFF
            g = int((g_sum / count)) & 0xFF
            b = int((b_sum / count)) & 0xFF

            result.extend([r, g, b])

    return result

def send_to_usb_device(device: hid.device, data):
    """Send data to the USB HID device.

    Args:
        device: HID device object
        data (list): List of integers to send

    Returns:
        bool: True if successful, False otherwise
    """
    try:
        # In HID reports, the first byte is typically the report ID
        # For devices that don't use report IDs, use 0
        report_data = [0] + data

        # Write the data to the device
        bytes_written = device.write(report_data)

        # Check if all data was written
        if bytes_written != len(report_data):
            print(f"Warning: Only {bytes_written} of {len(report_data)} bytes were written")
            return False

        return True
    except Exception as e:
        print(f"Error sending data to USB device: {e}")
        return False

def parse_arguments():
    """Parse command line arguments.

    Returns:
        args: Parsed arguments
    """
    parser = argparse.ArgumentParser(description='IIDX Light State Reader with USB HID Output')

    parser.add_argument('--port', type=int, required=True, help='SpiceAPI server port')
    parser.add_argument('--password', default='', help='SpiceAPI server password')

    return parser.parse_args()

def main():
    """Main function to run the script."""
    args = parse_arguments()

    # Find USB device
    device = find_usb_device(DEFAULT_VID, DEFAULT_PID)

    if device:
        print(f"Connected to Beef Board")
    else:
        print(f"Beef Board not found")
        sys.exit(1)

    conn = None

    try:
        print("Press Ctrl+C to exit")

        while True:
            if not conn:
                try:
                    # Connect to spiceapi server
                    conn = connect_to_spiceapi(args.port, args.password)
                except Exception:
                    print(f"Error connecting to spiceapi server, retrying in a bit...")
                    time.sleep(5)
                    continue

            # Read light states
            tape_leds = read_tape_leds(conn)

            # Format data for USB HID output
            tape_led_values = format_tape_leds(tape_leds)

            # Send to USB device
            success = send_to_usb_device(device, tape_led_values)
            if not success:
                print("Failed to send data to Beef Board")

            # Wait before next update
            time.sleep(1 / 120)

    except (ConnectionResetError, KeyboardInterrupt):
        print("\nExiting...")
    finally:
        # Close connection
        device.close()
        if conn:
            conn.close()

if __name__ == "__main__":
    main()
