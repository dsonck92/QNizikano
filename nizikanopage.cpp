#include "nizikanopage.h"

#include <QDebug>
#include <QMouseEvent>
#include <QNetworkRequest>
#include <QTimerEvent>
#include <QWebFrame>
#include <QWebElement>
#include <QNetworkReply>
#include <QBuffer>
#include <quazipfile.h>
#include <QMessageBox>
#include <QDesktopServices>

#include "quazip.h"

void NizikanoPage::navTo(QString path)
{
    connect(mainFrame(),SIGNAL(urlChanged(QUrl)),SLOT(newUrl(QUrl)));
    QUrl url("http://us-nizi2d-app.amz-aws.jp/");
    url.setPath(path);
    url.addQueryItem("P",m_P);

    mainFrame()->load(url);
}

QString NizikanoPage::userAgentForUrl(const QUrl &url) const
{
    Q_UNUSED(url)

    //return QWebPage::userAgentForUrl(url);

    return QString("AMBITION_UA/GooglePlay/Android-X/NIZIKANO_2D_GLB_ver05:%1:%2").arg(m_id,m_app);
}

void NizikanoPage::fetchAsset()
{
    while(m_assetIterator.value() == true && m_assetIterator != m_assets.end())
        m_assetIterator++;

    if(m_assetIterator != m_assets.end()) {
        QUrl url = m_downloadServer;
        url.setPath("/resource/app_get_resouce.php");
        url.addQueryItem("key1",m_assetIterator.key());

        m_zipDLs << networkAccessManager()->get(QNetworkRequest(url));

        m_assetIterator.value() = true;
    }
}

NizikanoPage::NizikanoPage(QUrl downloadServer, QDir root, QString id, QString app, QObject *parent)
    :QWebPage(parent)
    ,m_id(id)
    ,m_app(app)
    ,m_root(root)
    ,m_timeout(1)
    ,m_downloadServer(downloadServer)
{
    connect(mainFrame(),SIGNAL(urlChanged(QUrl)),SLOT(newUrl(QUrl)));
    mainFrame()->load(QUrl("http://us-nizi2d-app.amz-aws.jp/"));
    settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,true);

    connect(networkAccessManager(),SIGNAL(finished(QNetworkReply*)),SLOT(handleReply(QNetworkReply*)));

    m_requests.insert(networkAccessManager()->get(QNetworkRequest(QUrl("http://us-nizi2d-app.amz-aws.jp/app_dl_list.php?id=0"))),DownloadList);


}

bool NizikanoPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, QWebPage::NavigationType type)
{
    Q_UNUSED(frame)

    QUrl url = request.url();

    QString p = url.queryItemValue("P");

    if(!p.isEmpty()) {
        m_P = p;
    }

    if(url.scheme().compare("http",Qt::CaseInsensitive) == 0) {
        if(url.path().compare("/live2d_switch_log.php",Qt::CaseInsensitive) == 0) {
            /* Fix live2d which isn't supported */
            url.removeQueryItem("flg");
            url.addQueryItem("flg","0");
        }
        return true;
    } else if(url.scheme().compare("live2d",Qt::CaseInsensitive) == 0) {
        return false;
    } else if(url.scheme().compare("webview",Qt::CaseInsensitive) == 0) {
        QString host = url.host();

        if(host == "javascript_changefooterimghomechar")
        {
            emit setButtonToHome(url.queryItemValue("flg").compare("true") == 0);
            return false; /* Silly feature, change home/gf button */
        }
        if(host == "javascript_setfootereventflg")
        {
            emit setFooterEventFlag(url.queryItemValue("id").toInt(),url.queryItemValue("flg").compare("true") == 0);
            return false; /* Not possible */
        }
        if(host == "javascript_footerenable")
        {
            emit setFooterEnabled(url.queryItemValue("flg").compare("true") == 0);
            return false; /* Enable the bottom bar */
        }
        if(host == "javascript_playbgm")
        {
            emit playBGM(url.queryItemValue("name"));
            return false; /* Enable BGM playback handling */
        }
        if(host == "javascript_playse")
        {
            emit playSE(url.queryItemValue("name"),url.queryItemValue("repeat").compare("true") == 0);
            return false; /* Enable SFX playback handling */
        }
        if(host == "javascript_playvoice") {
            emit playVoice(url.queryItemValue("data"),url.queryItemValue("repeat").compare("true") == 0);
            return false;
        }
        if(host == "javascript_pushinfo") {
            emit pushInfo(url.queryItemValue("cate").toInt(),
                          url.queryItemValue("time").toInt(),
                          url.queryItemValue("text"),
                          url.queryItemValue("ticker"));
            return false;
        }
        if(host == "javascript_imgsave") {
            QUrl pictureUrl = url.queryItemValue("img_data");
            m_screenshot = networkAccessManager()->get(QNetworkRequest(pictureUrl));
            return false;
        }
        qDebug() << "ToDo:" << url.host() << "Q:" << url.encodedQuery();
        return false;
    } else if(url.scheme().compare("iab-inapp",Qt::CaseInsensitive) == 0) {
        QMessageBox::information(0,"Not Available", "This function is not available on this platform");
        return false;
    } else if(url.scheme().compare("browser",Qt::CaseInsensitive) == 0) {
        QUrl target = url.toString().mid(8);
        QDesktopServices::openUrl(target);
        return false;
    }
    if(url.toString() == "top:0") {
        mainFrame()->load(QUrl("http://us-nizi2d-app.amz-aws.jp/"));
        return false;
    }

    qDebug() << "NavRequest:" << url << "(" << type << ")";

    return false;
}

