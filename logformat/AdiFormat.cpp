#include <QSqlRecord>
#include <QDebug>
#include "data/Data.h"
#include "AdiFormat.h"
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.logformat.adiformat");

#define ALWAYS_PRESENT true

void AdiFormat::exportStart()
{
    FCT_IDENTIFICATION;

    stream << "### QLog ADIF Export\n";
    writeField("ADIF_VER", ALWAYS_PRESENT, ADIF_VERSION_STRING);
    writeField("PROGRAMID", ALWAYS_PRESENT, PROGRAMID_STRING);
    writeField("PROGRAMVERSION", ALWAYS_PRESENT, VERSION);
    writeField("CREATED_TIMESTAMP", ALWAYS_PRESENT,
               QDateTime::currentDateTimeUtc().toString("yyyyMMdd hhmmss"));
    stream << "<EOH>\n\n";
}

void AdiFormat::exportContact(const QSqlRecord& record,
                              QMap<QString, QString> *applTags)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters)<<record;

    writeSQLRecord(record, applTags);

    stream << "<eor>\n\n";
}

void AdiFormat::writeField(const QString &name, bool presenceCondition,
                           const QString &value, const QString &type)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters)<< name
                                << presenceCondition
                                << value
                                << type;

    if (!presenceCondition) return;

    /* ADIF does not support UTF-8 characterset therefore the Accents are remove */
    QString accentless(Data::removeAccents(value));

    qCDebug(runtime) << "Accentless: " << accentless;

    if ( value.isEmpty() || accentless.isEmpty() ) return;

    stream << "<" << name << ":" << accentless.size();

    if (!type.isEmpty()) stream << ":" << type;

    stream << ">" << accentless << '\n';
}

