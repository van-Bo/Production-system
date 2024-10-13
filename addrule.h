#ifndef ADDRULE_H
#define ADDRULE_H

#include <QDialog>
#include <QSqlDatabase>

namespace Ui {
class addRule;
}

class addRule : public QDialog
{
    Q_OBJECT

public:
    explicit addRule(QSqlDatabase* db = nullptr, QWidget *parent = nullptr);
    ~addRule();
    void onAccepted();

signals:
    void dataInserted();

private:
    Ui::addRule *ui;
    QSqlDatabase *dataBase;
};

#endif // ADDRULE_H
