#pragma once

#include <mutex>

struct AVCodecParameters;
class AVCodec;
class AVCodecContext;
class AVPacket;
class AVFrame;

// 解码器
class XDecode {
public:
    XDecode();
    virtual ~XDecode();

    // 打开解码器，不论成功与否都释放para空间
    virtual bool Open(AVCodecParameters *para, bool destroyPara = true);
    // 发送数据包给解码器线程，不论成功与否都释放pkt空间(对象和媒体内容)
    virtual bool Send(AVPacket *pkt);
    // 从解码器获取解码后的数据，一次send可能需要多次Recv。如果要读取缓冲中的数据， Send NULL再Recv多次
    virtual AVFrame *Recv(); // 每次复制一份，由调用者释放

    virtual void Close(); // 关闭解码器
    virtual void Clear(); // 清空数据

    const AVCodec* codec_ = nullptr;
    AVCodecContext *codecCtx_ = nullptr;

    // 需要锁，不能同时对解码器操作同时close
    std::mutex mtx_;

};
