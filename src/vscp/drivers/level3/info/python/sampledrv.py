#!/usr/bin/env python
import os
import sys
import socket
import threading
import time

exitFlag = 0

# https://www.tutorialspoint.com/python/python_multithreading.htm
class myThread (threading.Thread):
   def __init__(self, threadID, name, counter):
      threading.Thread.__init__(self)
      self.threadID = threadID
      self.name = name
      self.counter = counter
   def run(self):
      print "Starting " + self.name
      # Get lock to synchronize threads
      threadLock.acquire()
      print_time(self.name, 5, self.counter)
      # Free lock to release next thread
      threadLock.release()
      print "Exiting " + self.name

def print_time(threadName, counter, delay):
   while counter:
      if exitFlag:
         threadName.exit()
      time.sleep(delay)
      print "%s: %s" % (threadName, time.ctime(time.time()))
      counter -= 1

TCP_IP = '127.0.0.1'
TCP_PORT = 5005
BUFFER_SIZE = 1024

threadLock = threading.Lock()
threads = []

#fpid = os.fork()
#if fpid!=0:
#  # Running as daemon now. PID is fpid
#  sys.exit(0)


s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
s.bind( ( TCP_IP, TCP_PORT ) )
s.listen( 1 )

conn, addr = s.accept()
print 'Connection address:', addr
while 1:
    data = conn.recv(BUFFER_SIZE)
    if not data: break
    data  = data.strip()
    print "received data:[", data, "]"
    conn.send(data)  # echo
    if data[:5] == "start":
        print "*START*"
        # Create new threads
        thread1 = myThread(1, "Thread-1", 1)
        thread2 = myThread(2, "Thread-2", 2)

        # Start new Threads
        thread1.start()
        thread2.start()

        # Add threads to thread list
        threads.append(thread1)
        threads.append(thread2)
    elif data == "stop":
        print "*STOP*"
        # Wait for all threads to complete
        for t in threads:
            t.join()
        break
    elif data == "pause":
        print "*PAUSE*"
    elif data == "resume":
        print "RESUME"
    else:
        print "no match"
conn.close()
