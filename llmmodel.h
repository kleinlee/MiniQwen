#ifndef LLMMODEL_H
#define LLMMODEL_H

#include "common.h"
#include "llama.h"
#include <QString>
#include <QObject>
#include <QTimer>
class LLMModel : public QObject
{
    Q_OBJECT
public:
    LLMModel();
    ~LLMModel();

    int LoadModel();
    void Run(QString qstr_input);

    gpt_params params;
    // 输入的token
    std::vector<llama_token> embd_inp;
    std::vector<llama_token> embd;
    std::vector<llama_token> cml_pfx;
    std::vector<llama_token> cml_sfx;

    struct llama_sampling_context * ctx_sampling;
    int n_ctx;

    int n_remain           = 0;
    int n_consumed         = 0;

    llama_model * model;
    llama_context * ctx = NULL;
    llama_context * ctx_guidance = NULL;
    bool is_interacting = false;
    bool m_log_file = true;

    std::string m_input;
    QTimer* m_timer;

    QStringList m_output;
signals:
    void SignalNewAnswer(QString str, bool end = false);
public slots:
    void Update();
    void Reset();
};

#endif // LLMMODEL_H
