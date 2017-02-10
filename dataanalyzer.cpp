#include "dataanalyzer.h"
#include "ui_dataanalyzer.h"
#include <QSettings>
#include <QCloseEvent>
//#include <QSignalMapper>
#include <QDebug>
#include <QFileDialog>

DataAnalyzer::DataAnalyzer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataAnalyzer),
    m_activeTest(nullptr)
{
    ui->setupUi(this);
    readSettings();
    //TODO isolate the initialization on his own method
    ui->textBrowserInfo->append(QApplication::applicationName());
    ui->textBrowserInfo->append(QApplication::applicationVersion());

    qDebug() << "Cantidad de widgets: " << ui->stackedWidget->count();
    qDebug() << "Current index: " << ui->stackedWidget->currentIndex();
//    QSignalMapper *signalMapper;
//    signalMapper = new QSignalMapper(ui->stackedWidget);
//    signalMapper->setMapping(ui->actionInfo, 0);
//    signalMapper->setMapping(ui->actionData, 1);
//    signalMapper->setMapping(ui->actionGraph, 2);
//    connect(ui->actionInfo, &QAction::triggered, signalMapper,
//            static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
//    connect(ui->actionData, &QAction::triggered, signalMapper,
//            static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
//    connect(ui->actionGraph, &QAction::triggered, signalMapper,
//            static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
//    connect(signalMapper,
//            static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped),
//            ui->stackedWidget, &QStackedWidget::setCurrentIndex);
    //m_activeTest = new(Test);
    connect(ui->actionInfo, &QAction::triggered,
            [=]() {ui->stackedWidget->setCurrentIndex(0);});
    connect(ui->actionData, &QAction::triggered,
            [=]() {ui->stackedWidget->setCurrentIndex(1);});
    connect(ui->actionGraph, &QAction::triggered,
            [=]() {ui->stackedWidget->setCurrentIndex(2);});

    //BUG workaround por menubar action shortcuts not working (5.7 & 5.8 need it, 5.6 works ok)
    //http://stackoverflow.com/questions/23916623/qt5-doesnt-recognised-shortcuts-unless-actions-are-added-to-a-toolbar
    //https://bugs.launchpad.net/ubuntu/+source/appmenu-qt5/+bug/1313248
    this->addAction(ui->actionOpen);
    this->addAction(ui->actionQuit);
    m_model = new TestModel(nullptr);//TODO: ??????????? y si pongo como parent a tableView ????????????????
    connect(this, &DataAnalyzer::testChange, m_model, &TestModel::testChange);//WARNING: esta bien que emita datanalyzer o debería emitir test??
    ui->tableView->setModel(m_model);
    //m_fixedTicker = QSharedPointer<QCPAxisTickerFixed>::create();
    //m_ticker = QSharedPointer<QCPAxisTicker>::create();
    m_timeTicker = QSharedPointer<QCPAxisTickerTime>::create();
    m_timeTicker->setTimeFormat("%m:%s.%z"); //("mm:ss.zzz"
    m_timeTicker->setFieldWidth(QCPAxisTickerTime::TimeUnit::tuMinutes,2);
    m_timeTicker->setFieldWidth(QCPAxisTickerTime::TimeUnit::tuSeconds,2);
    m_timeTicker->setFieldWidth(QCPAxisTickerTime::TimeUnit::tuMilliseconds,3);
    //ui->qcpGraph->xAxis->setAutoTickStep(true); //TODO no se el equivalente nuevo
    m_ticker = ui->qcpGraph->xAxis->ticker();
    initializePlot();
    makePlot();
}

DataAnalyzer::~DataAnalyzer()
{
    delete ui;
}

void DataAnalyzer::displayInfo(QString msg)
{
    ui->textBrowserInfo->append(msg);
}

// --------->> Events
void DataAnalyzer::closeEvent(QCloseEvent *event)
{
    //TODO: delete test, if modified, ask if want to save
    writeSettings();
    event->accept();
}

