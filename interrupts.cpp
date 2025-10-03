/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 * @author Edited by Matthe Bekkers
 *
 */

#include"interrupts.hpp"
#include<fstream>
#include<vector>
#include<string>




int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elements is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;
    std::string execution;
    
    /******************ADD YOUR VARIABLES HERE*************************/
    

    static constexpr int KERNEL_SWITCH_MS = 1;
    static constexpr int CTX_SAVE_MS = 10;
    static constexpr int VECTOR_CALC_MS = 1;
    static constexpr int GET_ISR_MS = 1;
    static constexpr int ISR_BODY_MS = 40;
    static constexpr int IRET_MS = 1;

    int time = 0;
    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, value] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/
        
        
        if (activity == "CPU") {
            if (value > 0){
                execution += std::to_string(time) + ", " + std::to_string(value) + ", CPU burst\n";
                time += value;
            }
        } else if (activity == "SYSCALL" || activity == "END_IO") {
            
            int dev = value;
            std::string isr = (dev >= 0 && dev < (int)vectors.size()) ? vectors[dev] : "0x0000";
            std::string label = (activity == "SYSCALL" ? "syscall dev " : "end_io dev ") + std::to_string(dev);

            // switch to kernel
            execution += std::to_string(time) + ", " + std::to_string(KERNEL_SWITCH_MS)
                    + ", switch to kernel mode (" + label + ")\n";
            time += KERNEL_SWITCH_MS;

            // save context
            execution += std::to_string(time) + ", " + std::to_string(CTX_SAVE_MS)
                    + ", context saved\n";
            time += CTX_SAVE_MS;

            // find vector entry
            execution += std::to_string(time) + ", " + std::to_string(VECTOR_CALC_MS)
                    + ", find vector " + std::to_string(dev) + "\n";
            time += VECTOR_CALC_MS;

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
}
 
        
        
        else if (activity == "SYSCALL" || activity == "END_IO"){
            int dev = value;
            std::string isr = (dev >= 0 && dev < (int)vectors.size()) ? vectors[dev] : "0x0000";
            std::string label = (activity == "SYSCALL" ? "syscall dev" : "end_io dev") + std::to_string(dev);

            //switch to kernel
            execution += std::to_string(time) + ", " + std::to_string(KERNEL_SWITCH_MS) + ", switch to kernel mode (" + label + ")\n";
            time += KERNEL_SWITCH_MS;
            
            //save context
            execution += std::to_string(time) + ", " + std::to_string(CTX_SAVE_MS) + ", context saved\n";
            time += CTX_SAVE_MS;

            // find vector entry
            execution += std::to_string(time) + ", " + std::to_string(VECTOR_CALC_MS) + ", find vector " + std::to_string(dev) + "\n";
            time += VECTOR_CALC_MS;

            //obtain ISR address
            execution += std::to_string(time) + ", " + std::to_string(ISR_BODY_MS) + ", execute ISR body (device " + std::to_string(dev) + ")\n";
            time += ISR_BODY_MS;
            // execute ISR body (kept constant to stay tiny/simple)
            execution += std::to_string(time) + ", " + std::to_string((dev >=0 && dev < (int)delays.size()) ? delays[dev] : ISR_BODY_MS);

            // IRET
            execution += std::to_string(time) + ", " + std::to_string(IRET_MS) + ", IRET\n";
            time += IRET_MS;

            if (activity == "SYSCALL") {
                int d = (dev >= 0 && dev < (int)delays.size()) ? delays[dev] : 0;
                execution += std::to_string(time) + ", 1, issue I/O to device " + std::to_string(dev) + " (avg delay " + std::to_string(d) + " ms)\n";
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
