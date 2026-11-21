/*
    SPDX-FileCopyrightText: 2026 KWin Contributors

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QObject>
#include <QVariantList>

class PamAuthenticators;

class RemoteSessionsBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList activeSessions READ activeSessions NOTIFY activeSessionsChanged)
    Q_PROPERTY(bool hasActiveSessions READ hasActiveSessions NOTIFY activeSessionsChanged)
    Q_PROPERTY(bool disconnectOnUnlock READ disconnectOnUnlock WRITE setDisconnectOnUnlock NOTIFY disconnectOnUnlockChanged)

public:
    explicit RemoteSessionsBridge(PamAuthenticators *authenticators, QObject *parent = nullptr);
    ~RemoteSessionsBridge() override = default;

    QVariantList activeSessions() const;
    bool hasActiveSessions() const;
    bool disconnectOnUnlock() const;
    void setDisconnectOnUnlock(bool disconnect);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void requestDisconnect(const QString &sessionId);
    Q_INVOKABLE void requestDisconnectAll();

Q_SIGNALS:
    void activeSessionsChanged();
    void disconnectOnUnlockChanged();

private Q_SLOTS:
    void onActiveSessionsChanged();

private:
    PamAuthenticators *m_authenticators;
    QVariantList m_activeSessions;
    bool m_disconnectOnUnlock = false;
};
