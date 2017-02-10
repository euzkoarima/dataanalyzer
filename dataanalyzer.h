#ifndef DATAANALYZER_H
#define DATAANALYZER_H

#include <QMainWindow>
#include "test.h"
#include "testmodel.h"
#include "qcustomplot.h"

namespace Ui {
class DataAnalyzer;
}

class DataAnalyzer : public QMainWindow
{
    Q_OBJECT

public:
    explicit DataAnalyzer(QWidget *parent = 0);
    ~DataAnalyzer();

signals:
    void testChange(Test* t);

public slots:
    void displayInfo(QString msg);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_actionOpen_triggered();
    void mousePress(QMouseEvent* event);
    void mouseWheel(QWheelEvent* event);
    void axisDoubleClick(QCPAxis* axis, QCPAxis::SelectablePart part);
    void contextMenuRequest(QPoint pos);
    void selectionChanged();
    void dummyAction();

private:
    Ui::DataAnalyzer *ui;
    Test *m_activeTest;
    TestModel *m_model;
    QSharedPointer<QCPAxisTicker> m_ticker;
    QSharedPointer<QCPAxisTickerTime> m_timeTicker;
    void readSettings();
    void writeSettings();
    void initializePlot();
    void makePlot();
    Qt::Orientations setOrientation();
};

#endif // DATAANALYZER_H
