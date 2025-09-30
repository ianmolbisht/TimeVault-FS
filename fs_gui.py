import subprocess
import tkinter as tk
from tkinter import ttk, messagebox
import os
import re
import shutil


BASE_DIR = os.path.dirname(os.path.abspath(__file__)) if "__file__" in globals() else os.getcwd()
C_EXE_PATH = os.path.join(BASE_DIR, "my_fs.exe")      # executable expected next to gui.py
DATA_FOLDER = os.path.join(BASE_DIR, "data")          # active files
SNAPSHOT_ROOT = os.path.join(BASE_DIR, "snapshots") 

os.makedirs(DATA_FOLDER, exist_ok=True)
os.makedirs(SNAPSHOT_ROOT, exist_ok=True)

def run_command(command):
    """Send command to the C executable via subprocess."""
    try:
        result = subprocess.run([C_EXE_PATH], input=command + "\nexit\n",
                                text=True, capture_output=True)
        output_text.delete(1.0, tk.END)
        output_text.insert(tk.END, result.stdout)
        # refresh_file_list()
    except Exception as e:
        messagebox.showerror("Error", str(e))


def ensure_snapshot_dir_for_file(filename):
    """Return path to snapshots folder for this file; create if missing."""
    folder = os.path.join(SNAPSHOT_ROOT, filename)
    os.makedirs(folder, exist_ok=True)
    return folder

def next_snapshot_name(filename):
    """Return next incremental snapshot filename (file1_snap1.txt, file1_snap2.txt, etc.)."""
    base, ext = os.path.splitext(filename)
    folder = ensure_snapshot_dir_for_file(filename)
    pattern = re.compile(re.escape(base) + r"_snap(\d+)" + re.escape(ext) + r"$")
    max_n = 0
    for f in os.listdir(folder):
        m = pattern.match(f)
        if m:
            try:
                n = int(m.group(1))
                if n > max_n:
                    max_n = n
            except:
                pass
    return f"{base}_snap{max_n + 1}{ext}"

def populate_snapshot_list_for_file(filename):
    """Populate snapshot listbox for selected file."""
    snapshot_list.delete(0, tk.END)
    folder = os.path.join(SNAPSHOT_ROOT, filename)
    if os.path.isdir(folder):
        for f in sorted(os.listdir(folder)):
            snapshot_list.insert(tk.END, f)


def save_snapshot_for_selected_file():
    filename = entry_snap_file.get().strip()
    if not filename:
        messagebox.showerror("Error", "Enter a file name first.")
        return
    data_path = os.path.join(DATA_FOLDER, filename)
    if not os.path.isfile(data_path):
        messagebox.showerror("Error", f"{filename} not found in data/.")
        return
    snapshot_folder = ensure_snapshot_dir_for_file(filename)
    snap_name = next_snapshot_name(filename)
    snap_path = os.path.join(snapshot_folder, snap_name)
    shutil.copy2(data_path, snap_path)
    populate_snapshot_list_for_file(filename)
    output_text.delete(1.0, tk.END)
    output_text.insert(tk.END, f"Snapshot saved: {snap_name}")




def restore_snapshot_for_selected():
    filename = entry_snap_file.get().strip()
    snap_sel = snapshot_list.curselection()
    if not filename or not snap_sel:
        messagebox.showerror("Error", "Enter file name and select a snapshot.")
        return
    snap_name = snapshot_list.get(snap_sel[0])
    snap_path = os.path.join(SNAPSHOT_ROOT, filename, snap_name)
    data_path = os.path.join(DATA_FOLDER, filename)
    shutil.copy2(snap_path, data_path)
    # refresh_file_list()
    output_text.delete(1.0, tk.END)
    output_text.insert(tk.END, f"Restored {filename} from {snap_name}")

# refresh_file_list()


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

# Buttons
tk.Button(tab_snapshot, text="Save Snapshot", command=save_snapshot_for_selected_file, **button_style).grid(row=0, column=0, padx=5, pady=5)
tk.Button(tab_snapshot, text="Restore Snapshot", command=restore_snapshot_for_selected, **button_style).grid(row=0, column=1, padx=5, pady=5)


#FILE ENTRY
tk.Label(tab_snapshot, text="Enter File Name:", bg="#1e1e2f", fg="white").grid(row=1, column=0, padx=5, pady=5, sticky="w")
entry_snap_file = tk.Entry(tab_snapshot, width=40)
entry_snap_file.grid(row=1, column=1, padx=5, pady=5)

tk.Button(tab_snapshot, text="Show Snapshots", 
          command=lambda: populate_snapshot_list_for_file(entry_snap_file.get()), 
          **button_style).grid(row=1, column=2, padx=5, pady=5)


# Snapshot list
tk.Label(tab_snapshot, text="Snapshots for selected file:", bg="#1e1e2f", fg="white").grid(row=3, column=0, padx=5, pady=5, sticky="w")
snapshot_list = tk.Listbox(tab_snapshot, width=50, height=10, bg="#2e2e3e", fg="white")
snapshot_list.grid(row=4, column=0, columnspan=4, padx=5, pady=5)


# file_list.bind("<<ListboxSelect>>", on_file_select)

root.mainloop()
