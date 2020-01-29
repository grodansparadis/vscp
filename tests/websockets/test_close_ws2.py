#!/usr/bin/env python

# VSCP ws2 client example  (Need python3)
# Original from: https://websockets.readthedocs.io/en/stable/intro.html

import asyncio
import pathlib
import ssl
import websockets
import base64
import json
import time
#import hashlib
#from Crypto import Random
#from Crypto.Cipher import AES

async def connect():
    async with websockets.connect(
                 'ws://localhost:8884/ws2', ping_interval=20, ping_timeout=20, close_timeout=100) as websocket:

        # Get initial server response
        response = await websocket.recv()
        print(f"Initial response from server: {response}")

        # Log in as admin user

        cmdauth = {
            "type": "cmd",
            "command": "auth",
            "args": {
               "iv":"5a475c082c80dcdf7f2dfbd976253b24",
               "crypto": "69b1180d2f4809d39be34e19c750107f"
            }
        }

        print("Logging in as admin user")
        await websocket.send(json.dumps(cmdauth))
        response = await websocket.recv()
        print(f"Response from server: {response}")

        # Open Channel

        cmdopen = {
            "type": "cmd",
            "command": "open",
            "args": None
        }

        print(f"Open channel  {json.dumps(cmdopen)}")
        await websocket.send(json.dumps(cmdopen))
        response = await websocket.recv()
        print(f"Response from server: {response}")

        print("Waiting for three seconds.")
        time.sleep(3)

        # Close Channel

        cmdopen = {
            "type": "cmd",
            "command": "close",
            "args": None
        }

        print(f"Close channel  {json.dumps(cmdopen)}")
        await websocket.send(json.dumps(cmdopen))
        response = await websocket.recv()
        print(f"Response from server: {response}")

        await websocket.close()

asyncio.get_event_loop().run_until_complete( connect() )
