#include <QJsonDocument>
#include <QSqlQuery>
#include <QSqlError>
#include <QColor>
#include "Data.h"
#include "core/Callsign.h"
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.data.data");

Data::Data(QObject *parent) :
   QObject(parent),
   zd(nullptr),
   isDXCCQueryValid(false)
{
    FCT_IDENTIFICATION;

    loadContests();
    loadPropagationModes();
    loadLegacyModes();
    loadDxccFlags();
    loadSatModes();
    loadIOTA();
    loadSOTA();
    loadWWFF();
    loadPOTA();
    loadTZ();

    isDXCCQueryValid = queryDXCC.prepare(
                "SELECT "
                "    dxcc_entities.id, "
                "    dxcc_entities.name, "
                "    dxcc_entities.prefix, "
                "    dxcc_entities.cont, "
                "    CASE "
                "        WHEN (dxcc_prefixes.cqz != 0) "
                "        THEN dxcc_prefixes.cqz "
                "        ELSE dxcc_entities.cqz "
                "    END AS cqz, "
                "    CASE "
                "        WHEN (dxcc_prefixes.ituz != 0) "
                "        THEN dxcc_prefixes.ituz "
                "        ELSE dxcc_entities.ituz "
                "    END AS ituz , "
                "    dxcc_entities.lat, "
                "    dxcc_entities.lon, "
                "    dxcc_entities.tz "
                "FROM dxcc_prefixes "
                "INNER JOIN dxcc_entities ON (dxcc_prefixes.dxcc = dxcc_entities.id) "
                "WHERE (dxcc_prefixes.prefix = :callsign and dxcc_prefixes.exact = true) "
                "    OR (dxcc_prefixes.exact = false and :callsign LIKE dxcc_prefixes.prefix || '%') "
                "ORDER BY dxcc_prefixes.prefix "
                "DESC LIMIT 1 "
                );

    isSOTAQueryValid = querySOTA.prepare(
                "SELECT summit_code,"
                "       association_name,"
                "       region_name,"
                "       summit_name,"
                "       altm,"
                "       altft,"
                "       gridref1,"
                "       gridref2,"
                "       longitude,"
                "       latitude,"
                "       points,"
                "       bonus_points,"
                "       valid_from,"
                "       valid_to "
                "FROM sota_summits "
                "WHERE summit_code = UPPER(:code)"
                );

    isPOTAQueryValid = queryPOTA.prepare(
                "SELECT reference,"
                "       name,"
                "       active,"
                "       entityID,"
                "       locationDesc,"
                "       latitude,"
                "       longitude,"
                "       grid "
                "FROM pota_directory "
                "WHERE reference = UPPER(:code)"
                );

    isWWFFQueryValid = queryWWFF.prepare(
                "SELECT reference,"
                "       status,"
                "       name,"
                "       program,"
                "       dxcc,"
                "       state,"
                "       county,"
                "       continent,"
                "       iota,"
                "       iaruLocator,"
                "       latitude,"
                "       longitude,"
                "       iucncat,"
                "       valid_from,"
                "       valid_to "
                "FROM wwff_directory "
                "WHERE reference = UPPER(:reference)"
                );
}

Data::~Data()
{
    FCT_IDENTIFICATION;

    if ( zd )
    {
        ZDCloseDatabase(zd);
    }
}

Data* Data::instance() {
    FCT_IDENTIFICATION;

    static Data instance;
    return &instance;
}

