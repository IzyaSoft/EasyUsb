#ifndef SRC_TRANSCEIVERS_CONFIG_USBCONTROLTRANSFERINFO_H_
#define SRC_TRANSCEIVERS_CONFIG_USBCONTROLTRANSFERINFO_H_

namespace EasySoftUsb
{
    struct UsbControlTransferInfo
    {
        unsigned char _bRequest;
        unsigned char _bmRequestType;
        unsigned short _wValue;
        unsigned short _wIndex;
    };
}

#endif /* SRC_TRANSCEIVERS_CONFIG_USBCONTROLTRANSFERINFO_H_ */
