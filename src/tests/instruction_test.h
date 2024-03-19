#ifndef INSTRUCTION_TEST_H
#define INSTRUCTION_TEST_H

#include "../Z80.h"
#include "../Memory.h"
#include <iostream>
#include <functional>
#include <string>
#include <vector>

// A test case structure to hold information about each instruction test
struct TestCase {
    std::string description;                               // A description of what the test is checking
    std::function<void(Z80&, Spectrum48KMemory&)> setup;   // A setup function to prepare the CPU and memory state
    std::function<bool(Z80&, Spectrum48KMemory&)> test;    // The test to run, which should return true if the test passes
};

// Runs all tests for the Z80 CPU
void runAllTests();

#endif // INSTRUCTION_TEST_H
