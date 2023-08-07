#include "measurement.hpp"

namespace PT {
    Measurement::Measurement() {
		groupName = "blank";
		measurementName = "blank";
	}

	Measurement::Measurement(std::string name) : groupName(name), measurementName(name) {
	}
    
	Measurement::Measurement(std::string groupName, std::string measurementName) : groupName(groupName), measurementName(measurementName) {
	}

    std::string Measurement::formatSingle(std::string value) {
        return groupName + " " + measurementName + "=" + value + " \n";
    }

    void Measurement::valueUpdated() {
        this->newUpdate = true;
    }

    void Measurement::valueUsed() {
        this->newUpdate = false;
    }

    std::string Measurement::getName() {
        return groupName;
    }

    void Measurement::setName(std::string name) {
        this->groupName = name;
    }

	Measurement::~Measurement() {
	}
} // namespace PT
