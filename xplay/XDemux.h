#pragma once

#include <mutex>

class AVFormatContext;
class AVPacket;
class AVCodecParameters;

/*
    解封装类 XDemux
        --> 解码 XDecode
*/ 

class XDemux
{
public:
    XDemux();
    virtual ~XDemux();

    // 打开媒体文件，或者流媒体 rtmp http rstp
    virtual bool Open(const char *url);
    // 空间需要调用者释放，释放AVPacket对象空间，和数据空间 av_packet_free
    virtual AVPacket *Read();

    // 获取视频参数,返回的空间需要清理 avcodec_parameters_free
    AVCodecParameters *CopyVPara();
    // 获取音频参数
    AVCodecParameters *CopyAPara();

    // seek 位置 pos 0.0 - 1.0
    virtual bool Seek(double pos);

    // 清空读取缓存
    virtual void Clear();
    virtual void Close();

    int totalMs_ = 0;
    
private:
    // 解封装上下文
    AVFormatContext *ic_ = NULL; // 解封装上下文指针
    
    int videoStream_ = 0;
    int audioStream_ = 1;
    std::mutex mtx_; // 对ic的操作需要互斥
};