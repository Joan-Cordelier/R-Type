/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Log
*/

#pragma once
#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>

enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

class Logger {
public:
    static Logger &getInstance() {
        static Logger instance;
        return instance;
    }

    void log(LogLevel level, const std::string &msg) {
        // Filter by minimum level
        if (static_cast<int>(level) < static_cast<int>(_minLevel.load())) {
            return;
        }
        std::lock_guard<std::mutex> lock(_mutex);
        std::cout << "[" << timestamp() << "] " << levelToString(level) << ": " << msg << std::endl;
    }

    void debug(const std::string &msg) {
        log(LogLevel::DEBUG, msg);
    }
    void info(const std::string &msg) {
        log(LogLevel::INFO, msg);
    }
    void warn(const std::string &msg) {
        log(LogLevel::WARNING, msg);
    }
    void error(const std::string &msg) {
        log(LogLevel::ERROR, msg);
    }

    // Quiet mode - only show errors
    void setQuiet(bool quiet) {
        _minLevel = quiet ? LogLevel::ERROR : LogLevel::DEBUG;
    }

    bool isQuiet() const {
        return _minLevel.load() >= LogLevel::ERROR;
    }

    void setMinLevel(LogLevel level) {
        _minLevel = level;
    }

private:
    Logger() = default;
    std::mutex _mutex;
    std::atomic<LogLevel> _minLevel{LogLevel::DEBUG};

    std::string timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time), "%H:%M:%S");
        return oss.str();
    }

    const char *levelToString(LogLevel level) {
        switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
        }
        return "UNKNOWN";
    }
};

#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARN(msg) Logger::getInstance().warn(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)