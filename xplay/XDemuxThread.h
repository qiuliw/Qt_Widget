#pragma once

#include "IVideoCall.h"
#include <QThread>
#include <cstddef>
#include <mutex>


class XDemux;
class XVideoThread;
class XAudioThread;
class AVPacket;

class XDemuxThread : public QThread {
public:
    XDemuxThread();
    ~XDemuxThread();
    void run() override;
    // 创建对象并打开
    virtual bool Open(const char *url,IVideoCall *call);
    AVPacket *Read();
    void Clear();
    // 启动所有线程
    virtual void Start();


private:
    XDemux *demux_ = nullptr;
    XVideoThread *vt_ = nullptr;
    XAudioThread *at_ = nullptr;
    bool isExit_ = false;
    std::mutex mtx_;
};