# EasyUsb
Library for exchange with USB devices via C++ wrap around libusb-1.0

Provides 2 version of interaction with LibUsb via synchronous or asynchronous LibUsb API.

While using asynchronous LibUsb API data waiting implemented in Write and Read operation (in current version), However in next version we are going to provide user with callbacks with already completed operation (return buffer).

Using this Library it is easy to access any USB devices supports following transfer types:
Bulk
Control
Interrupt

# Build and Install
EasyUsb is a shared library that could be build from eclipse project (project file is already in repo) or via separate Makefile running make command

Dependencies:

 - libusb-1.0 itself (-lusb-1.0),  library searching path in Makefile are: -L. -L.. -L/lib64/ -L/lib/ -L/usr/lib64 -L/usr/lib, include is: -I/usr/include/libusb-1.0
 
 - standard C++11 runtime library
 
 it is possible to build libEasyUsb and install via <b>make install</b> command, during make install libEasyUsb.so and all include headers are copying to:
    
    /usr/lib (libEasyUsb-{Version}.so)
    
    /usr/include/(libEasyUsb-{Version}.so) 

Make produces so library file in following template libEasyUsb.so.VERSION,  if you are using make install it genereates symlink without version also (for more convenient link)

# How to link libEasyUsb to project

1. We should add library searching directories via -L: -L/usr/lib

2. We should link both 2 libs (libusb and libEasyUsb) -lusb-1.0 -lEasyUsb

3. We should include (header file location) -I/usr/include/libusb-1.0/ -I/usr/include/libEasyUsb/
