#include "nizikano.h"
#include "ui_nizikano.h"

#include <QInputDialog>
#include <QSpinBox>
#include <QMessageBox>
#include <QSettings>
#include <QSignalMapper>
#include "nizikanopage.h"
#include <Audio/SoundBuffer.hpp>
#include <QDebug>
#include <QWebFrame>

QString getRandomString(int len) {
    const QString chars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/+");

    QString rand;
    for(int i=0;i<len;i++) {
        int index = qrand() % chars.length();
        QChar nextChar = chars.at(index);
        rand.append(nextChar);
    }
    return rand;
}

Nizikano::Nizikano(QDir root, QWidget *parent) :
    QWidget(parent),
    m_repeatSfx(false),
    m_root(root),
    ui(new Ui::Nizikano)
{
    ui->setupUi(this);

    QSettings s;

    m_normal = m_attention = palette();

    QColor original = m_normal.color(QPalette::Button);
    QColor blend = QColor::fromHsvF(original.hueF()*.5,original.saturationF()*.5+.5,original.valueF()*.5+.5);

    m_attention.setBrush(QPalette::Button,blend);
    //m_attention.setBrush(QPalette::ButtonText,Qt::white);

    if(!s.contains("ID")) {
        QString rid = getRandomString(16*4);
        s.setValue("ID",QInputDialog::getText(this,"ID","Your device ID or a random string",QLineEdit::Normal,rid));
    }
    if(!s.contains("App")) {
        // "H5r7 Y7bh aIRW T7K+ cbiw jZY6 IW0X maHp 0Vut FEQS byk="
        s.setValue("App",QInputDialog::getText(this,"App","Your application key",QLineEdit::Normal,"H5r7Y7bhaIRWT7K+cbiwjZY6IW0XmaHp0VutFEQSbyk="));
    }
    QString m_app = s.value("App").toString();
    QString m_id = s.value("ID").toString();

    m_page = new NizikanoPage(QUrl("http://us-nizi2d-dl53.amz-aws.jp/"),root,m_id,m_app,this);

    ui->webview->setPage(m_page);

    connect(ui->pbTop,SIGNAL(clicked(bool)),m_page,SLOT(showTop()));

    m_bar = new QSignalMapper(this);

    m_bar->setMapping(ui->pbGacha,1);
    m_bar->setMapping(ui->pbEvents,2);
    m_bar->setMapping(ui->pbHome,3);
    m_bar->setMapping(ui->pbShops,4);
    m_bar->setMapping(ui->pbMenu,5);

    connect(ui->pbGacha,SIGNAL(clicked(bool)),m_bar,SLOT(map()));
    connect(ui->pbEvents,SIGNAL(clicked(bool)),m_bar,SLOT(map()));
    connect(ui->pbHome,SIGNAL(clicked(bool)),m_bar,SLOT(map()));
    connect(ui->pbShops,SIGNAL(clicked(bool)),m_bar,SLOT(map()));
    connect(ui->pbMenu,SIGNAL(clicked(bool)),m_bar,SLOT(map()));

    connect(m_bar,SIGNAL(mapped(int)),m_page,SLOT(footerFunc(int)));
    /*    connect(ui->pbGacha,SIGNAL(clicked(bool)),m_page,SLOT(showGacha()));
    connect(ui->pbShops,SIGNAL(clicked(bool)),m_page,SLOT(showShop()));
    connect(ui->pbEvents,SIGNAL(clicked(bool)),m_page,SLOT(showEvent()));
    connect(ui->pbHome,SIGNAL(clicked(bool)),m_page,SLOT(showRoom()));
    connect(ui->pbGirl,SIGNAL(clicked(bool)),m_page,SLOT(showCharacter()));*/

    connect(ui->pbItem,SIGNAL(clicked(bool)),m_page,SLOT(showItem()));
    connect(ui->pbGroup,SIGNAL(clicked(bool)),m_page,SLOT(showGroup()));
    connect(ui->pbDegree,SIGNAL(clicked(bool)),m_page,SLOT(showDegree()));
    connect(ui->pbAlbum,SIGNAL(clicked(bool)),m_page,SLOT(showAlbum()));
    connect(ui->pbFriend,SIGNAL(clicked(bool)),m_page,SLOT(showFriend()));
    connect(ui->pbRandom,SIGNAL(clicked(bool)),m_page,SLOT(showRandom()));
    connect(ui->pbBook,SIGNAL(clicked(bool)),m_page,SLOT(showBook()));
    connect(ui->pbCampain,SIGNAL(clicked(bool)),m_page,SLOT(showCampain()));
    connect(ui->pbStatus,SIGNAL(clicked(bool)),m_page,SLOT(showStatus()));
    connect(ui->pbOption,SIGNAL(clicked(bool)),m_page,SLOT(showOption()));
    connect(ui->pbHelp,SIGNAL(clicked(bool)),m_page,SLOT(showHelp()));

    connect(ui->pbBack,SIGNAL(clicked(bool)),m_page,SLOT(showPrevious()));

    connect(ui->sbTimeout,SIGNAL(valueChanged(int)),m_page,SLOT(setTimeout(int)));
    connect(ui->cbRefresh,SIGNAL(toggled(bool)),m_page,SLOT(setAutoRefresh(bool)));
    connect(m_page,SIGNAL(autoRefreshChanged(bool)),ui->cbRefresh,SLOT(setChecked(bool)));

    connect(m_page,SIGNAL(setFooterEnabled(bool)),ui->wFooter,SLOT(setEnabled(bool)));

    connect(m_page,SIGNAL(setButtonToHome(bool)),SLOT(setButtonToHome(bool)));

    connect(m_page,SIGNAL(playBGM(QString)),SLOT(newBgm(QString)));
    connect(m_page,SIGNAL(playVoice(QString,bool)),SLOT(newVoice(QString,bool)));
    connect(m_page,SIGNAL(playSE(QString,bool)),SLOT(newSfx(QString,bool)));

    connect(m_page,SIGNAL(setFooterEventFlag(int,bool)),SLOT(newAttention(int,bool)));

    connect(m_page,SIGNAL(pushInfo(int,int,QString,QString)),SLOT(newNotification(int,int,QString,QString)));

    connect(m_page->mainFrame(),SIGNAL(urlChanged(QUrl)),SLOT(newUrl(QUrl)));
}

