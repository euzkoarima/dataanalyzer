#include "test.h"
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QMessageBox>
#include <QProgressDialog>
#include <QDebug>
#include <QElapsedTimer>////////////

Test::Test(QObject *parent) : QObject {parent}, m_modified {false} //TODO is m_modified really needed???
{

}

Test::~Test()
{

}

void Test::showInfo()
{
    emit newInfo(tr("<b>Test File: </b> ") + m_fileName);
    emit newInfo(tr("<b>Test Title: </b> ") + m_title);
    emit newInfo(tr("<b>Description: </b> ") + m_description);
    emit newInfo(tr("<b>Date and Time: </b> ") +
                 m_dateTime.toString(tr("MM/dd/yyyy - HH:mm:ss")));
    emit newInfo(tr("<b>Number of Measurements: </b> ") +
                 QString::number(magnitudes.size()));
    emit newInfo(tr("<b>Delta time: </b> ") + QString::number(m_delta)
                 + " " + m_unit);
    emit newInfo(tr("<b>Number of Samples: </b> ") +
                 QString::number(time.size()));
}

bool Test::loadFileASC(const QString &fileName)
{
    QFile file(fileName);
    bool result = false;
    qint64 fsize = QFileInfo(fileName).size();
    qDebug() << "File size: " << fsize;
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(0, tr("Error"),
                                  tr("Cannot read file %1<br><b>Reason:</b> %2.")
                                  .arg(fileName).arg(file.errorString()));
            emit newInfo(tr("<b>File:</b> %1<br><b>Error:</b> %2").arg(fileName)
                         .arg(file.errorString()));
            return result;
        }
        QElapsedTimer timer;///////////////////////////
        timer.start();
        QTextStream in(&file);
        result = readHeaderASC(in);
        if (result) {
            const qint64 INITIAL_LAP = 2000;
            //NOTE: make LAP_PERCENT a preference ???
            const qint64 LAP_PERCENT = 35;
            //NOTE: make MIN_DURATION a preference ???
            const int MIN_DURATION = 1200;
            m_fileName = fileName;
            qint64 nlines;
            QProgressDialog progress("", tr("Cancel"), 0, fsize, 0);
            progress.setWindowModality(Qt::WindowModal);
            progress.setMinimumDuration(MIN_DURATION);
            qint64 posIni = in.pos();
            progress.setValue(posIni);
            qint64 lap = INITIAL_LAP;
            char sep;
            int channels = magnitudes.size();
            double number;
            nlines = 0;
            in >> number >> sep;
            while (in.status() == QTextStream::Ok) {
                time.push_back(number);
                for (int cha = 0; cha < channels; ++cha) {
                    in >> number >> sep;
                    magnitudes[cha].data.push_back(number);
                }
                ++nlines;
                if (nlines % lap == 0) {
                    if (progress.wasCanceled()) {
                        emit newInfo(tr("<br><b>File load canceled by user!</b> "));
                        result = false;
                        break;
                    }
                    if (posIni) {
                        qint64 pos = in.pos();
                        qDebug() << "Setting lap at pos " << pos;
                        qint64 lineSize = (pos - posIni) / nlines + 1;  //+1 for better estimation
                        qDebug() << "lineSize " << lineSize;
                        qint64 newlap =  fsize / lineSize;
                        qDebug() << "Estimated total lines " << newlap;
                        newlap = newlap /LAP_PERCENT;
                        lap = newlap > 1000 ? newlap : 1000;//NOTE: this 1000 in a define/preference??????
                        qDebug() << "New lap " << lap;
                        posIni = 0;
                    }
                    progress.setValue(in.pos());
                }
                in >> number >> sep;
            }
            if (result) {
                qDebug() << "nlines = " << nlines;
                progress.setValue(in.pos());
                qDebug().nospace()  << qSetRealNumberPrecision(10)
                                    << "Time[" << nlines-1 << "]= "
                                    << time[nlines-1];
                for (int i = 0; i < channels; ++i) {
                    qDebug().nospace() << qSetRealNumberPrecision(10)
                                       << magnitudes[i].m_title <<"["
                                       << nlines-1 << "]= "
                                       << magnitudes[i].data[nlines-1];
                }
                showInfo();
            }
        } else { //prueba
            qDebug() << "Error in header, no data procesed";
            emit newInfo(tr("<b>File:</b> %1<br><b>Error in header:</b> no data procesed").arg(fileName));
        }
        qDebug() << "TIME ELAPSED" << timer.elapsed() << " milliseconds";
        file.close();
    } else {
        emit newInfo(tr("The file %1 does not exist!").arg(fileName));
    }
    return result;
}

