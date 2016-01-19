#include "dataanalyzer.h"
#include "ui_dataanalyzer.h"
#include <QSettings>
#include <QCloseEvent>

DataAnalyzer::DataAnalyzer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataAnalyzer)
{
    ui->setupUi(this);
    readSettings();
}

DataAnalyzer::~DataAnalyzer()
{
    delete ui;
}

// --------->> Events
void DataAnalyzer::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

// --------->> Privates

void DataAnalyzer::readSettings()
{
    QSettings settings;
    //TODO: que habra la primera vez con un valor por defecto elegido por mi
    //TODO: que controle si entra en la pantalla o no, y redimensionar si hace falta

    restoreGeometry(settings.value("mainWindow/geometry").toByteArray());
    restoreState(settings.value("mainWindow/windowState").toByteArray());

}

void DataAnalyzer::writeSettings()
{
    QSettings settings;

    settings.setValue("mainWindow/geometry", saveGeometry());
    settings.setValue("mainWindow/windowState", saveState());
}

