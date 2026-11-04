#pragma once

#include <vector>
#include <memory>
#include <mutex>

#include "pros/rtos.hpp"

#include "encoding.hpp"
#include "logger.hpp"
#include "measurementTypes/measurement.hpp"
#include "measurementTypes/functionMeasurement.hpp"
#include "measurementTypes/variableMeasurement.hpp"
#include "telemetryRadio.hpp"
#include "transmitter.hpp"
#include "usb.hpp"

namespace PT {
	class TelemetryManager {
	private:
		static TelemetryManager* singleton_;
		static pros::Mutex mutex_;

		TelemetryManager();

		std::shared_ptr<pros::Task> updateTask;
		bool taskRunning;

		std::vector<std::shared_ptr<Transmitter>> transmitters;

		std::vector<std::shared_ptr<Measurement>> measurementSources;
		Logger* defaultLogger;

		uint32_t updateTime{10};
	protected:
		void updateScheduler();

		std::string generateTransmission();

		void sendTransmission(std::string transmission);

		void updateMeasurementSources();
	public:
		TelemetryManager(TelemetryManager& other) = delete;
		void operator=(const Logger &) = delete;

		static TelemetryManager* getInstance();

		/**
		 * @brief Sample every measurement, then send changed values and new log lines.
		 *
		 * enableUpdateScheduler() starts a task that calls this method. Calling update()
		 * from user code while that task is running sends a second copy of the data.
		 */
		void update();

		/**
		 * @brief Start a PROS task that calls update() about every updateTime milliseconds.
		 */
		bool enableUpdateScheduler();

		/**
		 * @brief Return taskRunning. The scheduler task is left running, and taskRunning is never assigned.
		 */
		bool disableUpdateScheduler();

		uint32_t getUpdateTime() {
			return updateTime;
		}

		void setUpdateTime(uint32_t updateTime) {
			this->updateTime = updateTime;
		}

		void addTransmitter(std::shared_ptr<Transmitter> newTransmitter);

		void addMeasurementSource(std::shared_ptr<Measurement> newMeasurementSource);

		~TelemetryManager();
	};	
} // namespace PT
