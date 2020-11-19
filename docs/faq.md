# faq

## 'service vscpd start' does not work
Check if */var/run/vscpd/vscpd.pid* is available. If it is, delete it and try to start the service again. 

Use **pgrep vscpd** to see if the VSCP daemon is already running. If it is stop it with **service vscpd stop** or if that does not work **kill -kill 'pid'** where *pid* is the process number you get from pgrep.

## How do I remove the VSCP daemon startup script

Use

> sudo update-rc.d vscpd remove


[filename](./bottom_copyright.md ':include')
