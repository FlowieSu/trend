#include "mainwindow.h"

#include "dynamic_result_types.h"
#include "zlg_can_device.h"

#include <QApplication>
#include <QMetaType>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication a(argc, argv);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif

    /*
     * 动态检测结果类型注册。
     * productFinished / allFinished 等信号会传这些结构。
     */
    qRegisterMetaType<DynamicItemResult>("DynamicItemResult");
    qRegisterMetaType<DynamicProductResult>("DynamicProductResult");
    qRegisterMetaType<QVector<DynamicProductResult>>("QVector<DynamicProductResult>");

    /*
     * ZLG CAN 设备配置类型注册。
     */
    qRegisterMetaType<ZlgCanDeviceType>("ZlgCanDeviceType");
    qRegisterMetaType<ZlgCanOpenConfig>("ZlgCanOpenConfig");
    qRegisterMetaType<ZlgCanFrame>("ZlgCanFrame");

    MainWindow w;
    w.show();

    return a.exec();
}
