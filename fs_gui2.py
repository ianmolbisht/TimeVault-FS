import subprocess
import tkinter as tk
from tkinter import ttk, messagebox
import os
import re
import shutil

BASE_DIR = os.path.dirname(os.path.abspath(__file__)) if "__file__" in globals() else os.getcwd()
C_EXE_PATH = os.path.join(BASE_DIR, "my_fs.exe")
DATA_FOLDER = os.path.join(BASE_DIR, "data")
SNAPSHOT_ROOT = os.path.join(BASE_DIR, "snapshots")

os.makedirs(DATA_FOLDER, exist_ok=True)
os.makedirs(SNAPSHOT_ROOT, exist_ok=True)

def run_command(command):
    try:
        result = subprocess.run([C_EXE_PATH], input=command + "\nexit\n", text=True, capture_output=True)
        output_text.delete(1.0, tk.END)
        output_text.insert(tk.END, result.stdout)
    except Exception as e:
        messagebox.showerror("Error", str(e))

def ensure_snapshot_dir_for_file(filename):
    folder = os.path.join(SNAPSHOT_ROOT, filename)
    os.makedirs(folder, exist_ok=True)
    return folder

def next_snapshot_name(filename):
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
    run_command(f"snapshot {filename}")
    populate_snapshot_list_for_file(filename)

def restore_snapshot_for_selected():
    filename = entry_snap_file.get().strip()
    snap_sel = snapshot_list.curselection()
    if not filename or not snap_sel:
        messagebox.showerror("Error", "Enter file name and select a snapshot.")
        return
    snap_name = snapshot_list.get(snap_sel[0])
    run_command(f"restore {snap_name}")

def rollback_latest_snapshot():
    filename = entry_file.get().strip()
    if not filename:
        messagebox.showerror("Error", "Enter a file name first.")
        return
    run_command(f"restore {filename}")

root = tk.Tk()
root.title("TimeVaultFS Manager")
root.configure(bg="#1e1e2f")
window_width = 700
window_height = 500
screen_width = root.winfo_screenwidth()
screen_height = root.winfo_screenheight()
x_cordinate = int((screen_width/2) - (window_width/2))
y_cordinate = int((screen_height/2) - (window_height/2))
root.geometry(f"{window_width}x{window_height}+{x_cordinate}+{y_cordinate}")

tab_control = ttk.Notebook(root)
tab_control.pack(expand=1, fill="both", padx=10, pady=10)

tab_file = ttk.Frame(tab_control)
tab_control.add(tab_file, text="File Operations")

tk.Label(tab_file, text="File Name:", bg="#1e1e2f", fg="white").grid(row=0, column=0, padx=5, pady=5, sticky="w")
entry_file = tk.Entry(tab_file, width=30)
entry_file.grid(row=0, column=1, padx=5, pady=5, sticky="w")

tk.Label(tab_file, text="Data:", bg="#1e1e2f", fg="white").grid(row=1, column=0, padx=5, pady=5, sticky="w")
entry_data = tk.Entry(tab_file, width=30)
entry_data.grid(row=1, column=1, padx=5, pady=5, sticky="w")

button_style = {"width": 15, "bg": "#ff5555", "fg": "white", "activebackground": "#ff8888"}

tk.Button(tab_file, text="Create File", command=lambda: run_command(f"create {entry_file.get()}"), **button_style).grid(row=2, column=0, padx=5, pady=5)
tk.Button(tab_file, text="Write File", command=lambda: run_command(f"write {entry_file.get()} {entry_data.get()}"), **button_style).grid(row=2, column=1, padx=5, pady=5)
tk.Button(tab_file, text="Read File", command=lambda: run_command(f"read {entry_file.get()}"), **button_style).grid(row=2, column=2, padx=5, pady=5)
tk.Button(tab_file, text="Delete File", command=lambda: run_command(f"delete {entry_file.get()}"), **button_style).grid(row=2, column=3, padx=5, pady=5)
tk.Button(tab_file, text="File Info", command=lambda: run_command(f"info {entry_file.get()}"), **button_style).grid(row=3, column=0, padx=5, pady=5)
tk.Button(tab_file, text="Lock File", command=lambda: run_command(f"lock {entry_file.get()}"), **button_style).grid(row=3, column=1, padx=5, pady=5)
tk.Button(tab_file, text="Unlock File", command=lambda: run_command(f"unlock {entry_file.get()}"), **button_style).grid(row=3, column=2, padx=5, pady=5)
tk.Button(tab_file, text="Compress", command=lambda: run_command(f"compress {entry_file.get()}"), **button_style).grid(row=3, column=3, padx=5, pady=5)
tk.Button(tab_file, text="Rollback", command=rollback_latest_snapshot, **button_style).grid(row=4, column=0, padx=5, pady=5)
tk.Button(tab_file, text="Exit", command=root.quit, **button_style).grid(row=4, column=3, padx=5, pady=5)

