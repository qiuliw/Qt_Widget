#include "XDemuxThread.h"
#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"
#include <iostream>
#include <mutex>

extern "C"{
    #include <libavcodec/packet.h>
}

XDemuxThread::XDemuxThread()
{
    
}

XDemuxThread::~XDemuxThread()
{
    isExit_ = true;
    Close();
    wait(); // 对象销毁在主线程，主线程等待子线程结束
}

void XDemuxThread::run()
{
    while(!isExit_){
        if(!demux_){
            msleep(5);
            continue;
        }

        if(vt_&&at_){
           vt_->synpts_ = at_->pts_;
        }
        std::unique_lock<std::mutex> lk(mtx_);
        AVPacket *pkt = demux_->Read();
        if(!pkt){
            lk.unlock();
            msleep(5);
            continue;
        }
        if(demux_->isVideo(pkt)){
            // 视频
            vt_->Push(pkt); 
        }else if(demux_->isAudio(pkt)){
            // 音频
            at_->Push(pkt);// 可能被阻塞
        }else {
            av_packet_free(&pkt);  // 非音视频包直接释放
        }
    }
}
bool XDemuxThread::Open(const char *url,IVideoCall *call)
{
    if(url==0 || url[0]==0){
        return false;
    }    
    
    // 先关闭之前的资源
    Close();
    

    std::lock_guard<std::mutex> lk(mtx_);
    if(!demux_){
        demux_ = new XDemux();
    }
    if(!vt_) vt_ = new XVideoThread();
    if(!at_) at_ = new XAudioThread();
    
    // 打开解封装（这部分不需要锁保护）
    if(!demux_->Open(url)){
        std::cout << "XDemuxThread::Open() failed" << std::endl;
        return false;
    }
    
    // 打开解码器和处理线程（这些操作也不应该在锁中进行）
    if(!vt_->Open(demux_->CopyVPara(),call)){
        std::cout << "XVideoThread::Open() failed" << std::endl;
        demux_->Close(); // 出错时清理
        return false;
    }
    if(!at_->Open(demux_->CopyAPara())){
        std::cout << "XAudioThread::Open() failed" << std::endl;
        demux_->Close(); // 出错时清理
        vt_->Close();    // 出错时清理
        return false;
    }

    std::cout << "XDemuxThread::Open() success" << std::endl;
    return true;
}

// 只清理资源
void XDemuxThread::Close()
{
    std::lock_guard<std::mutex> lk(mtx_);
    
    if(vt_) {
        vt_->Close();
    }
    if(at_) {
        at_->Close();
    }
    if(demux_) {
        demux_->Close();
    }
}
void XDemuxThread::Start()
{
    {
        std::lock_guard<std::mutex> lk(mtx_);
        if(!demux_){
            demux_ = new XDemux();
        }
        if(!vt_) vt_ = new XVideoThread();
        if(!at_) at_ = new XAudioThread();
    }
    
    vt_->start();
    at_->start();

    // 启动当前线程
    QThread::start();
    
}

