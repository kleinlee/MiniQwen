#include "humanassets.h"

HumanAssets* HumanAssets::instance_ = NULL;
HumanAssets::HumanAssets()
{
//    instance = NULL;
    m_chat_model = new LLMModel;
    m_chat_model->LoadModel();
//    connect(ui->pushButton_stop, SIGNAL(clicked()), m_chat_model, SLOT(Reset()));
}

void HumanAssets::ChatUpdated()
{
    emit ChatUpdatedSignal();
}

void HumanAssets::SlotStopChat()
{
    m_chat_model->Reset();
}
void HumanAssets::SlotNewChat(QString question)
{
    m_chat_model->Run(question);

}

void HumanAssets::SlotNewAnswer(QString str)
{

}
