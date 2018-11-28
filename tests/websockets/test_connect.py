#!/usr/bin/env python

# WS client example  (Depens on python3)
# Original from: https://websockets.readthedocs.io/en/stable/intro.html

import asyncio
import websockets
import base64
import hashlib
from Crypto import Random
from Crypto.Cipher import AES

async def connect():
    async with websockets.connect(
            'ws://localhost:8884/ws1') as websocket:
#        name = input("What's your name? ")

#        await websocket.send(name)
#        print(f"> {name}")

        greeting = await websocket.recv()
        print(f"Response from server: {greeting}")

        await websocket.send("C;AUTH;5a475c082c80dcdf7f2dfbd976253b24;69b1180d2f4809d39be34e19c750107f")
        greeting = await websocket.recv()
        print(f"Response from server: {greeting}")

asyncio.get_event_loop().run_until_complete( connect() )