void AdiFormat::writeSQLRecord(const QSqlRecord &record,
                               QMap<QString, QString> *applTags)
{
    FCT_IDENTIFICATION;

    QDateTime time_start = record.value("start_time").toDateTime().toTimeSpec(Qt::UTC);
    QDateTime time_end = record.value("end_time").toDateTime().toTimeSpec(Qt::UTC);

    writeField("call", record.value("callsign").isValid(),
               record.value("callsign").toString());
    writeField("qso_date", record.value("start_time").isValid(),
               time_start.toString("yyyyMMdd"), "D");
    writeField("time_on", record.value("start_time").isValid(),
               time_start.toString("hhmmss"), "T");
    writeField("qso_date_off", record.value("end_time").isValid(),
               time_end.toString("yyyyMMdd"), "D");
    writeField("time_off", record.value("end_time").isValid(),
               time_end.toString("hhmmss"), "T");
    writeField("rst_rcvd", record.value("rst_rcvd").isValid(),
               record.value("rst_rcvd").toString());
    writeField("rst_sent", record.value("rst_sent").isValid(),
               record.value("rst_sent").toString());
    writeField("name", record.value("name").isValid(),
               record.value("name").toString());
    writeField("qth", record.value("qth").isValid(),
               record.value("qth").toString());
    writeField("gridsquare", record.value("gridsquare").isValid(),
               record.value("gridsquare").toString());
    writeField("cqz", record.value("cqz").isValid(),
               record.value("cqz").toString());
    writeField("ituz", record.value("ituz").isValid(),
               record.value("ituz").toString());
    writeField("freq", record.value("freq").isValid(),
               record.value("freq").toString(), "N");
    writeField("band", record.value("band").isValid(),
               record.value("band").toString().toLower());
    writeField("mode", record.value("mode").isValid(),
               record.value("mode").toString());
    writeField("submode", record.value("submode").isValid(),
               record.value("submode").toString());
    writeField("cont", record.value("cont").isValid(),
               record.value("cont").toString());
    writeField("dxcc", record.value("dxcc").isValid(),
               record.value("dxcc").toString());
    writeField("country", record.value("country").isValid(),
               record.value("country").toString());
    writeField("pfx", record.value("pfx").isValid(),
               record.value("pfx").toString());
    writeField("state", record.value("state").isValid(),
               record.value("state").toString());
    writeField("cnty", record.value("cnty").isValid(),
               record.value("cnty").toString());
    writeField("iota", record.value("iota").isValid(),
               record.value("iota").toString().toUpper());
    writeField("qsl_rcvd", record.value("qsl_rcvd").isValid(),
               record.value("qsl_rcvd").toString());
    writeField("qslrdate", record.value("qsl_rdate").isValid(),
               record.value("qsl_rdate").toDate().toString("yyyyMMdd"));
    writeField("qsl_sent", record.value("qsl_sent").isValid(),
               record.value("qsl_sent").toString());
    writeField("qslsdate", record.value("qsl_sdate").isValid(),
               record.value("qsl_sdate").toDate().toString("yyyyMMdd"));
    writeField("lotw_qsl_rcvd", record.value("lotw_qsl_rcvd").isValid(),
               record.value("lotw_qsl_rcvd").toString());
    writeField("lotw_qslrdate", record.value("lotw_qslrdate").isValid(),
               record.value("lotw_qslrdate").toDate().toString("yyyyMMdd"));
    writeField("lotw_qsl_sent", record.value("lotw_qsl_sent").isValid(),
               record.value("lotw_qsl_sent").toString());
    writeField("lotw_qslsdate", record.value("lotw_qslsdate").isValid(),
               record.value("lotw_qslsdate").toDate().toString("yyyyMMdd"));
    writeField("tx_pwr", record.value("tx_pwr").isValid(),
               record.value("tx_pwr").toString());
    writeField("address", record.value("address").isValid(),
               record.value("address").toString());
    writeField("age", record.value("age").isValid(),
               record.value("age").toString());
    writeField("altitude", record.value("altitude").isValid(),
               record.value("altitude").toString());
    writeField("a_index", record.value("a_index").isValid(),
               record.value("a_index").toString());
    writeField("ant_az", record.value("ant_az").isValid(),
               record.value("ant_az").toString());
    writeField("ant_el", record.value("ant_el").isValid(),
               record.value("ant_el").toString());
    writeField("ant_path", record.value("ant_path").isValid(),
               record.value("ant_path").toString());
    writeField("arrl_sect", record.value("arrl_sect").isValid(),
               record.value("arrl_sect").toString());
    writeField("award_submitted", record.value("award_submitted").isValid(),
               record.value("award_submitted").toString());
    writeField("award_granted", record.value("award_granted").isValid(),
               record.value("award_granted").toString());
    writeField("band_rx", record.value("band_rx").isValid(),
               record.value("band_rx").toString().toLower());
    writeField("check", record.value("check").isValid(),
               record.value("check").toString());
    writeField("class", record.value("class").isValid(),
               record.value("class").toString());
    writeField("clublog_qso_upload_date", record.value("clublog_qso_upload_date").isValid(),
               record.value("clublog_qso_upload_date").toDate().toString("yyyyMMdd"));
    writeField("clublog_qso_upload_status", record.value("clublog_qso_upload_status").isValid(),
               record.value("clublog_qso_upload_status").toString());
    writeField("comment", record.value("comment").isValid(),
               record.value("comment").toString());
    writeField("contacted_op", record.value("contacted_op").isValid(),
               record.value("contacted_op").toString());
    writeField("contest_id", record.value("contest_id").isValid(),
               record.value("contest_id").toString());
    writeField("credit_submitted", record.value("credit_submitted").isValid(),
               record.value("credit_submitted").toString());
    writeField("credit_granted", record.value("credit_granted").isValid(),
               record.value("credit_granted").toString());
    writeField("darc_dok", record.value("darc_dok").isValid(),
               record.value("darc_dok").toString());
    writeField("distance", record.value("distance").isValid(),
               record.value("distance").toString());
    writeField("email", record.value("email").isValid(),
               record.value("email").toString());
    writeField("eq_call", record.value("eq_call").isValid(),
               record.value("eq_call").toString());
    writeField("eqsl_qslrdate", record.value("eqsl_qslrdate").isValid(),
               record.value("eqsl_qslrdate").toDate().toString("yyyyMMdd"));
    writeField("eqsl_qslsdate", record.value("eqsl_qslsdate").isValid(),
               record.value("eqsl_qslsdate").toDate().toString("yyyyMMdd"));
    writeField("eqsl_qsl_rcvd", record.value("eqsl_qsl_rcvd").isValid(),
               record.value("eqsl_qsl_rcvd").toString());
    writeField("eqsl_qsl_sent", record.value("eqsl_qsl_sent").isValid(),
               record.value("eqsl_qsl_sent").toString());
    writeField("fists", record.value("fists").isValid(),
               record.value("fists").toString());
    writeField("fists_cc", record.value("fists_cc").isValid(),
               record.value("fists_cc").toString());
    writeField("force_init", record.value("force_init").isValid(),
               record.value("force_init").toString());
    writeField("freq_rx", record.value("freq_rx").isValid(),
               record.value("freq_rx").toString());
    writeField("gridsquare_ext", record.value("gridsquare_ext").isValid(),
               record.value("gridsquare_ext").toString());
    writeField("guest_op", record.value("guest_op").isValid(),
               record.value("guest_op").toString());
    writeField("hamlogeu_qso_upload_date", record.value("hamlogeu_qso_upload_date").isValid(),
               record.value("hamlogeu_qso_upload_date").toDate().toString("yyyyMMdd"));
    writeField("hamlogeu_qso_upload_status", record.value("hamlogeu_qso_upload_status").isValid(),
               record.value("hamlogeu_qso_upload_status").toString());
    writeField("hamqth_qso_upload_date", record.value("hamqth_qso_upload_date").isValid(),
               record.value("hamqth_qso_upload_date").toDate().toString("yyyyMMdd"));
    writeField("hamqth_qso_upload_status", record.value("hamqth_qso_upload_status").isValid(),
               record.value("hamqth_qso_upload_status").toString());
    writeField("hrdlog_qso_upload_date", record.value("hrdlog_qso_upload_date").isValid(),
               record.value("hrdlog_qso_upload_date").toDate().toString("yyyyMMdd"));
    writeField("hrdlog_qso_upload_status", record.value("hrdlog_qso_upload_status").isValid(),
               record.value("hrdlog_qso_upload_status").toString());
    writeField("iota_island_id", record.value("iota_island_id").isValid(),
               record.value("iota_island_id").toString().toUpper());
    writeField("k_index", record.value("k_index").isValid(),
               record.value("k_index").toString());
    writeField("lat", record.value("lat").isValid(),
               record.value("lat").toString());
    writeField("lon", record.value("lon").isValid(),
               record.value("lon").toString());
    writeField("max_bursts", record.value("max_bursts").isValid(),
               record.value("max_bursts").toString());
    writeField("ms_shower", record.value("ms_shower").isValid(),
               record.value("ms_shower").toString());
    writeField("my_altitude", record.value("my_altitude").isValid(),
               record.value("my_altitude").toString());
    writeField("my_arrl_sect", record.value("my_arrl_sect").isValid(),
               record.value("my_arrl_sect").toString());
    writeField("my_antenna", record.value("my_antenna").isValid(),
               record.value("my_antenna").toString());
    writeField("my_city", record.value("my_city").isValid(),
               record.value("my_city").toString());
    writeField("my_cnty", record.value("my_cnty").isValid(),
               record.value("my_cnty").toString());
    writeField("my_country", record.value("my_country").isValid(),
               record.value("my_country").toString());
    writeField("my_cq_zone", record.value("my_cq_zone").isValid(),
               record.value("my_cq_zone").toString());
    writeField("my_dxcc", record.value("my_dxcc").isValid(),
               record.value("my_dxcc").toString());
    writeField("my_fists", record.value("my_fists").isValid(),
               record.value("my_fists").toString());
    writeField("my_gridsquare", record.value("my_gridsquare").isValid(),
               record.value("my_gridsquare").toString());
    writeField("my_gridsquare_ext", record.value("my_gridsquare_ext").isValid(),
               record.value("my_gridsquare_ext").toString());
    writeField("my_iota", record.value("my_iota").isValid(),
               record.value("my_iota").toString().toUpper());
    writeField("my_iota_island_id", record.value("my_iota_island_id").isValid(),
               record.value("my_iota_island_id").toString().toUpper());
    writeField("my_itu_zone", record.value("my_itu_zone").isValid(),
               record.value("my_itu_zone").toString());
    writeField("my_lat", record.value("my_lat").isValid(),
               record.value("my_lat").toString());
    writeField("my_lon", record.value("my_lon").isValid(),
               record.value("my_lon").toString());
    writeField("my_name", record.value("my_name").isValid(),
               record.value("my_name").toString());
    writeField("my_postal_code", record.value("my_postal_code").isValid(),
               record.value("my_postal_code").toString());
    writeField("my_pota_ref", record.value("my_pota_ref").isValid(),
               record.value("my_pota_ref").toString().toUpper());
    writeField("my_rig", record.value("my_rig").isValid(),
               record.value("my_rig").toString());
    writeField("my_sig", record.value("my_sig").isValid(),
               record.value("my_sig").toString());
    writeField("my_sig_info", record.value("my_sig_info").isValid(),
               record.value("my_sig_info").toString());
    writeField("my_sota_ref", record.value("my_sota_ref").isValid(),
               record.value("my_sota_ref").toString().toUpper());
    writeField("my_state", record.value("my_state").isValid(),
               record.value("my_state").toString());
    writeField("my_street", record.value("my_street").isValid(),
               record.value("my_street").toString());
    writeField("my_usaca_counties", record.value("my_usaca_counties").isValid(),
               record.value("my_usaca_counties").toString());
    writeField("my_vucc_grids", record.value("my_vucc_grids").isValid(),
               record.value("my_vucc_grids").toString().toUpper());
    writeField("my_wwff_ref", record.value("my_wwff_ref").isValid(),
               record.value("my_wwff_ref").toString().toUpper());
    writeField("notes", record.value("notes").isValid(),
               record.value("notes").toString());
    writeField("nr_bursts", record.value("nr_bursts").isValid(),
               record.value("nr_bursts").toString());
    writeField("nr_pings", record.value("nr_pings").isValid(),
               record.value("nr_pings").toString());
    writeField("operator", record.value("operator").isValid(),
               record.value("operator").toString());
    writeField("owner_callsign", record.value("owner_callsign").isValid(),
               record.value("owner_callsign").toString());
    writeField("pota_ref", record.value("pota_ref").isValid(),
               record.value("pota_ref").toString().toUpper());
    writeField("precedence", record.value("precedence").isValid(),
               record.value("precedence").toString());
    writeField("prop_mode", record.value("prop_mode").isValid(),
               record.value("prop_mode").toString());
    writeField("public_key", record.value("public_key").isValid(),
               record.value("public_key").toString());
    writeField("qrzcom_qso_upload_date", record.value("qrzcom_qso_upload_date").isValid(),
               record.value("qrzcom_qso_upload_date").toDate().toString("yyyyMMdd"));
    writeField("qrzcom_qso_upload_status", record.value("qrzcom_qso_upload_status").isValid(),
               record.value("qrzcom_qso_upload_status").toString());
    writeField("qslmsg", record.value("qslmsg").isValid(),
               record.value("qslmsg").toString());
    writeField("qsl_rcvd_via", record.value("qsl_rcvd_via").isValid(),
               record.value("qsl_rcvd_via").toString());
    writeField("qsl_sent_via", record.value("qsl_sent_via").isValid(),
               record.value("qsl_sent_via").toString());
    writeField("qsl_via", record.value("qsl_via").isValid(),
               record.value("qsl_via").toString());
    writeField("qso_complete", record.value("qso_complete").isValid(),
               record.value("qso_complete").toString());
    writeField("qso_random", record.value("qso_random").isValid(),
               record.value("qso_random").toString());
    writeField("region", record.value("region").isValid(),
               record.value("region").toString());
    writeField("rig", record.value("rig").isValid(),
               record.value("rig").toString());
    writeField("rx_pwr", record.value("rx_pwr").isValid(),
               record.value("rx_pwr").toString());
    writeField("sat_mode", record.value("sat_mode").isValid(),
               record.value("sat_mode").toString());
    writeField("sat_name", record.value("sat_name").isValid(),
               record.value("sat_name").toString());
    writeField("sfi", record.value("sfi").isValid(),
               record.value("sfi").toString());
    writeField("sig", record.value("sig").isValid(),
               record.value("sig").toString());
    writeField("sig_info", record.value("sig_info").isValid(),
               record.value("sig_info").toString());
    writeField("silent_key", record.value("silent_key").isValid(),
               record.value("silent_key").toString());
    writeField("skcc", record.value("skcc").isValid(),
               record.value("skcc").toString());
    writeField("sota_ref", record.value("sota_ref").isValid(),
               record.value("sota_ref").toString().toUpper());
    writeField("srx", record.value("srx").isValid(),
               record.value("srx").toString());
    writeField("srx_string", record.value("srx_string").isValid(),
               record.value("srx_string").toString());
    writeField("station_callsign", record.value("station_callsign").isValid(),
               record.value("station_callsign").toString());
    writeField("stx", record.value("stx").isValid(),
               record.value("stx").toString());
    writeField("stx_string", record.value("stx_string").isValid(),
               record.value("stx_string").toString());
    writeField("swl", record.value("swl").isValid(),
               record.value("swl").toString());
    writeField("ten_ten", record.value("ten_ten").isValid(),
               record.value("ten_ten").toString());
    writeField("uksmg", record.value("uksmg").isValid(),
               record.value("uksmg").toString());
    writeField("usaca_counties", record.value("usaca_counties").isValid(),
               record.value("usaca_counties").toString());
    writeField("ve_prov", record.value("ve_prov").isValid(),
               record.value("ve_prov").toString());
    writeField("vucc_grids", record.value("vucc_grids").isValid(),
               record.value("vucc_grids").toString().toUpper());
    writeField("web", record.value("web").isValid(),
               record.value("web").toString());
    writeField("wwff_ref", record.value("wwff_ref").isValid(),
               record.value("wwff_ref").toString().toUpper());

    QJsonObject fields = QJsonDocument::fromJson(record.value("fields").toByteArray()).object();

    QStringList keys = fields.keys();
    for (const QString &key : qAsConst(keys))
    {
        writeField(key, ALWAYS_PRESENT, fields.value(key).toString());
    }

    /* Add application-specific tags */
    if ( applTags )
    {
       QStringList appKeys = applTags->keys();
       for (const QString &appkey : qAsConst(appKeys))
       {
           writeField(appkey, ALWAYS_PRESENT, applTags->value(appkey));
       }
    }
}

