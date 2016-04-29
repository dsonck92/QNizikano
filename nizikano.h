#ifndef NIZIKANO_H
#define NIZIKANO_H

#include <QUrl>
#include <QWidget>
#include <qdir.h>

#include <Audio/Music.hpp>
#include <Audio/Sound.hpp>
#include <Audio/SoundBuffer.hpp>

class QWebEngineProfile;
class QSignalMapper;
class NizikanoPage;

namespace Ui {
class Nizikano;
}

class Nizikano : public QWidget
{
    Q_OBJECT

    QString m_bgm;
    QString m_sfx;

    QPalette m_normal, m_attention;
    bool m_repeatSfx;

    NizikanoPage *m_page;

    QSignalMapper *m_bar;

    sf::Music m_bgmMusic;

    sf::SoundBuffer m_voiceBuff,m_sfxBuff;

    sf::Sound m_voice,m_sfxSound;


    QDir m_root;

public:
    explicit Nizikano(QDir root, QWidget *parent = 0);
    ~Nizikano();

public slots:
    void setButtonToHome(bool state);

    void newBgm(QString name);
    void newSfx(QString name, bool repeat);
    void newVoice(QString data, bool repeat);

    void newAttention(int id, bool state);

private slots:
    void on_pbID_clicked();
    void newNotification(int cate, int time, QString msg, QString ticker);

    void on_pbGo_clicked();

    void newUrl(QUrl url);

private:
    Ui::Nizikano *ui;
};

#endif // NIZIKANO_H
