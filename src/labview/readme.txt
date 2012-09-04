In this section you find vscp-related labview code.

Always try to save contributions in the oldest possible version to allow as many people as possible to re-use the code.

All libraries containing universal code should be placed to
/src/labview/libs

The few system specific parts (especially CINs and DLL calls) should be placed in
/src/labview/libs/win
/src/labview/libs/linux
/src/labview/libs/mac

all libs should follow some sort of naming scheme:

Include 3-4 letters for showing type of vi, 2-3 numbers giving version it was saved in. E.g. conv_canfrm2vscp_60.vi contains "conv"ersion of a can-frame to vscp class/type/nick and it was saved as labview 6.0 vi.

Please include some text in your VI diagram to describe what it does, who did it and such.

If you have some sort of applications store it under 
/src/labview/apps.

If your application contains more than only one vi-file please create an folder below /apps.