DxccStatus Data::dxccStatus(int dxcc, const QString &band, const QString &mode) {
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << dxcc << " " << band << " " << mode;

    QString filter;

    QSettings settings;
    QVariant start = settings.value("dxcc/start");
    if (!start.isNull()) {
        filter = QString("AND start_time >= '%1'").arg(start.toDate().toString("yyyy-MM-dd"));
    }

    QString sql_mode = ":mode ";
    if (mode != Data::MODE_CW && mode != Data::MODE_PHONE && mode != Data::MODE_DIGITAL) {
        sql_mode = "(SELECT modes.dxcc FROM modes WHERE modes.name = :mode LIMIT 1) ";
    }

    QSqlQuery query;

    if ( ! query.prepare("WITH all_dxcc_qsos AS (SELECT DISTINCT contacts.mode, contacts.band, contacts.qsl_rcvd, contacts.lotw_qsl_rcvd FROM contacts WHERE dxcc = :dxcc " + filter + ") "
                         "  SELECT (SELECT 1 FROM all_dxcc_qsos LIMIT 1) as entity,"
                         "         (SELECT 1 FROM all_dxcc_qsos WHERE band = :band LIMIT 1) as band, "
                         "         (SELECT 1 FROM all_dxcc_qsos INNER JOIN modes ON (modes.name = all_dxcc_qsos.mode) WHERE modes.dxcc = " + sql_mode + " LIMIT 1) as mode, "
                         "         (SELECT 1 FROM all_dxcc_qsos INNER JOIN modes ON (modes.name = all_dxcc_qsos.mode) WHERE modes.dxcc = " + sql_mode + " AND all_dxcc_qsos.band = :band LIMIT 1) as slot, "
                         "         (SELECT 1 FROM all_dxcc_qsos INNER JOIN modes ON (modes.name = all_dxcc_qsos.mode) WHERE modes.dxcc = " + sql_mode + " AND all_dxcc_qsos.band = :band AND (all_dxcc_qsos.qsl_rcvd = 'Y' OR all_dxcc_qsos.lotw_qsl_rcvd = 'Y') LIMIT 1) as confirmed"
                         )
       )
    {
        qWarning() << "Cannot prepare Select statement";
        return DxccStatus::UnknownStatus;
    }

    query.bindValue(":dxcc", dxcc);
    query.bindValue(":band", band);
    query.bindValue(":mode", mode);

    if ( ! query.exec() )
    {
        qWarning() << "Cannot execute Select statement" << query.lastError();
        return DxccStatus::UnknownStatus;
    }

    if (query.next()) {
        if (query.value(0).isNull()) {
            return DxccStatus::NewEntity;
        }
        if (query.value(1).isNull()) {
            if (query.value(2).isNull()) {
                return DxccStatus::NewBandMode;
            }
            else {
                return DxccStatus::NewBand;
            }
        }
        if (query.value(2).isNull()) {
            return DxccStatus::NewMode;
        }
        if (query.value(3).isNull()) {
            return DxccStatus::NewSlot;
        }
        if ( query.value(4).isNull()) {
            return DxccStatus::Worked;
        }
        else {
            return DxccStatus::Confirmed;
        }
    }
    else {
        return DxccStatus::UnknownStatus;
    }
}

#define RETURNCODE(a) \
    qCDebug(runtime) << "new DXCC Status: " << (a); \
    return ((a))

DxccStatus Data::dxccFutureStatus(const DxccStatus &oldStatus,
                                  const qint32 oldDxcc,
                                  const QString &oldBand,
                                  const QString &oldMode,
                                  const qint32 newDxcc,
                                  const QString &newBand,
                                  const QString &newMode)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << oldStatus
                               << oldDxcc
                               << oldBand
                               << oldMode
                               << newDxcc
                               << newBand
                               << newMode;

    if ( oldDxcc != newDxcc )
    {
        /* No change */
        RETURNCODE(oldStatus);
    }

    if ( oldBand == newBand
         && oldMode == newMode )
    {
        RETURNCODE(DxccStatus::Worked);
    }

    /*************/
    /* NewEntity */
    /*************/
    if ( oldStatus == DxccStatus::NewEntity )
    {
        if ( oldBand != newBand
             && oldMode != newMode )
        {
            RETURNCODE(DxccStatus::NewBandMode);
        }

        if ( oldBand != newBand
             && oldMode == newMode )
        {
            RETURNCODE(DxccStatus::NewBand);
        }

        if ( oldBand == newBand
             && oldMode != newMode )
        {
            RETURNCODE(DxccStatus::NewMode);
        }
    }

    /***************/
    /* NewBandMode */
    /***************/
    if ( oldStatus == DxccStatus::NewBandMode )
    {
        if ( oldBand != newBand
             && oldMode != newMode )
        {
            RETURNCODE(DxccStatus::NewBandMode);
        }

        if ( oldBand != newBand
             && oldMode == newMode )
        {
            RETURNCODE(DxccStatus::NewBand);
        }

        if ( oldBand == newBand
             && oldMode != newMode )
        {
            RETURNCODE(DxccStatus::NewMode);
        }
    }

    /*************/
    /* NewBand   */
    /*************/
    if ( oldStatus == DxccStatus::NewBand )
    {
        if ( oldBand != newBand
             && oldMode != newMode )
        {
            RETURNCODE(DxccStatus::NewBand);
        }

        if ( oldBand != newBand
             && oldMode == newMode )
        {
            RETURNCODE(DxccStatus::NewBand);
        }

        if ( oldBand == newBand
             && oldMode != newMode )
        {
            RETURNCODE(DxccStatus::NewSlot);
        }
    }

    /*************/
    /* NewMode   */
    /*************/
    if ( oldStatus == DxccStatus::NewMode )
    {
        if ( oldBand != newBand
             && oldMode != newMode )
        {
            RETURNCODE(DxccStatus::NewMode);
        }

        if ( oldBand != newBand
             && oldMode == newMode )
        {
            RETURNCODE(DxccStatus::NewSlot);
        }

        if ( oldBand == newBand
             && oldMode != newMode )
        {
            RETURNCODE(DxccStatus::NewMode);
        }
    }

    /*************/
    /* NewSlot   */
    /*************/
    if ( oldStatus == DxccStatus::NewSlot )
    {
        RETURNCODE(DxccStatus::NewSlot);
    }

    /*************/
    /* Worked   */
    /*************/
    if ( oldStatus == DxccStatus::Worked )
    {
        RETURNCODE(DxccStatus::Worked);
    }

    /***************/
    /* Confirmed   */
    /***************/
    if ( oldStatus == DxccStatus::Confirmed )
    {
        RETURNCODE(DxccStatus::Confirmed);
    }

    RETURNCODE(DxccStatus::UnknownStatus);
}