// --------->> Actions (private slots)
void DataAnalyzer::on_actionOpen_triggered()
{
    //TODO: open file dialog in last user directory
    QString sFil;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                       tr("Text Data in ASC format (*.ASC *.txt)"), &sFil);
    qDebug() << "Selected Filter: " << sFil;
     if (!fileName.isEmpty()) {  //TODO: check if there's other file already loaded
         qDebug() << "Selected File: " << fileName;
         if (m_activeTest && m_activeTest->isModified()) {
             //TODO: ask if want to saver changes
             qDebug() << "Habría que haber guardado los cambios";
         }
         if (m_activeTest) {
             //TODO: think about load in temporary Test object, if the load fail still have the old one
             //Since a Test use a lot of memory, by now at least, simply delete the old test object
             emit testChange(nullptr);  // Before delete to avoid problems
             //m_activeTest->disconnect();
             //^^^ not needed, delete delete m_activeTest call qobject destructor wich call disconnect
             delete m_activeTest;
             m_activeTest = nullptr;
         }
         m_activeTest = new Test(this);
         connect(m_activeTest, &Test::newInfo, this, &DataAnalyzer::displayInfo);//WARNING: change this to Test creator?? problem is Test should know about DataAnalyzer class wich makes no sense
         ui->textBrowserInfo->clear();

         if (m_activeTest->loadFileASC(fileName)) {
             qDebug() << "Exito en abrir el archivo " << fileName;
             emit testChange(m_activeTest);  // After successfull load to avoid problems
         } else {
             qDebug() << "Fallo en abrir el archivo " << fileName;
             emit testChange(nullptr);  // Before delete to avoid problems
             //m_activeTest->disconnect();
             //^^^ not needed, delete delete m_activeTest call qobject destructor wich call disconnect
             delete m_activeTest;
             m_activeTest = nullptr;
         }
         makePlot();
     }
     else {
         qDebug() << "No file selected";
     }
}

void DataAnalyzer::mousePress(QMouseEvent *event)
{
    Q_UNUSED(event)
    ui->qcpGraph->axisRect()->setRangeDrag(setOrientation());
}

void DataAnalyzer::mouseWheel(QWheelEvent *event)
{
    Q_UNUSED(event)
    ui->qcpGraph->axisRect()->setRangeZoom(setOrientation());
}

void DataAnalyzer::axisDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
    if (part == QCPAxis::spTickLabels && axis == ui->qcpGraph->xAxis) {
        if (m_activeTest &&  ui->qcpGraph->xAxis->ticker() == m_ticker) {
            ui->qcpGraph->xAxis->setTicker(m_timeTicker);
            //ui->qcpGraph->xAxis->setDateTimeFormat("mm:ss.zzz");
        } else {    //QCPAxis::LabelType::ltDateTime*
            ui->qcpGraph->xAxis->setTicker(m_ticker);
            //ui->qcpGraph->xAxis->setNumberFormat("g");
        }
        ui->qcpGraph->replot();
    }
}

void DataAnalyzer::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (ui->qcpGraph->legend->selectTest(pos, false) >= 0) {
        int selectedItem = -1;
        QCPPlottableLegendItem *plItem;
        for (int i = 0; i < ui->qcpGraph->graphCount(); ++i) {
            plItem = ui->qcpGraph->legend->itemWithPlottable(ui->qcpGraph->graph(i));
            if (plItem->selectTest(pos, false) > 0.0) {
                selectedItem = i;
                break;
            }
        }
        if (selectedItem >= 0)
            menu->addAction("En Item de leyenda: " + plItem->plottable()->name(), this, SLOT(dummyAction()))->setData(selectedItem);
        else
            menu->addAction("En leyenda general", this, SLOT(dummyAction()));
    } else {
        QString menuText = QStringLiteral("Otra Parte");
        for (int i = 0; i < ui->qcpGraph->graphCount(); ++i) {
            double distance = ui->qcpGraph->graph(i)->selectTest(pos, false);
            if (distance >= 0.0 && distance < 1.0) {
                menuText = QStringLiteral("Graph: ") +  ui->qcpGraph->graph(i)->name();
                qDebug() << i << ": " << ui->qcpGraph->graph(i)->name() << " selected: " << distance;
                break;
            }
        }
        menu->addAction(menuText, this, SLOT(dummyAction()));
    }
    menu->popup(ui->qcpGraph->mapToGlobal(pos));
}

