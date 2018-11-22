#!/usr/bin/env python

# WS client example
# Original from: https://websockets.readthedocs.io/en/stable/intro.html

import asyncio
import websockets

async def connect():
    async with websockets.connect(
            'ws://localhost:8884/ws1') as websocket:
        name = input("What's your name? ")

        await websocket.send(name)
        print(f"> {name}")

        greeting = await websocket.recv()
        print(f"< {greeting}")

asyncio.get_event_loop().run_until_complete( connect() )
