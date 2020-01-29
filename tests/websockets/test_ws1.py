#!/usr/bin/env python

# VSCP ws1 client example  (Need python3)
# Demonstrates the use of the ws1 websocket interface of the VSCP daemon
# Sample event to send
#    send 0,20,3,,,0,-,15,14,13,12,11,10,9,8,7,6,5,4,3,2,0,0,1,35
# Original sample from: https://websockets.readthedocs.io/en/stable/intro.html

from signal import signal, SIGINT
from sys import exit
import asyncio
import pathlib
import ssl
import websockets
import base64

def handler(signal_received, frame):
    print('SIGINT or CTRL-C detected. Exiting')
    exit(0)

async def connect():
    async with websockets.connect(
                 'ws://localhost:8884/ws1', ping_interval=None, ping_timeout=1, close_timeout=100) as websocket:

        # Request challenge
        await websocket.send("C;CHALLENGE")

        greeting = await websocket.recv()
        print(f"Response from server: {greeting}")

        # Log in as admin user
        print("Logging in as admin user")
        await websocket.send("C;AUTH;5a475c082c80dcdf7f2dfbd976253b24;69b1180d2f4809d39be34e19c750107f")
        greeting = await websocket.recv()
        print(f"Response from server: {greeting}")

        # Open Channel
        await websocket.send("C;OPEN")
        greeting = await websocket.recv()
        print(f"Response from server: {greeting}")

        while True:
            greeting = await websocket.recv()
            print(f"Response from server: {greeting}")

if __name__ == '__main__':
    # Tell Python to run the handler() function when SIGINT is recieved
    signal(SIGINT, handler)
    asyncio.get_event_loop().run_until_complete( connect() )