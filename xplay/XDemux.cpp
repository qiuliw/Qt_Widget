#include "XDemux.h"
#include <mutex>
#include <iostream>
#include <qdebug.h>
#include <qlogging.h>

extern "C" {
    #include <libavutil/error.h>
    #include <libavcodec/codec.h>
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include <libavcodec/packet.h>
    #include <libavutil/avutil.h>
    #include <libavformat/avio.h>
    #include <libavcodec/codec_par.h>
    #include <libavutil/frame.h>
    #include <libswscale/swscale.h>
    #include <libavutil/pixfmt.h>
    #include <libswresample/swresample.h>
    #include <libavutil/channel_layout.h> // 新增：声道布局相关
}

XDemux::XDemux()
{
    static bool isInit = true;
    std::lock_guard<std::mutex> initLock(mtx_);
    if(isInit){
        // 初始化网络库
        avformat_network_init();
        isInit = false;
    }
}

XDemux::~XDemux()
{
    
}
bool XDemux::Open(const char *url)
{

    Close(); // 关闭可能留存的url

    AVDictionary *opts = NULL; // 字典，用于存储参数设置
    av_dict_set(&opts, "rtsp_transport", "tcp", 0); // 设置rtsp流以tcp协议打开
    av_dict_set(&opts, "max_delay", "500", 0); // 设置网络最大延迟为500毫秒
    
    std::unique_lock<std::mutex> lk(mtx_); 
    // 解封装初始化
    int re = avformat_open_input(
        &ic_, 
        url, // 输入文件路径
        0, // 0 表示自动选择解封器
        &opts // 参数设置，比如rtsp的延时时间
    );
    if (!ic_) {
        lk.unlock();
        char buf[1024] = {0};
        av_strerror(re, buf, sizeof(buf)-1);
        std::cout << "open input failed: " << buf << std::endl;
        return false;
    }
    std::cout << "open input success" << std::endl;

    // 获取流信息
    re = avformat_find_stream_info(ic_, 0);
    // 总时长 毫秒
    totalMs_ = ic_->duration / (AV_TIME_BASE / 1000);
    std::cout << "totalMs:" << totalMs_ << std::endl;
    // 打印流详细信息
    av_dump_format(ic_, 0, url, 0);

    // 获取视频流
    videoStream_ = av_find_best_stream(ic_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL,0);
    if (videoStream_ >= 0) {
        AVStream *as = ic_->streams[videoStream_];
        width_ = as->codecpar->width;
        height_ = as->codecpar->height;
        qDebug() << "视频流信息";
        std::cout << "width: " << as->codecpar->width << std::endl;
        std::cout << "height: " << as->codecpar->height << std::endl;
        std::cout << "format :" << as->codecpar->format << std::endl;
        // 在 FFmpeg 的较新版本中，AVCodecParameters 结构体的成员名称发生了变化。channels 成员已经被替换为 ch_layout 结构体，这是为了支持更全面的声道布局信息。
        std::cout << "codec_id: " << avcodec_get_name(as->codecpar->codec_id) << std::endl; 
        // 帧率 fps, 分数转换
        std::cout << "av_frame_rate: " << av_q2d(as->avg_frame_rate) << std::endl;
  
    
        switch (as->codecpar->format) {
            case AV_PIX_FMT_YUV420P:
                std::cout << "pixel format name: AV_PIX_FMT_YUV420P" << std::endl;
                break;
            case AV_PIX_FMT_YUV422P:
                std::cout << "pixel format name: AV_PIX_FMT_YUV422P" << std::endl;
                break;
            case AV_PIX_FMT_YUV444P:
                std::cout << "pixel format name: AV_PIX_FMT_YUV444P" << std::endl;
                break;
            case AV_PIX_FMT_NV12:
                std::cout << "pixel format name: AV_PIX_FMT_NV12" << std::endl;
                break;
            case AV_PIX_FMT_NV21:
                std::cout << "pixel format name: AV_PIX_FMT_NV21" << std::endl;
                break;
            case AV_PIX_FMT_RGB24:
                std::cout << "pixel format name: AV_PIX_FMT_RGB24" << std::endl;
                break;
            case AV_PIX_FMT_BGR24:
                std::cout << "pixel format name: AV_PIX_FMT_BGR24" << std::endl;
                break;
            default:
                std::cout << "pixel format name: Unknown or unsupported format" << std::endl;
                break;
        }
    } else {
        qDebug() << "未找到视频流";
    }

    // 获取音频流
    audioStream_ = av_find_best_stream(ic_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL,0);
    if (audioStream_ >= 0) {
        AVStream *as = ic_->streams[audioStream_];
        qDebug() << "音频流信息";
        std::cout << "sample_rate: " << as->codecpar->sample_rate << std::endl;
        std::cout << "format: " << as->codecpar->format << std::endl; // 像素格式
        // 在 FFmpeg 的较新版本中，AVCodecParameters 结构体的成员名称发生了变化。channels 成员已经被替换为 ch_layout 结构体，这是为了支持更全面的声道布局信息。
        std::cout << "channel layout: " << as->codecpar->ch_layout.u.mask << std::endl;
        std::cout << "number of channels: " << as->codecpar->ch_layout.nb_channels << std::endl;
        // 获取解码器名称
        std::cout << "codec_id: " << avcodec_get_name(as->codecpar->codec_id) << std::endl; 
        // 一帧数据：单通道的样本数 fps = sample_rate / frame_size   样本率：每秒采集的样本数 帧大小：每帧的样本数
        std::cout << "frame_size: " << as->codecpar->frame_size << std::endl;
    } else {
        qDebug() << "未找到音频流";
    }

    return true;
}

