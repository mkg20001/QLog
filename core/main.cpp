#include <QApplication>
#include <QtSql/QtSql>
#include <QMessageBox>
#include <QResource>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QTime>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QMessageBox>
#include <QDebug>
#include <QSplashScreen>

#include "debug.h"
#include "Migration.h"
#include "ui/MainWindow.h"
#include "Rig.h"
#include "Rotator.h"
#include "CWKeyer.h"
#include "AppGuard.h"
#include "logformat/AdxFormat.h"
#include "ui/SettingsDialog.h"
#include "data/StationProfile.h"
#include "core/zonedetect.h"
#include "ui/SplashScreen.h"
#include "core/MembershipQE.h"
#include "core/KSTChat.h"

MODULE_IDENTIFICATION("qlog.core.main");

QMutex debug_mutex;

static void setupTranslator(QApplication* app) {
    FCT_IDENTIFICATION;

    QTranslator* qtTranslator = new QTranslator(app);
    if ( qtTranslator->load("qt_" + QLocale::system().name(),
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                       QLibraryInfo::location(QLibraryInfo::TranslationsPath)) )
#else
                       QLibraryInfo::path(QLibraryInfo::TranslationsPath)) )
#endif

    {
        app->installTranslator(qtTranslator);
    }

    QTranslator* translator = new QTranslator(app);
    if ( translator->load(":/i18n/qlog_" + QLocale::system().name().left(2)) )
    {
        app->installTranslator(translator);
    }
}

static void createDataDirectory() {
    FCT_IDENTIFICATION;

    QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    qCDebug(runtime) << dataDir.path();

    if (!dataDir.exists()) {
        dataDir.mkpath(dataDir.path());
    }
}

static bool openDatabase() {
    FCT_IDENTIFICATION;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(Data::dbFilename());
    db.setConnectOptions("QSQLITE_ENABLE_REGEXP");

    if (!db.open()) {
        qCritical() << db.lastError();
        return false;
    }
    else {
        QSqlQuery query;
        if ( !query.exec("PRAGMA foreign_keys = ON") )
        {
            qCritical() << "Cannot set PRAGMA foreign_keys";
            return false;
        }

        if ( !query.exec("PRAGMA journal_mode = WAL") )
        {
            qCritical() << "Cannot set PRAGMA journal_mode";
            return false;
        }

        while ( query.next() )
        {
            QString pragma = query.value(0).toString();
            qCDebug(runtime) << "Pragma result:" << pragma;
        }

        return true;
    }
}

static bool backupDatabase()
{
    FCT_IDENTIFICATION;
    /* remove old backups */
    /* retention time is 30 days but a minimum number of backup files is 5 */
    const int retention_time = 30;
    const int min_backout_count = 5;

    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    QString path = dir.filePath("qlog_backup_" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".adx");
    QString filter("qlog_backup_%1%1%1%1%1%1%1%1%1%1%1%1%1%1.adx");
    filter = filter.arg("[0123456789]");

    QFileInfoList file_list = QDir(dir).entryInfoList(QStringList(filter), QDir::Files, QDir::Name);

    qCDebug(runtime) << file_list;

    /* Keep the minimum number of backups */
    /* If a number of backup is greater than min_backout_count then remove files older 30 days but always
       protect a minimum number of backup file files */
    for (int i = 0; i < qMin(min_backout_count, file_list.size()); i++)
    {
        file_list.takeLast();  // remove last 5 files from sorted list by name to protect them from removing.
    }

    /* remove those older than 30 days from the remaining files. */
    Q_FOREACH (auto fileInfo, file_list)
    {
        if (fileInfo.lastModified().date().daysTo(QDate::currentDate()) > retention_time)
        {
            QString filepath = fileInfo.absoluteFilePath();
            QDir deletefile;
            deletefile.setPath(filepath);
            deletefile.remove(filepath);
            qCDebug(runtime) << "Removing file: " << filepath;
        }
    }

    /* make a backup file */
    QFile backup_file(path);

    if ( !backup_file.open(QFile::ReadWrite | QIODevice::Text))
    {
        qWarning()<<"Cannot open backup file " << path << "for writing";
        return false;
    }

    qCDebug(runtime)<<"Exporting a Database backup to " << path;

    QTextStream stream(&backup_file);
    AdxFormat adx(stream);

    adx.runExport();
    stream.flush();
    backup_file.close();

    qCDebug(runtime)<<"Database backup finished";
    return true;
}
static bool migrateDatabase() {
    FCT_IDENTIFICATION;

    Migration m;
    return m.run();

}

static void startRigThread() {
    FCT_IDENTIFICATION;

    QThread* rigThread = new QThread;
    Rig* rig = Rig::instance();
    rig->moveToThread(rigThread);
    QObject::connect(rigThread, SIGNAL(started()), rig, SLOT(start()));
    rigThread->start();
}

static void startRotThread() {
    FCT_IDENTIFICATION;

    QThread* rotThread = new QThread;
    Rotator* rot = Rotator::instance();
    rot->moveToThread(rotThread);
    QObject::connect(rotThread, SIGNAL(started()), rot, SLOT(start()));
    rotThread->start();
}

static void startCWKeyerThread()
{
    FCT_IDENTIFICATION;

    QThread* cwKeyerThread = new QThread;
    CWKeyer* cwKeyer = CWKeyer::instance();
    cwKeyer->moveToThread(cwKeyerThread);
    QObject::connect(cwKeyerThread, SIGNAL(started()), cwKeyer, SLOT(start()));
    cwKeyerThread->start();
}

