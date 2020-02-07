#!/usr/bin/env python

# VSCP ws2 client example  (Need python3)
# Demonstrates the use of the ws2 websocket interface of the VSCP daemon
# Sample event to send
#    send 0,20,3,,,0,-,15,14,13,12,11,10,9,8,7,6,5,4,3,2,0,0,1,35
# Original sample from: https://websockets.readthedocs.io/en/stable/intro.html
# Copyright 2020 Ake Hedman, Grodans Paradis AB - MIT license

from signal import signal, SIGINT
from sys import exit
import asyncio
import pathlib
import ssl
import websockets
import base64
import json

def handler(signal_received, frame):
    print('SIGINT or CTRL-C detected. Exiting')
    exit(0)

async def connect():
    async with websockets.connect(
                 'ws://localhost:8884/ws2', ping_interval=20, ping_timeout=20, close_timeout=100) as websocket:

        # Get initial server response
        response = await websocket.recv()
        wsrply = json.loads(response)
        print(f"Initial response from server: {json.dumps(wsrply, indent=2)}")

        # Log in as admin user

        cmdauth = {
            "type": "cmd",
            "command": "auth",
            "args": {
               "iv":"5a475c082c80dcdf7f2dfbd976253b24",
               "crypto": "69b1180d2f4809d39be34e19c750107f"
            }
        }

        print("\nLogging in as admin user")
        await websocket.send(json.dumps(cmdauth))
        response = await websocket.recv()
        wsrply = json.loads(response)
        print(f"Response from server: {json.dumps(wsrply, indent=2)}")

        # NOOP

        cmdnoop = {
            "type": "cmd",
            "command": "noop",
            "args": None
        }

        print("\nNOOP command")
        await websocket.send(json.dumps(cmdnoop))
        response = await websocket.recv()
        wsrply = json.loads(response)
        print(f"Response from server: {json.dumps(wsrply, indent=2)}")

        # VERSION

        cmdver = {
            "type": "cmd",
            "command": "version",
            "args": None
        }

        print("\nVERSION command")
        await websocket.send(json.dumps(cmdver))
        response = await websocket.recv()
        wsrply = json.loads(response)
        print(f"Response from server: {json.dumps(wsrply, indent=2)}")

        # COPYRIGHT

        cmdcopy = {
            "type": "cmd",
            "command": "copyright",
            "args": None
        }

        print("\nCOPYRIGHT command")
        await websocket.send(json.dumps(cmdcopy))
        response = await websocket.recv()
        wsrply = json.loads(response)
        print(f"Response from server: {json.dumps(wsrply, indent=2)}")

        # Open Channel

        cmdopen = {
            "type": "cmd",
            "command": "open",
            "args": None
        }

        print(f"\nOpen channel  {json.dumps(cmdopen)}")
        await websocket.send(json.dumps(cmdopen))
        response = await websocket.recv()
        wsrply = json.loads(response)
        print(f"Response from server: {json.dumps(wsrply, indent=2)}")



        # Send event  - CLASS1.CONTROL, TurnOn
        sendEvent = {
            "type": "event",
            "event" : {
                "head" : 0,
                "obid": 0,
                "datetime": "2020-01-29T23:05:59Z",
                "timestamp": 0,
                "class": 30,
                "type": 5,
                "guid": "FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00:00:02:00:00",
                "data": [1,2,3,4,5,6]
            }
        }

        print(f"\nSend event  {json.dumps(sendEvent)}")
        await websocket.send(json.dumps(sendEvent))
        response = await websocket.recv()
        wsrply = json.loads(response)
        print(f"Response from server: {json.dumps(wsrply, indent=2)}")

        # Set filter
        # Receive only CLASS1.CONTROL, TurnOn
        cmdFilter = {
            "type": "cmd",
            "command": "setfilter",
            "args": {
                "mask_priority": 0,
                "mask_class": 65535,
                "mask_type": 65535,                                                     
                "mask_guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",                                               
                "filter_priority": 0,                                             
                "filter_class": 30,                                                  
                "filter_type": 5,                                                   
                "filter_guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
            }
        }

        print(f"\nSet filter  {json.dumps(cmdFilter)}")
        await websocket.send(json.dumps(cmdFilter))
        response = await websocket.recv()
        wsrply = json.loads(response)
        print(f"Response from server: {json.dumps(wsrply, indent=2)}")

        print("Waiting for class=30, Type=5 as events (Active FILTER) (Abort with ctrl+c)")
        while True:
            response = await websocket.recv()
            wsrply = json.loads(response)
            print(f"Response from server: {json.dumps(wsrply, indent=2)}")
            e = wsrply["event"] # Get event
            # get without extracted event
            print(f"VSCP Class={wsrply['event']['class']}")
            # use extracted event 
            print(f"VSCP Type={e['type']}")
            # Event can be edited and changed
            e['type'] = 99
            print(f"Modified event: {json.dumps(e, indent=2)}")

if __name__ == '__main__':
    # Tell Python to run the handler() function when SIGINT is recieved
    signal(SIGINT, handler)
    asyncio.get_event_loop().run_until_complete( connect() )
