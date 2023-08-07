#include "usb.hpp"

namespace PT {
	USB::USB(Encoding* encoder) : encoder(encoder), Transmitter() {
        
    }

    void USB::transmit(std::string data) {
        std::vector<uint8_t> rawData = Encoding::reinterpretString(data);
        std::vector<uint8_t> encodedData = encoder->encode(rawData);
        // std::cout << encodedData.size() << std::endl;
        for (size_t i = 0; i < encodedData.size(); i++) {
            printf("%c",encodedData.at(i));
        }
        // printf("\n\n");
    }
    
    USB::~USB() {
        delete encoder;
    }
} // namespace PT       
