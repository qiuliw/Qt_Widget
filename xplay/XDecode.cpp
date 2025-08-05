#include "XDecode.h"
#include <mutex>
#include <ostream>
#include <qdebug.h>
#include <qlogging.h>

extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include <libavcodec/packet.h>
    #include <libavutil/avutil.h>
    #include <libavformat/avio.h>
    #include <libavcodec/codec_par.h>
    #include <libavutil/frame.h>
    #include <libswscale/swscale.h>
    #include <libavutil/pixfmt.h>
    #include <libswresample/swresample.h>
    #include <libavutil/channel_layout.h> // 新增：声道布局相关
}




XDecode::XDecode()
{
    
}

XDecode::~XDecode()
{
    
}
// 打开解码器
bool XDecode::Open(AVCodecParameters *para,bool destroyPara)
{
    if(!para) return false;
    
    Close();

    // 初始化解码器
    codec_ = avcodec_find_decoder(para->codec_id);
    if (!codec_){
        avcodec_parameters_free(&para);
        qDebug() << "avcodec_find_decoder failed";
        return false;
    }

    std::lock_guard<std::mutex> lk(mtx_);

    // 创建解码器上下文
    codecCtx_ = avcodec_alloc_context3(codec_);
    avcodec_parameters_to_context(codecCtx_,para); // 配置上下文参数
    codecCtx_->thread_count = 8; // 8线程解码
    int re = avcodec_open2(codecCtx_, 0, 0);
    if (re < 0) {
        char buf[1024] = {0};
        av_strerror(re, buf, sizeof(buf)-1);
        qDebug() << "avcodec_open2 failed: " << buf;

        if(destroyPara){
            avcodec_parameters_free(&para);
        }
        avcodec_free_context(&codecCtx_);

        return false;
    }
    qDebug() << "avcodec_open2 success";

    return true;
}
// 发送数据包给解码器线程，不论成功与否都释放pkt空间(对象和媒体内容)
bool XDecode::Send(AVPacket *pkt)
{
    if(!pkt || pkt->size <= 0 ||  !pkt->data){
        return false;
    }
    // 对解码器的操作需要加锁
    std::lock_guard<std::mutex> lk(mtx_);
    if(!codecCtx_){
        return false;
    }
    int re = avcodec_send_packet(codecCtx_, pkt);
    av_packet_free(&pkt);
    if(re != 0) return false;
    return true;
}
// 从解码器获取解码后的数据，一次send可能需要多次Recv。如果要读取缓冲中的数据， Send NULL再Recv多次
// 每次复制一份，由调用者释放 av_frame_free(&frame);
AVFrame* XDecode::Recv()
{
    std::lock_guard<std::mutex> lk(mtx_);
    if(!codecCtx_){
        return nullptr;
    }
    // 解码器内部需要循环使用缓冲区池。所以解码后的数据需要传出腾空缓冲区。
    AVFrame *frame = av_frame_alloc();
    int re = avcodec_receive_frame(codecCtx_, frame);
    if(re != 0){
        av_frame_free(&frame);
        return nullptr;
    }
    pts_ = frame->pts;
    return frame;
}

void XDecode::Close()
{
    std::lock_guard<std::mutex> lk(mtx_);

    if(codecCtx_){
        avcodec_close(codecCtx_);
        avcodec_free_context(&codecCtx_);
    }
    pts_ = 0;
}

void XDecode::Clear()
{
    std::lock_guard<std::mutex> lk(mtx_);

    // 清理解码缓冲
    if(codecCtx_){
        avcodec_flush_buffers(codecCtx_);
    }
}

