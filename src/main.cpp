#include "gb.hpp"
#include "displays/sdl_io.hpp"
#include "displays/headless.hpp"

#include <iomanip>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <chrono>

std::array<std::array<const char *, 2>, 14> testROMs = {{
    {{"Blargg's CPU Tests",             "tests/cpu_instrs/cpu_instrs.gb"}},
    //Specific tests for extra debug info
    {{"   01-Special",      "tests/cpu_instrs/individual/01-special.gb"}},
    {{"   02-Interrupts",   "tests/cpu_instrs/individual/02-interrupts.gb"}},
    {{"   03-OP SP HL",     "tests/cpu_instrs/individual/03-op sp,hl.gb"}},
    {{"   04-OP i imm",     "tests/cpu_instrs/individual/04-op r,imm.gb"}},
    {{"   05-OP rp",        "tests/cpu_instrs/individual/05-op rp.gb"}},
    {{"   06-LD r r",       "tests/cpu_instrs/individual/06-ld r,r.gb"}},
    {{"   07-JP, CALL",     "tests/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb"}},
    {{"   08-Misc",         "tests/cpu_instrs/individual/08-misc instrs.gb"}},
    {{"   09-OP r r",       "tests/cpu_instrs/individual/09-op r,r.gb"}},
    {{"   10-BIT OPs",      "tests/cpu_instrs/individual/10-bit ops.gb"}},
    {{"   11-OP a,(HL)",    "tests/cpu_instrs/individual/11-op a,(hl).gb"}},

    {{"Blargg's Instructions Timing",   "tests/instr_timing/instr_timing.gb"}},
    {{"Blargg's Memory Timing 1",       "tests/mem_timing/mem_timing.gb"}}
}};


bool passesTest(const char *testROM)
{
    /*
    Returns a boolean: true if the test ROM passes.
    Will also log failure info to console if available.
    Test ROM is required to give serial output indicating failure status.
    - If the ROM gives no output before the timeout: return false
    - If the Emulator throws an exception:           return false
    - If the ROM outputs a failure code:             return false
    */

    std::streamsize previousSize; // Used for timeout
    std::stringstream output;
    Headless headless(output); // Fake display and IO

    try
    {
        // Generate a new cartridge and GameBoy for repeatability
        Cartridge card = Cartridge::loadRom(testROM);
        GB gb(card, headless);

        for(unsigned int i = 0; i < (1<<25); i++)
        {
            gb.update();

            // Check output every few CPU cycles
            if(i%0x1000 == 0)
            {
                // Check if test has already passed or failed
                if(output.str().find("Passed") != std::string::npos)    return true;
                if(output.str().find("Failed") != std::string::npos)    return false;

                // If test's still generating output prevent timout
                std::streamsize currentSize = output.gcount();
                if(currentSize != previousSize)
                {
                    previousSize = currentSize;
                    i = 0;
                }
            }
        }
        // Test probably got stuck in an infinite loop (or can't be automated)
        throw std::runtime_error("ROM timeout"); // Give an error hint
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << " -- ";
        return false;
    }
}

bool passesAllTests()
{
    /*
    Runs through all automated test ROMs and lists their status.
    Returns true if all tests pass.
    Automated ROMs are required to give a serial output and run in headless mode.
    */
    int testsRan = 0;
    int testsPassed = 0;
    std::cout << "Running test ROMs..." << std::endl;
    for(auto testROM : testROMs)
    {
        testsRan++;
        std::cout << "  " << std::left << std::setw(30); //Align to grid
        std::cout << testROM[0] << ": " << std::flush; //Prints the active test

        if(passesTest(testROM[1]))
        {
            testsPassed++;
            std::cout << "Passed" << std::endl;
            continue;
        }
        std::cerr << "Failed" << std::endl;
    }
    std::cout << std::endl;

    if(testsPassed == testsRan)
    {
        std::cout << "All Tests Passed!" << std::endl;
        return true;
    }
    std::cerr << testsPassed << "/" << testsRan << " Tests Passed!";
    return false;
}


void runBenchmarkHeadless(const char* rom, uint64_t updates)
{
    /*
    Loads a rom and times its emulation for a given number of updates.
    The internal clock of the GameBoy is used to determine the emulator's performance.
    Rom is run in headless mode... only CPU performance is measured.
    */
    using namespace std::chrono;

    std::stringstream serialOut;
    Headless display(serialOut);
    Cartridge card = Cartridge::loadRom(rom);

    GB gb(card, display);
    std::cout << "ROM loaded!" << std::endl;
    std::cout << "Starting Benchmark of " << updates << " updates" << std::endl;

    // Only start clock after GameBoy is loaded to memory
    high_resolution_clock::time_point start = high_resolution_clock::now();
    for(uint64_t i = 0; i < updates; i++)
    {
        gb.update();
    }
    high_resolution_clock::time_point end = high_resolution_clock::now();

    duration<double, std::milli> time = end-start;
    double emulatorTime = time.count();
    double gameboyTime = ((double)gb.io.cycle * 1000.0)/((double)FREQUENCY);
    double speedMult = gameboyTime/emulatorTime;

    std::cout << "Completed Benchmark in: " << emulatorTime << "ms" << std::endl << std::endl;
    std::cout << "Cycles completed: " << gb.io.cycle << " (" << gameboyTime << "ms)" << std::endl;
    std::cout << "The emulator runs @ " << (59.73 * speedMult) << " fps" << std::endl;
    std::cout << "That is " << speedMult << "x faster than the GameeBoy" << std::endl;
}

void runGame(const char* rom)
{
    /*
    Creates a graphical output and emulates the chosen ROM at 60Hz.
    Input, output and timing is handled by the SDL_IO class.
    */
    SDL_IO display;
    Cartridge card = Cartridge::loadRom(rom);

    GB gb(card, display);

    while(true)
    {
        gb.update();
    }
}


int main(int argc, char *argv[])
{
    switch (argc)
    {
    case 1:
        // Test mode
        if(!passesAllTests()) return EXIT_FAILURE;
        break;
    case 2:
        // Standard game mode
        runGame(argv[1]);
        break;
    case 3:
        // Benchmarking mode
        runBenchmarkHeadless(argv[1], atoll(argv[2]));
        break;
    default:
        std::cerr << "Program requires 1 argument: ROM Path" << std::endl;
        break;
    }
    return EXIT_SUCCESS;
}