#ifndef DATA_H
#define DATA_H

#include <QtCore>
#include <QSqlQuery>
#include "Dxcc.h"
#include "SOTAEntity.h"
#include "WWFFEntity.h"
#include "POTAEntity.h"
#include "Band.h"
#include "core/zonedetect.h"

class Data : public QObject
{
    Q_OBJECT
public:
    static const QString MODE_CW;
    static const QString MODE_DIGITAL;
    static const QString MODE_FT8;
    static const QString MODE_LSB; // use just generic label SSB
    static const QString MODE_USB; // use just generic label SSB
    static const QString MODE_PHONE;

    const QMap<QString, QString> qslSentEnum = {
        {"Y", tr("Yes")},
        {"N", tr("No")},
        {"R", tr("Requested")},
        {"Q", tr("Queued")},
        {"I", tr("Invalid")}
    };
    const QMap<QString, QString> qslSentViaEnum = {
        {"B", tr("Bureau")},
        {"D", tr("Direct")},
        {"E", tr("Electronic")},
        {" ", tr("Blank")}
    };
    const QMap<QString, QString> qslRcvdEnum = {
        {"Y", tr("Yes")},
        {"N", tr("No")},
        {"R", tr("Requested")},
        {"I", tr("Invalid")}
    };
    const QMap<QString, QString> uploadStatusEnum = {
        {"Y", tr("Yes")},
        {"N", tr("No")},
        {"M", tr("Modified")},
        {" ", tr("Blank")}
    };
    const QMap<QString, QString> antPathEnum = {
        {"G", tr("Grayline")},
        {"O", tr("Other")},
        {"S", tr("Short Path")},
        {"L", tr("Long Path")},
        {" ", tr("Blank")}
    };
    const QMap<QString, QString> boolEnum = {
        {"Y", tr("Yes")},
        {"N", tr("No")},
        {" ", tr("Blank")}
    };
    const QMap<QString, QString> qsoCompleteEnum = {
        {"Y", tr("Yes")},
        {"N", tr("No")},
        {"Nil", tr("Not Heard")},
        {"?", tr("Uncertain")},
        {" ", tr("Blank")}
    };

    explicit Data(QObject *parent = nullptr);
    ~Data();
    static Data* instance();

    static DxccStatus dxccStatus(int dxcc, const QString &band, const QString &mode);
    static DxccStatus dxccFutureStatus(const DxccStatus &oldStatus,
                                       const qint32 oldDxcc,
                                       const QString &oldBand,
                                       const QString &oldMode,
                                       const qint32 newDxcc,
                                       const QString &newBand,
                                       const QString &newMode);
    static Band band(double freq);
    static QList<Band> bandsList(const bool onlyDXCCBands = false, const bool onlyEnabled = false);
    static QString freqToDXCCMode(double freq);
    QString modeToDXCCMode(const QString &mode);
    static QColor statusToColor(const DxccStatus &status, const QColor &defaultColor);
    static QString colorToHTMLColor(const QColor&);
    static QString statusToText(const DxccStatus &status);
    static QString removeAccents(const QString &input);
    static int getITUZMin();
    static int getITUZMax();
    static int getCQZMin();
    static int getCQZMax();
    static QString dbFilename();

    QStringList contestList() { return contests.values(); }
    QStringList propagationModesList() { return propagationModes.values(); }
    QStringList propagationModesIDList() { return propagationModes.keys(); }
    QString propagationModeTextToID(const QString &propagationText) { return propagationModes.key(propagationText);}
    QString propagationModeIDToText(const QString &propagationID) { return propagationModes.value(propagationID);}
    DxccEntity lookupDxcc(const QString &callsign);
    SOTAEntity lookupSOTA(const QString &SOTACode);
    POTAEntity lookupPOTA(const QString &POTACode);
    WWFFEntity lookupWWFF(const QString &reference);
    QString dxccFlag(int dxcc);
    QPair<QString, QString> legacyMode(const QString &mode);
    QStringList satModeList() { return satModes.values();}
    QStringList satModesIDList() { return satModes.keys(); }
    QString satModeTextToID(const QString &satModeText) { return satModes.key(satModeText);}
    QString satModeIDToText(const QString &satModeID) { return satModes.value(satModeID);}
    QStringList iotaList() { return iotaRef.values();}
    QStringList iotaIDList() { return iotaRef.keys();}
    QString iotaTextToID(const QString &iotaText) { return iotaRef.key(iotaText);}
    QStringList sotaIDList() { return sotaRefID.keys();}
    QStringList wwffIDList() { return wwffRefID.keys();}
    QStringList potaIDList() { return potaRefID.keys();}
    QString getIANATimeZone(double, double);

signals:

public slots:

private:
    void loadContests();
    void loadPropagationModes();
    void loadLegacyModes();
    void loadDxccFlags();
    void loadSatModes();
    void loadIOTA();
    void loadSOTA();
    void loadWWFF();
    void loadPOTA();
    void loadTZ();

    QMap<int, QString> flags;
    QMap<QString, QString> contests;
    QMap<QString, QString> propagationModes;
    QMap<QString, QPair<QString, QString>> legacyModes;
    QMap<QString, QString> satModes;
    QMap<QString, QString> iotaRef;
    QMap<QString, QString> sotaRefID;
    QMap<QString, QString> wwffRefID;
    QMap<QString, QString> potaRefID;
    ZoneDetect * zd;
    QSqlQuery queryDXCC;
    QSqlQuery querySOTA;
    QSqlQuery queryWWFF;
    QSqlQuery queryPOTA;
    bool isDXCCQueryValid;
    bool isSOTAQueryValid;
    bool isWWFFQueryValid;
    bool isPOTAQueryValid;
};

#endif // DATA_H
