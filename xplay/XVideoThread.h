#pragma once

#include "IVideoCall.h"
#include "XVideoWidget.h"
#include <QThread>
#include <atomic>
#include <cstdint>
#include <list>
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
class XVideoThread : public XDecodeThread 
{ 
public:
    XVideoThread();
    virtual ~XVideoThread();

    void run() override;
    // 打开，不管是否成功都清理para。设置输出参数
    bool Open(AVCodecParameters *para,IVideoCall *call);
    int64_t synpts_ = 0;
protected:
    IVideoCall *call_ = nullptr;
    std::condition_variable cv_;
    // 同步时间由外部传入
};
