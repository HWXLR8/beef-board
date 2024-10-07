import tkinter as tk
import subprocess
import os
import sys

def get_resource_path(relative_path):
    try:
        # PyInstaller creates a temp folder and stores path in _MEIPASS
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")

    return os.path.join(base_path, relative_path)

def run_avrdude():
    try:
        avrdude_path = get_resource_path("avrdude/avrdude.exe")
        command = [avrdude_path, "-c", "flip1", "-p", "usb1286", "-U", "flash:w:beef.hex"]
        output = subprocess.check_output(command, universal_newlines=True, stderr=subprocess.STDOUT)
        output_text.delete(1.0, tk.END)
        output_text.insert(tk.END, output)
    except subprocess.CalledProcessError as e:
        output_text.delete(1.0, tk.END)
        output_text.insert(tk.END, f"Error: {e.output}")

root = tk.Tk()
root.title("BEEF BOARD FW UPDATE TOOL")

output_text = tk.Text(root, wrap=tk.WORD)
output_text.pack(expand=True, fill=tk.BOTH)

button = tk.Button(root, text="UPDATE FW", command=run_avrdude)
button.pack()

root.mainloop()
