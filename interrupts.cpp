/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 * 
 * @author Matthe Bekkers
 * @author Tomas Alvarez
 *
 */

#include"interrupts.hpp"
#include<fstream>
#include<vector>
#include<string>



static constexpr int KERNEL_SWITCH_MS = 1;
static constexpr int CTX_SAVE_MS = 10;
static constexpr int VECTOR_CALC_MS = 1;
static constexpr int GET_ISR_MS = 1;
static constexpr int ISR_BODY_MS = 40;
static constexpr int IRET_MS = 1;

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elements is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< string to store single line of trace file
    std::string execution = "";  //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/
    

    int current_time = 0; // global timer to track elapsed time
    int intr_count = 0; // incremented every time an interrupt is called from the table
    int ctx_time = 10; // ms to switch contextx
    bool isr_state = true; // tracks isr state
    std::pair<std::string , int> interrupt_boilerplate_out;

    int time = 0;
    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, value] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/

        if (activity == "CPU") {
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU burst\n"; // any CPU action
            current_time += duration_intr;
        }
        else if (activity == "SYSCALL") {
            execution += std::to_string(current_time) + ", 1, check interrupts enabled\n"; // must first check that interrupts are enabled
            current_time++;                                                              // assume that they are for this assignment

            execution += std::to_string(current_time) + ", 1, check flags\n"; // check that flags are properly set, assume they are
            current_time++;

            interrupt_boilerplate_out = intr_boilerplate(current_time, intr_count, ctx_time, vectors); // get basic interrupt actions

            execution += interrupt_boilerplate_out.first;
            current_time = interrupt_boilerplate_out.second;

            // now start executing the ISR
            // this assumes the system will always alternate between a SYSCALL and an END_IO

            // set flags. note that we are arbitrarly the tasks into pre-timed chunks; real-life durations will vary
            // 5% done
            execution += std::to_string(current_time) + ", " + std::to_string((int) round(delays[duration_intr] * 0.05)) + ", set flag\n";
            current_time += round(delays[duration_intr] * 0.05);

            if (isr_state) {
                // send data to buffer, 45% done
                execution += std::to_string(current_time) + ", " + std::to_string((int) round(delays[duration_intr] * 0.4)) + ", get data to buffer\n";
                current_time += round(delays[duration_intr] * 0.4);

                // call driver
                execution += std::to_string(current_time) + ", " + std::to_string((int) round(delays[duration_intr] * 0.55)) + ", call driver\n";
                current_time += round(delays[duration_intr] * 0.55);

                // swap ISR state (we assume END_IO is next)
                isr_state = false;
            }
            else {
                // call driver
                execution += std::to_string(current_time) + ", " + std::to_string((int) round(delays[duration_intr] * 0.55)) + ", call driver\n";
                current_time += round(delays[duration_intr]);

                // read buffer
                execution += std::to_string(current_time) + ", " + std::to_string((int) round(delays[duration_intr] * 0.4)) + ", get data from buffer\n";
                current_time += round(delays[duration_intr]);

                isr_state = true;
            }
        } else if (activity == "END_IO") {
            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time++;

            execution += std::to_string(current_time) + ", " + std::to_string(delays[duration_intr]) + ", END I/O\n";
            current_time += delays[duration_intr];
            execution += std::to_string(current_time) + ", " + std::to_string(ctx_time) + ", Restore context\n";
            current_time += ctx_time;
        }
        
        intr_count++;
        /************************************************************************/

            // obtain ISR address (required by spec)
            execution += std::to_string(time) + ", " + std::to_string(GET_ISR_MS)
                    + ", obtain ISR address " + isr + "\n";
            time += GET_ISR_MS;

            // execute ISR body (simple fixed 40 ms per spec’s “each activity ≈ 40ms”)
            execution += std::to_string(time) + ", " + std::to_string(ISR_BODY_MS)
                    + ", execute ISR body (device " + std::to_string(dev) + ")\n";
            time += ISR_BODY_MS;



            // IRET
            execution += std::to_string(time) + ", " + std::to_string(IRET_MS) + ", IRET\n";
            time += IRET_MS;

            // Optional: note I/O issuance on SYSCALL for readability
            if (activity == "SYSCALL") {
                int d = (dev >= 0 && dev < (int)delays.size()) ? delays[dev] : 0;
                execution += std::to_string(time) + ", 1, issue I/O to device "
                        + std::to_string(dev) + " (avg delay " + std::to_string(d) + " ms)\n";
                time += 1;
            }
            
         } else if (!activity.empty()) {
            // Unknown line: log and continue (duration 0 to keep timeline intact)
            execution += std::to_string(time) + ", 0, unknown activity: " + activity + "\n";
        };
            
            


        /************************************************************************/
    };

    input_file.close();

    write_output(execution);

    return 0;
}
