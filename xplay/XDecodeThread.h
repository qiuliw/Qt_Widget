#pragma once

#include "IVideoCall.h"
#include "XVideoWidget.h"
#include <QThread>
#include <mutex>
#include <QAudioSink>
#include <queue>
#include "XDecodeThread.h"

class AVCodecParameters;
class XDecode;
class XResample;
class XAudioPlay;
class AVPacket;

// 视频解码播放线程
// 负责视频解码，播放
class XDecodeThread : public QThread 
{ 
public:
    XDecodeThread();
    virtual ~XDecodeThread();

    virtual void Push(AVPacket *pkt); // 阻塞
    virtual AVPacket *Pop();

    std::atomic<bool> isExit_ = false;
protected:
    std::list<AVPacket*> packets_;
    std::mutex mtx_;
    int maxList_ = 100; // 最大缓存个数限制
    std::condition_variable cv_;
    XDecode *decode_;
};

