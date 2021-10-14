 # Unittests for the vscpremotetcpip client 
 
 ## Windows

need gtest vcpkg:x64-windows

 ```
 cmake .. -G "Visual Studio 16 2019" -DVCPKG_TARGET_TRIPLET=x64-windows  -DCMAKE_TOOLCHAIN_FILE=C:\Users\Administrator\Desktop\Development\vcpkg\scripts\buildsystems\vcpkg.cmake
 ```