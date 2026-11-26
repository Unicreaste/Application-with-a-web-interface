#include "theme_manager.h"
#include <QDebug>

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent), m_settings(QStringLiteral("Belousov"), QStringLiteral("Belousov"))
{
}

QString
ThemeManager::currentTheme() const
{
    return m_settings.value(QStringLiteral("appearance/theme"), QStringLiteral("dark")).toString();
}

void
ThemeManager::setTheme(const QString &theme)
{
    if (theme != QStringLiteral("dark") && theme != QStringLiteral("light")) {
        return;
    }

    m_settings.setValue(QStringLiteral("appearance/theme"), theme);
    m_settings.sync();

    qDebug() << "Тема сохранена:" << theme;
    qDebug() << "Статус записи:" << m_settings.status();
    qDebug() << "Прочитанная тема:" << m_settings.value(QStringLiteral("appearance/theme")).toString();

    emit themeChanged(theme);
}
