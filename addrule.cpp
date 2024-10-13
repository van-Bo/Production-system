#include "addrule.h"
#include "ui_addrule.h"

#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>


addRule::addRule(QSqlDatabase *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addRule), dataBase(db)
{
    ui->setupUi(this);

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

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &addRule::onAccepted);
}

addRule::~addRule()
{
    delete ui;
}

void addRule::onAccepted()
{
    int RuleID = ui->ruleID->text().toInt();
    QString Conditions = ui->conditions->text();

    static QRegularExpression re("\\s+");
    QStringList list = Conditions.split(re, Qt::SkipEmptyParts);
        QString formattedString = "[\"" + list.join("\", \"") + "\"]";
    QString Conclusion = ui->conclusion->text();
    qDebug() << formattedString;

    int Priority = ui->priority->text().toInt();
    int State = ui->state->text().toInt();

    if (addRule::dataBase->open())
    {
        QSqlQuery insertQuery;
        insertQuery.prepare("INSERT INTO RuleBase(RuleID, Conditions, Conclusion, Priority, State) VALUES (:ruleID, :conditions, :conclusion, :priority, :state)");
        insertQuery.bindValue(":ruleID", RuleID);
        insertQuery.bindValue(":conditions", formattedString);
        insertQuery.bindValue(":conclusion", Conclusion);
        insertQuery.bindValue(":priority", Priority);
        insertQuery.bindValue(":state", State);

        if (insertQuery.exec())
        {
            qDebug() << "<module:addRule-onAccepted> Data inserted successfully!";
        }
        else
        {
            qDebug() << "<module:addRule-onAccepted> Failed to insert data:" << insertQuery.lastError().text();
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
        qDebug() << "Failed to connect to database:" << addRule::dataBase->lastError();
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Database Error");
        msgBox.setText("Failed to connect to database:");
        msgBox.setInformativeText(addRule::dataBase->lastError().text());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setStyleSheet("QLabel{min-width: 200px;}");
        msgBox.exec();
    }
    emit dataInserted();
    accept();
}
