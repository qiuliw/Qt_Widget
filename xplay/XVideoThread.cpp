#include "XVideoThread.h"
#include "XDecode.h"
#include <cstdint>
#include <iostream>
#include <mutex>


extern "C"{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libavutil/imgutils.h"
    #include "libavutil/frame.h"
    #include "libavutil/time.h"
    #include "libavutil/pixfmt.h"
    #include "libavutil/pixdesc.h"
    #include <libavcodec/packet.h>
}

XVideoThread::XVideoThread()
{
    
}

XVideoThread::~XVideoThread()
{
}

void XVideoThread::run()
{
    auto pcm = std::make_unique<unsigned char[]>(1024 * 1024 * 10);
    while (!isExit_) {

        AVPacket *pkt = Pop();
        if(!pkt){
            msleep(1);
            continue;
        }

        if(!decode_) {
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
            int64_t pts = 0;
            if(!frame) break;

            // 音视频同步 - 在帧级别进行同步控制
            if ( decode_->pts_ > synpts_) {
                // 计算需要等待的时间，但设置上限避免无限等待
                long long sleep_ms = (decode_->pts_ - synpts_) / 1000; // 转换为毫秒
                if (sleep_ms > 100) sleep_ms = 100; // 最多等待100ms
                if (sleep_ms > 0) {
                    msleep(sleep_ms);
                }
            }

            // 播放视频
            call_->Repaint(frame);
            av_frame_free(&frame);
        }
        cv_.notify_one(); // 通知生产者可以继续推数据
    }
}

bool XVideoThread::Open(AVCodecParameters *para,IVideoCall *call)
{
    if(!para) return false;
    std::lock_guard<std::mutex> lk(mtx_);
    // 初始化显示窗口
    this->call_ = call;
    if(call){
        call->Init(para->width,para->height);
    }
    // 打开解码器
    if(!decode_) decode_ = new XDecode();
    if(!decode_->Open(para)){
        std::cout << "XVideoThread::Open() failed" << std::endl;
        return false;
    }
    std::cout << "XVideoThread::Open() ok" << std::endl;
    return true;
}

void XVideoThread::Close()
{
    std::lock_guard<std::mutex> lk(mtx_);
    
    // 清理队列中的数据包
    while(!packets_.empty()) {
        AVPacket* pkt = packets_.front();
        packets_.pop_front();
        av_packet_free(&pkt);
    }
    
    // 关闭解码器
    if(decode_) {
        delete decode_;
        decode_ = nullptr;
    }
    
    call_ = nullptr;
}