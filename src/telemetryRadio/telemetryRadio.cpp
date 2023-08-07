#include "telemetryRadio.hpp"

namespace PT {
    TelemetryRadio::TelemetryRadio(std::uint8_t port, Encoding* encoder) : encoder(encoder) {
		serial = std::make_shared<pros::Serial>(pros::Serial(port, TELEMETRY_RADIO_BAUDRATE));
	}

	void TelemetryRadio::transmit(std::vector<uint8_t> data) {
		printf("%d", data.size());
		printf("%d", serial->write(data.data(), data.size()));
		printf("%d\n", errno);
	}

	void TelemetryRadio::transmit(std::string data)  {
        std::vector<uint8_t> rawData = Encoding::reinterpretString(data);
        std::vector<uint8_t> encodedData = encoder->encode(rawData);

		this->transmit(encodedData);
	}
	
	std::shared_ptr<pros::Serial> TelemetryRadio::getSerial() {
		return std::shared_ptr<pros::Serial>(serial);
	}
	
	TelemetryRadio::~TelemetryRadio() {
	}
} // namespace PT