#undef RETURNCODE

Band Data::band(double freq) {
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << freq;

    QSqlQuery query;
    if ( ! query.prepare("SELECT name, start_freq, end_freq FROM bands WHERE :freq BETWEEN start_freq AND end_freq") )
    {
        qWarning() << "Cannot prepare Select statement";
        return Band();
    }
    query.bindValue(0, freq);

    if ( ! query.exec() )
    {
        qWarning() << "Cannot execute select statement" << query.lastError();
        return Band();
    }

    if (query.next()) {
        Band band;
        band.name = query.value(0).toString();
        band.start = query.value(1).toDouble();
        band.end = query.value(2).toDouble();
        return band;
    }
    else {
        return Band();
    }
}

QList<Band> Data::bandsList(const bool onlyDXCCBands, const bool onlyEnabled)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << onlyDXCCBands << onlyEnabled;

    QSqlQuery query;
    QList<Band> ret;

    QString stmt = QString(" SELECT name, start_freq, end_freq FROM bands WHERE 1 = 1 ");

    if ( onlyEnabled )
    {
        stmt.append("AND enabled = 1 ");
    }

    if ( onlyDXCCBands )
    {
        stmt.append("AND ((1.9 between start_freq and end_freq) "
                    "      OR (3.6 between start_freq and end_freq) "
                    "      OR (7.1 between start_freq and end_freq) "
                    "      OR (10.1 between start_freq and end_freq) "
                    "      OR (14.1 between start_freq and end_freq) "
                    "      OR (18.1 between start_freq and end_freq) "
                    "      OR (21.1 between start_freq and end_freq) "
                    "      OR (24.9 between start_freq and end_freq) "
                    "      OR (28.1 between start_freq and end_freq) "
                    "      OR (50.1 between start_freq and end_freq) "
                    "      OR (145.1 between start_freq and end_freq) "
                    "      OR (421.1 between start_freq and end_freq) "
                    "      OR (1241.0 between start_freq and end_freq) "
                    "      OR (2301.0 between start_freq and end_freq) "
                    "      OR (10001.0 between start_freq and end_freq)) ");
    }

    stmt.append("ORDER BY start_freq ");

    qCDebug(runtime) << stmt;

    if ( ! query.prepare(stmt) )
    {
        qWarning() << "Cannot prepare Select statement";
        return ret;
    }

    if ( ! query.exec() )
    {
        qWarning() << "Cannot execute select statement" << query.lastError();
        return ret;
    }

    while ( query.next() )
    {
        Band band;
        band.name = query.value(0).toString();
        band.start = query.value(1).toDouble();
        band.end = query.value(2).toDouble();
        ret << band;
    }

    return ret;
}

