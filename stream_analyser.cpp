#include "stream_analyser.h"

stream_analyser::stream_analyser(const unsigned char* data, 
                                    unsigned long length, 
                                    bool is_h265):
                                    m_data(data),
                                    m_length(length),
                                    m_is_h265(is_h265),
                                    m_current_pos(0),
                                    m_nal_index(0) {
}

unsigned long stream_analyser::find_start_bytes(unsigned long pos) {
    for (size_t i = pos; i + 3 < m_length; ++i) {
        if (m_data[i] == 0x00 && m_data[i + 1] == 0x00 && m_data[i + 2] == 0x01) {
            return i;
        }
        if (i + 4 < m_length && m_data[i] == 0x00 && m_data[i + 1] == 0x00 && m_data[i + 2] == 0x00 && m_data[i + 3] == 0x01) {
            return i;
        }
    }
    return std::string::npos;
}

std::string stream_analyser::get_h264_nal_type_name(int nal_type) {
    switch (nal_type) {
        case 1: return "Non-IDR Slice";
        case 5: return "IDR Slice";
        case 6: return "SEI";
        case 7: return "SPS";
        case 8: return "PPS";
        default: return "Other";
    }
}

std::string stream_analyser::get_h265_nal_type_name(int nal_type) {
    switch (nal_type) {
        case 32: return "VPS";
        case 33: return "SPS";
        case 34: return "PPS";
        case 39: return "SEI";
        case 19: return "IDR_W_RADL";
        case 20: return "IDR_N_LP";
        default: return "Other";
    }
}

nal_unit stream_analyser::parse_nal_unit(int index, unsigned long offset, unsigned long length) {
    const unsigned char* nal_data = m_data + offset;
    bool long_start_code = (nal_data[2] == 0x01) ? false : true;
    int nal_type;
    std::string nal_type_name;

    if (m_is_h265) {
        nal_type = nal_data[long_start_code ? 4 : 3] >> 1 & 0x3F;                 // type for H.265
        nal_type_name = get_h265_nal_type_name(nal_type);
    } else {
        nal_type = (long_start_code) ? nal_data[4] & 0x1F : nal_data[3] & 0x1F;   // type fot H.264
        nal_type_name = get_h264_nal_type_name(nal_type);
    }

    /* fill properties of NAL Unit. */
    nal_unit nal;
    nal.codec_type = m_is_h265 ? 265: 264;
    nal.data = m_data;
    nal.index = index;
    nal.nal_length = length;
    nal.nal_type = nal_type;
    nal.nal_type_name = nal_type_name;
    nal.offset = offset;
    if (m_is_h265) {
        nal.start_bytes.push_back(nal_data[0]);
        nal.start_bytes.push_back(nal_data[1]);
        nal.start_bytes.push_back(nal_data[2]);
        if (long_start_code) {
            nal.start_bytes.push_back(nal_data[3]);
            nal.head_bytes.push_back(nal_data[4]);
            nal.head_bytes.push_back(nal_data[5]);
        }
        else {
            nal.head_bytes.push_back(nal_data[3]);
            nal.head_bytes.push_back(nal_data[4]);
        }
    }
    else {
        nal.start_bytes.push_back(nal_data[0]);
        nal.start_bytes.push_back(nal_data[1]);
        nal.start_bytes.push_back(nal_data[2]);
        if (long_start_code) {
            nal.start_bytes.push_back(nal_data[3]);
            nal.head_bytes.push_back(nal_data[4]);
        }
        else {
            nal.head_bytes.push_back(nal_data[3]);
        }
    }
    
    return nal;
}

bool stream_analyser::analyse(std::vector<nal_unit>& nal_units) {
    nal_units.clear();
    while (m_current_pos < m_length) {
        auto nal_start = find_start_bytes(m_current_pos);
        if (nal_start == std::string::npos) {
            return false;
        }
        auto nal_end = find_start_bytes(nal_start + 4);
        auto nal_length = (nal_end == std::string::npos) ? m_length - nal_start : nal_end - nal_start;

        auto nal_unit = parse_nal_unit(m_nal_index++, nal_start, nal_length);
        nal_units.push_back(nal_unit);

        m_current_pos = (nal_end == std::string::npos) ? m_length : nal_end;
    }
    m_current_pos = 0;
    m_nal_index = 0;

    return true;
}

std::string stream_analyser::to_hex(const std::vector<unsigned char>& bytes, bool add_hex_flag) {
    std::ostringstream oss;
    if (add_hex_flag) {
        oss << "0x";
    }

    for (auto& b: bytes) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    }
    return oss.str();
}