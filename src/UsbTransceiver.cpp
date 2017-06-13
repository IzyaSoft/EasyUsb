#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include "UsbTransceiver.h"

namespace EasySoftUsb
{
    static bool libusbTransferCompleted;
    static void LIBUSB_CALL LibusbCallback(libusb_transfer* transfer)
    {
        libusbTransferCompleted = true;
    }

    UsbTransceiver :: UsbTransceiver(const unsigned short vid, const unsigned short pid, const std::vector<unsigned char>& interfaces,
                                     const unsigned char configuration, const unsigned int timeout, bool isAsync)
    {
        if(vid == 0 || pid ==0)
            throw std::runtime_error("Invalid product or vendor id!");
        _vendorId = vid;
        _productId = pid;
        if(interfaces.empty())
            throw std::runtime_error("Interface list can't be empty!");
        _timeout = timeout == 0 ? _defaultTimeout : timeout;
        _activeConfiguration = configuration;
        _isAsync = isAsync;
        try
        {
            libusb_init(&_context);
            #ifdef DEBUG_LIBUSB_PRINT
                libusb_set_debug(_context, LIBUSB_LOG_LEVEL_DEBUG);
            #endif
            _handle = libusb_open_device_with_vid_pid(_context, _vendorId, _productId);
            #ifdef __linux__
                libusb_detach_kernel_driver(_handle, interfaces[0]);
            #endif
            Reset();
            _deviceReady = Init(_activeConfiguration, interfaces);
        }
        catch(...)
        {
            std::cout << "An error has occured during device instantation" <<std::endl;
            _deviceReady = false;
        }
    }

    UsbTransceiver :: ~UsbTransceiver()
    {
        try
        {
           ReleaseConfiguration();
           libusb_close(_handle);
           libusb_exit(_context);
        }
        catch(std::exception& e)
        {
            std::cout << "Error occured at ~UsbTransceiver()" << std::endl;
        }
    }

    bool UsbTransceiver :: IsReady()
    {
        return _deviceReady;
    }

    bool UsbTransceiver :: SwitchConfiguration(const unsigned int activeConfiguration, const std::vector<unsigned char>& interfaces)
    {
        ReleaseConfiguration();
        _activeConfiguration = activeConfiguration;
        _interfaces.assign(interfaces.begin(), interfaces.end());
        return Init(_activeConfiguration, _interfaces);
    }

    bool UsbTransceiver :: Write(const std::vector<unsigned char>& data, const unsigned int endpoint,
                                 EasySoftUsb::UsbControlTransferInfo* controlInfo,
                                 EasySoftUsb::TransferType transferType)
    {
        return Write(data, endpoint, controlInfo, transferType);
    }

    bool UsbTransceiver :: Write(const EasySoftUsb::TransferPacket& packet)
    {
        if(_isAsync)
            return WriteAsync(packet);
        return WriteSync(packet);
    }

    bool UsbTransceiver :: Read(EasySoftUsb::TransferPacket& packet, int& bytesRead)
    {
        if(_isAsync)
            return ReadAsync(packet, bytesRead);
        return ReadSync(packet, bytesRead);
    }

    bool UsbTransceiver :: Read(std::vector<unsigned char>& data, int& bytesRead, const unsigned int endpoint,
                                EasySoftUsb::UsbControlTransferInfo* controlInfo,
                                EasySoftUsb::TransferType transferType)
    {
        EasySoftUsb::TransferPacket packet = BuildTransferPacket(data, endpoint, controlInfo, transferType);
        return Read(packet, bytesRead);
    }

    bool UsbTransceiver :: ReadSync(EasySoftUsb::TransferPacket& packet, int& bytesRead)
    {
        std::vector<unsigned char> buffer;
        buffer.resize(65535);
        packet._direction = 1;
        bool result = SyncTransactionImpl(packet, buffer, bytesRead);
        if(result)
            packet._data.assign(buffer.begin(), buffer.begin() + bytesRead);
        return result;
    }

