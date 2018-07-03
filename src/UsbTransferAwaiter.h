#ifndef SRC_USBTRANSFERAWAITER_H_
#define SRC_USBTRANSFERAWAITER_H_

#include <vector>
#include "TransferResult.h"
#include "UsbTransceiver.h"

namespace EasySoftUsb
{
    class UsbTransferAwaiter
    {
    public:
        std::vector<unsigned char> WaitSync(const EasySoftUsb::TransferResult& result,
                                            const EasySoftUsb::UsbTransceiver& transceiver);
        std::vector<unsigned char> WaitAsync(const EasySoftUsb::TransferResult& result,
                                             const EasySoftUsb::UsbTransceiver& transceiver);
    private:
        std::vector<unsigned char> WaitImpl(const EasySoftUsb::TransferResult& result,
                                            const EasySoftUsb::UsbTransceiver& transceiver, bool isAsync);
    };
}


#endif /* SRC_USBTRANSFERAWAITER_H_ */
