#ifndef SPINNER_H
#define SPINNER_H

#include <QObject>
#include <QWidget>
#include <QTimer>
#include <QPainter>

class Spinner : public QWidget
{
    Q_OBJECT
public:
    Spinner(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QTimer* timer;
    int angle;
};

#endif	  // SPINNER_H
