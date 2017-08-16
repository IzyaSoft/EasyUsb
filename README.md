# EasyUsb
Library for exchange with USB devices via C++ wrap around libusb-1.0

Provides 2 version of interaction with LibUsb via synchronous or asynchronous LibUsb API.

While using asynchronous LibUsb API data waiting implemented in Write and Read operation (in current version), However in next version we are going to provide user with callbacks with already completed operation (return buffer).

Using this Library it is easy to access any USB devices supports following transfer types:
Bulk
Control
Interrupt
