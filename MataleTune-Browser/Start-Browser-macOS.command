#!/bin/sh
cd "$(dirname "$0")" || exit 1
if command -v python3 >/dev/null 2>&1; then
    python3 Start-Browser.py
else
    echo "Python 3 is required. Install Python 3, then run this file again."
fi
printf '\nPress Enter to close.'
read -r reply
