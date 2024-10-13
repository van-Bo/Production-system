#include "rule.h"

Rule::Rule()
{

}

Rule::Rule(QStringList p, QString q, int ruleID, int priority, int state)
{
    this->p = p, this->q = q;
    this->ruleID = ruleID, this->priority = priority, this->state = state;
}

QStringList Rule::getP()
{
    return p;
}

QString Rule::getQ()
{
    return q;
}

int Rule::getRuleID()
{
    return ruleID;
}

int Rule::getPriority()
{
    return priority;
}

int Rule::getState()
{
    return state;
}

QString Rule::getRule()
{
    QString separator = ",";
    QString rule = "Rule p = [" + p.join(separator) + "]  q = [" + q + "]";
    return rule;
}