static void debugMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    debug_mutex.lock();

    QByteArray localMsg = msg.toLocal8Bit();
    QString severity_string;

    switch (type)
    {
    case QtDebugMsg:
        severity_string = "[DEBUG   ]";
        break;
    case QtInfoMsg:
        severity_string = "[INFO    ]";
        break;
    case QtWarningMsg:
        severity_string = "[WARNING ]";
        break;
    case QtCriticalMsg:
        severity_string = "[CRITICAL]";
        break;
    case QtFatalMsg:
        severity_string = "[FATAL   ]";
        break;
    default:
        severity_string = "[UNKNOWN ]";
    }

    QString cat(context.category);
    if ( cat == "default" )
    {
        cat = "[             ]";
    }
    else
    {
        cat = "[" + cat + "]";
    }

    QString idStr = QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()),16);

    fprintf(stderr, "%s %s [0x%s] %50s => %s [%s:%s:%u] \n",
                                                 QTime::currentTime().toString("HH:mm:ss.zzz").toLocal8Bit().constData(),
                                                 severity_string.toLocal8Bit().constData(),
                                                 idStr.toLocal8Bit().data(),
                                                 cat.toLocal8Bit().constData(),
                                                 localMsg.constData(),
                                                 context.function,
                                                 context.file,
                                                 context.line
                                                 );
    debug_mutex.unlock();

    if ( type == QtFatalMsg )
    {
        abort();
    }
}

int main(int argc, char* argv[])
{

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif


    bool stylePresent = false;

    /* Style option is deleted in QApplication constructor.
     * Therefore test for the parameter presence has to be performed here
     */
    for ( int i = 0; i < argc && !stylePresent; i++ )
    {
        stylePresent = QString(argv[i]).contains("-style");
    }

    QApplication app(argc, argv);
    app.setApplicationVersion(VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::tr("QLog Help"));
    parser.addHelpOption();
    parser.addVersionOption();

    /* Undocumented param used for debugging. A developer can run QLog with
     * a specific namespace. This helps in cases where it is possible
     * to simultaneously run a test/develop and production versions of QLog.
     *
     * The parameter changes the Application name. It causes that all runtime
     * files, settings, passwords and DB file are created in a different directory/namespace.
     *
     * however, it remains necessary only one instance of QLog to run at a time.
     * More Notes below (AppGuard Comment).
     *
     * NOTE: This is not a preparation for the ability to run separate databases.
     * It's just to make it easier for developers and testers.
     */

    QCommandLineOption environmentName(QStringList() << "n" << "namespace",
                QCoreApplication::translate("main", "Run with the specific namespace."),
                QCoreApplication::translate("main", "namespace"));
    parser.addOption(environmentName);
    parser.process(app);
    QString environment = parser.value(environmentName);

    app.setOrganizationName("hamradio");
    app.setApplicationName("QLog" + ((environment.isNull()) ? "" : environment.prepend("-")));

    /* If the Style parameter is not present then use a default - Fusion style */
    if ( !stylePresent )
    {
        app.setStyle(QStyleFactory::create("Fusion"));
    }

    qInstallMessageHandler(debugMessageOutput);
    qRegisterMetaType<VFOID>();
    qRegisterMetaType<ClubStatusQuery::ClubStatus>();
    qRegisterMetaType<QMap<QString, ClubStatusQuery::ClubStatus>>();
    qRegisterMetaType<KSTChatMsg>();
    qRegisterMetaType<KSTUsersInfo>();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qRegisterMetaTypeStreamOperators<QSet<int>>("QSet<int>");
#endif
    set_debug_level(LEVEL_PRODUCTION); // you can set more verbose rules via
                                       // environment variable QT_LOGGING_RULES (project setting/debug)

    setupTranslator(&app);

    /* Application Singleton
     *
     * Only one instance of QLog application is allowed
     *
     * It is always necessary to run only one QLog on the
     * system, because the FLDigi interface has a fixed port.
     * Therefore, in the case of two or more instances,
     * there is a port conflict.
     */
    AppGuard guard( "QLog" );
    if ( !guard.tryToRun() )
    {
        QMessageBox::critical(nullptr, QMessageBox::tr("QLog Error"),
                              QMessageBox::tr("QLog is already running"));
        return 1;
    }

    QPixmap pixmap(":/res/qlog.png");
    SplashScreen splash(pixmap);
    splash.show();
    splash.ensureFirstPaint();

    createDataDirectory();

    splash.showMessage(QObject::tr("Opening Database"), Qt::AlignBottom|Qt::AlignCenter );

    if (!openDatabase()) {
        QMessageBox::critical(nullptr, QMessageBox::tr("QLog Error"),
                              QMessageBox::tr("Could not connect to database."));
        return 1;
    }

    splash.showMessage(QObject::tr("Backuping Database"), Qt::AlignBottom|Qt::AlignCenter);

    /* a migration can break a database therefore a backup is call before it */
    if (!backupDatabase())
    {
        QMessageBox::critical(nullptr, QMessageBox::tr("QLog Error"),
                              QMessageBox::tr("Could not export a QLog database to ADIF as a backup.<p>Try to export your log to ADIF manually"));
    }

    splash.showMessage(QObject::tr("Migrating Database"), Qt::AlignBottom|Qt::AlignCenter);

    if (!migrateDatabase()) {
        QMessageBox::critical(nullptr, QMessageBox::tr("QLog Error"),
                              QMessageBox::tr("Database migration failed."));
        return 1;
    }

    splash.showMessage(QObject::tr("Starting Application"), Qt::AlignBottom|Qt::AlignCenter);

    startRigThread();
    startRotThread();
    startCWKeyerThread();

    MainWindow w;
    QIcon icon(":/res/qlog.png");
    splash.finish(&w);
    w.setWindowIcon(icon);

    w.show();

    return app.exec();
}