QString Data::freqToDXCCMode(double freq)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << freq;

    // currectly only IARU Region 1 is implemented
    // https://www.iaru-r1.org/wp-content/uploads/2019/08/hf_r1_bandplan.pdf
    // https://www.oevsv.at/export/shared/.content/.galleries/pdf-Downloads/OVSV-Bandplan_05-2019.pdf

    // 2200m
    if (freq >= 0.1357 && freq <= 0.1378) return Data::MODE_CW;

    // 630m
    else if (freq >= 0.472 && freq <= 0.475) return Data::MODE_CW;
    else if (freq <= 0.479) return Data::MODE_DIGITAL;

    // 160m
    else if ( freq >= 1.800 && freq < 2.000 )
    {
        if (freq >= 1.800 && freq < 1.838) return Data::MODE_CW;
        else if (freq >= 1.838 && freq < 1.840) return Data::MODE_DIGITAL;
        else if (freq >= 1.840 && freq < 1.842) return Data::MODE_FT8;
        else if (freq >= 1.842 && freq < 2.000) return Data::MODE_LSB;
    }
    // 80m
    else if (freq > 3.500 && freq < 4.000)
    {
        if (freq > 3.500 && freq < 3.573) return Data::MODE_CW;
        else if (freq >= 3.573 && freq < 3.575) return Data::MODE_FT8;
        else if (freq >= 3.575 && freq < 3.580) return Data::MODE_CW;
        else if (freq >= 3.580 && freq < 3.620) return Data::MODE_DIGITAL;
        else if (freq >= 3.620 && freq < 4.000) return Data::MODE_LSB;
    }
    // 60m
    else if (freq >= 5.3515 && freq <= 5.354) return Data::MODE_CW;
    else if (freq <= 5.500) return Data::MODE_LSB;

    // 40m
    else if (freq >= 7.000 && freq < 7.300)
    {
        if (freq >= 7.000 && freq < 7.040) return Data::MODE_CW;
        else if (freq >= 7.040 && freq < 7.053) return Data::MODE_DIGITAL;
        else if (freq >= 7.053 && freq < 7.074) return Data::MODE_LSB;
        else if (freq >= 7.074 && freq < 7.076) return Data::MODE_FT8;
        else if (freq >= 7.076 && freq < 7.300) return Data::MODE_LSB;
    }
    // 30m
    else if (freq >= 10.100 && freq < 10.150)
    {
        if (freq >= 10.100 && freq < 10.136) return Data::MODE_CW;
        else if (freq >= 10.136 && freq < 10.138) return Data::MODE_FT8;
        else if (freq >= 10.138 && freq < 10.140) return Data::MODE_CW;
        else if (freq >= 10.140 && freq < 10.150) return Data::MODE_DIGITAL;
    }
    // 20m
    else if (freq >= 14.000 && freq < 14.350)
    {
        if (freq >= 14.000 && freq < 14.070) return Data::MODE_CW;
        else if (freq >= 14.070 && freq < 14.074) return Data::MODE_DIGITAL;
        else if (freq >= 14.074 && freq < 14.076) return Data::MODE_FT8;
        else if (freq >= 14.076 && freq < 14.098) return Data::MODE_DIGITAL;
        else if (freq >= 14.098 && freq < 14.112) return Data::MODE_CW;
        else if (freq >= 14.112 && freq < 14.350) return Data::MODE_USB;
    }
    // 17m
    else if (freq >= 18.068 && freq < 18.268)
    {
        if (freq >= 18.068 && freq < 18.095) return Data::MODE_CW;
        else if (freq >= 18.095 && freq < 18.100) return Data::MODE_DIGITAL;
        else if (freq >= 18.100 && freq < 18.102) return Data::MODE_FT8;
        else if (freq >= 18.102 && freq < 18.109) return Data::MODE_DIGITAL;
        else if (freq >= 18.109 && freq < 18.120) return Data::MODE_CW;
        else if (freq >= 18.120 && freq < 18.268) return Data::MODE_USB;
    }
    // 15m
    else if (freq >= 21.000 && freq < 21.450)
    {
        if (freq >= 21.000 && freq < 21.070) return Data::MODE_CW;
        else if (freq >= 21.070 && freq < 21.074) return Data::MODE_DIGITAL;
        else if (freq >= 21.074 && freq < 21.076) return Data::MODE_FT8;
        else if (freq >= 21.076 && freq < 21.149) return Data::MODE_DIGITAL;
        else if (freq >= 21.149 && freq < 21.151) return Data::MODE_CW;
        else if (freq >= 21.151 && freq < 21.450) return Data::MODE_USB;
    }
    // 12m
    else if (freq >= 24.890 && freq < 24.990)
    {
        if (freq >= 24.890 && freq < 24.915) return Data::MODE_CW;
        else if (freq >= 24.915 && freq < 24.917) return Data::MODE_FT8;
        else if (freq >= 24.917 && freq < 24.929) return Data::MODE_DIGITAL;
        else if (freq >= 24.929 && freq < 24.940) return Data::MODE_CW;
        else if (freq >= 24.940 && freq < 24.990) return Data::MODE_USB;
    }
    // 10m
    else if (freq >= 28.000 && freq < 29.700)
    {
        if (freq >= 28.000 && freq < 28.074) return Data::MODE_CW;
        else if (freq >= 28.074 && freq < 28.076) return Data::MODE_FT8;
        else if (freq >= 28.076 && freq < 28.150) return Data::MODE_DIGITAL;
        else if (freq >= 28.150 && freq < 28.320) return Data::MODE_CW;
        else if (freq >= 28.320 && freq < 29.700) return Data::MODE_USB;
    }
    // 6m
    else if (freq >= 50.000 && freq < 54.000)
    {
        if (freq >= 50.000 && freq < 50.100) return Data::MODE_CW;
        else if (freq >= 50.100 && freq < 50.313) return Data::MODE_USB;
        else if (freq >= 50.313 && freq < 50.315) return Data::MODE_FT8;
        else if (freq >= 50.315 && freq < 50.400) return Data::MODE_DIGITAL;
        else if (freq >= 50.400 && freq < 50.500) return Data::MODE_CW;
        else if (freq >= 50.500 && freq < 54.000) return Data::MODE_PHONE;
    }
    // 4m
    else if (freq >=70.000 && freq < 70.500)
    {
        if (freq >=70.000 && freq < 70.100) return Data::MODE_CW;
        else if (freq >= 70.100 && freq < 70.102) return Data::MODE_FT8;
        else if (freq >= 70.102 && freq < 70.250) return Data::MODE_USB;
        else if (freq >=70.2500 && freq < 70.500) return Data::MODE_USB;
    }
    // 2m
    else if (freq >= 144.000 && freq < 148.000)
    {
        if (freq >= 144.000 && freq < 144.100) return Data::MODE_CW;
        else if (freq >= 144.100 && freq < 144.174) return Data::MODE_USB;
        else if (freq >= 144.174 && freq <= 144.176) return Data::MODE_FT8;
        else if (freq > 144.176 && freq < 148.000) return Data::MODE_USB;
    }
    // 1.25m
    else if (freq >= 222.0 && freq < 222.150) return Data::MODE_CW;
    else if (freq >= 222.150 && freq < 225.00) return Data::MODE_USB;

    // 70cm
    else if (freq >= 430.0 && freq < 432.0) return Data::MODE_USB;
    else if (freq >= 432.0 && freq < 432.10) return Data::MODE_CW;
    else if (freq >= 432.1 && freq < 440.0) return Data::MODE_USB;

    // 33cm
    else if (freq >= 902.0 && freq < 928.0) return Data::MODE_USB;

    // 23cm
    else if (freq >= 1240.0 && freq < 1296.15) return Data::MODE_USB;
    else if (freq >= 1296.15 && freq < 1296.4) return Data::MODE_CW;
    else if (freq >= 1296.4 && freq < 1300.0) return Data::MODE_PHONE;

    return QString();
}