// 空间需要调用者释放，释放AVPacket对象空间，和数据空间 av_packet_free
AVPacket* XDemux::Read()
{
    std::lock_guard<std::mutex> lk(mtx_);
    if(!ic_) return nullptr;
    AVPacket *pkt = av_packet_alloc(); // 分配一个AVPacket对象空间
    int re = av_read_frame(ic_, pkt); // 申请缓冲区并读取一个包
    if(re != 0) { 
        av_packet_free(&pkt);
        return nullptr;
    }
    // pts转为毫秒
    pkt->pts = pkt->pts * av_q2d(ic_->streams[pkt->stream_index]->time_base) * 1000;
    // dts转为毫秒
    pkt->dts = pkt->dts * av_q2d(ic_->streams[pkt->stream_index]->time_base) * 1000;

    qDebug() << "XDemux pkt->pts:" << pkt->pts << "pkt->dts:" << pkt->dts;
    
    return pkt;
}

// 获取视频参数,返回的空间需要清理 avcodec_parameters_free
AVCodecParameters* XDemux::CopyVPara()
{
    std::lock_guard<std::mutex> lk(mtx_);
    if(!ic_){
        return nullptr;
    }
    AVCodecParameters *pa = avcodec_parameters_alloc();
    avcodec_parameters_copy(pa, ic_->streams[videoStream_]->codecpar);
    return pa;
}

AVCodecParameters* XDemux::CopyAPara()
{
    std::lock_guard<std::mutex> lk(mtx_);
    if(!ic_){
        return nullptr;
    }
    AVCodecParameters *pa = avcodec_parameters_alloc();
    avcodec_parameters_copy(pa, ic_->streams[audioStream_]->codecpar);
    return pa;
}

// seek 位置 pos 0.0 ~ 1.0
bool XDemux::Seek(double pos) // 不保证跳转到关键帧
{
    std::lock_guard<std::mutex> lk(mtx_);
    if(!ic_)
        return false;


    int seekPos = ic_->streams[videoStream_]->duration * pos;
    
    int re = av_seek_frame(ic_, videoStream_, seekPos, AVSEEK_FLAG_BACKWARD|AVSEEK_FLAG_FRAME);
    if(re<0){
        qDebug() << "seek error";
        return false;
    }    

    // 是的，​**avformat_flush(ic_) 会清空 AVFormatContext 在 seek 之前的所有缓冲数据**，确保后续读取的数据来自 seek 后的新位置。
    avformat_flush(ic_);

    return true;
}

// 清理缓冲
void XDemux::Clear()
{
    std::lock_guard<std::mutex> lk(mtx_);
    if(!ic_)
        return;
    // 清理读取缓冲
    avformat_flush(ic_);
}

// 关闭ic
void XDemux::Close()
{
    std::lock_guard<std::mutex> lk(mtx_);
    if(!ic_) return;
    avformat_close_input(&ic_);
}
bool XDemux::isAudio(AVPacket *pkt)
{
    if(!pkt || audioStream_ < 0) return false;
    return pkt->stream_index == audioStream_;
}

bool XDemux::isVideo(AVPacket *pkt)
{
    if(!pkt || videoStream_ < 0) return false;
    return pkt->stream_index == videoStream_;
}