void AdiFormat::readField(QString& field, QString& value)
{
    FCT_IDENTIFICATION;

    //qCDebug(function_parameters)<<field<< " " << value;

    char c;

    QString typeString;
    QString lengthString;
    int length = 0;

    while (!stream.atEnd()) {
        switch (state) {
        case START:
            stream >> c;
            if (c == '<') {
                inHeader = false;
                state = KEY;
                field = "";
            }
            else {
                inHeader = true;
                state = FIELD;
            }
            break;

        case FIELD:
            stream >> c;
            if (c == '<') {
                state = KEY;
                field = "";
            }
            break;

        case KEY:
            stream >> c;
            if (c == ':') {
                state = SIZE;
                lengthString = "";
            }
            else if (c == '>') {
                state = FIELD;
                if (inHeader && field.toLower() == "eoh") {
                    inHeader = false;
                }
                else {
                    value = "";
                    return;
                }
            }
            else {
                field.append(c);
            }
            break;

        case SIZE:
            stream >> c;
            if (c == ':') {
                 if (!lengthString.isEmpty()) {
                    length = lengthString.toInt();
                 }
                 state = DATA_TYPE;
                 typeString = "";
            }
            else if (c == '>') {
                if (!lengthString.isEmpty()) {
                    length = lengthString.toInt();
                }

                if (length > 0) {
                    state = VALUE;
                    value = "";
                }
                else {
                    state = FIELD;
                    if (!inHeader) {
                        value = "";
                        return;
                    }

                }
            }
            else {
                lengthString.append(c);
            }
            break;

        case DATA_TYPE:
            stream >> c;
            if (c == '>') {
                if (length > 0) {
                    state = VALUE;
                    value = "";
                }
                else {
                    state = FIELD;
                    if (!inHeader) {
                        value = "";
                        return;
                    }
                }
            }
            else {
                typeString.append(c);
            }
            break;

        case VALUE:
            value = QString(stream.read(length));
            state = FIELD;
            if (!inHeader) {
                return;
            }
            break;
        }
    }
}

