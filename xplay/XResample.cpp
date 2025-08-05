#include "XResample.h"
#include <cstdint>
#include <mutex>

extern "C"{
    #include <libswresample/swresample.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/frame.h>
}

XResample::XResample(void)
{
    
}

XResample::~XResample(void)
{

}

// 返回重采样后大小
int XResample::Resample(AVFrame *in,unsigned char *out)
{
    if(!in) return 0;
    if(!out){
        av_frame_free(&in);
    }
    uint8_t *out_data[2] = {0};
    out_data[0] = out;
    int re = swr_convert(swr_ctx_,
        out_data,in->nb_samples,
        in->data,in->nb_samples
    );
    int bytes_per_sample = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    int size = re*in->ch_layout.nb_channels * bytes_per_sample; // 样本数*通道数*采样字节数
    av_frame_free(&in);
    return size;
}



// 输出参数和输入参数一致除了采样格式，输出为S16。会释放para
bool XResample::Open(AVCodecParameters *para,bool destroyPara)
{
    if(!para) return false;
    
    std::lock_guard<std::mutex> lck(mtx_);

    // 先释放之前的上下文（如果存在）
    if(swr_ctx_) {
        swr_free(&swr_ctx_);
        swr_ctx_ = nullptr;
    }

    // 分配并初始化重采样上下文 swr_alloc_set_opts2 没有空间则创建
    int ret = swr_alloc_set_opts2(
        &swr_ctx_, // 创建新上下文
        &para->ch_layout,  // 输出声道布局
        out_sample_fmt_, // 输出采样格式设为S16
        para->sample_rate, // 输出采样率（保持与输入相同）
        &para->ch_layout,  // 输入声道布局
        (AVSampleFormat)para->format, // 输入采样格式
        para->sample_rate, // 输入采样率
        0, // 日志级别
        NULL
    );

    // 释放para
    if(destroyPara)
        avcodec_parameters_free(&para);
    
    if (!swr_ctx_ || ret < 0) {
        char err_buf[1024] = {0};
        av_strerror(ret, err_buf, sizeof(err_buf));
        fprintf(stderr, "swr_alloc_set_opts2 failed: %s\n", err_buf);
        return false;
    }

    // 初始化重采样上下文
    ret = swr_init(swr_ctx_);
    if (ret < 0) {
        char err_buf[1024] = {0};
        av_strerror(ret, err_buf, sizeof(err_buf));
        fprintf(stderr, "swr_init failed: %s\n", err_buf);
        swr_free(&swr_ctx_);
        return false;
    }
    
    return true;
}

void XResample::Close()
{
    std::lock_guard<std::mutex> lck(mtx_);

    if(swr_ctx_) {
        swr_free(&swr_ctx_);
        swr_ctx_ = nullptr;
    }
}