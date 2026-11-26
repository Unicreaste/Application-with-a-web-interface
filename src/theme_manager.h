
#pragma once

#include <QObject>
#include <QString>
#include <QSettings>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    explicit ThemeManager(QObject *parent = nullptr);

    Q_INVOKABLE QString currentTheme() const;
    Q_INVOKABLE void setTheme(const QString &theme);

signals:
    void themeChanged(const QString &theme);

private:
    QSettings m_settings;
};