QString Data::modeToDXCCMode(const QString &mode)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << mode;

    QSqlQuery tmpQuery;
    QString ret;

    if ( tmpQuery.prepare("SELECT modes.dxcc FROM modes WHERE modes.name = :mode LIMIT 1"))
    {
        tmpQuery.bindValue(":mode", mode);

        if (tmpQuery.exec())
        {
            tmpQuery.next();
            ret = tmpQuery.value(0).toString();
        }
    }

    return ret;
}

QColor Data::statusToColor(const DxccStatus &status, const QColor &defaultColor) {
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << status;

    switch (status) {
        case DxccStatus::NewEntity:
            return QColor(255, 58, 9);
        case DxccStatus::NewBand:
        case DxccStatus::NewMode:
        case DxccStatus::NewBandMode:
            return QColor(76, 200, 80);
        case DxccStatus::NewSlot:
            return QColor(30, 180, 230);
        case DxccStatus::Worked:
            return QColor(255,165,0);
        default:
            return defaultColor;
    }
}

QString Data::colorToHTMLColor(const QColor &in_color)
{
    FCT_IDENTIFICATION;

    return in_color.name(QColor::HexRgb);
}

QString Data::statusToText(const DxccStatus &status) {
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << status;

    switch (status) {
        case DxccStatus::NewEntity:
            return tr("New Entity");
        case DxccStatus::NewBand:
            return tr("New Band");
        case DxccStatus::NewMode:
            return tr("New Mode");
        case DxccStatus::NewBandMode:
            return tr("New Band&Mode");
        case DxccStatus::NewSlot:
            return tr("New Slot");
        case DxccStatus::Confirmed:
            return tr("Confirmed");
        case DxccStatus::Worked:
            return tr("Worked");
        default:
            return QString("Unknown");
    }
}

QString Data::removeAccents(const QString &input)
{
    FCT_IDENTIFICATION;
    /* http://archives.miloush.net/michkap/archive/2007/05/14/2629747.html */
    /* https://www.medo64.com/2020/10/stripping-diacritics-in-qt/ */
    /* More about normalization https://unicode.org/reports/tr15/ */

    QString formD = input.normalized(QString::NormalizationForm_D);

    QString filtered;
    for (int i = 0; i < formD.length(); i++)
    {
        if (formD.at(i).category() != QChar::Mark_NonSpacing)
        {
            filtered.append(formD.at(i));
        }
    }

    QString ret = filtered.normalized(QString::NormalizationForm_C).toLatin1().replace('?',"");

    /* If stripped string is empty then QString to store NULL value do DB */
    if ( ret.length() == 0 )
    {
        ret = QString();
    }
    return ret;

}

int Data::getITUZMin()
{
    FCT_IDENTIFICATION;

    qCDebug(runtime) << 1;

    return 1;
}

int Data::getITUZMax()
{
    FCT_IDENTIFICATION;

    qCDebug(runtime) << 90;

    return 90;
}

int Data::getCQZMin()
{
    FCT_IDENTIFICATION;

    qCDebug(runtime) << 1;

    return 1;
}

int Data::getCQZMax()
{
    FCT_IDENTIFICATION;

    qCDebug(runtime) << 40;

    return 40;
}

