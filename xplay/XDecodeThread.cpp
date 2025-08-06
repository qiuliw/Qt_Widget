#include "XDecodeThread.h"
#include "XDecode.h"

extern "C"{
    #include <libavcodec/packet.h>
}

XDecodeThread::XDecodeThread()
{
    
}

XDecodeThread::~XDecodeThread()
{
    // 等待线程退出
    isExit_ = true;
    /*
        线程退出时，需要将条件变量唤醒。
        生产消费模型一端停止另一端休眠等待时，并不能知道另一端退出而去醒来检查退出条件，需要主动唤醒去检查退出条件。
    */
    cv_.notify_all(); // 唤醒所有等待的线程去退出
    Clear();
    // 删除解码器对象
    if (decode_) {
        delete decode_;
        decode_ = nullptr;
    }
    wait(); // 等待run线程任务循环主动退出然后回收一个线程资源
}

void XDecodeThread::Push(AVPacket *pkt)
{
    if (!pkt) return;
    std::unique_lock<std::mutex> lk(mtx_);
    // 队列满时等待，直到消费者取出数据
    cv_.wait(lk, [this] { return isExit_ || packets_.size() < maxList_; });
    if (isExit_){
        av_packet_free(&pkt);
        return;
    }

    packets_.push_back(pkt);
    cv_.notify_one(); // 通知消费者有新数据
}

AVPacket* XDecodeThread::Pop()
{
    std::unique_lock<std::mutex> lk(mtx_);
    cv_.wait(lk, [this] { return isExit_ || !packets_.empty(); });

    if (isExit_) return nullptr; // 线程退出，避免阻塞

    AVPacket* pkt = packets_.front();
    packets_.pop_front();
    lk.unlock();
    cv_.notify_one();
    return pkt;
}

// 清理队列
void XDecodeThread::Clear()
{

    if(decode_)
        decode_->Clear();

    std::lock_guard<std::mutex> lk(mtx_);
    
    // 清理队列中的数据包
    while(!packets_.empty()) {
        AVPacket* pkt = packets_.front();
        packets_.pop_front();
        av_packet_free(&pkt);
    }


}

void XDecodeThread::SetPause(bool isPause)
{
    isPause_ = isPause;
}

