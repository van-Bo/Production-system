#ifndef WIDGET_H
#define WIDGET_H

#include "rule.h"
#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QVector>
#include <QSet>
#include <QSqlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QSqlDatabase* db = nullptr, QWidget *parent = nullptr);
    ~Widget();
    void resizeEvent(QResizeEvent *event) override; // 实现 ruleBase table view 的动态比例缩放
    void updateDatabaseDisplay();

    QVector<Rule> getRuleBase_valid();
    QSet<QString> getFacts();
    QStringList inference(QVector<Rule> RuleBase, QSet<QString> Facts);
    void addConSetToWorMry(QStringList conflictSet);

    void initRuleBase_state();
    void initWorkingMemory();
    void initInferenceSteps();

private slots:
    void on_showSteps_clicked();

    void on_showMemories_clicked();

    void on_showRules_clicked();

    void on_deleteRule_clicked();

    void on_addRule_clicked();

    void on_pushDataBase_clicked();


    void on_initSystem_clicked();

    void on_deleteFact_clicked();

    void on_addFact_clicked();

    void on_initRuleBaseState_clicked();

signals:
    void inferenceOver();
    void initSystemOver();
    void initDataBaseState();

private:
    Ui::Widget *ui;
    QSqlTableModel *ruleBaseModel;
    QSqlTableModel *inferenceStepsModel;
    QSqlTableModel *workingMemoryModel;
    QSqlDatabase *dataBase;
    void adjustColumnWidths();  // 按照比例设置 ruleBase table view 的列宽
};
#endif // WIDGET_H
