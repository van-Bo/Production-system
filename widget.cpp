#include "widget.h"
#include "ui_widget.h"
#include "addrule.h"
#include "addfact.h"

#include <QSqlTableModel>
#include <QObject>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QModelIndexList>

Widget::Widget(QSqlDatabase *db, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::Widget), ruleBaseModel(nullptr), inferenceStepsModel(nullptr), workingMemoryModel(nullptr),
    dataBase(db)
{
//    QSqlTableModel *inferenceStepsModel = new QSqlTableModel(this);
//    inferenceStepsModel->setTable("InferenceSteps");
//    inferenceStepsModel->setHeaderData(0, Qt::Horizontal, QObject::tr("stepID"));
//    inferenceStepsModel->setHeaderData(1, Qt::Horizontal, QObject::tr("ruleID"));
//    inferenceStepsModel->setHeaderData(2, Qt::Horizontal, QObject::tr("immediateResult"));
//    inferenceStepsModel->select();

//    ui->inferenceSteps_content->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    ui->inferenceSteps_content->setModel(inferenceStepsModel);
//    ui->inferenceSteps_content->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 根据可用空间自动调整列宽
//    ui->inferenceSteps_content->show();
    ui->setupUi(this);
    connect(this, &Widget::inferenceOver, this, &Widget::updateDatabaseDisplay);
    connect(this, &Widget::initSystemOver, this, &Widget::updateDatabaseDisplay);
    connect(this, &Widget::initDataBaseState, this, &Widget::updateDatabaseDisplay);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    adjustColumnWidths();
}

void Widget::adjustColumnWidths()
{
    int totalWidth = ui->ruleBase_content->horizontalHeader()->width();
    ui->ruleBase_content->setColumnWidth(0, totalWidth * 0.1); // 第一列占10%
    ui->ruleBase_content->setColumnWidth(1, totalWidth * 0.5); // 第二列占50%
    ui->ruleBase_content->setColumnWidth(2, totalWidth * 0.2); // 第三列占20%
    ui->ruleBase_content->setColumnWidth(3, totalWidth * 0.1); // 第四列占10%
    ui->ruleBase_content->setColumnWidth(4, totalWidth * 0.1); // 第五列占10%
}

void Widget::on_showSteps_clicked()
{
    inferenceStepsModel = new QSqlTableModel;
    inferenceStepsModel->setTable("InferenceSteps");
    inferenceStepsModel->setHeaderData(0, Qt::Horizontal, QObject::tr("stepID"));
    inferenceStepsModel->setHeaderData(1, Qt::Horizontal, QObject::tr("ruleID"));
    inferenceStepsModel->setHeaderData(2, Qt::Horizontal, QObject::tr("immediateResult"));
    inferenceStepsModel->select();

    ui->inferenceSteps_content->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->inferenceSteps_content->setModel(inferenceStepsModel);
    ui->inferenceSteps_content->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 根据可用空间自动调整列宽
    ui->inferenceSteps_content->show();
}


void Widget::on_showMemories_clicked()
{
    workingMemoryModel = new QSqlTableModel;
    workingMemoryModel->setTable("WorkingMemory");
    workingMemoryModel->setHeaderData(0, Qt::Horizontal, QObject::tr("FactID"));
    workingMemoryModel->setHeaderData(1, Qt::Horizontal, QObject::tr("FactContent"));
    workingMemoryModel->select();

    ui->workingMemroy_content->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->workingMemroy_content->setModel(workingMemoryModel);
    ui->workingMemroy_content->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 根据可用空间自动调整列宽
    ui->workingMemroy_content->show();
}


void Widget::on_showRules_clicked()
{
    ruleBaseModel = new QSqlTableModel;
    ruleBaseModel->setTable("RuleBase");
    ruleBaseModel->setHeaderData(0, Qt::Horizontal, QObject::tr("RuleID"));
    ruleBaseModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Conditions"));
    ruleBaseModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Conclusion"));
    ruleBaseModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Priority"));
    ruleBaseModel->setHeaderData(4, Qt::Horizontal, QObject::tr("state"));
    ruleBaseModel->select();

    // 设置列宽，根据内容自动调整
//    ui->ruleBase_content->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 所有列宽均匀分布以填满整个表格
//    ui->ruleBase_content->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 设置列宽的调整模式为手动调整
//    ui->ruleBase_content->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    ui->ruleBase_content->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->ruleBase_content->setModel(ruleBaseModel);

    int totalWidth = ui->ruleBase_content->horizontalHeader()->width();
    ui->ruleBase_content->setColumnWidth(0, totalWidth * 0.1); // 第一列占10%
    ui->ruleBase_content->setColumnWidth(1, totalWidth * 0.5); // 第二列占50%
    ui->ruleBase_content->setColumnWidth(2, totalWidth * 0.2); // 第三列占20%
    ui->ruleBase_content->setColumnWidth(3, totalWidth * 0.1); // 第四列占10%
    ui->ruleBase_content->setColumnWidth(4, totalWidth * 0.1); // 第五列占10%
    ui->ruleBase_content->show();
}


