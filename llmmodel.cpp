#include "llmmodel.h"
#include <QDebug>
#include <QDateTime>
LLMModel::LLMModel()
{
    m_timer = new QTimer;
    connect(m_timer, &QTimer::timeout, this, &LLMModel::Update);
    n_remain           = params.n_predict;
}

void LLMModel::Reset()
{
    if (m_timer->isActive())
    {
        m_timer->stop();
        embd.clear();
    }
}

void LLMModel::Update()
{
    static int n_past             = 0;
    static int n_past_guidance    = 0;


    LOG("*********embd_inp: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, embd_inp).c_str());
    // predict
    if (!embd.empty()) {
        // infinite text generation via context swapping
        // if we run out of context:
        // - take the n_keep first tokens from the original prompt (via n_past)
        // - take half of the last (n_ctx - n_keep) tokens and recompute the logits in batches
//            if (n_past + (int) embd.size() + std::max<int>(0, guidance_offset) > n_ctx) {
        if (n_past + (int) embd.size() > 480) {
            if (params.n_predict == -2) {
                LOG_TEE("\n\n%s: context full and n_predict == -%d => stopping\n", __func__, params.n_predict);
            }

            const int n_left    = n_past - params.n_keep - 1;
            const int n_discard = n_left/2;

            LOG("context full, swapping: n_past = %d, n_left = %d, n_ctx = %d, n_keep = %d, n_discard = %d\n",
                n_past, n_left, n_ctx, params.n_keep, n_discard);

            llama_kv_cache_seq_rm   (ctx, 0, params.n_keep + 1            , params.n_keep + n_discard + 1);
            llama_kv_cache_seq_shift(ctx, 0, params.n_keep + 1 + n_discard, n_past, -n_discard);

            n_past -= n_discard;

            LOG("after swap: n_past = %d, n_past_guidance = %d\n", n_past, n_past_guidance);

            LOG("embd: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, embd).c_str());
        }

        for (int i = 0; i < (int) embd.size(); i += params.n_batch) {
            int n_eval = (int) embd.size() - i;
            if (n_eval > params.n_batch) {
                n_eval = params.n_batch;
            }

            LOG("eval: %s, embd:%d, n_past:%d\n", LOG_TOKENS_TOSTR_PRETTY(ctx, embd).c_str(), embd.size(), n_past);
            if (n_eval > 1)
            {
                LOG(" XXXXXXXXX %d\n", embd.size() + n_past);
            }

            if (llama_decode(ctx, llama_batch_get_one(&embd[i], n_eval, n_past, 0))) {
                LOG_TEE("%s : failed to eval\n", __func__);
                break;
            }

            n_past += n_eval;

            LOG("n_past = %d\n", n_past);
        }
    }

    embd.clear();
    const llama_token id = llama_sampling_sample(ctx_sampling, ctx, ctx_guidance);

    llama_sampling_accept(ctx_sampling, ctx, id, true);

    LOG("last: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, ctx_sampling->prev).c_str());

    embd.push_back(id);

    // decrement remaining sampling budget
    --n_remain;
    LOG("n_remain: %d\n", n_remain);

    // display text
    for (auto id : embd) {
        const std::string token_str = llama_token_to_piece(ctx, id);
        QString new_str = QString::fromStdString(token_str);
        m_output << new_str;
//        qDebug() << "xxx"  << new_str << embd.size();
        emit SignalNewAnswer(new_str, false);
    }

    if (m_output.size() >5)
    {
        for (int iii = m_output.size() - 5; iii < m_output.size(); iii++)
        {
            if (m_output.at(iii) != "" && m_output.at(iii) != "\n" )
            {
                break;
            }
            QString new_str = "";
            emit SignalNewAnswer(new_str, true);
            m_timer->stop();
        }
    }

    // end of text token
    if (!embd.empty() && embd.back() == llama_token_eos(model)) {
        LOG_TEE(" [end of text]\n");
        QString new_str = "";
        emit SignalNewAnswer(new_str, true);
        m_timer->stop();
    }
}
LLMModel::~LLMModel()
{
    if (ctx)
    {
        llama_free(ctx);
        llama_free_model(model);

        llama_sampling_free(ctx_sampling);
        llama_backend_free();
    }
    if (m_log_file)
    {
        LOG_TEE("Log end\n");
    }
}

int LLMModel::LoadModel()
{
    params.interactive = true;
    params.chatml = true;
    params.prompt = "You are a helpful assistant.";
    params.model = "ggml-model-q5_k_m.gguf";

    llama_sampling_params & sparams = params.sparams;

    if (m_log_file)
    {
        log_set_target(log_filename_generator("main", "log"));
        LOG_TEE("Log start\n");
    }
    LOG_TEE("%s: build = %d (%s)\n",      __func__, LLAMA_BUILD_NUMBER, LLAMA_COMMIT);
    LOG_TEE("%s: built with %s for %s\n", __func__, LLAMA_COMPILER, LLAMA_BUILD_TARGET);

    if (params.seed == LLAMA_DEFAULT_SEED) {
        params.seed = time(NULL);
    }
    LOG_TEE("%s: seed  = %u\n", __func__, params.seed);
    LOG("%s: llama backend init\n", __func__);
    llama_backend_init(params.numa);
    // load the model and apply lora adapter, if any
    LOG("%s: load the model and apply lora adapter, if any\n", __func__);
    std::tie(model, ctx) = llama_init_from_gpt_params(params);

    if (model == NULL) {
        LOG_TEE("%s: error: unable to load model\n", __func__);
        return -1;
    }

    const int n_ctx = llama_n_ctx(ctx);
    LOG("n_ctx: %d\n", n_ctx);

    // print system information
    {
        LOG_TEE("\n");
        LOG_TEE("%s\n", get_system_info(params).c_str());
    }

    const bool add_bos = llama_should_add_bos_token(model);
    LOG("add_bos: %d\n", add_bos);
    LOG("tokenize the prompt\n");
    if (params.chatml) {
        params.prompt = "<|im_start|>system\n" + params.prompt + "<|im_end|>";
    }
    embd_inp = ::llama_tokenize(ctx, params.prompt, add_bos, true);
    // chatml prefix & suffix
    cml_pfx = ::llama_tokenize(ctx, "\n<|im_start|>user\n", add_bos, true);
    cml_sfx = ::llama_tokenize(ctx, "<|im_end|>\n<|im_start|>assistant\n", false, true);
    LOG("cml_pfx: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, cml_pfx).c_str());
    LOG("cml_sfx: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, cml_sfx).c_str());
    LOG("prompt: \"%s\"\n", log_tostr(params.prompt));
    LOG("tokens: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, embd_inp).c_str());
    // Ensure the input doesn't exceed the context size by truncating embd if necessary.
    if ((int) embd_inp.size() > n_ctx - 100) {
        LOG_TEE("%s: error: prompt is too long (%d tokens, max %d)\n", __func__, (int) embd_inp.size(), n_ctx - 4);
        qDebug() << "Error";
        return -1;
    }

    // number of tokens to keep when resetting context
    if (params.n_keep < 0 || params.n_keep > (int) embd_inp.size() || params.instruct || params.chatml) {
        params.n_keep = (int)embd_inp.size();
    }

    // similar for chatml mode
    if (params.chatml) {
        params.interactive_first = true;
        params.antiprompt.push_back("<|im_start|>user\n");
    }

    // enable interactive mode if interactive start is specified
    if (params.interactive_first) {
        params.interactive = true;
    }

    if (params.verbose_prompt) {
        LOG_TEE("\n");
        LOG_TEE("%s: prompt: '%s'\n", __func__, params.prompt.c_str());
        LOG_TEE("%s: number of tokens in prompt = %zu\n", __func__, embd_inp.size());
        for (int i = 0; i < (int) embd_inp.size(); i++) {
            LOG_TEE("%6d -> '%s'\n", embd_inp[i], llama_token_to_piece(ctx, embd_inp[i]).c_str());
        }

        if (params.n_keep > 0) {
        LOG_TEE("%s: static prompt based on n_keep: '", __func__);
            for (int i = 0; i < params.n_keep; i++) {
                LOG_TEE("%s", llama_token_to_piece(ctx, embd_inp[i]).c_str());
            }
            LOG_TEE("'\n");
        }
        LOG_TEE("\n");
    }

    LOG_TEE("sampling: \n%s\n", llama_sampling_print(sparams).c_str());
    LOG_TEE("generate: n_ctx = %d, n_batch = %d, n_predict = %d, n_keep = %d\n", n_ctx, params.n_batch, params.n_predict, params.n_keep);
    LOG_TEE("\n\n");

    ctx_sampling = llama_sampling_init(sparams);
    return 1;
}
void LLMModel::Run(QString qstr_input)
{
    m_output.clear();
    llama_sampling_reset(ctx_sampling);
    static int chat_round_index = 0;
    m_input = qstr_input.toStdString();
    LOG("waiting for user input\n");
//    qDebug() << chat_round_index << QDateTime::currentDateTime();
    chat_round_index = chat_round_index + 1;

    if (m_input.length() > 1) {
        LOG("buffer: '%s'\n", m_input.c_str());

        // chatml mode: insert user chat prefix
        if (params.chatml) {
            LOG("inserting chatml prefix\n");
            n_consumed = embd_inp.size();
            embd_inp.insert(embd_inp.end(), cml_pfx.begin(), cml_pfx.end());
        }
        if (params.escape) {
            process_escapes(m_input);
        }

        const auto line_inp = ::llama_tokenize(ctx, m_input,              false, false);
        LOG("input tokens: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, line_inp).c_str());
        embd_inp.insert(embd_inp.end(), line_inp.begin(), line_inp.end());

        // chatml mode: insert assistant chat suffix
        if (params.chatml) {
            LOG("inserting chatml suffix\n");
            embd_inp.insert(embd_inp.end(), cml_sfx.begin(), cml_sfx.end());
        }

        n_remain -= line_inp.size();
        LOG("n_remain: %d\n", n_remain);
    }
    embd.clear();
    LOG("embd_inp.size(): %d, n_consumed: %d\n", (int) embd_inp.size(), n_consumed);
    while ((int) embd_inp.size() > n_consumed) {
        embd.push_back(embd_inp[n_consumed]);

        // push the prompt in the sampling context in order to apply repetition penalties later
        // for the prompt, we don't apply grammar rules
        llama_sampling_accept(ctx_sampling, ctx, embd_inp[n_consumed], false);

        ++n_consumed;
        if ((int) embd.size() >= params.n_batch) {
            break;
        }
    }

    m_timer->start(5);
}
