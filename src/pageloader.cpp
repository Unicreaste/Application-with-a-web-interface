#include "pageloader.h"

#include <QFileInfo>
#include <QUrl>

PageLoader::PageLoader(QWebEngineView *webView, const QString &htmlPath, QObject *parent)
    : QObject(parent), m_webView(webView), m_htmlPath(QFileInfo(htmlPath).absoluteFilePath())
{
}

void
PageLoader::loadPage()
{
    if (!m_webView) {
        return;
    }

    if (!QFileInfo::exists(m_htmlPath)) {
        m_webView->setHtml("<html><body style='background:#222;color:white'>"
                           "<h2>index.html not found</h2>"
                           "<p>Expected file: " +
                           m_htmlPath.toHtmlEscaped() + "</p></body></html>");
        return;
    }

    m_webView->load(QUrl::fromLocalFile(m_htmlPath));
}