void Widget::on_deleteRule_clicked()
{
    QModelIndexList selectedRows = ui->ruleBase_content->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Warning");

        // 设置文本格式为富文本
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setText("<div style='text-align: center;'><b>没有选中任何行！</b><br>请先选择一行再进行操作。</div>");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        // 设置样式表来使用 JetBrains Mono NF 字体
        msgBox.setStyleSheet(
            "QMessageBox {"
            "    min-width: 300px; "  // 设置最小宽度
            "    min-height: 150px; " // 设置最小高度
            "}"
            "QLabel {"
            "    font-family: 'JetBrains Mono NF';" // 设置文本字体
            "    font-size: 14px; "   // 设置文本字体大小
            "}"
            "QPushButton {"
            "    width: 80px; "
            "    font-size: 12px; "   // 设置按钮字体大小
            "}"
            "QMessageBox QLabel#qt_msgbox_label {"
            "    font-size: 16px; "   // 设置消息文本字体大小
            "}"
            "QMessageBox QLabel#qt_msgbox_title {"
            "    font-family: 'JetBrains Mono NF';" // 设置标题字体
            "    font-size: 18px; "   // 设置标题字体大小
            "    font-weight: bold; " // 设置标题字体加粗
            "}"
        );
        msgBox.exec();
        return;
    }

    for (const QModelIndex &index : selectedRows) {
        ruleBaseModel->removeRow(index.row());
    }

}


void Widget::on_addRule_clicked()
{
    addRule *demo = new addRule(Widget::dataBase, this);
    connect(demo, &addRule::dataInserted, this, &Widget::updateDatabaseDisplay);
    int result = demo->exec();
    qDebug() << "addRule Dialog closed with result:" << result;
}

// 刷新 table view 中的数据库内容
void Widget::updateDatabaseDisplay()
{
//    Widget::ruleBaseModel->select();
//    Widget::inferenceStepsModel->select();
//    Widget::workingMemoryModel->select();
    if (inferenceStepsModel != nullptr)
        Widget::inferenceStepsModel->select();
    else
    {
        inferenceStepsModel = new QSqlTableModel;
        inferenceStepsModel->setTable("InferenceSteps");
        inferenceStepsModel->setHeaderData(0, Qt::Horizontal, QObject::tr("stepID"));
        inferenceStepsModel->setHeaderData(1, Qt::Horizontal, QObject::tr("ruleID"));
        inferenceStepsModel->setHeaderData(2, Qt::Horizontal, QObject::tr("immediateResult"));
        inferenceStepsModel->select();
        ui->inferenceSteps_content->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->inferenceSteps_content->setModel(inferenceStepsModel);
        ui->inferenceSteps_content->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 根据可用空间自动调整列宽
        ui->inferenceSteps_content->show();
    }

    if (workingMemoryModel != nullptr)
        Widget::workingMemoryModel->select();
    else
    {
        workingMemoryModel = new QSqlTableModel;
        workingMemoryModel->setTable("WorkingMemory");
        workingMemoryModel->setHeaderData(0, Qt::Horizontal, QObject::tr("FactID"));
        workingMemoryModel->setHeaderData(1, Qt::Horizontal, QObject::tr("FactContent"));
        workingMemoryModel->select();
        ui->workingMemroy_content->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->workingMemroy_content->setModel(workingMemoryModel);
        ui->workingMemroy_content->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 根据可用空间自动调整列宽
        ui->workingMemroy_content->show();
    }

    if (ruleBaseModel != nullptr)
        Widget::ruleBaseModel->select();
    else
    {
        ruleBaseModel = new QSqlTableModel;
        ruleBaseModel->setTable("RuleBase");
        ruleBaseModel->setHeaderData(0, Qt::Horizontal, QObject::tr("RuleID"));
        ruleBaseModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Conditions"));
        ruleBaseModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Conclusion"));
        ruleBaseModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Priority"));
        ruleBaseModel->setHeaderData(4, Qt::Horizontal, QObject::tr("state"));
        ruleBaseModel->select();
        ui->ruleBase_content->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->ruleBase_content->setModel(ruleBaseModel);

        int totalWidth = ui->ruleBase_content->horizontalHeader()->width();
        ui->ruleBase_content->setColumnWidth(0, totalWidth * 0.1); // 第一列占10%
        ui->ruleBase_content->setColumnWidth(1, totalWidth * 0.5); // 第二列占50%
        ui->ruleBase_content->setColumnWidth(2, totalWidth * 0.2); // 第三列占20%
        ui->ruleBase_content->setColumnWidth(3, totalWidth * 0.1); // 第四列占10%
        ui->ruleBase_content->setColumnWidth(4, totalWidth * 0.1); // 第五列占10%
        ui->ruleBase_content->show();
    }
}

