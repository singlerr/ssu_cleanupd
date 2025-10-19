# 🧹 SSU-Cleanupd

## Overview

**SSU-Cleanupd** is a Linux-based automated file management program that periodically monitors user-specified directories, detects newly added files, and organizes them automatically.  

This project involves developing a new shell-style command (`ssu_cleanupd`) using system calls, creating a daemon process, handling standard and file I/O, and implementing directory structures with linked lists. The purpose is to enhance your understanding and application of Linux system programming.

---

## 🎯 Objectives

- Implement a custom command that behaves like a mini shell.
- Create and manage daemon processes using system calls.
- Utilize directory structures and system libraries (e.g., `fcntl`, `scandir`, `realpath`, `exec` functions, etc.).
- Practice concurrent file access synchronization via file locks.
- Improve system programming application and design skills.

---

## ⚙️ Basic Information

- **Executable name:** `ssu_cleanupd`  
- **Platform:** Linux  
- **Allowed paths:** Must be subdirectories within the user’s home directory (`/home/[username]/...`)  
- **Path types:** Both absolute and relative paths are supported  
- **Path limits:**  
  - Max path length: `4096 bytes`  
  - Max file name length: `255 bytes`  
- **Important:** `system()` usage is strictly prohibited (0 points if used).

---

## 🧩 Built-in Commands

`ssu_cleanupd` supports the following internal commands:

| Command | Description |
|----------|--------------|
| `show` | Display currently running daemon processes |
| `add` | Register a new daemon process to monitor and organize a directory |
| `modify` | Modify configuration of a running daemon process |
| `remove` | Unregister (stop) a daemon process |
| `help` | Display command usage |
| `exit` | Exit the program |

---

## 💻 Usage

### Command Prompt Example
When `ssu_cleanupd` starts, a custom prompt is displayed:

```
% ./ssu_cleanupd
ssu_cleanupd>
```

The user can input any of the following built-in commands.

---

## 🔍 1. `show`

**Usage:**  
```
show
```

**Description:**  
Displays all currently running daemon processes and allows the user to view detailed information about each one.

**Output:**  
- Lists daemons from `/.ssu_cleanupd/current_daemon_list`
- Each daemon entry displays:  
  - Monitoring path  
  - PID  
  - Start time  
  - Output directory  
  - Time interval  
  - Excluded paths  
  - Extensions  
  - Mode  
  - Last 10 log entries from `ssu_cleanupd.log`

**Example:**
```
Current working daemon process list
0. exit
1. /home/oslab/test1/A
2. /home/oslab/test1/B
3. /home/oslab/test1/C
Select one to see process info: 1
```

---

## ➕ 2. `add`

**Usage:**  
```
add  [OPTIONS...]
```

**Description:**  
Registers a daemon process that monitors `` and organizes unarranged files into an output directory.

### Options

| Option | Argument | Description | Default |
|---------|-----------|-------------|----------|
| `-d` | `` | Set output directory path | `_arranged` |
| `-i` | `` | Set monitoring interval (seconds) | `10` |
| `-l` | `` | Set maximum log line count | `none` |
| `-x` | `` | Exclude subdirectories (comma-separated) | `none` |
| `-e` | `` | Target specific file extensions (comma-separated) | `all` |
| `-m` | `` | Set duplicate handling mode (1–3) | `1` |

### Mode Reference

| Mode | Behavior |
|-------|-----------|
| 1 | Move the newest file if duplicates exist |
| 2 | Move the oldest file if duplicates exist |
| 3 | Do not move duplicates |

### Files Created

| File | Description |
|------|-------------|
| `ssu_cleanupd.config` | Configuration file (auto-created if not existing) |
| `ssu_cleanupd.log` | Log file with all file transfers |
| `/.ssu_cleanupd/current_daemon_list` | Daemon list file under user’s home directory |

**Log Format Example:**
```
[18:00:30] [277] [/home/oslab/test2/A/1.txt] [/home/oslab/test2/A_output/txt/1.txt]
```

---

## 🛠️ 3. `modify`

**Usage:**  
```
modify  [OPTIONS...]
```

**Description:**  
Modifies configuration (`ssu_cleanupd.config`) of a daemon currently monitoring the specified directory.

**Behavior:**
- Acquires a **file lock** before writing.
- Unspecified options remain unchanged.
- Changes take effect immediately.

---

## 🗑️ 4. `remove`

**Usage:**  
```
remove 
```

Stops and removes the daemon monitoring the given path.

**Example:**
```
ssu_cleanupd> remove test6/A
```

---

## 🧭 5. `help`

**Usage:**  
```
help
```

Prints descriptions of all supported commands.

---

## 🚪 6. `exit`

**Usage:**  
```
exit
```

Terminates the `ssu_cleanupd` program.

---

## 🧰 Recommended System Calls and Functions

The following Linux system APIs are recommended or required for implementation:

| Function | Header | Description |
|-----------|---------|-------------|
| `getopt()` | `` | For parsing command-line arguments |
| `scandir()` | `` | For listing directory contents |
| `realpath()` | `` | Convert relative paths to absolute paths |
| `strtok()` | `` | String tokenization |
| `exec*()` family | `` | Replace process image for daemon creation |
| `fcntl()` | `` | File locking for synchronization (`F_SETLKW`) |

---

## 🧾 Evaluation Criteria

| Component | Description | Score |
|------------|-------------|-------|
| `ssu_cleanupd` main program | Daemon shell environment | 5 |
| `show` command | Display daemon info | 15 |
| `add` command | Register and run daemon | 50 |
| `modify` command | Update running config | 10 |
| `remove` | Terminate daemon | 5 |
| `help` | Print command usage | 5 |
| `exit` | Quit the main program | 5 |
| `Makefile` | Proper build automation | 5 |
| **Total** |  | **100 points** |


---

## 🧠 Notes

- `system()` usage → **0 points**
- Your program must manage all files under the user’s home (`/home/[user]/`)
- Ensure correct handling of daemon lists and file locks
- Each built-in command must correctly handle all specified **exception cases**
- Report must include **overview, functionality, flowcharts, execution results, and commented source code**

---

### Example Directory Structure

```
/home/oslab/test1/
├── A/
│   ├── ssu_cleanupd.config
│   ├── ssu_cleanupd.log
│   └── ...
├── .ssu_cleanupd/
│   └── current_daemon_list
└── ...
```

---

Would you like me to include a short “How to Build and Run” section (Makefile example + run instructions) at the end of the README?  
It can make the README look even more like a real GitHub project.
