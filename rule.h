#ifndef RULE_H
#define RULE_H

#include <list>
#include <QString>
#include <QStringList>

class Rule
{
private:
    QStringList p;
    QString q;
    int ruleID;
    int priority;
    int state;
public:
    Rule();
    Rule(QStringList p, QString q, int ruleID, int priority, int state);

    QStringList getP();
    QString getQ();
    int getRuleID();
    int getPriority();
    int getState();
    QString getRule();
};

#endif // RULE_H
