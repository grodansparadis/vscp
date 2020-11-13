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