QString Data::dbFilename()
{
    FCT_IDENTIFICATION;

    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    return dir.filePath("qlog.db");
}

QPair<QString, QString> Data::legacyMode(const QString &mode)
{
    FCT_IDENTIFICATION;

    // used in the case of external programs that generate an invalid ADIF modes.
    // Database Mode Table cannot be used because these programs have different mode-strings.

    qCDebug(function_parameters) << mode;
    return legacyModes.value(mode);
}

QString Data::getIANATimeZone(double lat, double lon)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << lat << lon;

    QString ret;

    if ( zd )
    {
        ret = ZDHelperSimpleLookupString(zd,
                                         static_cast<float>(lat),
                                         static_cast<float>(lon));
    }

    qCDebug(runtime) << ret;
    return ret;
}

void Data::loadContests() {
    FCT_IDENTIFICATION;

    QFile file(":/res/data/contests.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray data = file.readAll();

    auto objectList = QJsonDocument::fromJson(data).toVariant().toList();
    for (auto &object : qAsConst(objectList))
    {
        QVariantMap contestData = object.toMap();

        QString id = contestData.value("id").toString();
        QString name = contestData.value("name").toString();

        contests.insert(id, name);
    }
}

void Data::loadPropagationModes() {
    FCT_IDENTIFICATION;

    QFile file(":/res/data/propagation_modes.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray data = file.readAll();

    auto objects = QJsonDocument::fromJson(data).toVariant().toList();
    for (auto &object : qAsConst(objects))
    {
        QVariantMap propagationModeData = object.toMap();

        QString id = propagationModeData.value("id").toString();
        QString name = propagationModeData.value("name").toString();

        propagationModes.insert(id, name);
    }
}

void Data::loadLegacyModes()
{
    FCT_IDENTIFICATION;

    // Load conversion table from non-ADIF mode to ADIF mode/submode
    // used in the case of external programs that generate an invalid ADIF modes.
    // Database Mode Table cannot be used because these programs have different mode-strings.

    QFile file(":/res/data/legacy_modes.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray data = file.readAll();

    QVariantMap modes = QJsonDocument::fromJson(data).toVariant().toMap();
    auto keys = modes.keys();

    for (auto &key : qAsConst(keys))
    {
        QVariantMap legacyModeData = modes[key].toMap();

        QString mode = legacyModeData.value("mode").toString();
        QString submode = legacyModeData.value("submode").toString();
        QPair<QString, QString> modes = QPair<QString, QString>(mode, submode);

        legacyModes.insert(key, modes);
    }
}

void Data::loadDxccFlags()
{
    FCT_IDENTIFICATION;

    QFile file(":/res/data/dxcc.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray data = file.readAll();

    auto objects = QJsonDocument::fromJson(data).toVariant().toList();
    for (auto &object : qAsConst(objects))
    {
        QVariantMap dxccData = object.toMap();

        int id = dxccData.value("id").toInt();
        QString flag = dxccData.value("flag").toString();

        flags.insert(id, flag);
    }
}

void Data::loadSatModes()
{
    FCT_IDENTIFICATION;

    QFile file(":/res/data/sat_modes.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray data = file.readAll();

    auto objects = QJsonDocument::fromJson(data).toVariant().toList();
    for (auto &object : qAsConst(objects))
    {
        QVariantMap satModesData = object.toMap();

        QString id = satModesData.value("id").toString();
        QString name = satModesData.value("name").toString();

        satModes.insert(id, name);
    }
}

void Data::loadIOTA()
{
    FCT_IDENTIFICATION;

    QSqlQuery query("SELECT iotaid, islandname FROM iota");

    while ( query.next() )
    {
        QString iotaID = query.value(0).toString();
        QString islandName = query.value(1).toString();
        iotaRef.insert(iotaID, islandName);
    }
}

void Data::loadSOTA()
{
    FCT_IDENTIFICATION;

    QSqlQuery query("SELECT summit_code FROM sota_summits");

    while ( query.next() )
    {
        QString summitCode = query.value(0).toString();
        sotaRefID.insert(summitCode, QString());
    }
}

void Data::loadWWFF()
{
    QSqlQuery query("SELECT reference FROM wwff_directory");

    while ( query.next() )
    {
        QString reference = query.value(0).toString();
        wwffRefID.insert(reference, QString());
    }
}

void Data::loadPOTA()
{
    FCT_IDENTIFICATION;

    QSqlQuery query("SELECT reference FROM pota_directory");

    while ( query.next() )
    {
        QString reference = query.value(0).toString();
        potaRefID.insert(reference, QString());
    }
}

void Data::loadTZ()
{
    FCT_IDENTIFICATION;

    QFile file (":/res/data/timezone21.bin");
    file.open(QIODevice::ReadOnly);
    uchar *tzMap = file.map(0, file.size());

    if ( tzMap )
    {
        zd = ZDOpenDatabaseFromMemory(tzMap, file.size());
        if ( !zd )
        {
            qWarning() << "Cannot open TZ Database";
        }
    }
    else
    {
        qWarning() << "Cannot map TZ File to memory";
    }

}

DxccEntity Data::lookupDxcc(const QString &callsign)
{
    FCT_IDENTIFICATION;
    static QCache<QString, DxccEntity> localCache(1000);

    qCDebug(function_parameters) << callsign;

    if ( callsign.isEmpty())
        return  DxccEntity();

    DxccEntity dxccRet;
    DxccEntity * dxccCached = localCache.object(callsign);

    if ( dxccCached )
    {
        dxccRet.dxcc = dxccCached->dxcc;
        dxccRet.country = dxccCached->country;
        dxccRet.prefix = dxccCached->prefix;
        dxccRet.cont = dxccCached->cont;
        dxccRet.cqz = dxccCached->cqz;
        dxccRet.ituz = dxccCached->ituz;
        dxccRet.latlon[0] = dxccCached->latlon[0];
        dxccRet.latlon[1] = dxccCached->latlon[1];
        dxccRet.tz = dxccCached->tz;
        dxccRet.flag = dxccCached->flag;
    }
    else
    {
        if ( ! isDXCCQueryValid )
        {
            qWarning() << "Cannot prepare Select statement";
            return DxccEntity();
        }

        QString lookupPrefix = callsign; // use the callsign with optional prefix as default to find the dxcc
        Callsign parsedCallsign(callsign); // use Callsign to split the callsign into its parts

        if ( parsedCallsign.isValid() )
        {
            QString suffix = parsedCallsign.getSuffix();
            if ( suffix.length() == 1 ) // some countries add single numbers as suffix to designate a call area, e.g. /4
            {
                bool isNumber = false;
                (void)suffix.toInt(&isNumber);
                if ( isNumber )
                {
                    lookupPrefix = parsedCallsign.getBasePrefix() + suffix; // use the call prefix and the number from the suffix to find the dxcc
                }
            }
            else if ( suffix.length() > 1
                      && !parsedCallsign.secondarySpecialSuffixes.contains(suffix) ) // if there is more than one character and it is not one of the special suffixes, we definitely have a call prefix as suffix
            {
                lookupPrefix = suffix;
            }
        }

        queryDXCC.bindValue(":callsign", lookupPrefix);

        if ( ! queryDXCC.exec() )
        {
            qWarning() << "Cannot execute Select statement" << queryDXCC.lastError() << queryDXCC.lastQuery();
            return DxccEntity();
        }
        if (queryDXCC.next())
        {
            dxccRet.dxcc = queryDXCC.value(0).toInt();
            dxccRet.country = queryDXCC.value(1).toString();
            dxccRet.prefix = queryDXCC.value(2).toString();
            dxccRet.cont = queryDXCC.value(3).toString();
            dxccRet.cqz = queryDXCC.value(4).toInt();
            dxccRet.ituz = queryDXCC.value(5).toInt();
            dxccRet.latlon[0] = queryDXCC.value(6).toDouble();
            dxccRet.latlon[1] = queryDXCC.value(7).toDouble();
            dxccRet.tz = queryDXCC.value(8).toFloat();
            dxccRet.flag = flags.value(dxccRet.dxcc);

            dxccCached = new DxccEntity;

            if ( dxccCached )
            {

                dxccCached->dxcc = dxccRet.dxcc;
                dxccCached->country = dxccRet.country;
                dxccCached->prefix = dxccRet.prefix;
                dxccCached->cont = dxccRet.cont;
                dxccCached->cqz = dxccRet.cqz;
                dxccCached->ituz = dxccRet.ituz;
                dxccCached->latlon[0] = dxccRet.latlon[0];
                dxccCached->latlon[1] = dxccRet.latlon[1];
                dxccCached->tz = dxccRet.tz;
                dxccCached->flag = dxccRet.flag;

                localCache.insert(callsign, dxccCached);
            }
        }
        else
        {
            dxccRet.dxcc = 0;
            dxccRet.ituz = 0;
            dxccRet.cqz = 0;
            dxccRet.tz = 0;
        }
    }

    return dxccRet;
}

SOTAEntity Data::lookupSOTA(const QString &SOTACode)
{
    FCT_IDENTIFICATION;

    if ( ! isSOTAQueryValid )
    {
        qWarning() << "Cannot prepare Select statement";
        return SOTAEntity();
    }

    querySOTA.bindValue(":code", SOTACode);

    if ( ! querySOTA.exec() )
    {
        qWarning() << "Cannot execte Select statement" << querySOTA.lastError();
        return SOTAEntity();
    }

    SOTAEntity SOTARet;

    if (querySOTA.next())
    {
        SOTARet.summitCode = querySOTA.value(0).toString();
        SOTARet.associationName = querySOTA.value(1).toString();
        SOTARet.regionName = querySOTA.value(2).toString();
        SOTARet.summitName = querySOTA.value(3).toString();
        SOTARet.altm = querySOTA.value(4).toInt();
        SOTARet.altft = querySOTA.value(5).toInt();
        SOTARet.gridref1 = querySOTA.value(6).toDouble();
        SOTARet.gridref2 = querySOTA.value(7).toDouble();
        SOTARet.longitude = querySOTA.value(8).toDouble();
        SOTARet.latitude = querySOTA.value(9).toDouble();
        SOTARet.points = querySOTA.value(10).toInt();
        SOTARet.bonusPoints = querySOTA.value(11).toInt();
        SOTARet.validFrom = querySOTA.value(12).toDate();
        SOTARet.validTo = querySOTA.value(13).toDate();
    }
    else
    {
        SOTARet.altft = 0;
        SOTARet.altm  = 0;
        SOTARet.gridref1 = 0.0;
        SOTARet.gridref2  = 0.0;
        SOTARet.longitude = 0.0;
        SOTARet.latitude  = 0.0;
        SOTARet.points = 0;
        SOTARet.bonusPoints  = 0;
    }

    return SOTARet;
}

POTAEntity Data::lookupPOTA(const QString &POTACode)
{
    FCT_IDENTIFICATION;

    if ( ! isPOTAQueryValid )
    {
        qWarning() << "Cannot prepare Select statement";
        return POTAEntity();
    }

    queryPOTA.bindValue(":code", POTACode);

    if ( ! queryPOTA.exec() )
    {
        qWarning() << "Cannot execte Select statement" << queryPOTA.lastError();
        return POTAEntity();
    }

    POTAEntity POTARet;

    if (queryPOTA.next())
    {
        POTARet.reference = queryPOTA.value(0).toString();
        POTARet.name = queryPOTA.value(1).toString();
        POTARet.active = queryPOTA.value(2).toBool();
        POTARet.entityID = queryPOTA.value(3).toInt();
        POTARet.locationDesc = queryPOTA.value(4).toString();
        POTARet.longitude = queryPOTA.value(5).toDouble();
        POTARet.latitude = queryPOTA.value(6).toDouble();
        POTARet.grid = queryPOTA.value(7).toString();
    }
    else
    {
        POTARet.active = false;
        POTARet.entityID = 0;
        POTARet.longitude = 0.0;
        POTARet.latitude  = 0.0;
    }

    return POTARet;
}

WWFFEntity Data::lookupWWFF(const QString &reference)
{
    FCT_IDENTIFICATION;

    if ( ! isWWFFQueryValid )
    {
        qWarning() << "Cannot prepare Select statement";
        return WWFFEntity();
    }

    queryWWFF.bindValue(":reference", reference);

    if ( ! queryWWFF.exec() )
    {
        qWarning() << "Cannot execte Select statement" << queryWWFF.lastError();
        return WWFFEntity();
    }

    WWFFEntity WWFFRet;

    if (queryWWFF.next())
    {
        WWFFRet.reference = queryWWFF.value(0).toString();
        WWFFRet.status = queryWWFF.value(1).toString();
        WWFFRet.name = queryWWFF.value(2).toString();
        WWFFRet.program = queryWWFF.value(3).toString();
        WWFFRet.dxcc = queryWWFF.value(4).toString();
        WWFFRet.state = queryWWFF.value(5).toString();
        WWFFRet.county = queryWWFF.value(6).toString();
        WWFFRet.continent = queryWWFF.value(7).toString();
        WWFFRet.iota = queryWWFF.value(8).toString();
        WWFFRet.iaruLocator = queryWWFF.value(9).toString();
        WWFFRet.latitude = queryWWFF.value(10).toDouble();
        WWFFRet.longitude = queryWWFF.value(11).toDouble();
        WWFFRet.iucncat = queryWWFF.value(12).toString();
        WWFFRet.validFrom = queryWWFF.value(13).toDate();
        WWFFRet.validTo = queryWWFF.value(14).toDate();
    }
    else
    {
        WWFFRet.longitude = 0.0;
        WWFFRet.latitude  = 0.0;
    }

    return WWFFRet;
}

QString Data::dxccFlag(int dxcc) {
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << dxcc;
    return flags.value(dxcc);
}

const QString Data::MODE_CW = "CW";
const QString Data::MODE_DIGITAL = "DIGITAL";
const QString Data::MODE_FT8 = "FT8";
const QString Data::MODE_LSB = "PHONE"; // use just generic label
const QString Data::MODE_USB = "PHONE"; // use just generic label
const QString Data::MODE_PHONE = "PHONE";