// 获取有效的规则
QVector<Rule> Widget::getRuleBase_valid()
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
QSet<QString> Widget::getFacts()
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
QStringList Widget::inference(QVector<Rule> RuleBase, QSet<QString> Facts)
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
void Widget::addConSetToWorMry(QStringList conflictSet)
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


void Widget::on_pushDataBase_clicked()
{
    bool isKeep = true;
    while (isKeep)
    {
        QVector<Rule> RuleBase = getRuleBase_valid();
        QSet<QString> Facts = getFacts();

        QStringList conflictSet =  inference(RuleBase, Facts);
        qDebug() << "conflictSet is Belowing ...";
        for (auto &c : conflictSet)
            qDebug() << c, Facts.insert(c);
        addConSetToWorMry(conflictSet);
        qDebug() << "";

        if (conflictSet.size() == 0) isKeep = false;
    }

    emit inferenceOver();
}

// 初始化规则的使用状态为 0
void Widget::initRuleBase_state()
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
void Widget::initWorkingMemory()
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

void Widget::initInferenceSteps()
{
    QSqlQuery cleanQuery;
    cleanQuery.exec("delete from inferenceSteps");
}

void Widget::on_initSystem_clicked()
{
    // 初始化事实库
    initWorkingMemory();

    // 初始化规则库中的规则状态为 0
    initRuleBase_state();
    qDebug() << "";

    // 初始化推理过程库
    initInferenceSteps();

    emit initSystemOver();
}

void Widget::on_deleteFact_clicked()
{
    QModelIndexList selectedRows = ui->workingMemroy_content->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Warning");

        // 设置文本格式为富文本
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setText("<div style='text-align: center;'><b>没有选中任何行！</b><br>请先选择一行再进行操作。</div>");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        // 设置样式表来使用 JetBrains Mono NF 字体
        msgBox.setStyleSheet(
            "QMessageBox {"
            "    min-width: 300px; "  // 设置最小宽度
            "    min-height: 150px; " // 设置最小高度
            "}"
            "QLabel {"
            "    font-family: 'JetBrains Mono NF';" // 设置文本字体
            "    font-size: 14px; "   // 设置文本字体大小
            "}"
            "QPushButton {"
            "    width: 80px; "
            "    font-size: 12px; "   // 设置按钮字体大小
            "}"
            "QMessageBox QLabel#qt_msgbox_label {"
            "    font-size: 16px; "   // 设置消息文本字体大小
            "}"
            "QMessageBox QLabel#qt_msgbox_title {"
            "    font-family: 'JetBrains Mono NF';" // 设置标题字体
            "    font-size: 18px; "   // 设置标题字体大小
            "    font-weight: bold; " // 设置标题字体加粗
            "}"
        );
        msgBox.exec();
        return;
    }

    for (const QModelIndex &index : selectedRows) {
        workingMemoryModel->removeRow(index.row());
    }
}


void Widget::on_addFact_clicked()
{
    addFact *demo = new addFact(Widget::dataBase, this);
    connect(demo, &addFact::dataInserted, this, &Widget::updateDatabaseDisplay);
    int result = demo->exec();
    qDebug() << "addFact Dialog closed with result:" << result;
}


void Widget::on_initRuleBaseState_clicked()
{
    initRuleBase_state();
    emit initDataBaseState();
}

