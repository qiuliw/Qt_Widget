#include "XDecode.h"
#include "XDemux.h"
#include "XVideoWidget.h"
#include "widget.h"

#include <QApplication>
#include <iostream>

#include <ostream>
#include <qdebug.h>
#include <Qthread.h>
#include <qlogging.h>
#include "XResample.h"

extern "C"{
    #include <libavutil/frame.h>
    #include <libavcodec/packet.h>
}

class TestThread : public QThread{
public:
    TestThread(QObject *parent = nullptr) : QThread(parent) {}

    void Init(){
        char *url;
        url = "hm.mp4";
        // 解封装器
        if (!demux.Open(url)){
            qDebug() << "打开文件失败:" << url;
            return;
        }
        qDebug() << "打开文件成功:" << url;
        // demux.Seek(0.9);

        // 解码器
        adec.Open(demux.CopyAPara());
        vdec.Open(demux.CopyVPara());
        // 音频重采样
        resample.Open(demux.CopyAPara());

    }

    void run() override
    {

        /*
        边 send 边 recv（同步流水线，不一次性全部 send）。
        Read 返回 nullptr 以后，向解码器 send(nullptr) 以进入 flush 模式。
        一次 send 之后，循环 recv 直到返回 nullptr（“一次 send，多次 recv”）。
        */

        unsigned char *pcm = new unsigned char[1024 * 1024];

        for (;;)
        {
            // 1. 读包（读到 EOF 后 pkt == nullptr）
            AVPacket *pkt = demux.Read();

            // 2. 根据流类型 send 到对应解码器
            XDecode *dec = nullptr;
            if (pkt && demux.isVideo(pkt))
                dec = &vdec;
            else if (pkt)
                dec = &adec;

            if (dec)
                dec->Send(pkt);   // 正常包

            // 3. 读完文件后，向两个解码器各发一次 nullptr 进入 flush
            if (!pkt)
            {
                vdec.Send(nullptr);
                adec.Send(nullptr);
            }

            // 4. 对应解码器“只 recv，不再次 send”，直到没帧
            if (dec == &vdec || !pkt)   // 视频解码器或 flush 阶段
            {
                for (;;)
                {
                    AVFrame *frame = vdec.Recv();
                    if (!frame) break;
                    video->Repaint(frame);
                    av_frame_free(&frame);
                }
            }

            if (dec == &adec || !pkt)   // 音频解码器或 flush 阶段
            {
                for (;;)
                {
                    AVFrame *frame = adec.Recv();
                    if (!frame) break;
                    int re =resample.Resample(frame, pcm);

                    std::cout << "resample:" << re;
                    // audio->Play(frame);
                    av_frame_free(&frame);
                }
            }

            

            if (!pkt)
                break;

            // nullptr error?
            // std::cout << "pkt_ptr:" << pkt;
            // av_packet_free(&pkt);

            msleep(30);
        }
    }
    // 解封装
    XDemux demux;
    // 解码
    XDecode adec;
    XDecode vdec;
    // 音频重采样
    XResample resample;

    XVideoWidget *video; // 必须在QApplication之后创建
};


int main(int argc, char *argv[])
{

    // 设置FFmpeg日志级别（只显示错误和致命错误）
    av_log_set_level(AV_LOG_ERROR);

    TestThread tt; // 解码测试
    tt.Init();

    QApplication a(argc, argv);
    Widget w;
    w.show();

    // 初始化gl宽高
    w.videoWidget->Init(tt.demux.width_, tt.demux.height_);
    tt.video = w.videoWidget;
    tt.start();

    return a.exec();
}
