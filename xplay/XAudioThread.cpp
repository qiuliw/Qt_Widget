#include "XAudioThread.h"
#include "XAudioPlay.h"
#include "XDecode.h"
#include "XResample.h"
#include <iostream>
#include <memory>
#include <mutex>

extern "C"{
    #include <libavcodec/codec_par.h>
}

XAudioThread::XAudioThread()
{
    
}

XAudioThread::~XAudioThread()
{
    // 等待线程退出
    isExit_ = true;
    wait(); // 等待run线程任务循环主动退出
}


void XAudioThread::run() {
    
    auto pcm = std::make_unique<unsigned char[]>(1024 * 1024 * 10);
    while (!isExit_) {
        std::unique_lock<std::mutex> lk(mtx_);

        // 确保有数据
        cv_.wait(lk, [this] { return isExit_ || !packets_.empty(); });
        if (isExit_) break;
        AVPacket* pkt = packets_.front();
        packets_.pop_front();
        lk.unlock(); // 提前释放锁，减少临界区
        cv_.notify_one(); // 通知生产者可以继续推数据
        
        if(!decode_||!resample_||!ap_) {
            msleep(1);
            continue;
        }

        bool re = decode_->Send(pkt);
        if(!re){
            msleep(1);
            continue;
        }
        // 一次send 多次recv
        while(!isExit_){
            AVFrame *frame = decode_->Recv();
            if(!frame) break;
            // 音频重采样
            int size = resample_->Resample(frame,pcm.get());
            if(size<=0) break;
            // 播放音频
            while(!isExit_){
                // 缓冲未播完，空间不够
                if(ap_->GetFree() < size){
                    msleep(1);
                    continue;
                }
                ap_->Play(pcm.get(),size);
                break;
            }
        }
    }
}

bool XAudioThread::Open(AVCodecParameters *para,int sampleRate, int channels, QAudioFormat::SampleFormat sampleFormat,bool destroyPara)
{
    if(!para) return false;
    
    std::lock_guard<std::mutex> lk(mtx_);
    
    if(!decode_)
        decode_ = new XDecode();
    if(!resample_)
        resample_ = new XResample();
    if(!ap_)
        ap_ = XAudioPlay::Get();
    
    if(!decode_->Open(para,false)){
        std::cout << "XDecode open failed" << std::endl;
        return false;
    }
    if(!resample_->Open(para,false)){
        std::cout << "XResample open failed" << std::endl;
        return false;
    }
    if(!ap_->Open(sampleRate,channels,sampleFormat)){
        std::cout << "XAudioPlay open failed" << std::endl;
        return false;
    }

    if(destroyPara)
        avcodec_parameters_free(&para);

    std::cout << "XAudioThread open success" << std::endl;

    return true;
}


void XAudioThread::Push(AVPacket* pkt) {
    if (!pkt) return;
    std::unique_lock<std::mutex> lk(mtx_);
    // 队列满时等待，直到消费者取出数据
    cv_.wait(lk, [this] { return isExit_ || packets_.size() < maxList_; });
    if (isExit_) return;

    packets_.push_back(pkt);
    cv_.notify_one(); // 通知消费者有新数据
}