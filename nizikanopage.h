#ifndef NIZIKANOPAGE_H
#define NIZIKANOPAGE_H

#include <QBasicTimer>
#include <QMap>
#include <qdir.h>
#include <QtWebKit/QWebPage>

class QNetworkReply;

class NizikanoPage : public QWebPage
{
    Q_OBJECT

    enum Request {
        NoRequest = 0,
        DownloadList,
        MaxRequestType
    };

    QString m_P;

    QString m_id,m_app;

    QDir m_root;

    int m_timeout;
    QBasicTimer m_timer;

    QStringList m_refresh;

    QUrl m_downloadServer;

    QMap<QNetworkReply*,Request> m_requests;
    QMap<QString,bool> m_assets;
    QMap<QString,bool>::iterator m_assetIterator;

    QList<QNetworkReply*> m_zipDLs;

    QNetworkReply *m_screenshot;

    void navTo(QString path);

    QString userAgentForUrl(const QUrl &url) const;

    void fetchAsset();
public:
    NizikanoPage(QUrl downloadServer, QDir root, QString id, QString app, QObject* parent = 0);

    virtual bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type);
    /*virtual bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);*/

    void timerEvent(QTimerEvent *ev);
    bool event(QEvent *ev);

private slots:
    void newUrl(QUrl url);
    void handleReply(QNetworkReply* rply);

signals:
    void autoRefreshChanged(bool state);

    void setFooterEnabled(bool state);
    void setButtonToHome(bool state);
    void setFooterEventFlag(int id, bool state);

    void playBGM(QString name);
    void playSE(QString name, bool repeat);
    void playVoice(QString data, bool repeat);
    void pushInfo(int cate, int time, QString text, QString ticker);

public slots:
    void showTop(){ navTo(""); }

    void showGacha(){ navTo(("/gacha/index.php")); }
    void showShop(){ navTo(("/shop/index.php")); }
    void showEvent(){ navTo(("/event/index.php")); }
    void showRoom(){ navTo(("/room/index.php")); }
    void showCharacter(){ navTo(("/char/index.php")); }

    void showItem(){ navTo("/item/index.php"); }
    void showGroup(){ navTo("/group/index.php"); }
    void showDegree(){ navTo("/degree/index.php"); }
    void showAlbum(){ navTo("/album/index.php"); }
    void showFriend(){ navTo("/friend/list.php"); }
    void showRandom(){ navTo("/random/index.php"); }
    void showBook(){ navTo("/book/index.php"); }
    void showCampain(){ navTo("/room/campain.php"); }
    void showStatus(){ navTo("/user/status.php"); }
    void showOption(){ navTo("/option/index.php"); }
    void showHelp(){ navTo("/help/index.php"); }

    void showPrevious(){ triggerAction(Back); }

    void footerFunc(int id);

    void setID(QString id);

public slots:
    void setTimeout(int minutes);
    void setAutoRefresh(bool enable);
};

#endif // NIZIKANOPAGE_H
