#include "testmodel.h"

TestModel::TestModel(QObject *parent) : m_test {nullptr}
{
    Q_UNUSED(parent)
}

int TestModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_test ? m_test->samples() : 0;
}

int TestModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_test ? m_test->measurements() : 1;
}

QVariant TestModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        return m_test->y(index.column())[index.row()];
    }
    return QVariant();
}

QVariant TestModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return m_test ? m_test->title(section) : tr("No Data");
        } else if (orientation == Qt::Vertical) {
            int msec = static_cast<int>(m_test->x()[section] * 1000);
            QTime t = QTime(0, 0, 0, 0);
            t = t.addMSecs(msec);
            QString r = t.toString("mm:ss.zzz");
            return r;
//            return QString::number(m_test->x()[section]);   //, 'f', 4) if want fixed decimals
            //TODO: mirar delta en archivo texto y de ahí sacar la cantidad de decimales
            //o que la cantidad de decimales sea preferencia o puesto a mano (prefiero deducir la mejor opcion)
            //y como dar forma minutos y segundos!!!
        }
    }
    return QVariant();
}

void TestModel::testChange(Test *t)
{
    this->beginResetModel();
    m_test = t;
    this->endResetModel();
}
