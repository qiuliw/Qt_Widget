#pragma once

#include "IVideoCall.h"
#include "XVideoWidget.h"
#include <QThread>
#include <mutex>
#include <QAudioSink>
#include <queue>

class AVCodecParameters;
class XDecode;
class XResample;
class XAudioPlay;
class AVPacket;

// 视频解码播放线程
// 负责视频解码，播放
class XVideoThread : public QThread 
{ 
public:
    XVideoThread();
    virtual ~XVideoThread();

    void run() override;
    // 打开，不管是否成功都清理para。设置输出参数
    bool Open(AVCodecParameters *para,IVideoCall *call);
    // 向队列添加数据包
    virtual void Push(AVPacket *pkt); // 阻塞
    std::atomic<bool> isExit_ = false;
protected:
    XDecode *decode_ = nullptr;
    IVideoCall *call_ = nullptr;
    std::mutex mtx_;
    std::list<AVPacket *> packets_;
    int maxList_ = 100; // 最大缓存个数限制
    std::condition_variable cv_;
};
