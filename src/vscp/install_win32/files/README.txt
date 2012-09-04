VSCP & Friends Version 0.3.3 - Oxygen
==================================================================
This is the 0.3.3 of VSCP & Friends. Some features is still missing 
and there are probably some bugs left so please report all your findings to
bugs@vscp.org

- Eurosource change name to Grodans Paradis AB and this is reflected 
	in copyright messages.
- Improvements of scan interface in VSCP Works.
- Register configurations can now be saved/loaded in VSCP Works.
- Fixed memory leak in tcp/ip interface of VSCP daemon. 

VSCP & Friends Version 0.3.2 - Internal release
==================================================================

VSCP & Friends Version 0.3.1 - Nitrogen
==================================================================
This is the 0.3.1 of VSCP & Friends. Some features is still missing 
and there are probably some bugs left so please report all your findings to
bugs@vscp.org

- fixed. Minimized session windows on start up in vscpworks.
- bug 2870025 fixed. Re-edit of txlines in vscpworks.
- Added DM action escape substitutions for differenet standard system paths. See 15.7.6 for 
					complete list.
- Helper interface now have full variable handling.
- All Level I drivers moved to drivers/level1. Windows build files changed accordingly.
- Timestamp was missing on internal events. Added for all. 
- Fixed bug in %hour substitution in DM action strings.
- Other minor fixes.

VSCP & Friends Version 0.3.0 - Carbon
==================================================================
This is the 0.3.0 of VSCP & Friends. Some features is still missing 
and there are probably some bugs left so please report all your findings to
bugs@vscp.org

Measurement conversions in place. All datatypes  handled and supported.

Helpers to get variables from the server in place. Much easier to read variables from code.

VSCP Driver interface (level II) in place. A new driver model that connect to the VSCP
TCP/IP instead of CANAL. Gives a lot more possibilities.

Level II logger driver in place. Log files can be created that log from specific channels
in textformat or VSCP works XML format for later investigation.

Level II bluetooth proximity driver in place.  Detect bluetooth devices.

Level II raw ethernet driver in place. Support for devices that communicate over ethernet without 
a TCP/IP stack.

Bugfixes.

VSCP & Friends Version 0.2.10 - Boron
==================================================================
This is the 0.2.10 of VSCP & Friends. Some features is still missing 
and there are probably some bugs left so please report all your findings to
bugs@vscp.org

- The internal decision matrix with all actons is now in place. This means
  that the daemon now is a full featured control machine. Read all about it 
  in the specification Part III 15.7	

VSCP & Friends Version 0.2.9 - Beryllium
==================================================================
This is the 0.2.9 of VSCP & Friends. Some features is still missing 
and there are probably some bugs left so please report all your findings to
bugs@vscp.org

- Bootloading moved to VSCP Works. 
- Many many fixes and enhancements.


VSCP & Friends Version 0.2.8 - Lithium
==================================================================
This is the 0.2.8 of VSCP & Friends. Some features is still missing 
and there are probably some bugs left so please report all your findings to
bugs@vscp.org

- Fixed a bug that crached VSCP Works after a register update containing a DM.
- Driver for the Tellstick home automation adapter added. http://www.telldus.se/


VSCP & Friends Version 0.2.7 - The Helium (midsummer) release 2009
==================================================================
This is the 0.2.7 of VSCP & Friends. Some features is still missing 
and there are probably some bugs left so please report all your findings to
bugs@vscp.org

This release add a lot of feature to mainly the VSCP Works program. It
now have a fully functional configuration interface among other things.

Many bugs has also been fixed.





Configuration files
===================

Previous versions of VSCP & Friends installed the vscpservice and set it to startup automatically.
From this version it is set to startup manually and you must use the control panel applet to
either start it or change so iit starts up automatically. You find information about this here
http://www.vscp.org/wiki/doku.php?id=vscpservice

The location for configuration files has been changed. For the daemon and the service the configuration
file is called vscpd.conf and is located in \Documents and Settings\All Users\Application Data\vscp
The format for the file is described here http://www.vscp.org/wiki/doku.php?id=vscp_daemon_configuration_file 
and here http://www.vscp.org/wiki/doku.php?id=configuration_file_walkthrough

For VSCP Works the configuration file is located in \Documents and Settings\user\Application Data\vscpworks
where user is the user that did the installation. The configuration file content is described here 
http://www.vscp.org/wiki/doku.php?id=vscp_works_configuration_file_format


USe the daemon or the service  
=============================

You can run the server in two ways on Windows. Either you can run it as an ordinary program and
as such it will open a console window and output some diagnostic information. Just clock on close to 
end the program. This is the best choice if you want to test the system.

You can also run the server as a service, described here http://www.vscp.org/wiki/doku.php?id=vscpservice, 
and this is the best choice for a final installation.


Test the installation
=====================

To test the installation either start the daemon or the service. Then try a telnet session to the 
server. Open a console window and type

telnet localhost 9598
user admin
pass secret
quit

You should see the server answere if everything works. The admin/secret usere credentials works if
you used the default configuration file that comes with the installation.

If this works try to start the VSCP Works client and connect two sessions to the server. 

Further information
===================
Information about the package is available at http://www.vscp.org and 
http://www.vscp.org/wiki/doku.php?id=canal_vscp_applications  The information available here will
grow over type and please feel free to add information here. Also consider joining the can-vscp
mailing list https://lists.sourceforge.net/lists/listinfo/m2m-development.
The archive is at http://sourceforge.net/mailarchive/forum.php?forum_name=m2m-development


Los, Sweden, 2010-05-04
Åke Hedman, <akhe@eurosource.se>
eurosource

 