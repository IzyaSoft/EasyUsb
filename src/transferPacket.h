#ifndef SRC_DATA_USB_TRANSFERPACKET_H_
#define SRC_DATA_USB_TRANSFERPACKET_H_

#include <vector>
#include "UsbControlTransferInfo.h"
#include "TransferType.h"


namespace EasySoftUsb
{
    struct TransferPacket
    {
        EasySoftUsb::UsbControlTransferInfo _controlInfo;  // only 4 control info ...
        EasySoftUsb::TransferType _transferType;           // type of transfer
        unsigned int _endpoint;
        bool _direction;                                   // 0 - OUT (From host to Device), 1 - IN (From device to host)
        int _timeout;                                      //to use custom timeout for this transaction, instead of default
        std::vector<unsigned char> _data;
    };
}
#endif /* SRC_DATA_USB_TRANSFERPACKET_H_ */
