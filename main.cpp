#include "nizikano.h"
#include <QApplication>
#include <Qt>
#include <qdir.h>
#include "time.h"

int main(int argc, char *argv[])
{
    qsrand(time(NULL));

    QApplication a(argc, argv);

    a.setStyle("breeze");
//    a.setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents);

    a.setApplicationName("QNizikano");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("DanielSonckSolutions");
    a.setOrganizationDomain("danielsoncksolutions.nl");

    QDir rootDir;

    if(!rootDir.exists("assets")) {
        rootDir.mkdir("assets");
    }

    Nizikano w(rootDir);
    w.show();

    return a.exec();
}
