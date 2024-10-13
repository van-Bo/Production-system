#include "widget.h"

#include <QApplication>
#include <rule.h>
#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <QSet>
#include <QFile>

// 加载样式表文件
void loadStyleSheet(const QString &sheetName) {
    QFile file(sheetName);
    if (file.exists()) {
       qDebug() << "File exists.";
    } else {
       qDebug() << "File does not exist.";
    }

    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
        file.close();
        qDebug() << "Style sheet loaded successfully!!!";
    }
    else
        qDebug() << "Fail to load style sheet >_<";
}

// 初始化规则的使用状态为 0
void initRuleBase_state()
{
    QSqlQuery selectQuery, updateQuery;
    selectQuery.exec("select * from RuleBase");

    while (selectQuery.next())
    {
        int ruleID = selectQuery.value("ruleID").toInt();
        int state = selectQuery.value("state").toInt();

        if (!state) continue;

        updateQuery.prepare("update RuleBase set state = :zero where ruleID = :id");
        updateQuery.bindValue(":zero", 0);
        updateQuery.bindValue(":id", ruleID);

        if (!updateQuery.exec())
            qDebug() << "<module:initRuleBase_state> Error updating the state of the rule " << ruleID << " >_<";
        else
            qDebug() << "<module:initRuleBase_state> rule " << ruleID << "updated successfully ^_^";
    }
}

// 初始化事实库
void initWorkingMemory()
{
    QSqlQuery cleanQuery, insertQuery;
    cleanQuery.exec("delete from WorkingMemory");

    QStringList Facts;
    Facts.append("有毛");
    Facts.append("反刍动物");

    int mark = 1;
    insertQuery.prepare("insert WorkingMemory(FactID, FactContent) values(:FactID, :FactContent)");
    for (auto &f : Facts)
    {
        insertQuery.bindValue(":FactID", mark);
        insertQuery.bindValue(":FactContent", f);

        if (!insertQuery.exec())
            qDebug() << "<module:initWorkingMemory> fail to insert new tuple >_<";
        else
            qDebug() << "<module:initWorkingMemory> inserted new tuple successfully ^_^";
        mark ++;
    }
}

void initInferenceSteps()
{
    QSqlQuery cleanQuery;
    cleanQuery.exec("delete from inferenceSteps");
}

// 获取有效的规则
QVector<Rule> getRuleBase_valid()
{
    QVector<Rule> RuleBase;

    QSqlQuery query;
    query.exec("SELECT * FROM RuleBase");

    while (query.next()) {

        int state = query.value("state").toInt();
        if (state) continue;

        QString jsonString = query.value("Conditions").toString();
        QString conclusion = query.value("Conclusion").toString();
        int ruleID = query.value("ruleID").toInt();
        int priority = query.value("priority").toInt();

        // 解析 JSON 数据
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
        if (!jsonDoc.isArray()) {
            qDebug() << "Invalid JSON format";
        }
        QJsonArray conditions_temp = jsonDoc.array();
        QStringList conditions;
        for (const QJsonValue &value : conditions_temp) {
                conditions.append(value.toString());
            }
//        qDebug() << conditions;

        Rule temp(conditions, conclusion, ruleID, priority, state);
        RuleBase.push_back(temp);
    }
    return RuleBase;
}

// 获取事实
QSet<QString> getFacts()
{
    QSet<QString> Facts;
    QSqlQuery query;
    query.exec("SELECT * FROM WorkingMemory");

    while (query.next())
    {
        QString fact = query.value("FactContent").toString();
        Facts.insert(fact);
    }
    return Facts;
}

