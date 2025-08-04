#pragma once

#include <memory>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/frame.h>
    #include <libavcodec/packet.h>
}

// 定义智能指针类型别名，模仿muduo库的风格
using AVCodecParametersPtr = std::shared_ptr<AVCodecParameters>;
using AVCodecContextPtr = std::shared_ptr<AVCodecContext>;
using AVFormatContextPtr = std::shared_ptr<AVFormatContext>;
using AVFramePtr = std::shared_ptr<AVFrame>;
using AVPacketPtr = std::shared_ptr<AVPacket>;

// AVCodecParameters的封装类，用于自动管理内存
class CodecParams {
public:
    // 构造函数
    CodecParams() : params_(avcodec_parameters_alloc()) {}
    
    // 析构函数，自动释放参数内存
    ~CodecParams() {
        if (params_) {
            avcodec_parameters_free(&params_);
        }
    }
    
    // 禁止拷贝构造和赋值
    CodecParams(const CodecParams&) = delete;
    CodecParams& operator=(const CodecParams&) = delete;
    
    // 支持移动构造和赋值
    CodecParams(CodecParams&& other) noexcept : params_(other.params_) {
        other.params_ = nullptr;
    }
    
    CodecParams& operator=(CodecParams&& other) noexcept {
        if (this != &other) {
            if (params_) {
                avcodec_parameters_free(&params_);
            }
            params_ = other.params_;
            other.params_ = nullptr;
        }
        return *this;
    }
    
    // 获取原始指针（用于FFmpeg函数调用）
    AVCodecParameters* get() { return params_; }
    const AVCodecParameters* get() const { return params_; }
    
    // 重载->操作符，方便访问成员
    AVCodecParameters* operator->() { return params_; }
    const AVCodecParameters* operator->() const { return params_; }
    
    // 重载*操作符
    AVCodecParameters& operator*() { return *params_; }
    const AVCodecParameters& operator*() const { return *params_; }
    
    // 显式转换为AVCodecParameters*
    operator AVCodecParameters*() { return params_; }
    operator const AVCodecParameters*() const { return params_; }

private:
    AVCodecParameters* params_;
};

// 使用智能指针管理CodecParams对象
using CodecParamsPtr = std::shared_ptr<CodecParams>;