Nizikano::~Nizikano()
{
    delete ui;
}

void Nizikano::setButtonToHome(bool state)
{
    if(state) {
        ui->pbHome->setText("Room");
    } else {
        ui->pbHome->setText("Girlfriend");
    }
}

void Nizikano::newBgm(QString name)
{
    if(m_bgm != name) {
        m_bgm = name;

        ui->lDebug->setText(QString("Debug:\nBGM: %1\nSFX: %2%3").arg(m_bgm,m_sfx,m_repeatSfx?"R":""));

        QDir asset = m_root;
        asset.cd("assets");

        if(m_bgmMusic.openFromFile(asset.absoluteFilePath(m_bgm).toStdString())) {
            m_bgmMusic.setLoop(true);
            m_bgmMusic.play();
        }
    }
}

void Nizikano::newSfx(QString name, bool repeat)
{
    m_sfx = name;
    m_repeatSfx = repeat;

    ui->lDebug->setText(QString("Debug:\nBGM: %1\nSFX: %2%3").arg(m_bgm,m_sfx,m_repeatSfx?":R":":N"));

    QDir asset = m_root;
    asset.cd("assets");

    if(m_sfxSound.getStatus() == sf::Sound::Playing) {
        m_sfxSound.stop();
    }

    if(m_sfxBuff.loadFromFile(asset.absoluteFilePath(name).toStdString())) {
        m_sfxSound.setBuffer(m_sfxBuff);
        m_sfxSound.setLoop(m_repeatSfx);
        m_sfxSound.play();
    } else {
        qDebug() << "Could not open file!";
    }
}

void Nizikano::newVoice(QString data, bool repeat)
{
    QStringList arg = data.split(":");

    QDir asset = m_root;
    asset.cd("assets");

    if(m_voice.getStatus() == sf::Sound::Playing) {
        m_voice.stop();
    }

    if(m_voiceBuff.loadFromFile(asset.absoluteFilePath(arg[0]).toStdString())) {
        m_voice.setBuffer(m_voiceBuff);
        m_voice.setLoop(repeat);
        m_voice.play();
    } else {
        qDebug() << "Could not open file!";
    }
}

void Nizikano::newAttention(int id, bool state)
{
    QWidget * w = static_cast<QWidget*>(m_bar->mapping(id));
    if(w)
        w->setPalette(state?m_attention:m_normal);
}

void Nizikano::on_pbID_clicked()
{
    QSettings s;

    QString id = s.value("ID").toString();

    bool ok;
    id = QInputDialog::getText(this,"New ID","Please specify a new ID (or empty for random)",QLineEdit::Normal,id,&ok);

    if(ok) {
        if(id.isEmpty())
            id = getRandomString(16*4);

        m_page->setID(id);

        s.setValue("ID",id);
    }
}

void Nizikano::newNotification(int cate, int time, QString msg, QString ticker)
{
    Q_UNUSED(cate)
    Q_UNUSED(time)

    ui->gbTicker->setTitle(ticker);
    ui->lNotification->setText(msg);
}

void Nizikano::on_pbGo_clicked()
{
    m_page->mainFrame()->load(QUrl(ui->leURL->text()));
}

void Nizikano::newUrl(QUrl url)
{
    ui->leURL->setText(url.toString());
}
