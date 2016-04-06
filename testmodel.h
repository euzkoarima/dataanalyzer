#ifndef TESTMODEL_H
#define TESTMODEL_H

#include <QAbstractTableModel>
#include "test.h"

class TestModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TestModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

public slots:
    void testChange(Test* t);

private:
    Test* m_test;
};

#endif // TESTMODEL_H
