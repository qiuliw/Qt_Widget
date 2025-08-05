#pragma once

#include <QThread>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <QAudioSink>
#include <queue>
#include "XDecodeThread.h"


class AVCodecParameters;
class XDecode;
class XResample;
class XAudioPlay;
class AVPacket;

// 音频解码播放线程
// 负责音频解码，重采样，播放
class XAudioThread : public XDecodeThread 
{ 
public:
    XAudioThread();
    virtual ~XAudioThread();

    void run() override;
    // 打开，不管是否成功都清理para。设置输出参数
    bool Open(AVCodecParameters *para,int sampleRate = 44100, int channels = 2, QAudioFormat::SampleFormat sampleFormat = QAudioFormat::Int16,bool destroyPara = true);
    int64_t pts_ = 0; // 当前音频播放的pts
protected:
    XAudioPlay *ap_ = nullptr;
    XResample *resample_ = nullptr;
};
