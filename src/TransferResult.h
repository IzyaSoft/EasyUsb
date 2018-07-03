#ifndef SRC_TRANSFERRESULT_H_
#define SRC_TRANSFERRESULT_H_

#include <libusb.h>
#include <vector>

namespace EasySoftUsb
{

    enum class TransferStatus : unsigned char
    {
        Incomplete,
        Complete
    };

    struct TransferResult
    {
        TransferStatus _status;                // in Complete, _data is not empty
        unsigned int _transferNumber;          // transfer number
        std::vector<unsigned char> _data;
        libusb_transfer* _transfer;
    };
}

#endif