    bool UsbTransceiver :: SyncTransactionImpl(EasySoftUsb::TransferPacket& packet, std::vector<unsigned char>& buffer, int& bytesTransferred)
    {
        int result;
        unsigned int timeout = packet._timeout > 0 ? packet._timeout : _timeout;
        if(packet._transferType == EasySoftUsb::TransferType::Control)
        {
            result = libusb_control_transfer(_handle, packet._controlInfo._bmRequestType, packet._controlInfo._bRequest,
                                             packet._controlInfo._wValue, packet._controlInfo._wIndex,
                                             buffer.data(), buffer.size(), timeout);
            bytesTransferred = result;
        }

        else if(packet._transferType == EasySoftUsb::TransferType::Bulk)
            result = libusb_bulk_transfer(_handle, (packet._direction << 7) | packet._endpoint, buffer.data(), buffer.size(), &bytesTransferred, timeout);
        else if(packet._transferType == EasySoftUsb::TransferType::Interrupt)
            result = libusb_interrupt_transfer(_handle, (packet._direction << 7) | packet._endpoint, buffer.data(), buffer.size(), &bytesTransferred, timeout);
        else
            throw std::runtime_error("[UsbTransceiver, SyncTransactionImpl] Not implemented yet!");
        bool success = (bytesTransferred >= 0 && result == 0 && packet._transferType != EasySoftUsb::TransferType::Control) ||
                       (result >= 0 && packet._transferType == EasySoftUsb::TransferType::Control);
        #ifdef DEBUG_USB_PRINT
            if(packet._direction == 0)
            {
                std::cout << "[UsbTransceiver, SyncTransactionImpl] Data to send: ";
                for(unsigned int i = 0; i < packet._data.size(); i++)
                    std::cout << std::hex << (int)packet._data[i] << " ";
                std::cout << std::endl;
            }
            else
            {
                std::cout << "[UsbTransceiver, SyncTransactionImpl] Data received: ";
                for(int i = 0; i < bytesTransferred; i++)
                    std::cout << std::hex << (int)buffer[i] << " ";
                std::cout << std::endl;
            }
            std::cout << "[UsbTransceiver, SyncTransactionImpl] Bytes transferred: " << bytesTransferred << std::endl;
        #endif
        if(!success)
            DisplayError(result);
        return success;
    }

    bool UsbTransceiver :: ReadAsync(EasySoftUsb::TransferPacket& packet, int& bytesRead)
    {
        libusbTransferCompleted = false;
        int completed = 0;
        libusb_transfer* transfer = libusb_alloc_transfer(0);
        unsigned int offset = packet._transferType == EasySoftUsb::TransferType::Control ? 8 : 0;
        std::vector<unsigned char> setupPacket(packet._data.size() + offset);
        unsigned int timeout = packet._timeout > 0 ? packet._timeout : _timeout;
        #ifdef DEBUG_USB_PRINT
            std::cout << "[UsbTransceiver, ReadAsync] Offset value is: " << std::dec << offset << std::endl;
            std::cout << "[UsbTransceiver, ReadAsync] Timeout value is: " << std::dec << timeout << std::endl;
        #endif
        if(packet._transferType == EasySoftUsb::TransferType::Control)
        {
            #ifdef DEBUG_USB_PRINT
                std::cout << "[UsbTransceiver, ReadAsync] Control transfer packet setup procedure" << std::endl;
            #endif
            libusb_fill_control_setup(setupPacket.data(), packet._controlInfo._bmRequestType, packet._controlInfo._bRequest,
            packet._controlInfo._wValue, packet._controlInfo._wIndex, packet._data.size());
            libusb_fill_control_transfer(transfer, _handle, setupPacket.data(), LibusbCallback, &completed, timeout);
        }
        else if (packet._transferType == EasySoftUsb::TransferType::Bulk)
        {
            #ifdef DEBUG_USB_PRINT
                std::cout << "[UsbTransceiver, ReadAsync] Bulk transfer packet setup procedure" << std::endl;
            #endif
            libusb_fill_bulk_transfer(transfer, _handle, 0x80 | packet._endpoint, setupPacket.data(), setupPacket.size(),
                                      LibusbCallback, transfer, timeout);
        }
        else
        {
            throw std::runtime_error("Not implemented yet!");
        }
        bool result = AsyncTransactionImpl(transfer);
        bytesRead = transfer->actual_length;
        packet._data.assign(setupPacket.begin() + offset, setupPacket.end());
        #ifdef DEBUG_USB_PRINT
            std::cout <<"[UsbTransceiver, ReadAsync] Data in transfer structure buffer: ";
            for(unsigned int i = 0; i < packet._data.size() + offset; i++)
                std::cout << std::hex << (int)(transfer->buffer[i]) << " ";
            std::cout << std::endl;
            std::cout <<"[UsbTransceiver, ReadAsync] Data to output buffer: ";
            for(unsigned int i = 0; i < packet._data.size(); i++)
                std::cout << std::hex << (int)(packet._data[i]) << " ";
            std::cout << std::endl;
        #endif

        libusb_free_transfer(transfer);
        return result;
    }

