#ifndef MEASUREMENT_H
#define MEASUREMENT_H
#include <QString>
#include <QVector>

class Measurement
{
friend class Test;
public:
    Measurement();
    ~Measurement();

private:
    QString m_title;
    QVector<double> data;
};

#endif // MEASUREMENT_H
