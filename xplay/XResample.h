#pragma once

#include <mutex>

class AVCodecParameters;
class SwrContext;
class AVFrame;
class AVFormatContext;

extern "C" {
    #include <libavutil/samplefmt.h>
}

// 音频重采样
class XResample
{
public:
	XResample(void);
	~XResample(void);

    // 返回重采样后的数据长度。不管成功与否内部都释放in空间
    virtual int Resample(AVFrame *in,unsigned char *out);

    // 输出参数和输入参数一致除了采样格式，输出为S16
    virtual bool Open(AVCodecParameters *para);
    virtual void Close();

protected:
    std::mutex mtx_;
    SwrContext *swr_ctx_ = nullptr;
    AVSampleFormat out_sample_fmt_ = AV_SAMPLE_FMT_S16;
};
