#pragma once

#include <iomanip>
#include <vector>

/**
 * NAL Unit in h264/h265 streams.
 * 
 * h264 with 4 start bytes:
 * | 0x00 | 0x00 | 0x00 | 0x01 |     0x65     |    ....   |
 * --------------------------------------------------------
 * |        start bytes        |   head bytes | body data |
 * 
 * h265 with 4 start bytes:
 * | 0x00 | 0x00 | 0x00 | 0x01 | 0x65 | 0x48 |    ....   |
 * -------------------------------------------------------
 * |        start bytes        |  head bytes | body data |
 * 
 */
struct nal_unit {
    /**
     * the start pointer of packet to be analysed.
     */
    const unsigned char* data;

    /**
     * the codec type to be analysed, 264 or 265. 
     */
    int codec_type;

    /**
     * the index of NAL Unit in packet, begin from 0.
     */
    int index;

    /**
     * the offset value of NAL Unit relative to start pointer of packet, begin from 0.
     */
    unsigned long offset;

    /** 
     * the size of whole NAL Unit in bytes, including start bytes and head bytes.
     */
    unsigned long nal_length;

    /** 
     * the type of NAL Unit, differ according to codec type.
     */
    int nal_type;

    /** 
     * the type name(description) of NAL Unit, differ according to codec type.
     */
    std::string nal_type_name;

    /** 
     * the start bytes of NAL Unit, 3 bytes(0x000001) or 4 bytes(0x00000001).
     */
    std::vector<unsigned char> start_bytes;

    /** 
     * the head bytes of NAL Unit, only 1 byte for h264, and 2 bytes for h265.
     */
    std::vector<unsigned char> head_bytes;
};

/**
 * bit stream analysis for video (only support annexb format with h264 or h265), mainly used to parse out NAL units (index, offset, type, length) from byte packets before decoding or after encoding.
 * 
 * we can determine if ignore packets or not according to the analysis. for example, 
 * 1. ignore the IDR NAL unit which has no SPS or PPS units before it.
 * 2. ignore all non-IDR NAL units when write to file/ send to network unitl a IDR NAL unit appears.
 */
class stream_analyser {
private:
    /**
     * the start pointer of packet to be analysed.
     */
    const unsigned char* m_data;

    /**
     * the size of packet to be analysed in bytes. 
     */
    unsigned long m_length;

    /**
     * current position for inner usage.
     */
    unsigned long m_current_pos;

    /**
     * current index for inner usage.
     */
    int m_nal_index;

    /**
     * h265 or not.
     */
    bool m_is_h265;

    /**
     * find start bytes from specific position.
     */
    unsigned long find_start_bytes(unsigned long pos);
    
    /**
     * get h264 NAL type name from type id.
     */
    std::string get_h264_nal_type_name(int nal_type);
    
    /**
     * get h265 NAL type name from type id.
     */
    std::string get_h265_nal_type_name(int nal_type);
    
    /**
     * parse out a NAL Unit using index, offset, length.
     * 
     * @note
     * it will always succeed because has checked the data before calling this function.
     */
    nal_unit parse_nal_unit(int index, unsigned long offset, unsigned long length);
public:
    /**
     * create stream_analyser instance using initial parameters.
     * 
     * @param data start pointer of packet to be analysed.
     * @param length size of packet in bytes.
     * @param is_h265 h265 or not (not by default).
     * 
     * @note
     * packet can be any format of byte streams, not just something like `AVPacket->data` returned from `av_read_frame` in FFmpeg.
     * we can read batch of bytes manually from h264/h265 file also, and then using `stream_analyser` to analyse it.
     */
    stream_analyser(const unsigned char* data, unsigned long length, bool is_h265 = false);

    /**
     * analyse packet and parse out NAL Units from packet.
     * 
     * @param nal_units [out]NAL Units parsed out successfully.
     * 
     * @return
     * true if succeed: at least 1 NAL Unit parsed out from packet. 
     * false otherwise: no NAL Unit parsed out from packet at all.
     * 
     * @note
     * it works based on start bytes and head bytes of NAL Unit, and no care about its body data.
     * so it could not guarantee the NAL Unit which has been parsed out successfully is valid and data complete.
     * 
     * make sure the packet to be analysed is large enough and contains at least 1 NAL Unit.
     */
    bool analyse(std::vector<nal_unit>& nal_units);

    /**
     * format bytes to string of hex.
     * 
     * examples:
     * 4 bytes(0,103,75,217) to: 0x00674BD9
     * 
     * @param bytes bytes to be converted.
     */
    static std::string to_hex(const std::vector<unsigned char>& bytes, bool add_hex_flag = true);
};