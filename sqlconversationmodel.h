// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SQLCONVERSATIONMODEL_H
#define SQLCONVERSATIONMODEL_H

#include <QSqlTableModel>
#include <QTimer>
#include "humanassets.h"
#include <QQmlEngine>
class SqlConversationModel : public QSqlTableModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SqlConversationModel)
    Q_PROPERTY(QString recipient READ recipient WRITE setRecipient NOTIFY recipientChanged)

public:
    SqlConversationModel(QObject *parent = nullptr);

    QString recipient() const;
    void setRecipient(const QString &recipient);

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void sendMessage(const QString &recipient, const QString &message);
    Q_INVOKABLE void removeAllMessage();

    HumanAssets* m_human_assets;

signals:
    void recipientChanged();
public slots:
    void updateMsg();
    void SlotNewAnswer(QString str, bool end = false);
private:
    QString m_recipient;

    int m_current_msg_length;
    QString m_message;
    QString m_message_receive;
    QString m_timestamp;

    int sub_text_index = 0;
    int m_chat_round;

    bool m_chatEnded = true;
};

#endif // SQLCONVERSATIONMODEL_H
