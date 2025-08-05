#pragma once
#include <QAudioFormat>
#include <QAudioSink>
#include <QMediaDevices>
#include <mutex>

class AVCodecParameters;

class XAudioPlay  {
public:
    virtual ~XAudioPlay() ;

    static XAudioPlay *Get();
    // 音频格式 音频输出格式
    virtual bool Open(int sampleRate = 44100, int channels = 2, QAudioFormat::SampleFormat sampleFormat = QAudioFormat::Int16);
    void InitSink(); // sink必须在播放线程中初始化
    virtual bool Play(unsigned char *data, int len) ;
    virtual void Stop();
    virtual void Pause(bool isPause = true);
    virtual void SetVolume(float volume) ;
    virtual int GetState();
    virtual int GetFree();
    // 获取未播放时长
    virtual long long GetNoPlayMs();
private:
    explicit XAudioPlay();
    QAudioFormat format_;
    QAudioDevice device_;
    QAudioSink *audio_sink_ = nullptr; // 必须在播放线程创建
    QIODevice *audio_io_ = nullptr;  // 新增：用于写入数据的QIODevice
    bool is_initialized_ = false;
    std::mutex mtx_; // 对流和设备的操作是线程安全的，因此使用互斥锁
};