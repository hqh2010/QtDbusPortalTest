#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusObjectPath>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("org.deepin.portaldemo");
    ui->statusBar->hide();
    ui->menuBar->hide();
    int bx = ui->captureButton->width();
    int by = ui->captureButton->height();
    // button居中
    ui->captureButton->move((this->width() - bx) / 2, (this->height() - by) / 2);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::startScreenCapture()
{
    // https://flatpak.github.io/xdg-desktop-portal/
    QDBusInterface interface("org.freedesktop.impl.portal.desktop.kde", "/org/freedesktop/portal/desktop",
                             "org.freedesktop.impl.portal.Screenshot", QDBusConnection::sessionBus());
    if (!interface.isValid())
    {
        qCritical() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        return "";
    }

    // https://github.com/KDE/xdg-desktop-portal-kde
    // 使用qdbus调用 xdg-desktop-portal-kde 提供的gdbus截屏接口 函数原型如上

    // Screenshot (IN o objpath
    //             IN s app_id
    //             IN s parent_window,
    //             IN a{sv} options,
    //             OUT a{sv} handle);

    // Under X11, the "parent_window" argument should have the form "x11:XID", where XID is the XID of the application window in hexadecimal notation.
    // Under Wayland, it should have the form "wayland:HANDLE", where HANDLE is a surface handle obtained with the xdg_foreign protocol.

    QDBusObjectPath objpath("/org/freedesktop/portal/desktop");
    // 查看源码实现 app_id 没有使用，随便写一个
    const QString appId = "org.deepin.portaldemo";
    // 查看源码实现 parent_window 非x11场景未使用，暂时随便写一个
    const QString wdHandle = "wayland:0x20211230";
    // 截屏弹框显示模式和非模式设置
    // const bool modal = options.value(QStringLiteral("modal"), false).toBool();

    QVariantMap optionMap;
    QString savePath = "";
    QDBusMessage result = interface.call("Screenshot", objpath, appId, wdHandle, optionMap);
    // QDBusMessage(type=MethodReturn, service=":1.1038", signature="ua{sv}",
    // contents=(0, [Argument: a{sv} {"uri" = [Variant(QString): "file:///home/uthuqinghong/Pictures/Screenshot_20211230_172001.png"]}]) )
    qInfo() << result;
    if (result.type() == QDBusMessage::ReplyMessage)
    {
        // QDBusMessage的arguments不仅可以用来存储发送的参数，也用来存储返回值
        QList<QVariant> outArgs = result.arguments();
        QVariant data = outArgs.at(1);
        // 注意这里&不能少
        const QDBusArgument &dbusArgs = data.value<QDBusArgument>();
        qInfo() << "QDBusArgument current type is" << dbusArgs.currentType();
        dbusArgs.beginMap();
        QString key;
        QVariant val;
        while (!dbusArgs.atEnd())
        {
            dbusArgs.beginMapEntry();
            dbusArgs >> key >> val;
            dbusArgs.endMapEntry();
        }
        dbusArgs.endMap();
        //qInfo() << val.toString();
        savePath = val.toString();
    }

    // QDBusPendingReply<uint> reply = interface.call("Screenshot", objpath, appId, wdHandle, optionMap);
    // reply.waitForFinished();
    // if (reply.isValid())
    // {
    //     int ret = reply.value();
    //     qInfo() << ret;
    // }
    return savePath;
}

void MainWindow::on_captureButton_clicked()
{
    qInfo() << "start screen capture";
    QString savePath = startScreenCapture();
    qInfo() << "start screen capture done, path:" << savePath;
}
