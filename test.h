#ifndef TEST_H
#define TEST_H
#include "measurement.h"
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QTextStream>
#include <QVector>
#include <QTextBrowser>

class Test : public QObject
{
    Q_OBJECT
public:
    explicit Test(QObject *parent = 0);
    ~Test();
    bool isModified() const {return m_modified;}
    void showInfo();
    bool loadFileASC(const QString &fileName);
    QVector<double>& x() { return time; }
    QVector<double>& y(int i) { return magnitudes[i].data; }
    int measurements() { return magnitudes.size(); }
    int samples() { return time.size(); }
    QString title() { return m_title; }
    QString title(int i) { return magnitudes[i].m_title; }

signals:
    void newInfo(QString msg);

private:
    QString m_fileName;
    bool m_modified;    //WARNING is this really needed???
    QString m_title;
    QString m_description;
    QDateTime m_dateTime;
    double m_delta;
    QString m_unit; //WARNING is this really needed???
    QVector<Measurement> magnitudes;
    QVector<double> time;

    bool readHeaderASC(QTextStream& in);
};

#endif // TEST_H
