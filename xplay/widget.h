#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "XDemux.h"
#include "XVideoWidget.h"
#include <QPushButton>
#include <qpushbutton.h>
#include <qtmetamacros.h>
#include <QSlider>

class XDemuxThread;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    XVideoWidget *videoWidget;

protected:
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void OpenFile();
    void SliderPressed();
    void SliderReleased();
    void SliderMoved(int value);
    void timerEvent(QTimerEvent *e);
    void mouseDoubleClicked(QMouseEvent *event);
private:

    void updateVideoGeometry();
    int videoOrgWidth = 0;
    int videoOrgHeight = 0;

    QPushButton *openFileBtn;
    QSlider *slider;
    bool isSliderPressed = false;
};
#endif // WIDGET_H