void AdiFormat::mapContact2SQLRecord(QMap<QString, QVariant> &contact,
                                     QSqlRecord &record)
{
    FCT_IDENTIFICATION;

    preprocessINTLFields<QMap<QString, QVariant>>(contact);

    /* Set default values if not present */
    if ( defaults )
    {
        QStringList keys = defaults->keys();

        for ( const QString &key : qAsConst(keys) )
        {
            if ( contact.value(key).isNull() )
            {
                contact.insert(key, defaults->value(key));
            }
        }
        // re-evaluate the fields
        preprocessINTLFields<QMap<QString, QVariant>>(contact);
    }

    contactFields2SQLRecord(contact, record);

    /* If we have something unparsed then stored it as JSON to Field column */
    if ( contact.count() > 0 )
    {
        QJsonDocument doc = QJsonDocument::fromVariant(QVariant(contact));
        record.setValue("fields", QString(doc.toJson()));
    }
}

void AdiFormat::contactFields2SQLRecord(QMap<QString, QVariant> &contact, QSqlRecord &record)
{
    FCT_IDENTIFICATION;

    record.setValue("callsign", contact.take("call"));
    record.setValue("rst_rcvd", contact.take("rst_rcvd"));
    record.setValue("rst_sent", contact.take("rst_sent"));
    record.setValue("gridsquare", contact.take("gridsquare").toString().toUpper());
    record.setValue("cqz", contact.take("cqz"));
    record.setValue("ituz", contact.take("ituz"));
    record.setValue("freq", contact.take("freq"));
    record.setValue("band", contact.take("band").toString().toLower());
    record.setValue("cont", contact.take("cont").toString().toUpper());
    record.setValue("dxcc", contact.take("dxcc"));
    record.setValue("pfx", contact.take("pfx").toString().toUpper());
    record.setValue("state", contact.take("state"));
    record.setValue("cnty", contact.take("cnty"));
    record.setValue("iota", contact.take("iota").toString().toUpper());
    record.setValue("qsl_rcvd", parseQslRcvd(contact.take("qsl_rcvd").toString()));
    record.setValue("qsl_rdate", parseDate(contact.take("qslrdate").toString()));  //TODO: DIFF MAPPING
    record.setValue("qsl_sent", parseQslSent(contact.take("qsl_sent").toString()));
    record.setValue("qsl_sdate", parseDate(contact.take("qslsdate").toString()));   //TODO: DIFF MAPPING
    record.setValue("lotw_qsl_rcvd", parseQslRcvd(contact.take("lotw_qsl_rcvd").toString()));
    record.setValue("lotw_qslrdate", parseDate(contact.take("lotw_qslrdate").toString()));
    record.setValue("lotw_qsl_sent", parseQslSent(contact.take("lotw_qsl_sent").toString()));
    record.setValue("lotw_qslsdate", parseDate(contact.take("lotw_qslsdate").toString()));
    record.setValue("tx_pwr", contact.take("tx_pwr"));
    record.setValue("address", contact.take("address"));
    record.setValue("address_intl", contact.take("address_intl"));
    record.setValue("age", contact.take("age"));
    record.setValue("altitude", contact.take("altitude"));
    record.setValue("a_index", contact.take("a_index"));
    record.setValue("ant_az", contact.take("ant_az"));
    record.setValue("ant_el", contact.take("ant_el"));
    record.setValue("ant_path", contact.take("ant_path").toString().toUpper());
    record.setValue("arrl_sect", contact.take("arrl_sect"));
    record.setValue("award_submitted",contact.take("award_submitted"));
    record.setValue("award_granted",contact.take("award_granted"));
    record.setValue("band_rx",contact.take("band_rx").toString().toLower());
    record.setValue("check",contact.take("check"));
    record.setValue("class",contact.take("class"));
    record.setValue("clublog_qso_upload_date",parseDate(contact.take("clublog_qso_upload_date").toString()));
    record.setValue("clublog_qso_upload_status",parseUploadStatus(contact.take("clublog_qso_upload_status").toString()));
    record.setValue("contacted_op",contact.take("contacted_op"));
    record.setValue("comment",contact.take("comment"));
    record.setValue("comment_intl",contact.take("comment_intl"));
    record.setValue("contest_id",contact.take("contest_id"));
    record.setValue("country",contact.take("country"));
    record.setValue("country_intl",contact.take("country_intl"));
    record.setValue("credit_submitted",contact.take("credit_submitted"));
    record.setValue("credit_granted",contact.take("credit_granted"));
    record.setValue("darc_dok",contact.take("darc_dok"));
    record.setValue("distance",contact.take("distance"));
    record.setValue("email",contact.take("email"));
    record.setValue("eq_call",contact.take("eq_call"));
    record.setValue("eqsl_qslrdate",parseDate(contact.take("eqsl_qslrdate").toString()));
    record.setValue("eqsl_qslsdate",parseDate(contact.take("eqsl_qslsdate").toString()));
    record.setValue("eqsl_qsl_rcvd",parseQslRcvd(contact.take("eqsl_qsl_rcvd").toString()));
    record.setValue("eqsl_qsl_sent",parseQslSent(contact.take("eqsl_qsl_sent").toString()));
    record.setValue("fists",contact.take("fists"));
    record.setValue("fists_cc",contact.take("fists_cc"));
    record.setValue("force_init",contact.take("force_init").toString().toUpper());
    record.setValue("freq_rx",contact.take("freq_rx"));
    record.setValue("gridsquare_ext",contact.take("gridsquare_ext"));
    record.setValue("guest_op",contact.take("guest_op"));
    record.setValue("hamlogeu_qso_upload_date",parseDate(contact.take("hamlogeu_qso_upload_date").toString()));
    record.setValue("hamlogeu_qso_upload_status",parseUploadStatus(contact.take("hamlogeu_qso_upload_status").toString()));
    record.setValue("hamqth_qso_upload_date",parseDate(contact.take("hamqth_qso_upload_date").toString()));
    record.setValue("hamqth_qso_upload_status",parseUploadStatus(contact.take("hamqth_qso_upload_status").toString()));
    record.setValue("hrdlog_qso_upload_date",parseDate(contact.take("hrdlog_qso_upload_date").toString()));
    record.setValue("hrdlog_qso_upload_status",parseUploadStatus(contact.take("hrdlog_qso_upload_status").toString()));
    record.setValue("iota_island_id",contact.take("iota_island_id").toString().toUpper());
    record.setValue("k_index",contact.take("k_index"));
    record.setValue("lat",contact.take("lat"));
    record.setValue("lon",contact.take("lon"));
    record.setValue("max_bursts",contact.take("max_bursts"));
    record.setValue("ms_shower",contact.take("ms_shower"));
    record.setValue("my_antenna",contact.take("my_antenna"));
    record.setValue("my_antenna_intl",contact.take("my_antenna_intl"));
    record.setValue("my_altitude",contact.take("my_altitude"));
    record.setValue("my_arrl_sect",contact.take("my_arrl_sect"));
    record.setValue("my_city",contact.take("my_city"));
    record.setValue("my_city_intl",contact.take("my_city_intl"));
    record.setValue("my_country",contact.take("my_country"));
    record.setValue("my_country_intl",contact.take("my_country_intl"));
    record.setValue("my_cnty",contact.take("my_cnty"));
    record.setValue("my_cq_zone",contact.take("my_cq_zone"));
    record.setValue("my_dxcc",contact.take("my_dxcc"));
    record.setValue("my_fists",contact.take("my_fists"));
    record.setValue("my_gridsquare",contact.take("my_gridsquare").toString().toUpper());
    record.setValue("my_gridsquare_ext",contact.take("my_gridsquare_ext").toString().toUpper());
    record.setValue("my_iota",contact.take("my_iota").toString().toUpper());
    record.setValue("my_iota_island_id",contact.take("my_iota_island_id").toString().toUpper());
    record.setValue("my_itu_zone",contact.take("my_itu_zone"));
    record.setValue("my_lat",contact.take("my_lat"));
    record.setValue("my_lon",contact.take("my_lon"));
    record.setValue("my_name",contact.take("my_name"));
    record.setValue("my_name_intl",contact.take("my_name_intl"));
    record.setValue("my_postal_code",contact.take("my_postal_code"));
    record.setValue("my_postal_code_intl",contact.take("my_postal_code_intl"));
    record.setValue("my_pota_ref",contact.take("my_pota_ref").toString().toUpper());
    record.setValue("my_rig",contact.take("my_rig"));
    record.setValue("my_rig_intl",contact.take("my_rig_intl"));
    record.setValue("my_sig",contact.take("my_sig"));
    record.setValue("my_sig_intl",contact.take("my_sig_intl"));
    record.setValue("my_sig_info",contact.take("my_sig_info"));
    record.setValue("my_sig_info_intl",contact.take("my_sig_info_intl"));
    record.setValue("my_sota_ref",contact.take("my_sota_ref").toString().toUpper());
    record.setValue("my_state",contact.take("my_state"));
    record.setValue("my_street",contact.take("my_street"));
    record.setValue("my_street_intl",contact.take("my_street_intl"));
    record.setValue("my_usaca_counties",contact.take("my_usaca_counties"));
    record.setValue("my_vucc_grids",contact.take("my_vucc_grids").toString().toUpper());
    record.setValue("my_wwff_ref",contact.take("my_wwff_ref").toString().toUpper());
    record.setValue("name",contact.take("name"));
    record.setValue("name_intl",contact.take("name_intl"));
    record.setValue("notes",contact.take("notes"));
    record.setValue("notes_intl",contact.take("notes_intl"));
    record.setValue("nr_bursts",contact.take("nr_bursts"));
    record.setValue("nr_pings",contact.take("nr_pings"));
    record.setValue("operator",contact.take("operator"));
    record.setValue("owner_callsign",contact.take("owner_callsign"));
    record.setValue("pota_ref",contact.take("pota_ref").toString().toUpper());
    record.setValue("precedence",contact.take("precedence"));
    record.setValue("prop_mode",contact.take("prop_mode"));
    record.setValue("public_key",contact.take("public_key"));
    record.setValue("qrzcom_qso_upload_date",parseDate(contact.take("qrzcom_qso_upload_date").toString()));
    record.setValue("qrzcom_qso_upload_status",parseUploadStatus(contact.take("qrzcom_qso_upload_status").toString()));
    record.setValue("qsl_rcvd_via",contact.take("qsl_rcvd_via").toString().toUpper());
    record.setValue("qsl_sent_via",contact.take("qsl_sent_via").toString().toUpper());
    record.setValue("qsl_via",contact.take("qsl_via"));
    record.setValue("qso_complete",contact.take("qso_complete").toString().toUpper());
    record.setValue("qso_random",contact.take("qso_random").toString().toUpper());
    record.setValue("qslmsg",contact.take("qslmsg"));
    record.setValue("qslmsg_intl",contact.take("qslmsg_intl"));
    record.setValue("qth",contact.take("qth"));
    record.setValue("qth_intl",contact.take("qth_intl"));
    record.setValue("region",contact.take("region"));
    record.setValue("rig",contact.take("rig"));
    record.setValue("rig_intl",contact.take("rig_intl"));
    record.setValue("rx_pwr",contact.take("rx_pwr"));
    record.setValue("sat_mode",contact.take("sat_mode"));
    record.setValue("sat_name",contact.take("sat_name"));
    record.setValue("sfi",contact.take("sfi"));
    record.setValue("sig",contact.take("sig"));
    record.setValue("sig_intl",contact.take("sig_intl"));
    record.setValue("sig_info",contact.take("sig_info"));
    record.setValue("sig_info_intl",contact.take("sig_info_intl"));
    record.setValue("silent_key",contact.take("silent_key").toString().toUpper());
    record.setValue("skcc",contact.take("skcc"));
    record.setValue("sota_ref",contact.take("sota_ref").toString().toUpper());
    record.setValue("srx",contact.take("srx"));
    record.setValue("srx_string",contact.take("srx_string"));
    record.setValue("station_callsign",contact.take("station_callsign").toString().toUpper());
    record.setValue("stx",contact.take("stx"));
    record.setValue("stx_string",contact.take("stx_string"));
    record.setValue("swl",contact.take("swl").toString().toUpper());
    record.setValue("ten_ten",contact.take("ten_ten"));
    record.setValue("uksmg",contact.take("uksmg"));
    record.setValue("usaca_counties",contact.take("usaca_counties"));
    record.setValue("ve_prov",contact.take("ve_prov"));
    record.setValue("vucc_grids",contact.take("vucc_grids").toString().toUpper());
    record.setValue("web",contact.take("web"));
    record.setValue("wwff_ref",contact.take("wwff_ref").toString().toUpper());

    QString mode = contact.take("mode").toString().toUpper();
    QString submode = contact.take("submode").toString().toUpper();

    QPair<QString, QString> legacy = Data::instance()->legacyMode(mode);

    if ( !legacy.first.isEmpty() )
    {
        mode = legacy.first;
        submode = legacy.second;
    }

    record.setValue("mode", mode);
    record.setValue("submode", submode);

    QDate date_on = parseDate(contact.take("qso_date").toString());
    QDate date_off = parseDate(contact.take("qso_date_off").toString());

    if ( date_off.isNull() || !date_off.isValid() )
    {
        date_off = date_on;
    }

    QTime time_on = parseTime(contact.take("time_on").toString());
    QTime time_off = parseTime(contact.take("time_off").toString());

    if ( time_on.isValid() && time_off.isNull() )
    {
        time_off = time_on;
    }

    if ( time_off.isValid() && time_on.isNull() )
    {
        time_on = time_off;
    }

    QDateTime start_time(date_on, time_on, Qt::UTC);
    QDateTime end_time(date_off, time_off, Qt::UTC);

    if ( end_time < start_time )
    {
        qCDebug(runtime) << "End time before start time!" << record;
    }

    record.setValue("start_time", start_time);
    record.setValue("end_time", end_time);
}