bool Test::readHeaderASC(QTextStream &in)
{
    QString line;
    bool notDone = true;
    bool success = true;
    int channels = 0;

    QString titleBegin = QStringLiteral("Worksheet name: ");
    QString dateBegin = QStringLiteral("Recording date     : ");
    QString deltaBegin = QStringLiteral("Delta              : ");
    QString chanelsBegin = QStringLiteral("Number of channels : ");
    QString measurementBegin = QStringLiteral("Measurement time[s];");

    while (notDone) {
        if(in.readLineInto(&line)) {
            qDebug() << line;
            if (line.startsWith(titleBegin)) {
                m_title = line.mid(titleBegin.size());
                qDebug() << "Detected Title:" << m_title << " Size: "
                         << m_title.size();
                continue;
            }
            if (line.startsWith(dateBegin)) {
                m_dateTime = QDateTime::fromString(line.mid(dateBegin.size()),
                                                   QStringLiteral("dd/MM/yyyy, hh:mm:ss AP"));
                qDebug() << "Detected date:" << m_dateTime.toString(Qt::ISODate);
                continue;
            }
            if (line.startsWith(deltaBegin)) {
                QStringRef str = line.midRef(deltaBegin.size());
                qDebug() << "Delta substr: " << str;
                int spaceidx = str.indexOf(QChar(' '));
                qDebug() << "Index of space: " << spaceidx;
                bool ok;
                m_delta = str.left(spaceidx).toDouble(&ok); //FIXME: must check bool and take actions if false
                m_unit = str.mid(++spaceidx).toString();
                qDebug() << "Ok: " << ok << "m_delta: " << m_delta << "m_unit: " << m_unit;
                continue;
            }
            if (line.startsWith(chanelsBegin)) {
                bool ok;
                channels = line.mid(chanelsBegin.size()).toInt(&ok);
                if (ok) {
                    qDebug() << "ChanelsBegin #:" << channels;
                    qDebug() << "magnitudes capacity before resize"
                             << magnitudes.capacity();
                    magnitudes.resize(channels);
                    qDebug() << "magnitudes capacity after resize"
                             << magnitudes.capacity();
                    magnitudes.squeeze();
                     qDebug() << "magnitudes capacity after squeeze"
                              << magnitudes.capacity();
                    continue;
                } else {
                    qDebug() << "ERROR in ChanelsBegin. Can't convert " << line;
                    emit newInfo(tr("<b>ERROR in line of channels. Can't convert:</b> %1").arg(line));
                    success = false;
                    break;
                }
            }
            if (line.startsWith(measurementBegin)) {
                 qDebug() << "Detected measurement";
                 if (channels > 0) {
                     line = line.mid(measurementBegin.size());
                     for (int i = 0; i < channels; ++i) {
                        magnitudes[i].m_title = line.section(';', i, i);
                     }
                 } else {
                     qDebug() << "Error measurement before channels";
                     emit newInfo(tr("<b>Error:</b> line of measurements found before line of channels"));
                     success = false;
                 }
                 notDone = false;
            }
        } else {
            success = false;
            notDone = false;
        }
    }
    return success;
}
