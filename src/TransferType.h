#ifndef SRC_TRANSCEIVERS_CONFIG_TRANSFERTYPE_H_
#define SRC_TRANSCEIVERS_CONFIG_TRANSFERTYPE_H_

namespace EasySoftUsb
{
    enum class TransferType : unsigned char
    {
        Control = 0,
        Isochronous = 1,
        Bulk = 2,
        Interrupt = 3
    };
}

#endif /* SRC_TRANSCEIVERS_CONFIG_TRANSFERTYPE_H_ */
