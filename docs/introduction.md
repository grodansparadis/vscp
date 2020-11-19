# Introduction to the VSCP Daemon Software

There are so many capabilities of the VSCP daemon so even how many words we ever write we will only just touch on it's capabilities, and even if we managed to describe all features they will still be extended by you and others with drivers and other things to give the software new amazing capabilities. It's a moving target.

Yes and it's **open source**, **free to download** and **free to modify**. And yes it is also **free to use**. And it will stay free forever.

The VSCP daemon is like a HUB. It can connect to other VSCP servers and hardware nodes forming larger systems. It can connect to your devices. They may be VSCP enabled devices or not. The driver system of the daemon served as a hardware abstraction layer for anything that is out there. Two sorts of drivers are available. **Level I** and **Level II** with slightly different capabilities. It is easy to construct new drivers and there are plenty of sample code on how to do it available.

The VSCP daemon handles **variables**. Variables is just persistent typed objects that can be reached and/or read/updated from remote interfaces or when things happen in the system. A typical variable can be the state of a physical thing out in the world. The "thing" updates it's status from time to time and then also updates the variable. A variable may also be the energy consumption over a mount or a year. Variables can be viewed as a sort of database but with discrete values.

The daemon exports a **TCP/IP interface** that other services and humans can connect to and and perform tasks on a VSCP daemon system from a remote location. The interface is protected with user/password credentials and a system that only allows host you specify to connect. On top of this SSL can also be enabled for the interface giving it **state of the art security**.

The VSCP daemon have a full **web server** that can server users with local web pages as any other web seserver but also have a built in configuration interface to remotely manage the VSCP daemon. Also this server have the same security as the tcp/ip interface. It is even possible to extend it with cgi scripts.

A **REST interface** is available on the daemon that can be used by clients to log and read information and to control and be controlled. SSL, username/password and limited remote host capabilities are enabled also here.

The **websocket interface** can be used to build beautiful dynamic web pages for presenting and interacting with devices. Suddenly this becomes a very simple task. Same security level is available here as for tcp/ip/web and rest interfaces.

A local **scheduler** makes it possible to construct build advanced control systems. It is combined with a **decision matrix** which can be programmed to perform things when other things happen. Typical samples are 

 * Send an event when something happen
 * execute an external program when something happens.
 * Log data to a table that later can be displayed as a dynamic diagram or a table on a web page.
 * Handle variables.
 * deliver/collect data to/from a remote web site.

And there is more as we said above...






[filename](./bottom_copyright.md ':include')
