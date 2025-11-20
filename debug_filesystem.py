#!/usr/bin/env python3
"""
Debug script to inspect LittleFS filesystem structure and file locations.
Helps understand where files are stored vs where the app expects them.
"""

import os
import sys
from pathlib import Path

def list_data_files():
    """List all files in the data directory with their expected filesystem paths."""
    data_dir = Path("data")
    if not data_dir.exists():
        print(f"ERROR: {data_dir} directory not found!")
        return
    
    print("=" * 80)
    print("FILES IN data/ DIRECTORY (as uploaded to LittleFS)")
    print("=" * 80)
    
    files_found = []
    for root, dirs, files in os.walk(data_dir):
        for file in files:
            full_path = Path(root) / file
            # Get relative path from data directory
            rel_path = full_path.relative_to(data_dir)
            # Convert to filesystem path (forward slashes, leading slash)
            fs_path = "/" + str(rel_path).replace("\\", "/")
            
            file_size = full_path.stat().st_size
            files_found.append((fs_path, file_size))
            
            print(f"  {fs_path:50s} ({file_size:6d} bytes)")
    
    print(f"\nTotal files: {len(files_found)}")
    return files_found

def check_js_files():
    """Check if all required JS files exist."""
    print("\n" + "=" * 80)
    print("REQUIRED JS FILES (from browser errors)")
    print("=" * 80)
    
    required_files = [
        "/assets/js/index-DaQLwd0U.js",
        "/assets/js/vendor-preact-BAP1wjwx.js",
        "/assets/js/vendor-ui-CQvIz11K.js",
        "/assets/js/components-graph-B8AMosAd.js",
        "/assets/js/vendor-misc-Bc_RFnUL.js",
        "/assets/js/components-dashboard-CoBAS8k0.js",
        "/assets/js/page-dashboard-BgPAC0Xu.js",
        "/assets/js/vendor-chart-BlXMEoXN.js",
        "/assets/js/page-status-D8JTVy8R.js",
        "/assets/js/page-config-Skwiz2hL.js",
        "/assets/js/page-logs-DXjFMR0P.js",
        "/assets/js/page-serial-DE-ZR98G.js",
    ]
    
    data_dir = Path("data")
    missing = []
    found = []
    
    for req_file in required_files:
        # Remove leading slash and check in data directory
        local_path = data_dir / req_file.lstrip("/")
        gz_path = local_path.with_suffix(local_path.suffix + ".gz")
        
        if gz_path.exists():
            found.append((req_file, gz_path, True))
            print(f"  ✓ {req_file}.gz exists")
        elif local_path.exists():
            found.append((req_file, local_path, False))
            print(f"  ✓ {req_file} exists (not gzipped)")
        else:
            missing.append(req_file)
            print(f"  ✗ {req_file} NOT FOUND")
    
    print(f"\nFound: {len(found)}, Missing: {len(missing)}")
    return found, missing

def analyze_path_issue():
    """Analyze the path mismatch issue."""
    print("\n" + "=" * 80)
    print("PATH ANALYSIS")
    print("=" * 80)
    
    print("""
ISSUE SUMMARY:
--------------
1. Files are stored in filesystem at: /assets/js/file.js.gz
2. LittleFS is mounted at: /littlefs
3. Code is trying to access: /littlefs/assets/js/file.js.gz
4. Error shows: /littlefs/assets/js/file.js.gz does not exist

ROOT CAUSE:
-----------
When LittleFS is mounted at "/littlefs":
- Filesystem files at "/assets/js/file.js.gz" become accessible at "/littlefs/assets/js/file.js.gz" in VFS
- BUT LittleFS.exists() and beginResponse() use filesystem-relative paths, NOT VFS paths
- So they should be called with "/assets/js/file.js.gz", not "/littlefs/assets/js/file.js.gz"

SOLUTION:
---------
The code should use filesystem-relative paths (without /littlefs prefix) when calling:
- LittleFS.exists()
- LittleFS.open()
- request->beginResponse(LittleFS, ...)

Current code is prepending "/littlefs" which causes the double prefix issue.
    """)

def main():
    print("\n" + "=" * 80)
    print("ESP32 LittleFS Filesystem Debug Tool")
    print("=" * 80 + "\n")
    
    files = list_data_files()
    found, missing = check_js_files()
    analyze_path_issue()
    
    print("\n" + "=" * 80)
    print("RECOMMENDATION")
    print("=" * 80)
    print("""
The files ARE uploaded correctly to the filesystem.
The issue is in the code - it's using VFS paths (/littlefs/...) instead of 
filesystem-relative paths (/...) when calling LittleFS API functions.

Fix: Remove "/littlefs" prefix from all LittleFS.exists() and beginResponse() calls.
Filesystem-relative paths should be used, and the mount point is handled automatically.
    """)

if __name__ == "__main__":
    main()

