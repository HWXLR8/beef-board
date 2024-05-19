import tkinter as tk
import subprocess

def run_avrdude():
    try:
        command = ["avrdude/avrdude.exe", "-c", "flip1", "-p", "usb1286", "-U", "flash:w:fw.hex"]
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