void AdiFormat::preprocessINTLField(const QString &fieldName,
                                    const QString &fieldIntlName,
                                    QMap<QString, QVariant> &contact)
{
    FCT_IDENTIFICATION;

    // NOTE: If modify this, modify also function below!!!!

    QVariant fld = contact.value(fieldName);
    QVariant fldIntl = contact.value(fieldIntlName);

    /* In general, it is a hack because ADI must not contain
     * _INTL fields. But some applications generate _INTL fields in ADI files
     * therefore it is needed to implement a logic how to convert INTL fields
     * to standard
     */
    if ( !fld.isNull() && !fldIntl.isNull() )
    {
        /* ascii and intl are present */
        //no action
    }
    else if ( !fld.isNull() && fldIntl.isNull() )
    {
        /* ascii is present but Intl is not present */
        contact[fieldIntlName] = fld;
    }
    else if ( fld.isNull() && !fldIntl.isNull() )
    {
        /* ascii is empty but Intl is present */
        contact[fieldName] = Data::removeAccents(fldIntl.toString());
    }
    else
    {
        /* both are empty */
        /* do nothing */
    }
}

void AdiFormat::preprocessINTLField(const QString &fieldName,
                                    const QString &fieldIntlName,
                                    QSqlRecord &contact)
{
    FCT_IDENTIFICATION;

    // NOTE: If modify this, modify also function above!!!!
    QVariant fld = contact.value(fieldName);
    QVariant fldIntl = contact.value(fieldIntlName);

    /* In general, it is a hack because ADI must not contain
     * _INTL fields. But some applications generate _INTL fields in ADI files
     * therefore it is needed to implement a logic how to convert INTL fields
     * to standard
     */
    if ( !fld.isNull() && !fldIntl.isNull() )
    {
        /* ascii and intl are present */
        //no action
    }
    else if ( !fld.isNull() && fldIntl.isNull() )
    {
        /* ascii is present but Intl is not present */
        contact.setValue(fieldIntlName, fld);
    }
    else if ( fld.isNull() && !fldIntl.isNull() )
    {
        /* ascii is empty but Intl is present */
        contact.setValue(fieldName, Data::removeAccents(fldIntl.toString()));
    }
    else
    {
        /* both are empty */
        /* do nothing */
    }
}

