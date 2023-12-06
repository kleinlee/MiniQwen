#ifndef HUMANASSETS_H
#define HUMANASSETS_H
#include <QString>
#include <QTimer>
#include <QObject>
#include <QDateTime>
#include "llmmodel.h"
class HumanAssets : public QObject
{
    Q_OBJECT
private:
    HumanAssets();    //构造函数是私有的，这样就不能在其它地方创建该实例

    static HumanAssets *instance_;  //定义一个唯一指向实例的静态指针，并且是私有的。
public:
    static HumanAssets* GetInstance()   //定义一个公有函数，可以获取这个唯一的实例，并且在需要的时候创建该实例。
    {
        if(instance_ == NULL)  //判断是否第一次调用
            instance_ = new HumanAssets();
        return instance_;
    }

    void ChatUpdated();
Q_SIGNALS:
    void ChatUpdatedSignal();
public:
    QString m_text;
    QString m_task_id;
    QList<float> m_blendshape;
    QDateTime m_start_time_ms;

    LLMModel* m_chat_model;

public slots:
    void SlotNewChat(QString question);
    void SlotStopChat();
    void SlotNewAnswer(QString str);
};

#endif // HUMANASSETS_H
