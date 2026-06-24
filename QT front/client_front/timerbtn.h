#ifndef TIMERBTN_H
#define TIMERBTN_H

#include <QPushButton>
#include <QTimer>

class TimerBtn : public QPushButton
{
    Q_OBJECT

public:
    explicit TimerBtn(QWidget *parent = nullptr);
    ~TimerBtn();

private slots:
    void onTimeout();
    void startCountdown();

private:
    QTimer timer_;
    int counter_;
    QString normal_text_;
};

#endif // TIMERBTN_H
