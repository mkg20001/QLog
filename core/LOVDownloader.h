#ifndef LOVDOWNLOADER_H
#define LOVDOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QRegularExpression>

class LOVDownloader : public QObject
{
    Q_OBJECT

public:
    enum SourceType
    {
        CTY = 0,
        SATLIST = 1,
        SOTASUMMITS = 2,
        WWFFDIRECTORY = 3,
        IOTALIST = 4,
        POTADIRECTORY = 5,
        MEMBERSHIPCONTENTLIST = 6,
        UNDEF = 7
    };

public:
    LOVDownloader(QObject *parent = nullptr);
    ~LOVDownloader();
    void update(const SourceType &);

public slots:
    void abortRequest();

signals:
    void processingSize(qint64);
    void progress(qint64 count);
    void finished(bool result);
    void noUpdate();

private:
    class SourceDefinition
    {
    public:
        SourceDefinition(const SourceType type,
                         const QString &URL,
                         const QString &fileName,
                         const QString &lastTimeConfigName,
                         const QString &tableName,
                         int ageTime) : type(type),
                                        URL(URL),
                                        fileName(fileName),
                                        lastTimeConfigName(lastTimeConfigName),
                                        tableName(tableName),
                                        ageTime(ageTime) {};
        SourceDefinition() : type(LOVDownloader::UNDEF),
                                  ageTime(0){};
        SourceType type;
        QString URL;
        QString fileName;
        QString lastTimeConfigName;
        QString tableName;
        int ageTime;
    };

    QMap<SourceType, SourceDefinition> sourceMapping = {
        {CTY, SourceDefinition(CTY,
                               "http://www.country-files.com/cty/cty.csv",
                               "cty.csv",
                               "last_cty_update",
                               "dxcc_entities",
                               21)},
        {SATLIST, SourceDefinition(SATLIST,
                                   "https://foldynl.github.io/QLog/data/satslist.csv",
                                   "satslist.csv",
                                   "last_sat_update",
                                   "sat_info",
                                   30)},
        {SOTASUMMITS, SourceDefinition(SOTASUMMITS,
                                   "https://mapping.sota.org.uk/summitslist.csv",
                                   "summitslist.csv",
                                   "last_sotasummits_update",
                                   "sota_summits",
                                   30)},
        {WWFFDIRECTORY, SourceDefinition(WWFFDIRECTORY,
                                   "https://wwff.co/wwff-data/wwff_directory.csv",
                                   "wwff_directory.csv",
                                   "last_wwffdirectory_update",
                                   "wwff_directory",
                                   21)},
        {IOTALIST, SourceDefinition(IOTALIST,
                                   "https://foldynl.github.io/QLog/data/iota.csv",
                                   "iota.csv",
                                   "last_iota_update",
                                   "iota",
                                   30)},
        {POTADIRECTORY, SourceDefinition(POTADIRECTORY,
                                   "https://pota.app/all_parks_ext.csv",
                                   "all_parks_ext.csv",
                                   "last_pota_update",
                                   "pota_directory",
                                   30)},
        {MEMBERSHIPCONTENTLIST, SourceDefinition(MEMBERSHIPCONTENTLIST,
                                   "https://raw.githubusercontent.com/foldynl/hamradio-membeship-lists/main/lists/content.csv",
                                   "content.csv",
                                   "last_membershipcontent_update",
                                   "membership_directory",
                                   7)}
    };

    QNetworkAccessManager* nam;
    QNetworkReply *currentReply;
    bool abortRequested;
    QRegularExpression CSVRe;
    QRegularExpression CTYPrefixSeperatorRe;
    QRegularExpression CTYPrefixFormatRe;

private:
    bool isTableFilled(const QString &);
    bool deleteTable(const QString &);
    void download(const SourceDefinition &);
    void parseData(const LOVDownloader::SourceDefinition &,
                   QTextStream &);
    void parseCTY(const SourceDefinition &sourceDef, QTextStream& data);
    void parseSATLIST(const SourceDefinition &sourceDef, QTextStream& data);
    void parseSOTASummits(const SourceDefinition &sourceDef, QTextStream& data);
    void parseWWFFDirectory(const SourceDefinition &sourceDef, QTextStream& data);
    void parseIOTA(const SourceDefinition &sourceDef, QTextStream& data);
    void parsePOTA(const SourceDefinition &sourceDef, QTextStream& data);
    void parseMembershipContent(const SourceDefinition &sourceDef, QTextStream& data);

private slots:
    void processReply(QNetworkReply*);
    void loadData(const LOVDownloader::SourceDefinition &);

};

#endif // LOVDOWNLOADER_H
