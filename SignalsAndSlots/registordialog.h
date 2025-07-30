#ifndef REGISTORDIALOG_H
#define REGISTORDIALOG_H

#include <QDialog>
#include <QWidget>

class RegistorDialog : public QDialog
{
    Q_OBJECT
public:
    RegistorDialog(QWidget *parent);

    QPushButton* backBtn;

    QPushButton* getBackBtn(){
        return this->backBtn;        
    }

signals:
    void registorSignal();

};

#endif // REGISTORDIALOG_H
