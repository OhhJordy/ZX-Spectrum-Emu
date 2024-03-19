#include "instruction_test.h"

// Define a global vector to hold all our test cases
std::vector<TestCase> allTests;

// Add a test case to the global test vector
void addTestCase(const TestCase& testCase) {
    allTests.push_back(testCase);
}

// Function to initialize and add test cases
void initializeTests() {
    // Example test: NOP Instruction
    addTestCase({
        "NOP instruction",
        [](Z80& cpu, Spectrum48KMemory& mem) { 
            // Setup: Ensure PC starts at 0
            cpu.getRegisters()->PC = 0;
            mem[0] = 0x00; // NOP instruction
            std::cout << "Test setup complete: NOP instruction at PC=0." << std::endl;
        },
        [](Z80& cpu, Spectrum48KMemory& mem) -> bool {
            std::cout << "Test execution started." << std::endl;
            cpu.nextInstruction();
            std::cout << "Test execution finished." << std::endl;

            // Log the result of executing the instruction
            std::cout << "After NOP: PC=" << cpu.getRegisters()->PC << std::endl;
            return cpu.getRegisters()->PC == 1; // NOP should advance PC by 1
        }
    });

    // Add more tests here
    std::cout << "All tests initialized." << std::endl; // Debugging output
}

// The runner function that goes through each test case
void runAllTests() {
    std::cout << "Starting tests..." << std::endl;

    try {
        Z80 cpu(nullptr, nullptr, nullptr); // Adjust as necessary for your Z80 constructor
        Spectrum48KMemory mem;
        initializeTests();

        int passedTests = 0;
        for (const auto& test : allTests) {
            std::cout << "Running test: " << test.description << std::endl;
            try {
                test.setup(cpu, mem);
                bool result = test.test(cpu, mem);
                if (result) {
                    std::cout << "Passed: " << test.description << std::endl;
                    passedTests++;
                } else {
                    std::cout << "Failed: " << test.description << std::endl;
                }
            } catch (const std::exception& e) {
                std::cout << "Exception caught during test \"" << test.description << "\": " << e.what() << std::endl;
            } catch (...) {
                std::cout << "Unknown exception caught during test \"" << test.description << "\"." << std::endl;
            }
        }

        std::cout << "Tests passed: " << passedTests << "/" << allTests.size() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception caught in runAllTests: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception caught in runAllTests." << std::endl;
    }
}

// Remember to call runAllTests() in your main or wherever appropriate
