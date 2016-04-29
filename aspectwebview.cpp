#include "aspectwebview.h"

#include <QResizeEvent>
#include <QMouseEvent>

#include <QDebug>

#include <QTest>
#include <qtesttouch.h>

AspectWebView::AspectWebView(QWidget *parent)
    :QWebView(parent)
{
}

void AspectWebView::resizeEvent(QResizeEvent *ev)
{
    qreal w_optimal = ev->size().height()/1.334;

    setMaximumWidth(w_optimal);

    if(ev->size().height() == ev->oldSize().height()) {
        /* We changed ourselves, this is 2nd resize */
        reload();
        QWebView::resizeEvent(ev);
    } else if(ev->size().width() != ev->oldSize().width()) {
        /* A complete resize was done, let's notify the webview */
        QWebView::resizeEvent(ev);
    }
}

bool AspectWebView::event(QEvent *ev)
{
//    qDebug() << "Event:" << ev->type();

    if(ev->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mev = static_cast<QMouseEvent*>(ev);

        /* Emulate TouchScreen for the webapp */
        if(mev->button() == Qt::LeftButton)
        {
            QTest::touchEvent(this).press(0,mev->pos());
        }

        /* First set the event data */
        /*        mainFrame()->evaluateJavaScript(QString("window.event = "
                                                "{ "
                                                "  changedTouches:"
                                                "  ["
                                                "    {"
                                                "      pageX:%1,"
                                                "      pageY:%2"
                                                "    }"
                                                "  ]"
                                                "};"));

        QString p = mainFrame()->url().path();

        if(p.compare("/")
        */
        /* Then determine the page and trigger the right event */
        //        if(                                        "$('.avaFrame').trigger('touchstart');").arg(mev->pos().x()).arg(mev->pos().y()));
    }
    else if(ev->type() == QEvent::MouseMove)
    {
        QMouseEvent *mev = static_cast<QMouseEvent*>(ev);

        /* Emulate TouchScreen for the webapp */
        if(mev->button() == Qt::LeftButton) {
            QTest::touchEvent(this).move(0,mev->pos());
        }
    }
    else if(ev->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *mev = static_cast<QMouseEvent*>(ev);

        /* Emulate TouchScreen for the webapp */
        if(mev->button() == Qt::LeftButton) {
            QTest::touchEvent(this).release(0,mev->pos());
        }
    }

    return QWebView::event(ev);
}

