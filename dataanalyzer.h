#ifndef DATAANALYZER_H
#define DATAANALYZER_H

#include <QMainWindow>

namespace Ui {
class DataAnalyzer;
}

class DataAnalyzer : public QMainWindow
{
    Q_OBJECT

public:
    explicit DataAnalyzer(QWidget *parent = 0);
    ~DataAnalyzer();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::DataAnalyzer *ui;
    void readSettings();
    void writeSettings();
};

#endif // DATAANALYZER_H