bool AdiFormat::readContact(QMap<QString, QVariant>& contact)
{
    FCT_IDENTIFICATION;

    while (!stream.atEnd())
    {
        QString field;
        QString value;

        readField(field, value);
        field = field.toLower();

        if (field == "eor")
        {
            return true;
        }

        if (!value.isEmpty())
        {
            contact[field] = QVariant(value);
        }
    }

    return false;
}

bool AdiFormat::importNext(QSqlRecord& record)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters)<<record;

    QMap<QString, QVariant> contact;

    if ( !readContact(contact) )
    {
        return false;
    }

    mapContact2SQLRecord(contact, record);

    return true;
}

QDate AdiFormat::parseDate(const QString &date)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters)<<date;

    if (date.length() == 8) {
        return QDate::fromString(date, "yyyyMMdd");
    }
    else {
        return QDate();
    }
}

QTime AdiFormat::parseTime(const QString &time)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters)<<time;

    switch (time.length()) {
    case 4:
        return QTime::fromString(time, "hhmm");

    case 6:
        return QTime::fromString(time, "hhmmss");

    default:
        return QTime();
    }
}


QString AdiFormat::parseQslRcvd(const QString &value) {
    FCT_IDENTIFICATION;

    qCDebug(function_parameters)<<value;

    if (!value.isEmpty())
    {
        switch (value.toUpper().at(0).toLatin1())
        {
        case 'Y': return "Y";
        case 'N': return "N";
        case 'R': return "R";
        case 'I': return "I";
        case 'V': return "Y";
        default: return "N";
        }
    }
    return "N";
}

