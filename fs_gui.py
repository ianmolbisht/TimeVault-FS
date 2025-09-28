import subprocess
import tkinter as tk
from tkinter import ttk, messagebox
import os

# Path to your C executable
C_EXE_PATH = r"E:\TimeVaultFS\my_fs.exe"
DATA_FOLDER = r"E:\TimeVaultFS\data"

def run_command(command):
    """Send command to the C executable via subprocess."""
    try:
        result = subprocess.run([C_EXE_PATH], input=command + "\nexit\n",
                                text=True, capture_output=True)
        output_text.delete(1.0, tk.END)
        output_text.insert(tk.END, result.stdout)
        refresh_file_list()
    except Exception as e:
        messagebox.showerror("Error", str(e))

def refresh_file_list():
    """Refresh the list of files in the data folder."""
    file_list.delete(0, tk.END)
    if os.path.exists(DATA_FOLDER):
        for f in os.listdir(DATA_FOLDER):
            path = os.path.join(DATA_FOLDER, f)
            if os.path.isfile(path):
                file_list.insert(tk.END, f)

# GUI root
root = tk.Tk()
root.title("TimeVaultFS Manager")
root.configure(bg="#1e1e2f")

# Center the window
window_width = 700
window_height = 500
screen_width = root.winfo_screenwidth()
screen_height = root.winfo_screenheight()
x_cordinate = int((screen_width/2) - (window_width/2))
y_cordinate = int((screen_height/2) - (window_height/2))
root.geometry(f"{window_width}x{window_height}+{x_cordinate}+{y_cordinate}")

# Tabs
tab_control = ttk.Notebook(root)
tab_control.pack(expand=1, fill="both", padx=10, pady=10)

# --- File Operations Tab ---
tab_file = ttk.Frame(tab_control)
tab_control.add(tab_file, text="File Operations")

tk.Label(tab_file, text="File Name:", bg="#1e1e2f", fg="white").grid(row=0, column=0, padx=5, pady=5, sticky="w")
entry_file = tk.Entry(tab_file, width=30)
entry_file.grid(row=0, column=1, padx=5, pady=5, sticky="w")

tk.Label(tab_file, text="Data:", bg="#1e1e2f", fg="white").grid(row=1, column=0, padx=5, pady=5, sticky="w")
entry_data = tk.Entry(tab_file, width=30)
entry_data.grid(row=1, column=1, padx=5, pady=5, sticky="w")

# Buttons
button_style = {"width": 15, "bg": "#ff5555", "fg": "white", "activebackground": "#ff8888"}

tk.Button(tab_file, text="Create File", command=lambda: run_command(f"create {entry_file.get()}"), **button_style).grid(row=2, column=0, padx=5, pady=5)
tk.Button(tab_file, text="Write File", command=lambda: run_command(f"write {entry_file.get()} {entry_data.get()}"), **button_style).grid(row=2, column=1, padx=5, pady=5)
tk.Button(tab_file, text="Read File", command=lambda: run_command(f"read {entry_file.get()}"), **button_style).grid(row=2, column=2, padx=5, pady=5)
tk.Button(tab_file, text="Delete File", command=lambda: run_command(f"delete {entry_file.get()}"), **button_style).grid(row=2, column=3, padx=5, pady=5)
tk.Button(tab_file, text="Rollback", command=lambda: run_command("rollback"), **button_style).grid(row=3, column=0, padx=5, pady=5)
tk.Button(tab_file, text="Exit", command=root.quit, **button_style).grid(row=3, column=3, padx=5, pady=5)

# Output box
output_text = tk.Text(tab_file, height=10, width=80, bg="#2e2e3e", fg="white")
output_text.grid(row=4, column=0, columnspan=4, pady=10, padx=5)

# --- Snapshot Tab ---
tab_snapshot = ttk.Frame(tab_control)
tab_control.add(tab_snapshot, text="Snapshots")

tk.Label(tab_snapshot, text="Snapshot Name:", bg="#1e1e2f", fg="white").grid(row=0, column=0, padx=5, pady=5, sticky="w")
entry_snapshot = tk.Entry(tab_snapshot, width=30)
entry_snapshot.grid(row=0, column=1, padx=5, pady=5, sticky="w")

tk.Button(tab_snapshot, text="Save Snapshot", command=lambda: run_command(f"snapshot {entry_snapshot.get()}"), **button_style).grid(row=1, column=0, padx=5, pady=5)
tk.Button(tab_snapshot, text="Restore Snapshot", command=lambda: run_command(f"restore {entry_snapshot.get()}"), **button_style).grid(row=1, column=1, padx=5, pady=5)

# File list
tk.Label(tab_snapshot, text="Files in data/:", bg="#1e1e2f", fg="white").grid(row=2, column=0, padx=5, pady=5, sticky="w")
file_list = tk.Listbox(tab_snapshot, width=50, height=15, bg="#2e2e3e", fg="white")
file_list.grid(row=3, column=0, columnspan=4, padx=5, pady=5)

refresh_file_list()

root.mainloop()
