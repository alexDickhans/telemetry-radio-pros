#include "telemetryManager.hpp"

namespace PT {
	TelemetryManager::TelemetryManager() {
		defaultLogger = Logger::getInstance();
	}

	TelemetryManager* TelemetryManager::singleton_{nullptr};
	pros::Mutex TelemetryManager::mutex_;

	TelemetryManager* TelemetryManager::getInstance() {
		std::lock_guard<pros::Mutex> lock(mutex_);

		if (singleton_ == nullptr) {
			singleton_ = new TelemetryManager();
		}

		return singleton_;
	}
	
	std::string TelemetryManager::generateTransmission() {
		std::string newData = "";

		for (std::shared_ptr<Measurement> measurement : measurementSources) {
			// Only include updated measurement data in the transmissions
			if (measurement.get()->hasUpdated()) {
				newData.append(measurement.get()->to_string());
			}
		}

		std::string loggerData = defaultLogger->getNewEntries();

		newData.append(loggerData);

		return newData;
	}

	void TelemetryManager::sendTransmission(std::string transmission) {
		for (std::shared_ptr<Transmitter> transmitter : transmitters) {
			transmitter->transmit(transmission);
		}
	}

	void TelemetryManager::update() {
		// Update all the data sources
		this->updateMeasurementSources();

		// Get data from logger and sources
		std::string transmission = this->generateTransmission();

		// Send transmission
		this->sendTransmission(transmission);
	}

	void TelemetryManager::updateScheduler() {
		while (true) {
			uint32_t startTime = pros::millis();
			this->update();
			pros::Task::delay(std::min(updateTime - (pros::millis() - startTime), (long unsigned int) updateTime));
		}
	}

	void TelemetryManager::updateMeasurementSources()  {
		for (std::shared_ptr<Measurement> m : measurementSources) {
			m->update();
		}
	}

	bool TelemetryManager::enableUpdateScheduler() {
		updateTask = std::make_shared<pros::Task>([this] {this->updateScheduler();});
		return taskRunning;
	}

	bool TelemetryManager::disableUpdateScheduler() {
		return taskRunning;
	}

	void TelemetryManager::addTransmitter(std::shared_ptr<Transmitter> newTransmitter) {
		transmitters.emplace_back(newTransmitter);
	}

	void TelemetryManager::addMeasurementSource(std::shared_ptr<Measurement> newMeasurementSource) {
		measurementSources.emplace_back(newMeasurementSource);
	}
	
	TelemetryManager::~TelemetryManager() {
	}
    
} // namespace PT
