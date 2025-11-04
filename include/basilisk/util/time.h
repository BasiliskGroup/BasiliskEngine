#ifndef BSK_TIME_H
#define BSK_TIME_H

#include <basilisk/util/includes.h>
#include <basilisk/util/print.h>
#include <chrono>
#include <thread>

namespace bsk::internal {

std::chrono::time_point<std::chrono::high_resolution_clock> timeNow();

void printTimeNow();

void sleepS(uint seconds);
void sleepMS(uint milliseconds);
void sleepUS(uint microseconds);

void printDurationUS(std::chrono::time_point<std::chrono::high_resolution_clock> t1, std::chrono::time_point<std::chrono::high_resolution_clock> t2, std::string title);

void printPrimalDuration(std::chrono::time_point<std::chrono::high_resolution_clock> t1, std::chrono::time_point<std::chrono::high_resolution_clock> t2);

void printDualDuration(std::chrono::time_point<std::chrono::high_resolution_clock> t1, std::chrono::time_point<std::chrono::high_resolution_clock> t2);

}

#endif