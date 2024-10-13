# 平台环境
- Qt Creator 10.0.1
- Qt6 6.2.4

# 产生式系统实验
## 实验内容
设计并编程实现一个基于产生式系统的小型专家系统

## 实验要求
1. 应用领域根据自己兴趣选择；编程语言不限，使用自己擅长的语言。
2. 确定推理方法，根据问题设计并实现一个简单的推理机（要求涉及：匹配、冲突消解）。
3. 规则库要求至少包含15条规则；查询资料了解：知识规则如何存储，都有哪些可行方案和方法，选择确定适合自己系统的知识规则存储方法。
4. 推理机和知识库必须分离；
5. （可选，非必需）在不修改推理机程序的前提下，能够向知识库添加、删除、修改规则。
6. 设计合理的人机交互界面。


# Rule 规则类
- 条件 `p`，数据类型为 `QStringList`；结论 `q`，数据类型为 `QString`
- 其他数据类型同数据库规则表中的属性类型
  
# 数据库存储信息
## 规则库（Rule Base）
### table content
- 规则ID `RuleID`：唯一标识每条规则
- 前提条件 `Conditions`：规则的条件部分。前提条件不唯一，在数据库中使用 `json` 的格式存储
- 结论 `Conclusion`：规则的结论部分
- 优先级 `Priority`：用于冲突消解
- 状态 `State`：标注是否已经被匹配使用过

### 创建表结构、添加元组信息、
```SQL
CREATE TABLE RuleBase (
    RuleID INT PRIMARY KEY,
    Conditions NVARCHAR(MAX),
    Conclusion NVARCHAR(255),
    Priority INT,
    state INT,
);

INSERT INTO RuleBase (RuleID, Conditions, Conclusion, Priority, state)
VALUES (1, N'["有奶"]', N'哺乳动物', null, 0);

INSERT INTO RuleBase (RuleID, Conditions, Conclusion, Priority, state)
VALUES (2, N'["有毛"]', '哺乳动物', null, 0);
```

### 比对查询条件
```SQL
SELECT RuleID, Conclusion
FROM RuleBase
WHERE Conditions = N'["temperature <= 30"]';
```

### 拆分提取 `json` 格式的条件中的信息，并设法进行比对
```C++
#include <QCoreApplication>
#include <QtSql>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

void compareConditions(const QJsonArray &conditions) {
    for (const QJsonValue &value : conditions) {
        QString condition = value.toString();
        qDebug() << "Condition:" << condition;
        // 在这里添加你的条件比对逻辑
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    // 连接到 SQL Server 数据库
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setHostName("your_server_name");
    db.setDatabaseName("your_database_name");
    db.setUserName("your_username");
    db.setPassword("your_password");

    if (!db.open()) {
        qDebug() << "Failed to connect to database:" << db.lastError().text();
        return -1;
    }

    // 执行查询获取 JSON 数据
    QSqlQuery query;
    query.exec("SELECT Conditions FROM RuleBase WHERE RuleID = 1");

    if (query.next()) {
        QString jsonString = query.value(0).toString();
        qDebug() << "JSON String:" << jsonString;

        // 解析 JSON 数据
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
        if (!jsonDoc.isArray()) {
            qDebug() << "Invalid JSON format";
            return -1;
        }

        QJsonArray conditions = jsonDoc.array();
        compareConditions(conditions);
    } else {
        qDebug() << "No data found";
    }

    return a.exec();
}
```

## 综合数据库（Working Memory）
### table content
- 事实ID `FactID`：唯一标识每个事实
- 事实内容 `FactContent`：具体的事实内容

### 创建表结构
```SQL
CREATE TABLE WorkingMemory (
    FactID INT PRIMARY KEY,
    FactContent NVARCHAR(MAX)
);
```

## 推理机（Inference Engine）
### table content
- 推理步骤ID `stepID`：记录推理过程的每一步
- 使用的规则ID `ruleID`：每一步使用的规则
- 生成的中间结果 `immediateResult`：每一步推理生成的中间结果

### 创建表结构
```SQL
CREATE TABLE InferenceSteps (
    StepID INT PRIMARY KEY,
    RuleID INT,
    IntermediateResult NVARCHAR(MAX),
    FOREIGN KEY (RuleID) REFERENCES RuleBase(RuleID)
);
```

# 要点回顾
## 类的构造
- 类的构造函数的初始化列表中的赋值顺序，应该与该类头文件中，变量属性的声明顺序统一，否则会报警告

## 冲突消解方式
- 使用优先级参数进行冲突的消解。
- 在每一轮的逻辑推理过程中，获取在冲突消解之前的冲突集中的最高优先级参数，然后只保留该优先级参数对应的规则的结论，此时完成本轮的冲突消解，之后将冲突集的结论添加到实时库中，进行再次推理，直到冲突集为空

## Table View 的列宽设置
- $InferenceSteps$、$WorkingMemory$ 的表视图设置为根据可用空间自动调整列宽，使用 `ViewTablePoint->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);`
- $RuleBase$ 的表视图想要实现根据可用空间自动设置列宽，又想同时每一列拥有各自的空间分配比例（每次会获取当前表视图的最新宽度，再进行比例分配）
```C++
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
```

## 数据库实例对象的使用
- 在主程序的 $main$ 接口中创建了数据库实例 `db`
- 对于之后创建的窗口对象，使用参数传递的方式使用 $main$ 接口中的数据库实例 `db`

## 数据库操作与窗口数据映射显示的衔接
- 每个 table view 会对应一个 `QSqlTableModel*` 类型的对象，该对象需要获取数据，才可达成显示的效果。感觉 `QSqlTableModel*` 对象和数据库中的表是一体的，在进行删除信息操作时，针对 `QSqlTableModel*` 对象操作即可实现数据库的同步。
- 在增删数据库信息时，程序窗口中的 table view 会出现清空的现象，猜测和数据源中的信息改动有关，此时可以再次调用相关的函数，针对 `QSqlTableModel*` 对象进行定义或重新获取数据。
- 为何针对 QSqlTableModel 的操作是定义或重新获取数据？主窗口中，3个 table view 的 `QSqlTableModel*` 变量（私有属性）会在主窗口对象创建时自动赋值为 `nullptr`。只有在关联不同表的显示按钮触发时，才会针对 `QSqlTableModel*` 对象开辟空间，从而达到显示数据库相关表的作用，若此前并没有相关按钮的触发，则那些 table view 的 `QSqlTableModel*` 的值依然为 `nullptr`，故有如下的刷新选择模式
```C++
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
```
- 此后涉及到 table view 的刷新操作，即可通过发送 signals，connect 触发 `updateDatabaseDisplay()` 函数即可