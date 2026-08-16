#pragma once
#include <chrono>
#include <string>
#include <unordered_map>
#include <iostream>

class Benchmark {
    struct Stat {
        double totalTimeMs = 0.0;
        int callCount = 0;
        double lastLoggedMs = 0.0;
    };

    std::unordered_map<std::string, Stat> stats;
    std::chrono::high_resolution_clock::time_point printTimer = std::chrono::high_resolution_clock::now();

public:
    class ScopedTimer {
        std::string name;
        Benchmark& owner;
        std::chrono::high_resolution_clock::time_point start;
    public:
        ScopedTimer(std::string zoneName, Benchmark& bench) 
            : name(std::move(zoneName)), owner(bench), start(std::chrono::high_resolution_clock::now()) {}
        
        ~ScopedTimer() {
            auto end = std::chrono::high_resolution_clock::now();
            double durationMs = std::chrono::duration<double, std::milli>(end - start).count();
            owner.addMeasurement(name, durationMs);
        }
    };

    void addMeasurement(const std::string& name, double ms) {
        auto& stat = stats[name];
        stat.totalTimeMs += ms;
        stat.callCount++;
    }

    void tick(int entityCount) {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = now - printTimer;
        
        if (elapsed.count() >= 1.0) {
            std::cout << "\n=== [BENCHMARK] Entities: " << entityCount << " ===\n";
            for (auto& [name, stat] : stats) {
                if (stat.callCount > 0) {
                    double avg = stat.totalTimeMs / stat.callCount;
                    std::cout << "  - " << name << ": avg " << avg << " ms (Calls: " << stat.callCount << ")\n";
                    stat.totalTimeMs = 0;
                    stat.callCount = 0;
                }
            }
            std::cout << "========================================\n";
            printTimer = now;
        }
    }
};

inline Benchmark& getBenchmark() {
    static Benchmark instance;
    return instance;
}

#define ZONE_SCOPED(name) Benchmark::ScopedTimer UNIQUE_TIMER_NAME_##__LINE__(name, getBenchmark())