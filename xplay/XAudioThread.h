#pragma once

#include <QThread>
#include <mutex>
#include <QAudioSink>
#include <queue>

class AVCodecParameters;
class XDecode;
class XResample;
class XAudioPlay;
class AVPacket;

// 音频解码播放线程
// 负责音频解码，重采样，播放
class XAudioThread : public QThread 
{ 
public:
    XAudioThread();
    virtual ~XAudioThread();

    void run() override;
    // 打开，不管是否成功都清理para。设置输出参数
    bool Open(AVCodecParameters *para,int sampleRate = 44100, int channels = 2, QAudioFormat::SampleFormat sampleFormat = QAudioFormat::Int16,bool destroyPara = true);
    // 向队列添加数据包
    virtual void Push(AVPacket *pkt); // 阻塞

    std::atomic<bool> isExit_ = false;
    int64_t pts_ = 0; // 当前音频播放的pts
protected:
    XDecode *decode_ = nullptr;
    XAudioPlay *ap_ = nullptr;
    XResample *resample_ = nullptr;
    std::mutex mtx_;
    std::list<AVPacket *> packets_;
    int maxList_ = 100; // 最大缓存个数限制
    std::condition_variable cv_;
};
