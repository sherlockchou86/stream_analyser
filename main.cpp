
#include <iostream>
#include <fstream>
#include <vector>
#include <assert.h>
#include "stream_analyser.h"

/**
 * run command usage:  
 * (1)  ./stream_analyser video1.h264 h264
 * (2)  ./stream_analyser video2.h265 h265
 * 
 * note only work fine for h264 or h265 raw data.
*/
int main(int argc, char* argv[]) {
    // process name, file name, video codec type
    assert(argc == 3);
    std::string filename(argv[1]);
    bool is_h264 = std::string(argv[2]) == std::string("h264");
    std::ifstream file(filename, std::ios::binary);
    
    auto read_len = 1024 * 1024; // read 1MB bytes each time
    auto real_read_len = 0;
    unsigned char read_bytes[read_len];
    auto total_index = 0;
    auto total_bytes = 0;
    auto total_times = 0;
    std::vector<nal_unit> total_nal_units;

    do {
        file.read(reinterpret_cast<char*>(read_bytes), read_len);
        real_read_len = file.gcount();
        total_times++;
        if (real_read_len) {
            total_bytes += real_read_len;
            stream_analyser analyser(read_bytes, real_read_len, !is_h264);
            
            std::vector<nal_unit> nal_units;
            auto ret = analyser.analyse(nal_units);

            total_nal_units.insert(total_nal_units.end(), nal_units.begin(), nal_units.end());
        }
    } while (real_read_len);

    std::ostringstream oss;
    oss << "total read times:" << total_times << std::endl;
    oss << "total read bytes:" << total_bytes << std::endl;

    /**
     * NAL index relative to the whole video data (start from 0),
     * NAL index relative the readed data (start from 0),
     * offset for current NAL unit relative to readed data (bytes),
     * length of current NAL unit (bytes),
     * start bytes & head bytes for current NAL unit,
     * NAL type (int),
     * NAL type name (string)
    */
    oss << std::setw(8) << std::setfill(' ') << "index"
        << std::setw(8) << std::setfill(' ') << "i-index" 
        << std::setw(16) << std::setfill(' ') << "i-offset"
        << std::setw(8) << std::setfill(' ') << "length" 
        << std::setw(24) << std::setfill(' ') << "start-flag"
        << std::setw(16) << std::setfill(' ') << "nal-type"
        << std::setw(24) << std::setfill(' ') << "nal-type-name" << std::endl;
    for (auto& nal: total_nal_units) {
        oss << std::setw(8) << std::setfill(' ') << total_index++
            << std::setw(8) << std::setfill(' ') << nal.index 
            << std::setw(16) << std::setfill(' ') << nal.offset
            << std::setw(8) << std::setfill(' ') << nal.nal_length 
            << std::setw(24) << std::setfill(' ') << stream_analyser::to_hex(nal.start_bytes) << stream_analyser::to_hex(nal.head_bytes, false) 
            << std::setw(16) << std::setfill(' ') <<  nal.nal_type
            << std::setw(24) << std::setfill(' ') << nal.nal_type_name << std::endl;
    }

    std::cout << oss.str();

    std::ofstream ofile("./analyse.txt", std::ios::out);
    ofile.write(oss.str().c_str(), oss.str().length());
}