#!/bin/sh

bakefile -fborland wxctb.bkl
bakefile -fgnu -o makefile.unx wxctb.bkl
bakefile -fmingw wxctb.bkl
bakefile -fmsvc wxctb.bkl
bakefile -fwatcom wxctb.bkl

bakefile -fborland -o sample.bcc sample.bkl
bakefile -fgnu -o sample.unx sample.bkl
bakefile -fmingw -o sample.gcc sample.bkl
bakefile -fmsvc -o sample.vc sample.bkl
bakefile -fwatcom -o sample.wat sample.bkl
