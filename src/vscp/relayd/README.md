# relayd - VSCP Level I/CANAL relay

This software connectes a Level I /CANAL driver and a VSCP daemon using the VSCP Link protocol. 


relayd [options] -d"driver-path" -c"configstr" -f"flags" 

-s --standalone - Don't run in daemon mode (default: daemon).
-p --port       - Port to use (default: 9598).
-i --interface  - Interface to listen on (default: all).
-v --verbose    - Give extra info.
-d --driver     - Path to driver (mandatory).
-c --config     - Configuration string for driver (default:empty).
-z --flags      - Flags for driver (default: 0).
-f --filter     - Filter (default: 0).
-m --mask       - Mask (default: 0).
-? --help       - Give some help.
