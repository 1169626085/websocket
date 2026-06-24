#include "timerbtn.h"

TimerBtn::TimerBtn(QWidget *parent)
    : QPushButton(parent)
    , counter_(10)
{
    timer_.setInterval(1000);
    connect(&timer_, &QTimer::timeout, this, &TimerBtn::onTimeout);
    connect(this, &QPushButton::clicked, this, &TimerBtn::startCountdown);
}

TimerBtn::~TimerBtn() = default;

void TimerBtn::startCountdown()
{
    normal_text_ = text();
    counter_ = 10;
    setEnabled(false);
    setText(QString::number(counter_) + "s");
    timer_.start();
}

void TimerBtn::onTimeout()
{
    --counter_;
    if (counter_ <= 0) {
        timer_.stop();
        setText(normal_text_);
        setEnabled(true);
        return;
    }

    setText(QString::number(counter_) + "s");
}
