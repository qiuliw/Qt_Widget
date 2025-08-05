#include "XAudioThread.h"
#include "XAudioPlay.h"
#include "XDecode.h"
#include "XResample.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <qdebug.h>

extern "C"{
    #include <libavcodec/codec_par.h>
    #include <libavcodec/avcodec.h>
    #include <libavcodec/packet.h>

}

XAudioThread::XAudioThread()
{
    
}

XAudioThread::~XAudioThread()
{
}


void XAudioThread::run() {
    
    auto pcm = std::make_unique<unsigned char[]>(1024 * 1024 * 10);
    while (!isExit_) {
        
        AVPacket *pkt = Pop();
        
        if(!pkt){
            msleep(1);
            continue;
        }
        
        if(!decode_||!resample_||!ap_) {
            msleep(1);
            continue;
        }

        std::unique_lock<std::mutex> lk(mtx_);
        bool re = decode_->Send(pkt);
        if(!re){
            lk.unlock();
            msleep(1);
            continue;
        }
        // 一次send 多次recv
        while(!isExit_){
            AVFrame *frame = decode_->Recv();
            if(!frame) break;
            // 计算当前正在播放的音频时间戳
            // 解码的时间戳减去缓冲未播放时间（微秒）
            pts_ = frame->pts - ap_->GetNoPlayMs();
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
        lk.unlock();
        cv_.notify_one(); // 通知生产者可以继续推数据
    }
}

bool XAudioThread::Open(AVCodecParameters *para,int sampleRate, int channels, QAudioFormat::SampleFormat sampleFormat,bool destroyPara)
{
    if(!para) return false;
    
    // 先清理之前的状态
    Clear();
    std::lock_guard<std::mutex> lk(mtx_);
    pts_ = 0;
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

