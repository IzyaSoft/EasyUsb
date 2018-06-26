#ifndef SRC_USBTRANSCEIVER_H_
#define SRC_USBTRANSCEIVER_H_

#include <vector>
#include <libusb.h>

#include "TransferPacket.h"

//#define DEBUG_USB_PRINT
//#define DEBUG_LIBUSB_PRINT

namespace EasySoftUsb
{
    class UsbTransceiver
    {
    public:
        UsbTransceiver(const unsigned short vid, const unsigned short pid, const std::vector<unsigned char>& interfaces,
                       const unsigned char configuration, const unsigned int timeout = 0, bool isAsync = true);
        ~UsbTransceiver();
        bool SwitchConfiguration(const unsigned int activeConfiguration, const std::vector<unsigned char>& interfaces);
        bool Write(const std::vector<unsigned char>& data, const unsigned int endpoint,
                   EasySoftUsb::UsbControlTransferInfo* controlInfo = NULL,
                   EasySoftUsb::TransferType transferType = EasySoftUsb::TransferType::Bulk);
        bool Write(const EasySoftUsb::TransferPacket& packet);
        bool Read(EasySoftUsb::TransferPacket& packet, int& bytesRead);
        bool Read(std::vector<unsigned char>& data, int& bytesRead, const unsigned int endpoint,
                  EasySoftUsb::UsbControlTransferInfo* controlInfo = NULL,
                  EasySoftUsb::TransferType transferType = EasySoftUsb::TransferType::Bulk);
        void Reset();
        bool IsReady();
    private:
        bool Init(const unsigned int configuration, const std::vector<unsigned char>& interfaces);
        void ReleaseConfiguration();
        void ReleaseInterfaces();
        bool WriteSync(const EasySoftUsb::TransferPacket& packet);
        bool ReadSync(EasySoftUsb::TransferPacket& packet, int& bytesRead);
        bool SyncTransactionImpl(EasySoftUsb::TransferPacket& packet, std::vector<unsigned char>& buffer, int& bytesTransferred);
        bool WriteAsync(const EasySoftUsb::TransferPacket& packet);
        bool ReadAsync(EasySoftUsb::TransferPacket& packet, int& bytesRead);
        EasySoftUsb::TransferPacket BuildTransferPacket(const std::vector<unsigned char>& data, const unsigned int endpoint,
                                                        EasySoftUsb::UsbControlTransferInfo* controlInfo, EasySoftUsb::TransferType transferType);
        bool AsyncTransactionImpl(libusb_transfer* transfer, const unsigned int retransmissionCounter = 1);
        void DisplayError(int result);
    private:
        // libusb entities
        libusb_device_handle* _handle;
        libusb_context* _context;
        // non libusb entities
        bool _deviceReady;
        bool _isAsync;
        const unsigned int _defaultTimeout = 2000;
        unsigned int _timeout;
        unsigned short _productId;
        unsigned short _vendorId;
        unsigned int _activeConfiguration;
        std::vector<unsigned char> _interfaces;

    };
}

#endif /* SRC_USBTRANSCEIVER_H_ */