QString AdiFormat::parseQslSent(const QString &value) {
    FCT_IDENTIFICATION;

    qCDebug(function_parameters)<<value;

    if (!value.isEmpty())
    {
        switch (value.toUpper().at(0).toLatin1())
        {
        case 'Y': return "Y";
        case 'N': return "N";
        case 'R': return "R";
        case 'Q': return "Q";
        case 'I': return "I";
        default: return "N";
        }
    }
    return "N";
}

QString AdiFormat::parseUploadStatus(const QString &value)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters)<<value;

    if (!value.isEmpty())
    {
        switch (value.toUpper().at(0).toLatin1())
        {
        case 'Y': return "Y";
        case 'N': return "N";
        case 'M': return "M";
        default: QString();
        }
    }
    return QString();
}


QMap<QString, QString> AdiFormat::fieldname2INTLNameMapping =
{
    {"address", "address_intl"},
    {"comment", "comment_intl"},
    {"country", "country_intl"},
    {"my_antenna", "my_antenna_intl"},
    {"my_city", "my_city_intl"},
    {"my_country", "my_country_intl"},
    {"my_name", "my_name_intl"},
    {"my_postal_code", "my_postal_code_intl"},
    {"my_rig", "my_rig_intl"},
    {"my_sig", "my_sig_intl"},
    {"my_sig_info", "my_sig_info_intl"},
    {"my_street", "my_street_intl"},
    {"name", "name_intl"},
    {"notes", "notes_intl"},
    {"qslmsg", "qslmsg_intl"},
    {"qth", "qth_intl"},
    {"rig", "rig_intl"},
    {"sig", "sig_intl"},
    {"sig_info", "sig_info_intl"}
};