output_text = tk.Text(tab_file, height=10, width=80, bg="#2e2e3e", fg="white")
output_text.grid(row=5, column=0, columnspan=4, pady=10, padx=5)

tab_snapshot = ttk.Frame(tab_control)
tab_control.add(tab_snapshot, text="Snapshots")

tk.Button(tab_snapshot, text="Save Snapshot", command=save_snapshot_for_selected_file, **button_style).grid(row=0, column=0, padx=5, pady=5)
tk.Button(tab_snapshot, text="Restore Snapshot", command=restore_snapshot_for_selected, **button_style).grid(row=0, column=1, padx=5, pady=5)

tk.Label(tab_snapshot, text="Enter File Name:", bg="#1e1e2f", fg="white").grid(row=1, column=0, padx=5, pady=5, sticky="w")
entry_snap_file = tk.Entry(tab_snapshot, width=40)
entry_snap_file.grid(row=1, column=1, padx=5, pady=5)

tk.Button(tab_snapshot, text="Show Snapshots", command=lambda: populate_snapshot_list_for_file(entry_snap_file.get()), **button_style).grid(row=1, column=2, padx=5, pady=5)

tk.Label(tab_snapshot, text="Keep N snapshots:", bg="#1e1e2f", fg="white").grid(row=2, column=0, padx=5, pady=5, sticky="w")
entry_keep_count = tk.Entry(tab_snapshot, width=10)
entry_keep_count.insert(0, "5")
entry_keep_count.grid(row=2, column=1, padx=5, pady=5, sticky="w")
tk.Button(tab_snapshot, text="Cleanup Snapshots", command=lambda: run_command(f"cleanup-snapshots {entry_snap_file.get()} {entry_keep_count.get()}"), **button_style).grid(row=2, column=2, padx=5, pady=5)

tk.Label(tab_snapshot, text="Snapshots for selected file:", bg="#1e1e2f", fg="white").grid(row=3, column=0, padx=5, pady=5, sticky="w")
snapshot_list = tk.Listbox(tab_snapshot, width=50, height=10, bg="#2e2e3e", fg="white")
snapshot_list.grid(row=4, column=0, columnspan=4, padx=5, pady=5)

# Search Tab
tab_search = ttk.Frame(tab_control)
tab_control.add(tab_search, text="Search & Tools")

tk.Label(tab_search, text="Search Pattern:", bg="#1e1e2f", fg="white").grid(row=0, column=0, padx=5, pady=5, sticky="w")
entry_search_pattern = tk.Entry(tab_search, width=40)
entry_search_pattern.grid(row=0, column=1, padx=5, pady=5)

search_output = tk.Text(tab_search, height=15, width=80, bg="#2e2e3e", fg="white")
search_output.grid(row=2, column=0, columnspan=4, pady=10, padx=5)

def run_search_command(command):
    try:
        result = subprocess.run([C_EXE_PATH], input=command + "\nexit\n", text=True, capture_output=True)
        search_output.delete(1.0, tk.END)
        search_output.insert(tk.END, result.stdout)
    except Exception as e:
        messagebox.showerror("Error", str(e))

tk.Button(tab_search, text="Search by Name", command=lambda: run_search_command(f"search {entry_search_pattern.get()}"), **button_style).grid(row=1, column=0, padx=5, pady=5)
tk.Button(tab_search, text="Search by Content", command=lambda: run_search_command(f"grep {entry_search_pattern.get()}"), **button_style).grid(row=1, column=1, padx=5, pady=5)
tk.Button(tab_search, text="List All Files", command=lambda: run_search_command("list-files"), **button_style).grid(row=1, column=2, padx=5, pady=5)
tk.Button(tab_search, text="List Locked Files", command=lambda: run_search_command("list-locks"), **button_style).grid(row=1, column=3, padx=5, pady=5)

root.mainloop()
