#include "XDemux.h"
#include "widget.h"

#include <QApplication>
#include <iostream>
#include <libavcodec/codec_par.h>
#include <qdebug.h>
#include <qlogging.h>

int main(int argc, char *argv[])
{
    XDemux demux;

    // 香港卫视
    char *url;
    url = "hm.mp4";
    qDebug() << "demux Open = " << demux.Open(url);
    AVCodecParameters *apa = demux.CopyAPara();
    AVCodecParameters *vpa = demux.CopyVPara();
    qDebug() << "apa->sample_rate = " << apa->sample_rate;
    qDebug() << "vpa->width = " << vpa->width;

    demux.Seek(0.95);
    for(;;){
        AVPacket *pkt = demux.Read();
        if(!pkt) break;
    }
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