// 根据有效的规则、事实推理得到冲突集，同时将使用到的规则状态设置为 1
QStringList inference(QVector<Rule> RuleBase, QSet<QString> Facts)
{
    QSqlQuery updateQuery;
    QStringList conflictSet;

    // 获取有效的规则
    QVector<Rule> validRule;
    int maxPriority = 0;
    for (auto r : RuleBase)
    {
        bool flag = true;
        QStringList conditions = r.getP();
        for (auto &condition : conditions)
        {
            if (Facts.find(condition) == Facts.end())
            {
                flag = false;
                break;
            }
        }
        if (!flag) continue;
        validRule.push_back(r);
        maxPriority = std::max(maxPriority, r.getPriority());
    }

    // 获取当前推理过程库中的 maxStepID
    QSqlQuery getMaxStepID;
    int maxStepID = 0;
    getMaxStepID.exec("select max(StepID) as maxID from inferenceSteps");
    if (!getMaxStepID.next())
        qDebug() << "<module:inference> Error fetching max StepID !!!";
    else
    {
        maxStepID = getMaxStepID.value("maxID").toInt();
        qDebug() << "<module:inference> fetched the max StepID successfully";
    }

    // 依据优先级冲突消解
    QSqlQuery insertQuery;
    insertQuery.prepare("insert inferenceSteps(StepID, RuleID, IntermediateResult) values(:stepID, :ruleID, :result)");
    for (auto &vr : validRule)
    {
        if (vr.getPriority() != maxPriority) continue;

        conflictSet.append(vr.getQ()); // 结论添加至冲突集
        // 修改所使用到的规则的 state
        int ruleID = vr.getRuleID();
        updateQuery.prepare("update RuleBase set state = :true where ruleID = :id");
        updateQuery.bindValue(":true", 1);
        updateQuery.bindValue(":id", ruleID);
        if (!updateQuery.exec())
            qDebug() << "<module:inference> fail to update rule " << ruleID << " 's state >_<";
        else
            qDebug() << "<module:inference> update rule " << ruleID << " 's state successfully ^_^";

        // 添加推理过程到表 inferenceSteps
        insertQuery.bindValue(":stepID", ++ maxStepID);
        insertQuery.bindValue(":ruleID", ruleID);
        insertQuery.bindValue(":result", vr.getQ());
        if (!insertQuery.exec())
            qDebug() << "<module:inference> fail to insert value into inferenceSteps";
        else
            qDebug() << "<module:inference> insert value into inferenceSteps successfully!!!";
    }

    return conflictSet;
}

// 将冲突集中的结果添加至事实库 WorkingMemory 中
void addConSetToWorMry(QStringList conflictSet)
{
    QSqlQuery getMaxID, insertQuery;
    int maxID = -1;
    getMaxID.exec("select max(FactID) as maxID from WorkingMemory");
    if (!getMaxID.next())
        qDebug() << "<module:addConSetToWorMry> Error fetching max FactID !!!";
    else
    {
        maxID = getMaxID.value("maxID").toInt();
        qDebug() << "<module:addConSetToWorMry> fetched the max FactID successfully";
    }

    maxID ++;
    insertQuery.prepare("insert WorkingMemory(FactID, FactContent) values(:FactID, :FactContent)");
    for (auto &c : conflictSet)
    {
        insertQuery.bindValue(":FactID", maxID);
        insertQuery.bindValue(":FactContent", c);
        maxID ++;
        if (!insertQuery.exec())
            qDebug() << "<module:addConSetToWorMry> fail to add conflictSet value to WorkingMemory !!!";
        else
            qDebug() << "<module:addConSetToWorMry> add conflictSet value to WorkingMemory successfully ^_^";
    }
}



int main(int argc, char *argv[])
{
    // 连接 SQL server 数据库
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");   //数据库驱动类型为 SQL server
    qDebug() << "ODBC driver?" << db.isValid();
    QString dsn = QString::fromLocal8Bit("ProductionSystem");    //数据源名称
    db.setHostName("localhost");    //选择本地主机
    db.setDatabaseName(dsn);        //设置数据源名称

    if (!db.open())                 //打开数据库
    {
        qDebug() << db.lastError().text();
        QMessageBox::critical(0, QObject::tr("Database error!"), db.lastError().text());
        return false;
    }
    else
    {
        qDebug() << "Database open success!";
    }

    QApplication a(argc, argv);

    loadStyleSheet(":/styleForWidget.qss");

    Widget w(&db);
    w.show();

    // 初始化事实库
    initWorkingMemory();

    // 初始化规则库中的规则状态为 0
    initRuleBase_state();
    qDebug() << "";

    // 初始化推理过程库
    initInferenceSteps();

//    // 获取有效的规则：条件（QStringList）、结论（QString）
//    QVector<Rule> RuleBase = getRuleBase_valid();
//    for (auto &r : RuleBase)
//        qDebug() << r.getRuleID() << r.getRule();

//    // 获取事实
//    QSet<QString> Facts = getFacts();
//    for (auto &f : Facts)
//        qDebug() << f;

//    bool isKeep = true;
//    while (isKeep)
//    {
//        QVector<Rule> RuleBase = getRuleBase_valid();
//        QSet<QString> Facts = getFacts();

//        QStringList conflictSet =  inference(RuleBase, Facts);
//        qDebug() << "conflictSet is Belowing ...";
//        for (auto &c : conflictSet)
//            qDebug() << c, Facts.insert(c);
//        addConSetToWorMry(conflictSet);
//        qDebug() << "";

//        if (conflictSet.size() == 0) isKeep = false;
//    }
    int result = a.exec();

    // 在应用程序退出时关闭数据库连接
    db.close();
    if (db.isOpen())
        qDebug() << "fail to close the connection to SQL server >_<";
    else
        qDebug() << "close the connection to SQL server successfully ^_^";

    return result;
}