void DataAnalyzer::selectionChanged()
{
    for (int i = 0; i < ui->qcpGraph->graphCount(); ++i) {
        QCPGraph *graph = ui->qcpGraph->graph(i);
        QCPPlottableLegendItem *item = ui->qcpGraph->legend->itemWithPlottable(graph);
        if (item->selected() || graph->selected()) {
            item->setSelected(true);
            //graph->setSelected(true);
            //quizas: si no se empty usar enforceType
            QCPDataSelection datasel = graph->selection();
            if (datasel.isEmpty()) {
                QCPDataRange dr(0, graph->dataCount());
                datasel += dr;
                //datasel.enforceType(QCP::SelectionType::stWhole);
                graph->setSelection(datasel);
            }
        }
    }
}

void DataAnalyzer::dummyAction()
{
    if (QAction* contextAction = qobject_cast<QAction*>(sender())) { // make sure this slot is really called by a context menu action, so it carries the data we need
        bool ok;
        int dataInt = contextAction->data().toInt(&ok);
        if (ok) {
            qDebug() << "dummyAction Ok - data: " << dataInt;
            QCPGraph *graph = ui->qcpGraph->graph(dataInt);
            QCPPlottableLegendItem *item = ui->qcpGraph->legend->itemWithPlottable(graph);
            graph->setVisible(!graph->visible());
            QFont font = item->font();
            font.setStrikeOut(!font.strikeOut());
            item->setFont(font);
            QColor color = item->textColor();
            if (color == Qt::black)
                color.setNamedColor("gray");
            else
                color.setNamedColor("black");
            item->setTextColor(color);
            ui->qcpGraph->replot();
        } else { //caso general (por ahora, esto debería ir en otra acción
            qDebug() << "dummyAction caso general";
            bool enlarge = false;
            for (int i = 0; i < ui->qcpGraph->graphCount(); ++i) {
                if (ui->qcpGraph->graph(i)->visible()) {
                    qDebug() << "Graph " << i << "enlarge " << enlarge;
                    ui->qcpGraph->graph(i)->rescaleValueAxis(enlarge++);
                }
            }
            ui->qcpGraph->replot();
        }
    }
}

// --------->> Privates

void DataAnalyzer::readSettings()
{
    QSettings settings;
    //TODO: Open the first time with a default value choosed by me
    //TODO: Check if fit in the current display, resize if not

    restoreGeometry(settings.value("mainWindow/geometry").toByteArray());
    restoreState(settings.value("mainWindow/windowState").toByteArray());

}

void DataAnalyzer::writeSettings()
{
    QSettings settings;

    settings.setValue("mainWindow/geometry", saveGeometry());
    settings.setValue("mainWindow/windowState", saveState());
}

void DataAnalyzer::initializePlot()
{
    ui->qcpGraph->plotLayout()->insertRow(0);
    ui->qcpGraph->plotLayout()->addElement(0,0,new QCPTextElement(ui->qcpGraph, tr("No Data")));
    // Use sublayout and dummyElement to fit legend properly
    QCPLayoutGrid *sublayout = new QCPLayoutGrid;
    QCPLayoutElement *dummyElement = new QCPLayoutElement;
    ui->qcpGraph->plotLayout()->addElement(1,1,sublayout);
    ui->qcpGraph->plotLayout()->setColumnStretchFactor(1,0.01);//Use all posible space for graph
    //dummyElement above and below legend
    sublayout->addElement(0, 0, dummyElement);
    sublayout->addElement(1, 0, ui->qcpGraph->legend);
    sublayout->addElement(2, 0, dummyElement);
    //Another dummyElement into right to properly display legend's borders
    sublayout->addElement(1, 1, dummyElement);
    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->qcpGraph, &QCustomPlot::mousePress, this, &DataAnalyzer::mousePress);
    connect(ui->qcpGraph, &QCustomPlot::mouseWheel, this, &DataAnalyzer::mouseWheel);
    // connect slots that takes care of doubleCliks on axes
    connect(ui->qcpGraph, &QCustomPlot::axisDoubleClick, this, &DataAnalyzer::axisDoubleClick);
    // connect slot for context menu popup:
    connect(ui->qcpGraph, &QCustomPlot::customContextMenuRequested, this, &DataAnalyzer::contextMenuRequest);
    // connect slot that ties legend items with graphs
    connect(ui->qcpGraph, &QCustomPlot::selectionChangedByUser, this,  &DataAnalyzer::selectionChanged);

}

