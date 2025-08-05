#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "XDemux.h"
#include "XVideoWidget.h"
#include <QPushButton>
#include <qpushbutton.h>
#include <qtmetamacros.h>

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
private:

    void updateVideoGeometry();
    int videoOrgWidth = 0;
    int videoOrgHeight = 0;

    QPushButton *openFileBtn;
};
#endif // WIDGET_H