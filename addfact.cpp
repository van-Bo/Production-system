#include "addfact.h"
#include "ui_addfact.h"

addFact::addFact(QSqlDatabase* db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addFact), dataBase(db)
{
    this->setStyleSheet(
        "QLineEdit {"
        "    font-size: 16px;"  // 设置字体大小
        "    padding: 5px;"     // 设置内边距，使文本框更宽
        "    border: 2px solid #8f8f91;"  // 设置边框颜色和宽度
        "    border-radius: 10px;"  // 设置圆角
        "    background: #f0f0f0;"  // 设置背景颜色
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #00aaff;"  // 设置聚焦时的边框颜色
        "}"
        "QLineEdit:placeholder {"
        "    color: #a9a9a9;"  // 设置 placeholder 文本颜色
        "    font-style: italic;"  // 设置 placeholder 文本样式
        "}"
    );
    ui->setupUi(this);
}

addFact::~addFact()
{
    delete ui;
}

void addFact::on_buttonBox_accepted()
{
    int FactID = ui->factID->text().toInt();
    QString FactContent = ui->factContent->text();

    if (addFact::dataBase->open())
    {
        QSqlQuery insertQuery;
        insertQuery.prepare("INSERT INTO WorkingMemory(FactID, FactContent) VALUES (:FactID, :FactContent)");
        insertQuery.bindValue(":FactID", FactID);
        insertQuery.bindValue(":FactContent", FactContent);

        if (insertQuery.exec())
        {
            qDebug() << "<module:addFact-onAccepted> Data inserted successfully!";
        }
        else
        {
            qDebug() << "<module:addFact-onAccepted> Failed to insert data:" << insertQuery.lastError().text();
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setWindowTitle("Database Error");
            msgBox.setText("Failed to insert data:");
            msgBox.setInformativeText(insertQuery.lastError().text());
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);

            // 设置样式表
            msgBox.setStyleSheet(
                "QLabel{"
                "min-width: 200px;"
                "font-family: 'JetBrains Mono NF';"  // 设置字体
                "font-size: 16px;"       // 设置字体大小
                "color: #333333;"        // 设置字体颜色
                "font-weight: bold;"
                "text-align: center;"    // 居中显示
                "}"
                "QPushButton{"
                "font-family: 'JetBrains Mono NF';"
                "font-size: 15px;"
                "}"
            );
            msgBox.exec();
        }
    }
    else
    {
        qDebug() << "Failed to connect to database:" << addFact::dataBase->lastError();
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Database Error");
        msgBox.setText("Failed to connect to database:");
        msgBox.setInformativeText(addFact::dataBase->lastError().text());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setStyleSheet("QLabel{min-width: 200px;}");
        msgBox.exec();
    }
    emit dataInserted();
    accept();
}

