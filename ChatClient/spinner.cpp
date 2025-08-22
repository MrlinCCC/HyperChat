#include "spinner.h"

Spinner::Spinner(QWidget* parent)
    : QWidget(parent), angle(0)
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        angle = (angle + 30) % 360;
        update();
    });
    timer->start(100);
    setFixedSize(50, 50);
}

void Spinner::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(width() / 2, height() / 2);
    p.rotate(angle);
    int r = width() / 2 - 5;
    for (int i = 0; i < 12; ++i)
    {
        QColor c(0, 0, 0, i * 20 + 50);
        p.setBrush(c);
        p.setPen(Qt::NoPen);
        p.save();
        p.rotate(i * 30);
        p.drawRoundedRect(r - 5, -2, 10, 4, 2, 2);
        p.restore();
    }
}
