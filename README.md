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

# Synchronous and Asynchronous examples

Synchronous operations blocks execution until operation is not complete or timeout is expired, in async way we done 
first version almost the same (data wait is implemented in library Write and Read operation, in 1.1 it WILL BE implemented smarter logic. last param in UsbTransceiver construction related with usage sync or async lib usb API. I discovered that sometimes async API works better for same purposes (don't know why).

1. init device instance

`
    _transceiver = std::shared_ptr<EasySoftUsb::UsbTransceiver>(new EasySoftUsb::UsbTransceiver(_vendorId, _productId,               
                                                                                                interfaces, 1));
`

interfaces - static std::vector<unsigned char> interfaces = {0, 1, 2, 3, 4}; vector of available device Usb interfaces
 
2. Write data to device

  `
  
      std::vector<EasySoftUsb::TransferPacket> firmware = EasyUsrp::ImageLoader::GetFirmwareLoader().Load(firmareFile);
      
      for(std::vector<EasySoftUsb::TransferPacket>::iterator it = firmware.begin(); it != firmware.end(); it++)
            _transceiver->Write(*it);
`
Here we form vector of bytes from firmware image, transfer packet forms in following manner (example is out of context):
`

    EasySoftUsb::TransferPacket packet;
    packet._timeout = -1;
    packet._transferType = EasySoftUsb::TransferType::Control;
    // Data
    if(type == 0x00)
    {
        packet._data.assign(buffer.begin(), buffer.end());
        packet._controlInfo._wValue = address;
        packet._controlInfo._wIndex = upperAddress;
        packet._controlInfo._bRequest = FIRMWARE_LOAD_REQUEST;
        packet._controlInfo._bmRequestType = COMMAND_REQUEST_TYPE;
        _buffer.push_back(packet);
    }

`
each packet has transfer type (1 of 4 USB transfer types: bulk, control, e.t.c), also every packet could have it own timeout

3. Read data from device
`

        int bytesRead = 0;
        EasySoftUsb::TransferPacket packet = EepromHelper::GetReadRequest();
        _transceiver->Read(packet, bytesRead);
        return packet._data;
`
data read is also uses Transfer Packet which describes read data

`

    EasySoftUsb::TransferPacket EepromHelper :: GetReadRequest()
    {
        EasySoftUsb::TransferPacket packet;
        packet._transferType = EasySoftUsb::TransferType::Control;
        packet._controlInfo._bmRequestType = DATA_REQUEST_TYPE;
        packet._controlInfo._bRequest = EEPROM_READ_REQUEST;
        packet._controlInfo._wIndex = 0;
        packet._controlInfo._wValue = 0;
        packet._data.resize(EEPROM_SIZE);
        return packet;
    }
`