void NizikanoPage::timerEvent(QTimerEvent *ev)
{
    if(ev->timerId() == m_timer.timerId()) {
        triggerAction(Reload);
    }
}

bool NizikanoPage::event(QEvent *ev)
{
    return QWebPage::event(ev);
}

void NizikanoPage::newUrl(QUrl url)
{
    if(m_refresh.contains(url.path())) {
        if(!m_timer.isActive()) {
            m_timer.start(m_timeout*60000,this);

            emit autoRefreshChanged(true);
        }
    } else {
        if(m_timer.isActive()) {
            m_timer.stop();

            emit autoRefreshChanged(false);
        }
    }
}

void NizikanoPage::handleReply(QNetworkReply *rply)
{
    switch(m_requests.value(rply,NoRequest)) {
    case DownloadList:
    {
        rply->open(QIODevice::ReadOnly);
        QTextStream str(rply);

        while(!str.atEnd()) {
            QString line = str.readLine();

            if(line.isEmpty())
                continue;

            QStringList args = line.split('\t');

            QString file, key;
            int version;

            if(args.count() > 0) {
                file = args.takeFirst();
            }
            if(args.count() > 0) {
                key = args.takeFirst();
            }
            if(args.count() > 0) {
                version = args.takeFirst().toInt();
            }

            QDir asset = m_root;
            asset.cd("assets");

            if(file.startsWith("bgm_")) {

            } else {
                file.prepend("voice_");
            }

            m_assets.insert(key,m_assets.value(key,true) && asset.exists(QString("%1.ogg").arg(file)));

        }

        foreach(QString key, m_assets.keys()) {
            if(m_assets.value(key) == false)
                qDebug() << "Need fetch:" << key;

        }

        m_assetIterator = m_assets.begin();

        fetchAsset();
    }
        break;
    default:
        if(m_zipDLs.contains(rply)) {
            m_zipDLs.removeOne(rply);

            {
                QByteArray d = rply->readAll();
                qDebug() << "Open:" << rply->url();
                QBuffer b(&d);

                QuaZip zip(&b);

                if(zip.open(QuaZip::mdUnzip)) {

                    QuaZipFile file(&zip);
                    for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile()) {
                        QString fn = zip.getCurrentFileName();
                        QDir asset = m_root;
                        asset.cd("assets");
                        if(!asset.exists(fn)) {
                            qDebug() << "Creating file:" << asset.absoluteFilePath(fn);
                            QFile out(asset.absoluteFilePath(fn));
                            out.open(QIODevice::WriteOnly);
                            file.open(QIODevice::ReadOnly);
                            {
                                QDebug dbg = qDebug();
                                while(!file.atEnd()) {
                                    out.write(file.read(1024));
                                    dbg << ".";
                                }
                            }
                            file.close();
                            out.close();
                        }
                    }
                    zip.close();
                }
            }

            fetchAsset();
        }
        else if(m_screenshot == rply) {
            QDir screenshot = m_root;

            if(!screenshot.exists("screenshots")) {
                screenshot.mkdir("screenshots");
            }

            screenshot.cd("screenshots");

            QString file = screenshot.absoluteFilePath(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.jpg"));
            QFile out(file);

            rply->open(QIODevice::ReadOnly);
            out.open(QIODevice::WriteOnly);

            while(!rply->atEnd()) {
                out.write(rply->read(1024));
            }
            out.close();
            rply->close();

            QMessageBox::information(0,"Screenshot saved", QString("Written to: '%1'").arg(file));
        } else {
            //qDebug() << "Unhandled DL:" << rply->url();
        }
        break;
    }
}

void NizikanoPage::footerFunc(int id)
{
    mainFrame()->evaluateJavaScript(QString("footerFunc(%1)").arg(id));
}

void NizikanoPage::setID(QString id)
{
    m_id = id;
    triggerAction(Reload);
}

void NizikanoPage::setTimeout(int minutes)
{
    m_timeout = minutes;

    if(m_refresh.contains(mainFrame()->url().path())) {
        if(!m_timer.isActive()) {
            m_timer.start(m_timeout*60000,this);

            emit autoRefreshChanged(true);
        }
    } else {
        if(m_timer.isActive()) {
            m_timer.stop();
            emit autoRefreshChanged(false);
        }
    }
}

void NizikanoPage::setAutoRefresh(bool enable)
{
    if(enable) {
        if(!m_refresh.contains(mainFrame()->url().path())) {
            m_refresh.append(mainFrame()->url().path());
            m_timer.start(m_timeout*60000,this);
        }
    } else {
        if(m_refresh.contains(mainFrame()->url().path())) {
            m_refresh.removeAll(mainFrame()->url().path());
            m_timer.stop();
        }
    }
}
