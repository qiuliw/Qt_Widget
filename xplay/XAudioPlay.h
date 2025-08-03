#pragma once
#include <QAudioFormat>
#include <QAudioSink>
#include <QMediaDevices>
#include <mutex>

class XAudioPlay  {
public:
    virtual ~XAudioPlay() ;

    static XAudioPlay *Get();

    // 音频格式
    virtual void Init(int sampleRate = 44100, int channels = 2, int sampleFormat = QAudioFormat::Int16) ;
    virtual bool Play(unsigned char *data, int len) ;
    virtual void Stop() ;
    virtual void SetVolume(float volume) ;
    virtual int GetState() const ;
    virtual int GetFree() const ;
private:
    explicit XAudioPlay();
    QAudioFormat format_;
    QAudioDevice device_;
    QAudioSink *audio_sink_ = nullptr;
    QIODevice *audio_io_ = nullptr;  // 新增：用于写入数据的QIODevice
    bool is_initialized_ = false;
    std::mutex mtx_; // 对流和设备的操作是线程安全的，因此使用互斥锁
};