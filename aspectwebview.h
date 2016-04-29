#ifndef ASPECTWEBVIEW_H
#define ASPECTWEBVIEW_H

#include <QWebView>

class QResizeEvent;
class QMouseEvent;

class AspectWebView : public QWebView
{
public:
    AspectWebView(QWidget *parent = 0);

    void resizeEvent(QResizeEvent *ev);

    bool event(QEvent *ev);

};

#endif // ASPECTWEBVIEW_H