    bool UsbTransceiver :: WriteSync(const EasySoftUsb::TransferPacket& packet)
    {
        int bytesWritten;
        EasySoftUsb::TransferPacket packetCopy = const_cast<EasySoftUsb::TransferPacket&>(packet);
        packetCopy._direction = 0;
        return SyncTransactionImpl(packetCopy, packetCopy._data, bytesWritten);
    }

    bool UsbTransceiver :: WriteAsync(const EasySoftUsb::TransferPacket& packet)
    {
        libusbTransferCompleted = false;   // global flag ... //todo: umv: think about possible multi threading queue
        bool handleResult = false;
        int completed = 0;
        libusb_transfer* transfer = libusb_alloc_transfer(0);
        unsigned int offset = packet._transferType == EasySoftUsb::TransferType::Control ? 8 : 0;
        std::vector<unsigned char> packetData(packet._data.size() + offset);
        unsigned int timeout = packet._timeout > 0 ? packet._timeout : _timeout;
        #ifdef DEBUG_USB_PRINT
            std::cout << "[UsbTransceiver, WriteAsync] Offset value is: " << std::dec << offset << std::endl;
            std::cout << "[UsbTransceiver, WriteAsync] Timeout value is: " << std::dec << timeout << std::endl;
        #endif
        if(packet._transferType == EasySoftUsb::TransferType::Control)
        {
            #ifdef DEBUG_USB_PRINT
                std::cout << "[UsbTransceiver, WriteAsync] Control transfer packet setup procedure" << std::endl;
            #endif
            libusb_fill_control_setup(packetData.data(), packet._controlInfo._bmRequestType, packet._controlInfo._bRequest,
                                      packet._controlInfo._wValue, packet._controlInfo._wIndex, packet._data.size());
            std::copy(packet._data.begin(), packet._data.end(), packetData.begin() + offset);
            libusb_fill_control_transfer(transfer, _handle, packetData.data(), LibusbCallback, &completed, timeout);
        }
        else if(packet._transferType == EasySoftUsb::TransferType::Bulk)
        {
            #ifdef DEBUG_USB_PRINT
                std::cout << "[UsbTransceiver, WriteAsync] Bulk transfer packet setup procedure" << std::endl;
            #endif
            libusb_fill_bulk_transfer(transfer, _handle, packet._endpoint, (unsigned char*)packet._data.data(), packet._data.size(),
                                      LibusbCallback, transfer, timeout);
        }
        else
        {
            throw std::runtime_error("[UsbTransceiver, WriteAsync] not implemented yet");
        }
        #ifdef DEBUG_USB_PRINT
            std::cout <<"[UsbTransceiver, WriteAsync] Original data: ";
            for(unsigned int i = 0; i < packet._data.size(); i++)
                std::cout << std::hex << (int)(packet._data[i]) << " ";
            std::cout << std::endl;
            std::cout <<"[UsbTransceiver, WriteAsync] Data in transfer structure buffer: ";
            for(unsigned int i = 0; i < packet._data.size() + offset; i++)
                std::cout << std::hex << (int)(transfer->buffer[i]) << " ";
            std::cout << std::endl;
        #endif
        handleResult = AsyncTransactionImpl(transfer);
        libusb_free_transfer(transfer);
        return handleResult;
    }

    bool UsbTransceiver :: AsyncTransactionImpl(libusb_transfer* transfer, const unsigned int retransmissionCounter)
    {
        libusbTransferCompleted = false;
        bool handleResult = false;
        for(unsigned int counter = 0; counter < retransmissionCounter; counter++)
        {
            int result = libusb_submit_transfer(transfer);
            if(!result)
                std::runtime_error("[UsbTransceiver, AsyncTransactionImpl] An error has occured at libusb transfer submit");
            while(!libusbTransferCompleted)
                result = libusb_handle_events(_context);
            handleResult = transfer->status == LIBUSB_TRANSFER_COMPLETED || !result;
            if(!handleResult)  // todo: umv: Check Display Error function is proper
            {
                std::cout << "Error occured, code: " << transfer->status << ", related to ";
                DisplayError(result);
            }
            else break;
        }
        return handleResult;
    }

