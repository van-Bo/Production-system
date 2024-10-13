#ifndef ADDFACT_H
#define ADDFACT_H

#include <QDialog>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

namespace Ui {
class addFact;
}

class addFact : public QDialog
{
    Q_OBJECT

public:
    explicit addFact(QSqlDatabase* db = nullptr, QWidget *parent = nullptr);
    ~addFact();

private slots:
    void on_buttonBox_accepted();

signals:
    void dataInserted();

private:
    Ui::addFact *ui;
    QSqlDatabase *dataBase;
};

#endif // ADDFACT_H
