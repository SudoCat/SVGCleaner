#include <QDesktopServices>
#include <QMouseEvent>
#include <QUrl>
#include <QBitmap>
#include <QSvgRenderer>
#include <QTimer>
#include <QCursor>
#include <QPainter>
#include <QtDebug>

#include "iconswidget.h"

IconsWidget::IconsWidget(QWidget *parent) :
    QWidget(parent)
{
    toolTip = new QLabel();
    toolTip->setWindowFlags(Qt::SplashScreen);
    setMouseTracking(true);
}

void IconsWidget::setPaths(const QString &pathIn,const QString &pathOut,const bool compare)
{
    qDebug()<<Q_FUNC_INFO;
    inpath = pathIn;
    outpath = pathOut;
    compareView = compare;
    newToolTip = true;
    if (toolTip->isVisible())
        makeToolTip();

    if (compare) {
        setFixedWidth(height()*2+20);
    } else {
        setFixedWidth(height()+20);
    }
    crashed = false;
    repaint();
}

void IconsWidget::makeToolTip()
{
    qDebug()<<Q_FUNC_INFO;
    QImage image(620,310,QImage::Format_ARGB32);
    image.fill(0);
    mainPix = QPixmap::fromImage(image,Qt::NoOpaqueDetection|Qt::AutoColor);
    QPainter painter(&mainPix);
    QPalette pal;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setBrush(QBrush(pal.color(QPalette::Background)));
    painter.setPen(QPen(QColor("#636363"),2));
    painter.drawRoundedRect(2,2,image.width()-4,image.height()-4,5,5);

    QSvgRenderer renderer(inpath);
    QSize size = renderer.viewBox().size();
    size.scale(300,300,Qt::KeepAspectRatio);
    renderer.render(&painter,QRect(5,(image.height()-size.height())/2,
                                   size.width(),size.height()));
    renderer.load(outpath);
    size = renderer.viewBox().size();
    size.scale(300,300,Qt::KeepAspectRatio);
    renderer.render(&painter,QRect(305,(image.height()-size.height())/2,
                                   size.width(),size.height()));
    painter.end();

    toolTip->setPixmap(mainPix);
    toolTip->setMask(mainPix.mask());
//    toolTip->repaint();
    newToolTip = false;
}

void IconsWidget::setCrashed(bool flag)
{
    crashed = flag;
    repaint();
}

void IconsWidget::enterEvent(QEvent *)
{
    qDebug()<<Q_FUNC_INFO;
    QTimer::singleShot(500,this,SLOT(showToolTip()));
}

void IconsWidget::leaveEvent(QEvent *)
{
    qDebug()<<Q_FUNC_INFO;
    QCursor curs;
    QPoint cursPos = mapFromGlobal(curs.pos());
    if (!rect().contains(cursPos))
        toolTip->hide();
}

void IconsWidget::showToolTip()
{
    qDebug()<<Q_FUNC_INFO;
    if (crashed)
        return;

    if (newToolTip)
        makeToolTip();

    QCursor curs;
    QPoint cursPos = mapFromGlobal(curs.pos());
    if (rect().contains(cursPos)) {
        toolTip->show();
    }
}

void IconsWidget::mouseMoveEvent(QMouseEvent *event)
{
    qDebug()<<Q_FUNC_INFO;
    if (crashed)
        return;

    QCursor curs;
    QPoint cursPos = mapFromGlobal(curs.pos());

    if (rect().contains(cursPos)) {
        int border = 5;
        QPoint p = mapToGlobal(event->pos());
        QPoint p2 = mapToGlobal(QPoint(0,0));
        if (p.y()-mainPix.height()-12 > 0) {
            toolTip->setGeometry(p.x()-mainPix.width()/2,p2.y()-mainPix.height()-6,
                                 mainPix.width(),mainPix.height());
        } else {
            toolTip->setGeometry(p.x()-mainPix.width()/2,p2.y()+height()+border,
                                  mainPix.width(),mainPix.height());
        }
    }
}

void IconsWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (crashed) {
        painter.setPen(QPen(Qt::red));
        painter.drawText(rect(),Qt::AlignCenter,tr("Crashed"));
        return;
    }

    if (compareView) {
        renderSvg(inpath, &painter,QRect(0,0,width()/2,height()));
        renderSvg(outpath,&painter,QRect(width()/2,0,width()/2,height()));
//        QPixmap in(inpath);
//        if (in.height() > height() || in.width() > width()/2)
//            in = in.scaled(height(),height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
//        int heightBorder = (height()-in.height())/2;
//        painter.drawPixmap(width()/4-in.width()/2,heightBorder,in.width(),in.height(),in);
//        QPixmap out(outpath);
//        if (out.height() > height() || out.width() > width()/2)
//            out = out.scaled(height(),height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
//        painter.drawPixmap(width()/2+width()/4-in.width()/2,heightBorder,out.width(),out.height()
//        ,out);
    } else {
        renderSvg(outpath,&painter,QRect(0,0,width(),height()));

//        QPixmap out(outpath);
//        if (out.height() > height() || out.width() > width())
//            out = out.scaled(height(),height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
//        int heightBorder = (height()-out.height())/2;
//        painter.drawPixmap(width()/2-out.width()/2,heightBorder,out.width(),out.height(),out);
    }
}

void IconsWidget::renderSvg(const QString path, QPainter *painter, QRect rect)
{
    QSvgRenderer renderer(path);
    QSize size = renderer.viewBox().size();
    size.scale(rect.width(),rect.height(),Qt::KeepAspectRatio);
    renderer.render(painter,QRect(rect.x(),(rect.height()-size.height())/2,
                                  size.width(),size.height()));
}

void IconsWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || crashed)
        return;

    if (event->x() < width()/2)
        QDesktopServices::openUrl(QUrl(inpath));
    else
        QDesktopServices::openUrl(QUrl(outpath));
}
