/*
    SPDX-FileCopyrightText: 2026 KWin Contributors

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "remotesessionsbridge.h"
#include "pamauthenticators.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QVariantMap>

static constexpr QLatin1StringView KWIN_SERVICE("org.kde.KWin");
static constexpr QLatin1StringView REMOTE_SESSIONS_PATH("/RemoteSessions");
static constexpr QLatin1StringView REMOTE_SESSIONS_IFACE("org.kde.KWin.RemoteSessions");

RemoteSessionsBridge::RemoteSessionsBridge(PamAuthenticators *authenticators, QObject *parent)
    : QObject(parent)
    , m_authenticators(authenticators)
{
    QDBusConnection::sessionBus()
        .connect(KWIN_SERVICE, REMOTE_SESSIONS_PATH, REMOTE_SESSIONS_IFACE, QStringLiteral("activeSessionsChanged"), this, SLOT(onActiveSessionsChanged()));
    refresh();
}

QVariantList RemoteSessionsBridge::activeSessions() const
{
    return m_activeSessions;
}

bool RemoteSessionsBridge::hasActiveSessions() const
{
    return !m_activeSessions.isEmpty();
}

bool RemoteSessionsBridge::disconnectOnUnlock() const
{
    return m_disconnectOnUnlock;
}

void RemoteSessionsBridge::setDisconnectOnUnlock(bool disconnect)
{
    if (m_disconnectOnUnlock != disconnect) {
        m_disconnectOnUnlock = disconnect;
        Q_EMIT disconnectOnUnlockChanged();
    }
}

void RemoteSessionsBridge::onActiveSessionsChanged()
{
    refresh();
}

void RemoteSessionsBridge::refresh()
{
    QDBusInterface iface(KWIN_SERVICE, REMOTE_SESSIONS_PATH, REMOTE_SESSIONS_IFACE, QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        if (!m_activeSessions.isEmpty()) {
            m_activeSessions.clear();
            Q_EMIT activeSessionsChanged();
        }
        return;
    }

    const QVariantList raw = iface.property("activeSessions").toList();
    QVariantList next;
    next.reserve(raw.size());
    for (const QVariant &entry : raw) {
        if (entry.canConvert<QDBusArgument>()) {
            next.append(qdbus_cast<QVariantMap>(entry.value<QDBusArgument>()));
        } else {
            next.append(entry.toMap());
        }
    }
    if (next != m_activeSessions) {
        m_activeSessions = next;
        Q_EMIT activeSessionsChanged();
    }
}

void RemoteSessionsBridge::requestDisconnect(const QString &sessionId)
{
    if (!m_authenticators || !m_authenticators->isUnlocked()) {
        return;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(KWIN_SERVICE, REMOTE_SESSIONS_PATH, REMOTE_SESSIONS_IFACE, QStringLiteral("requestDisconnect"));
    msg << sessionId;
    QDBusConnection::sessionBus().asyncCall(msg);
}

void RemoteSessionsBridge::requestDisconnectAll()
{
    if (!m_authenticators || !m_authenticators->isUnlocked()) {
        return;
    }

    for (const QVariant &entry : std::as_const(m_activeSessions)) {
        const QString sid = entry.toMap().value(QStringLiteral("sessionId")).toString();
        if (!sid.isEmpty()) {
            requestDisconnect(sid);
        }
    }
}

#include "moc_remotesessionsbridge.cpp"