    void UsbTransceiver :: Reset()
    {
        libusb_reset_device(_handle);
    }

    EasySoftUsb::TransferPacket UsbTransceiver :: BuildTransferPacket(const std::vector<unsigned char>& data, const unsigned int endpoint,
                                                                      EasySoftUsb::UsbControlTransferInfo* controlInfo,
                                                                      EasySoftUsb::TransferType transferType)
    {
        EasySoftUsb::TransferPacket packet;
        packet._data = data;
        packet._endpoint = endpoint;
        if(controlInfo != NULL)
            packet._controlInfo = *controlInfo;
        packet._transferType = transferType;
        return packet;
    }

    bool UsbTransceiver :: Init(const unsigned int configuration, const std::vector<unsigned char>& interfaces)
    {
        int result = libusb_set_configuration(_handle, configuration);
        if(!result)
        {
            for(std::vector<unsigned char>::const_iterator it = interfaces.begin(); it != interfaces.end(); it++)
            {
                result = libusb_claim_interface(_handle, *it);
                if(result != 0)
                    return false;
                #ifdef DEBUG_USB_PRINT
                    std::cout << "[UsbTransceiver, Init] Interface: " << (int)(*it) << " was claimed" << std::endl;
                #endif
                _interfaces.push_back(*it);
            }
            return true;
        }
        return false;
    }

    void UsbTransceiver :: ReleaseConfiguration()
    {
        if(_deviceReady)
        {
            ReleaseInterfaces();
            #ifdef __linux__
                libusb_attach_kernel_driver(_handle, _interfaces[0]);
                #ifdef DEBUG_USB_PRINT
                    std::cout << "[UsbTransceiver, ReleaseConfiguration] Kernel driver was attached back" << std::endl;
                #endif
            #endif
            Reset();
        }
    }

    void UsbTransceiver :: ReleaseInterfaces()
    {
        #ifdef DEBUG_USB_PRINT
            std::cout << "[UsbTransceiver, ReleaseInterfaces] Interfaces release procedure." << std::endl;
        #endif
        for(std::vector<unsigned char> :: iterator it = _interfaces.begin(); it != _interfaces.end(); it++)
        {
            if(!libusb_kernel_driver_active(_handle, *it))
            {
                libusb_release_interface(_handle, *it);
                #ifdef DEBUG_USB_PRINT
                    std::cout << "[UsbTransceiver, ReleaseInterfaces] Interface: " << (int)(*it) << " was released" << std::endl;
                #endif
            }
        }
    }

    void UsbTransceiver :: DisplayError(int result)
    {
        // logging ...
        std::cout << "Error data exchange with device, reason: ";
        if(result == LIBUSB_ERROR_TIMEOUT)
            std::cout <<"LIBUSB_ERROR_TIMEOUT" << std::endl;
        else if(result == LIBUSB_ERROR_PIPE)
            std::cout <<"LIBUSB_ERROR_PIPE (control transfers are not supported by device)" << std::endl;
        else if(result == LIBUSB_ERROR_NO_DEVICE)
            std::cout <<"LIBUSB_ERROR_NO_DEVICE" << std::endl;
        else if(result == LIBUSB_ERROR_IO)
            std::cout <<"LIBUSB_ERROR_IO" << std::endl;
        else if(result ==  LIBUSB_ERROR_INVALID_PARAM)
            std::cout <<" LIBUSB_ERROR_INVALID_PARAM" << std::endl;
        else if(result ==  LIBUSB_ERROR_ACCESS)
            std::cout <<" LIBUSB_ERROR_ACCESS" << std::endl;
        else if(result == LIBUSB_ERROR_BUSY)
            std::cout <<" LIBUSB_ERROR_BUSY" << std::endl;
        else if(result ==  LIBUSB_ERROR_INTERRUPTED)
            std::cout <<" LIBUSB_ERROR_INTERRUPTED" << std::endl;
        else if(result ==  LIBUSB_ERROR_NO_MEM)
            std::cout <<" LIBUSB_ERROR_NO_MEM" << std::endl;
        else std::cout <<"Other LIBUSB error, code: " << result << std::endl;
    }
}
