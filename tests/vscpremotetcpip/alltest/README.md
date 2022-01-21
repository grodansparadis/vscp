 # Unittests for the vscpremotetcpip client 
 
This is a test that test the vscpremotetcpip class  against the vscp2 tcp/ip server.

 ## Windows

need vcpkg gtest:vcpkg:x64-windows

 ```
 cmake .. -G "Visual Studio 16 2019" -DVCPKG_TARGET_TRIPLET=x64-windows  -DCMAKE_TOOLCHAIN_FILE=C:\Users\Administrator\Desktop\Development\vcpkg\scripts\buildsystems\vcpkg.cmake
 ```