void DataAnalyzer::makePlot()
{
    ui->qcpGraph->clearGraphs();
    QCPTextElement *title = dynamic_cast<QCPTextElement*>(ui->qcpGraph->plotLayout()->element(0, 0));
    ui->qcpGraph->deselectAll();
    ui->qcpGraph->xAxis->setUpperEnding(QCPLineEnding::esLineArrow);
    ui->qcpGraph->yAxis->setUpperEnding(QCPLineEnding::esLineArrow);
    if (!m_activeTest) {
        title->setText(tr("No Data"));
        //a graph and data is needed in order to properly display the axes (yAxis in particular)
        ui->qcpGraph->addGraph();
        QVector<double> x;
        x << 0.0 << 1.0;
        ui->qcpGraph->graph()->setData(x, x);
        ui->qcpGraph->graph()->setPen(Qt::NoPen);
        ui->qcpGraph->xAxis->rescale();
        ui->qcpGraph->yAxis->rescale();
        ui->qcpGraph->legend->clearItems(); //legend no needed if there's no data
        ui->qcpGraph->legend->setVisible(false);
        ui->qcpGraph->setInteractions(0);
        //ui->qcpGraph->xAxis->setTickLabelType(QCPAxis::LabelType::ltNumber);
        ui->qcpGraph->xAxis->setNumberFormat("g");
        //ui->qcpGraph->xAxis->setAutoTickStep(false);// just cosmetic
        //ui->qcpGraph->xAxis->setTickStep(0.20);
        ui->qcpGraph->xAxis->setTicker(m_ticker);
        //ui->qcpGraph->yAxis->setAutoTickStep(false);
        //ui->qcpGraph->yAxis->setTickStep(0.25);
        ui->qcpGraph->setContextMenuPolicy(Qt::NoContextMenu);  // setup policy for context menu popup:
        ui->qcpGraph->replot();
        return;
    }
    title->setText(m_activeTest->title());
    enum Qt::GlobalColor color[] {Qt::black, Qt::blue, Qt::red, Qt::green, Qt::yellow, Qt::darkGray, Qt::cyan, Qt::magenta, Qt::darkBlue, Qt::darkRed, Qt::darkGreen};
    constexpr int colorSize = sizeof(color) / sizeof(color[0]);
    int i;
    for (i = 0; i < m_activeTest->measurements(); ++i) {
        ui->qcpGraph->addGraph();
        ui->qcpGraph->graph(i)->setData(m_activeTest->x(), m_activeTest->y(i));
        QPen graphPen;
        graphPen.setColor(color[i%colorSize]);
        ui->qcpGraph->graph(i)->setPen(graphPen);
        ui->qcpGraph->graph(i)->setName(m_activeTest->title(i));
    }
    ui->qcpGraph->xAxis->rescale();
    ui->qcpGraph->yAxis->rescale();
    //ui->qcpGraph->xAxis->setAutoTickStep(true); //was disabled when no data
    //ui->qcpGraph->yAxis->setAutoTickStep(true);
    //ui->qcpGraph->xAxis->setTickLabelType(QCPAxis::LabelType::ltDateTime);
    //ui->qcpGraph->xAxis->setDateTimeFormat("mm:ss.zzz");
    ui->qcpGraph->xAxis->setTicker(m_timeTicker);
    ui->qcpGraph->legend->setVisible(true); //was disabled when no data
    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
    ui->qcpGraph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes
                                  | QCP::iSelectLegend | QCP::iSelectPlottables);
    ui->qcpGraph->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items
    // setup policy for context menu popup:
    ui->qcpGraph->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->qcpGraph->replot();
}


Qt::Orientations DataAnalyzer::setOrientation()
{
    if (ui->qcpGraph->xAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
        if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
            return Qt::Vertical;
        else
            return Qt::Horizontal;
    } else if (ui->qcpGraph->yAxis->selectedParts().testFlag(QCPAxis::spAxis)) {
        if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
            return Qt::Horizontal;
        else
            return Qt::Vertical;
    } else if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier) {
        return Qt::Horizontal;
    } else if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier) {
        return Qt::Vertical;
    } else {
        return Qt::Horizontal | Qt::Vertical;
    }
}
