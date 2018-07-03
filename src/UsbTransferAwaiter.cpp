#include "UsbTransferAwaiter.h"

namespace EasySoftUsb
{
    std::vector<unsigned char> UsbTransferAwaiter :: WaitSync(const EasySoftUsb::TransferResult& result,
                                                              const EasySoftUsb::UsbTransceiver& transceiver)
    {
        return WaitImpl(result, transceiver, false);
    }

    std::vector<unsigned char> UsbTransferAwaiter :: WaitAsync(const EasySoftUsb::TransferResult& result,
                                                               const EasySoftUsb::UsbTransceiver& transceiver)
    {
        return WaitImpl(result, transceiver, true);
    }

    std::vector<unsigned char> UsbTransferAwaiter :: WaitImpl(const EasySoftUsb::TransferResult& result,
                                                              const EasySoftUsb::UsbTransceiver& transceiver, bool isAsync)
    {
        std::vector<unsigned char> deviceData;
        return deviceData;
    }
}
