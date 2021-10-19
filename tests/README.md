This folder contains testfiles for the VSCP & friends suite

test_vesphelper  - test for the tcp/if interface and the libvscphelper library code.
tables-tcp - Tests for tables creating, logging, handling in the tcp/ip interface.


Tests use GoogleTest which is part of this repository in the rootfoolder as googletest. 
Googeltest must be configured and built before tests can be run.

To build

- Go to googletest folder
- Make folder *build* (*mkdir build*)
- Go to folder (**cd build**)
- Do **cmake ..**
- Do **make**

For new tests link with  

>-L../../googletest/build/lib -lgtest -lgtest_main 

and set include path

>_I ../../googletest/googletest/include

here with a test located in it's own folder under **tests**

see tests/helperlib/Makefile.in for example

Use 

```
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -DVCPKG_TARGET_TRIPLET=x64-windows  -DCMAKE_TOOLCHAIN_FILE=C:\Users\Administrator\Desktop\Development\vcpkg\scripts\buildsystems\vcpkg.cmake
```

in each test folder to create a makefile. Then do

```
cmake --build . --config Release
```

or 

```
msbuild libvscphelper.sln /p:Configuration=Release
```

to build the unittest.   Run the test with

```
./unittest
```