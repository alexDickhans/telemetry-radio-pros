#include "logger.hpp"

namespace PT {
	Logger::Logger(/* args */) {
		this->outputLogLevel = DEFAULT_LOG_LEVEL;
	}

	Logger* Logger::logger_{nullptr};
	pros::Mutex Logger::mutex_;

	Logger* Logger::getInstance() {
		std::lock_guard<pros::Mutex> lock(mutex_);

		if (logger_ == nullptr) {
			logger_ = new Logger();
		}

		return logger_;
	}

    void Logger::debug(std::string debug) {
        if (outputLogLevel < LogLevel::debug) { return; }
        logBuffer.append("#DEBUG,");
        logBuffer.append(std::to_string(pros::millis()));
        logBuffer.append(": ");
        logBuffer.append(debug);
        logBuffer.append("\n");
    }

    void Logger::info(std::string info) {
        if (outputLogLevel < LogLevel::info) { return; }
        logBuffer.append("#INFO,");
        logBuffer.append(std::to_string(pros::millis()));
        logBuffer.append(": ");
        logBuffer.append(info);
        logBuffer.append("\n");
    }

    void Logger::warn(std::string warn) {
        if (outputLogLevel < LogLevel::warn) { return; }
        logBuffer.append("#WARN,");
        logBuffer.append(std::to_string(pros::millis()));
        logBuffer.append(": ");
        logBuffer.append(warn);
        logBuffer.append("\n");
    }

    void Logger::error(std::string error) {
        if (outputLogLevel < LogLevel::error) { return; }
        logBuffer.append("#ERROR,");
        logBuffer.append(std::to_string(pros::millis()));
        logBuffer.append(": ");
        logBuffer.append(error);
        logBuffer.append("\n");
    }

    LogLevel Logger::getLogLevel() {
        return outputLogLevel;
    }

    void Logger::setLogLevel(LogLevel logLevel) {
        this->outputLogLevel = logLevel;
    }

    std::string Logger::getNewEntries() {
        std::string tempBuffer = logBuffer;
        logBuffer.clear();
        return tempBuffer;
    }

	Logger::~Logger() {
	}
} // namespace PT
