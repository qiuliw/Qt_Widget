#include "XDecode.h"
#include "XDemux.h"
#include "widget.h"

#include <QApplication>
#include <iostream>

#include <ostream>
#include <qdebug.h>
#include <qlogging.h>

extern "C"{
    #include <libavutil/frame.h>
}

int main(int argc, char *argv[])
{

    // 设置FFmpeg日志级别（只显示错误和致命错误）
    av_log_set_level(AV_LOG_ERROR);

    char *url;
    url = "hm.mp4";
    // 解封装器
    XDemux demux;
    if (!demux.Open(url)){
        return -1;
    }
    qDebug() << "打开文件成功:" << url;

    demux.Seek(0.95);

    //////////////////////
    /// 解码测试
    // 解码器
    XDecode adec, vdec;
    adec.Open(demux.CopyAPara());
    vdec.Open(demux.CopyVPara());
    for(;;){
        AVPacket *pkt = demux.Read();
        if(!pkt) break;
        // 发送到对应解码器
        AVFrame *frame;
        if(demux.isAudio(pkt)){
            adec.Send(pkt);
            frame = adec.Recv();
            if(!frame){
                continue;
            }
            // std::cout << "Audio:" << frame->linesize[0] << std::endl;
            
        }
        else{
            vdec.Send(pkt);
            frame = vdec.Recv();
            if(!frame){
                continue;
            }
            // std::cout << "Video:" << frame->linesize[0] << std::endl;
        }
        av_frame_free(&frame);

    }


    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
