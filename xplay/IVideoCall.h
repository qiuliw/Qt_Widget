#pragma once

class AVFrame;

// 图像显示接口
class IVideoCall
{
public:
    virtual void Init(int width,int height) = 0;
    virtual void Repaint(AVFrame *frame) = 0;
};