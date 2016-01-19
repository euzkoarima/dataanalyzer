#include "dataanalyzer.h"
#include <QApplication>
#include <QTranslator>
#include <QDebug>
//#include <QLibraryInfo>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(dataanalyzer);

    QApplication app(argc, argv);
    app.setOrganizationName("UNLZ");
    app.setOrganizationDomain("gmon.ingenieria.unlz.edu.ar");
    app.setApplicationName("Data Analyzer");
    app.setApplicationVersion("0.4");

    QString locale = QLocale::system().name();
    bool charged;
    QString path;

//    QTranslator qtTranslator;
//    qtTranslator.load("qt_" +locale,QLibraryInfo::location(QLibraryInfo::TranslationsPath));

    QTranslator translator;
    path = ":/translations/DataAnalyzer_" + locale;
    charged = translator.load(path);
    qDebug() << "Traducción de mi aplicación " << path <<" : " << charged;
    app.installTranslator(&translator);

    DataAnalyzer w;
    w.show();

    return app.exec();
}
