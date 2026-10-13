#pragma once

#include <QObject>
#include <QWebEngineView>
#include <QString>

class PageLoader : public QObject
{

  public:
    explicit PageLoader(QWebEngineView *webView, const QString &htmlPath, QObject *parent = nullptr);

    void loadPage();

  private:
    QWebEngineView *m_webView = nullptr;
    QString m_htmlPath;
};
