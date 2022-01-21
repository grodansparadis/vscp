#!/bin/sh
if ! ./unittest; then
    echo command returned some error
else
    echo command returned true
fi
