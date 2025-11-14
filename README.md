TimeVault-FS
A version-controlled file system with snapshots, metadata tracking, compression, locks, and search.

Table of Contents

* Features
* Installation
* Usage
* Commands
* Project Structure
* Feature Details
* Examples
* Contributing

Features

Core

* File operations: create, read, write, delete
* Journal logging for audit and rollback
* Snapshot system with delta storage
* Rollback using journal logs

Advanced

* Metadata tracking: creation time, modification time, size
* Search by name or content
* RLE compression
* File locking for write/delete protection
* Snapshot cleanup and retention

Interfaces

* CLI interface
* Optional Python GUI

Installation

Prerequisites

* GCC or Clang
* Python 3 (optional, for GUI)
* Make (optional)

Build

Windows

```
gcc -o my_fs.exe main.c file_manager.c journal_manager.c rollback_controller.c snapshot_manager.c metadata_manager.c search_manager.c compression_manager.c lock_manager.c
```

Linux/Mac

```
gcc -o my_fs main.c file_manager.c journal_manager.c rollback_controller.c snapshot_manager.c metadata_manager.c search_manager.c compression_manager.c lock_manager.c
```

Run

```
my_fs.exe
./my_fs
python fs_gui2.py
```

Usage

Start

```
my_fs.exe
```

Basic commands

```
create file.txt  
write file.txt hello  
read file.txt  
info file.txt  
snapshot file.txt  
help
```

Commands

File

* `create <file>`
* `write <file> <data>`
* `read <file>`
* `delete <file>`

Snapshots

* `snapshot <file>`
* `restore <file>`
* `restore <snapshot>`
* `listsnapshots <file>`
* `cleanup-snapshots <file> N`

Metadata

* `info <file>`
* `list-files`

Search

* `search <pattern>`
* `grep <pattern>`

Compression

* `compress <file>`
* `decompress <file>`

Locking

* `lock <file>`
* `unlock <file>`
* `list-locks`

System

* `rollback`
* `help`
* `exit`

Project Structure

```
TimeVault-FS/
  data/
    .metadata/
    .locks/
  snapshots/
  *.c, *.h source files
  fs_gui2.py
  README.md
```

Feature Details

Snapshots

* First snapshot stores full file
* Later snapshots store deltas
* Useful for append/update workflows

Metadata

* Tracks size, creation time, last modification time

Compression

* RLE encoding
* Compresses only when it reduces size

Locks

* Blocks writes and deletes until unlocked

Search

* Name-based search
* Content-based search

Examples

Workflow

```
create notes.txt
write notes.txt version1
snapshot notes.txt
write notes.txt version2
listsnapshots notes.txt
restore notes.txt
```

Compression

```
write log.txt aaaaaaaaaa
compress log.txt
decompress log.txt
```

Locking

```
lock config.txt
unlock config.txt
```

Snapshot cleanup

```
cleanup-snapshots file.txt 3
```

Troubleshooting

* Add `<time.h>` if `time_t` errors appear
* On Windows, use MinGW-w64 for `dirent.h`
* Compression only works on repetitive data

Academic Context
Operating Systems course project covering file operations, versioning, compression, locking, and search.
