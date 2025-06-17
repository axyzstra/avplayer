#include <iostream>
#include <string>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
}

int main() {
    const char* input_filename = "F:/test/1.mp4";
    
    AVFormatContext* format_context = nullptr;
    
    // 打开文件
    int ret = avformat_open_input(&format_context, input_filename, nullptr, nullptr);
    if (ret < 0) {
        std::cerr << "Cannot open input file: " << input_filename << std::endl;
        return -1;
    }
    
    std::cout << "Successfully opened file: " << input_filename << std::endl;
    std::cout << "AVIOContext created automatically" << std::endl;
    
    // 加载流信息
    ret = avformat_find_stream_info(format_context, nullptr);
    if (ret < 0) {
        std::cerr << "Cannot get stream info" << std::endl;
        avformat_close_input(&format_context);
        return -1;
    }
    
    std::cout << "File contains " << format_context->nb_streams << " streams" << std::endl;
    
    // 找到视频流
    int video_stream_index = -1;
    for (unsigned int i = 0; i < format_context->nb_streams; i++) {
        if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    
    if (video_stream_index == -1) {
        std::cerr << "No video stream found" << std::endl;
        avformat_close_input(&format_context);
        return -1;
    }
    
    std::cout << "Found video stream, index: " << video_stream_index << std::endl;
    
    // 获取视频流解码器参数
    AVStream* video_stream = format_context->streams[video_stream_index];
    AVCodecParameters* codec_params = video_stream->codecpar;
    
    // 定义一致的解码器
    const AVCodec* codec = avcodec_find_decoder(codec_params->codec_id);
    if (!codec) {
        std::cerr << "Decoder not found" << std::endl;
        avformat_close_input(&format_context);
        return -1;
    }
    
    std::cout << "Found decoder: " << codec->name << std::endl;
    
    // 分配解码器 ctx
    AVCodecContext* codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        std::cerr << "Cannot allocate codec context" << std::endl;
        avformat_close_input(&format_context);
        return -1;
    }
    
    // 设置解码器参数
    ret = avcodec_parameters_to_context(codec_context, codec_params);
    if (ret < 0) {
        std::cerr << "Cannot copy codec parameters" << std::endl;
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }
    
    // 打开解码器
    ret = avcodec_open2(codec_context, codec, nullptr);
    if (ret < 0) {
        std::cerr << "Cannot open decoder" << std::endl;
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }
    
    std::cout << "Decoder initialized successfully" << std::endl;
    std::cout << "Video resolution: " << codec_context->width << "x" << codec_context->height << std::endl;
    
    // 分配 packet 存储压缩数据
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        std::cerr << "Cannot allocate packet" << std::endl;
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }
    
    // 分配 frame 存储原始数据
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "Cannot allocate frame" << std::endl;
        av_packet_free(&packet);
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }
    
    std::cout << std::endl << "Starting to read and decode video frames..." << std::endl;
    
    int frame_count = 0;
    const int max_frames = 10;
    
    // 循环读取 packet 并进行解码
    while (av_read_frame(format_context, packet) >= 0 && frame_count < max_frames) {
        if (packet->stream_index == video_stream_index) {
            // 将 packet 数据放入解码器
            ret = avcodec_send_packet(codec_context, packet);
            if (ret < 0) {
                std::cerr << "Failed to send packet to decoder" << std::endl;
                break;
            }
            
            // 循环接收解码器数据到 frame
            while (ret >= 0) {
                ret = avcodec_receive_frame(codec_context, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    std::cerr << "Failed to receive decoded frame" << std::endl;
                    break;
                }
                
                frame_count++;
                std::cout << "Decoded frame " << frame_count << " - ";
                std::cout << "Format: " << av_get_pix_fmt_name((AVPixelFormat)frame->format) << ", ";
                std::cout << "PTS: " << frame->pts << ", ";
                std::cout << "Key frame: " << (frame->key_frame ? "Yes" : "No") << std::endl;
                av_frame_unref(frame);
            }
        }
        av_packet_unref(packet);
    }
    
    std::cout << std::endl << "Total processed " << frame_count << " video frames" << std::endl;
    
    // 处理剩余数据
    std::cout << "Processing remaining frames in buffer..." << std::endl;
    avcodec_send_packet(codec_context, nullptr);
    
    while (avcodec_receive_frame(codec_context, frame) >= 0) {
        frame_count++;
        std::cout << "Buffered frame " << frame_count << " - PTS: " << frame->pts << std::endl;
        av_frame_unref(frame);
    }
    
    // 释放资源
    std::cout << std::endl << "Starting to release resources..." << std::endl;
    
    av_frame_free(&frame);
    av_packet_free(&packet);
    std::cout << "AVFrame and AVPacket released" << std::endl;
    
    avcodec_free_context(&codec_context);
    std::cout << "AVCodecContext released" << std::endl;
    
    avformat_close_input(&format_context);
    std::cout << "AVFormatContext closed" << std::endl;

    std::cout << std::endl << "Video processing completed!" << std::endl;
    return 0;
}