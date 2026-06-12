/*
    Source: https://medium.com/@keshavdikhani1/designing-a-distributed-unique-id-generator-in-c-snowflake-explained-with-implementation-55ae65519a7c
*/

#include <iostream>
#include <chrono>
#include <atomic>
#include <mutex>

using namespace std;

class Snowflake {
private:
    const long long epoch = 1767225600000ULL; // Jan 1, 2026 (custom epoch)

    const int machineIdBits = 10;
    const int sequenceBits = 12;

    const long long maxMachineId = (1LL << machineIdBits) - 1;
    const long long maxSequence = (1LL << sequenceBits) - 1;

    long long machineId;
    long long lastTimestamp;
    long long sequence;
    mutex mtx;

    long long currentTimeMillis() {
        return chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    long long waitNextMillis(long long timestamp) {
        long long now = currentTimeMillis();
        while (now <= timestamp) {
            now = currentTimeMillis();
        }
        return now;
    }

public:
    Snowflake(long long machineId) {
        if (machineId > maxMachineId || machineId < 0) {
            throw invalid_argument("Invalid machine ID");
        }
        this->machineId = machineId;
        lastTimestamp = -1;
        sequence = 0;
    }

    long long generate() {
        lock_guard<mutex> lock(mtx);

        long long timestamp = currentTimeMillis();

        if (timestamp < lastTimestamp) {
            throw runtime_error("Clock moved backwards!");
        }

        if (timestamp == lastTimestamp) {
            sequence = (sequence + 1) & maxSequence;

            if (sequence == 0) {
                timestamp = waitNextMillis(lastTimestamp);
            }
        } else {
            sequence = 0;
        }

        lastTimestamp = timestamp;

        return ((timestamp - epoch) << 22) |
               (machineId << 12) |
               sequence;
    }
};