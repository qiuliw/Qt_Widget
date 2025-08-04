#include "XAudioPlay.h"
#include <QDebug>
#include <mutex>
#include <qlogging.h>
#include <QAudioSink>

XAudioPlay::XAudioPlay(){

}

XAudioPlay::~XAudioPlay() {
    Stop();
}

XAudioPlay* XAudioPlay::Get()
{
    static XAudioPlay ap;
    return &ap;
}

// 初始化设备和格式
bool XAudioPlay::Open(int sampleRate, int channels, QAudioFormat::SampleFormat sampleFormat) 
{
    if(is_initialized_){
        Stop();
    }
    
    // 1. 设置音频格式
    format_.setSampleRate(sampleRate);
    format_.setChannelCount(channels);
    format_.setSampleFormat(sampleFormat);

    // 2. 检查设备支持
    device_ = QMediaDevices::defaultAudioOutput();
    if (!device_.isFormatSupported(format_)) {
        // format_ = device_.preferredFormat();
        // qWarning() << "格式不支持，已切换为设备推荐格式";
        qWarning() << "格式不支持";
        return false;
    }

    std::lock_guard<std::mutex> lk(mtx_);

    is_initialized_ = true;

    return true;
}

void XAudioPlay::InitSink()
{
     // 3. 提前创建 QAudioSink 和 QIODevice
    if (!audio_sink_) { // audio_sink_ 必须在播放线程创建
        audio_sink_ = new QAudioSink(device_, format_);
        audio_io_ = audio_sink_->start();  // 初始化音频设备
        if (!audio_io_) {
            qCritical() << "无法初始化音频设备";
            Stop();
        }
    }
}

// 在外部判断足够写入才写入
bool XAudioPlay::Play(unsigned char *data, int len) {

    if (!is_initialized_) {
        qWarning() << "未初始化或初始化失败，请先调用 Init()";
        return false;
    }

    if(!audio_sink_){
        InitSink();
    }

    if(!data || len <= 0) return false;

    std::lock_guard<std::mutex> lk(mtx_);

    // 通过QIODevice写入数据
    if (audio_io_ && audio_sink_->state() != QAudio::StoppedState) {
        qint64 written = audio_io_->write(reinterpret_cast<const char*>(data), len);
        if (written != len) {
            qWarning() << "Failed to write all data (expected:" << len << "actual:" << written << ")";
            return false;
        }
    }

    return true;
}

void XAudioPlay::Stop() {

    std::lock_guard<std::mutex> lk(mtx_);

    if (audio_sink_) {
        audio_sink_->stop();
        delete audio_sink_;
        audio_sink_ = nullptr;
        audio_io_ = nullptr;
    }
    is_initialized_ = false;  // 重置初始化状态
}

void XAudioPlay::SetVolume(float volume) {

    std::lock_guard<std::mutex> lk(mtx_);

    if (audio_sink_) {
        audio_sink_->setVolume(qBound(0.0f, volume, 1.0f));
    }
}

int XAudioPlay::GetState(){
    return audio_sink_ ? static_cast<int>(audio_sink_->state()) : 0;
}

int XAudioPlay::GetFree()
{
    if(!audio_io_ || !audio_sink_){
        InitSink();
    }
    return audio_sink_->bytesFree();
}