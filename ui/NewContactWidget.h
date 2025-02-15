#ifndef NEWCONTACTWIDGET_H
#define NEWCONTACTWIDGET_H

#include <QWidget>
#include <QSqlRecord>
#include <QCompleter>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QHash>
#include <QFormLayout>

#include "data/Data.h"
#include "core/Gridsquare.h"
#include "data/DxSpot.h"
#include "core/Rig.h"
#include "core/CallbookManager.h"
#include "data/StationProfile.h"
#include "core/PropConditions.h"
#include "core/LogLocale.h"
#include "models/LogbookModel.h"
#include "ui/EditLine.h"

namespace Ui {
class NewContactWidget;
}

enum CoordPrecision {
    COORD_NONE = 0,
    COORD_DXCC = 1,
    COORD_GRID = 2,
    COORD_FULL = 3
};

class NewContactDynamicWidgets
{
public:
    QLabel *nameLabel;
    NewContactEditLine *nameEdit;

    QLabel *qthLabel;
    NewContactEditLine *qthEdit;

    QLabel *gridLabel;
    NewContactEditLine *gridEdit;

    QLabel *commentLabel;
    NewContactEditLine *commentEdit;

    QLabel *contLabel;
    QComboBox *contEdit;

    QLabel *ituLabel;
    NewContactEditLine *ituEdit;

    QLabel *cqzLabel;
    NewContactEditLine *cqzEdit;

    QLabel *stateLabel;
    NewContactEditLine *stateEdit;

    QLabel *countyLabel;
    NewContactEditLine *countyEdit;

    QLabel *ageLabel;
    NewContactEditLine *ageEdit;

    QLabel *vuccLabel;
    NewContactEditLine *vuccEdit;

    QLabel *dokLabel;
    NewContactEditLine *dokEdit;

    QLabel *iotaLabel;
    NewContactEditLine *iotaEdit;

    QLabel *potaLabel;
    NewContactEditLine *potaEdit;

    QLabel *sotaLabel;
    NewContactEditLine *sotaEdit;

    QLabel *wwffLabel;
    NewContactEditLine *wwffEdit;

    QLabel *sigLabel;
    NewContactEditLine *sigEdit;

    QLabel *sigInfoLabel;
    NewContactEditLine *sigInfoEdit;

    QLabel *emailLabel;
    NewContactEditLine *emailEdit;

    QLabel *urlLabel;
    NewContactEditLine *urlEdit;

    QLabel *satNameLabel;
    NewContactEditLine *satNameEdit;

    QLabel *satModeLabel;
    QComboBox *satModeEdit;

    explicit NewContactDynamicWidgets(bool allocateWidgets,
                                      QWidget *parent);
    QWidget* getRowWidget(int index);
    QWidget* getLabel(int index);
    QWidget* getEditor(int index);
    QStringList getAllFieldLabelNames() const;
    int getIndex4FieldLabelName(const QString&) const;
    QString getFieldLabelName4Index(int) const;

private:
    struct DynamicWidget
    {
       QWidget* label;
       QWidget* editor;
       QWidget* rowWidget;
       QString baseObjectName;
       QString fieldLabelName;
    };

    void initializeWidgets(int DBIndexMapping,
                           const QString &,
                           QLabel *&retLabel,
                           NewContactEditLine *&retWidget);

    void initializeWidgets(int DBIndexMapping,
                           const QString &,
                           QLabel *&retLabel,
                           QComboBox *&retWidget);

    // Mapping from DB Index to <Label, Editor>
    QHash<int, DynamicWidget> widgetMapping;
    QWidget *parent;
    QPointer<LogbookModel> logbookmodel;
    bool widgetsAllocated;
};

class NewContactWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NewContactWidget(QWidget *parent = nullptr);
    ~NewContactWidget();

    void assignPropConditions(PropConditions *);
    QString getCallsign() const;
    QString getName() const;
    QString getRST() const;
    QString getGreeting() const;
    QString getMyCallsign() const;
    QString getMyName() const;
    QString getMyQTH() const;
    QString getMyLocator() const;
    QString getMySIG() const;
    QString getMySIGInfo() const;
    QString getMyIOTA() const;
    QString getMySOTA() const;
    QString getMyPOTA() const;
    QString getMyWWFT() const;
    QString getMyVUCC() const;
    QString getMyPWR() const;
    QString getBand() const;
    QString getMode() const;
    double getQSOBearing() const;
    double getQSODistance() const;

signals:
    void contactAdded(QSqlRecord record);
    void newTarget(double lat, double lon);
    void filterCallsign(QString call);
    void userFrequencyChanged(VFOID, double, double, double);
    void markQSO(DxSpot spot);

    void stationProfileChanged();
    void rigProfileChanged();
    void antProfileChanged();

public slots:
    void refreshRigProfileCombo();
    void saveExternalContact(QSqlRecord record);
    void readGlobalSettings();
    void tuneDx(const QString &callsign, double frequency);
    void fillCallsignGrid(const QString &callsign, const QString& grid);
    void showDx(const QString &callsign, const QString &grid);
    void resetContact();
    void saveContact();

    // to receive RIG instructions
    void changeFrequency(VFOID, double, double, double);
    void changeModefromRig(VFOID, const QString &rawMode, const QString &mode,
                    const QString &subMode, qint32 width);
    void changePower(VFOID, double power);
    void rigConnected();
    void rigDisconnected();
    void nearestSpot(const DxSpot &);
    void setNearestSpotColor(const QString &call);
    void setManualMode(bool);
    void exitManualMode();

    void setupCustomUi();

private slots:
    void callsignChanged();
    void frequencyTXChanged();
    void frequencyRXChanged();
    void changeMode();
    void subModeChanged();
    void gridChanged();
    void updateTime();
    void updateTimeOff();
    void startContactTimer();
    void stopContactTimer();
    void markContact();
    void editCallsignFinished();
    void callsignResult(const QMap<QString, QString>& data);
    void clubQueryResult(const QString&, QMap<QString, ClubStatusQuery::ClubStatus>);
    void propModeChanged(const QString&);
    void sotaChanged(const QString&);
    void sotaEditFinished();
    void potaChanged(const QString&);
    void potaEditFinished();
    void wwffEditFinished();
    void wwffChanged(const QString&);
    void formFieldChangedString(const QString&);
    void formFieldChanged();
    void useNearestCallsign();
    void timeOnChanged();

    void stationProfileComboChanged(const QString&);
    void rigProfileComboChanged(const QString&);
    void antProfileComboChanged(const QString&);
    void webLookup();

private:
    void fillFieldsFromLastQSO(const QString &callsign);
    void queryDxcc(const QString &callsign);
    void clearCallbookQueryFields();
    void clearMemberQueryFields();
    void readWidgetSettings();
    void writeWidgetSetting();
    void __modeChanged(qint32);
    void refreshStationProfileCombo();
    void updateTXBand(double freq);
    void updateRXBand(double freq);
    void updateCoordinates(double lat, double lon, CoordPrecision prec);
    void updateDxccStatus();
    void updatePartnerLocTime();
    void setDefaultReport();
    void refreshAntProfileCombo();
    void addAddlFields(QSqlRecord &record, const StationProfile &profile);
    bool eventFilter(QObject *object, QEvent *event);
    bool isQSOTimeStarted();
    void QSYContactWiping(double);
    void connectFieldChanged();
    void changeCallsignManually(const QString &);
    void changeCallsignManually(const QString &, double);
    void __changeFrequency(VFOID, double vfoFreq, double ritFreq, double xitFreq);
    void showRXTXFreqs(bool);
    void setComboBaseData(QComboBox *, const QString &);
    void queryMemberList();
    QList<QWidget*> setupCustomUiRow(QHBoxLayout *row, const QList<int>& widgetsList);
    QList<QWidget*> setupCustomDetailColumn(QFormLayout *column, const QList<int>& widgetsList);

    void setupCustomUiRowsTabOrder(const QList<QWidget *> &customWidgets);
    void setBandLabel(const QString &);

    bool isPOTAValid(POTAEntity *entity);
    bool isSOTAValid(SOTAEntity *entity);
    bool isWWFFValid(WWFFEntity *entity);

private:
    Rig* rig;
    double realRigFreq;
    double realFreqForManualExit;
    QString callsign;
    double dxDistance; // QSO distance in km - used for ADIF and internal usage
    DxccEntity dxccEntity;
    QString defaultReport;
    CallbookManager callbookManager;
    QTimer* contactTimer;
    Ui::NewContactWidget *ui;
    NewContactDynamicWidgets *uiDynamic;
    CoordPrecision coordPrec;
    PropConditions *prop_cond;
    QCompleter *satCompleter;
    QCompleter *sotaCompleter;
    QCompleter *potaCompleter;
    QCompleter *wwffCompleter;
    QTimeZone partnerTimeZone;
    double QSOFreq;
    qint32 bandwidthFilter;
    bool rigOnline;
    QMap<QString, QString> lastCallbookQueryData;
    SOTAEntity lastSOTA;
    POTAEntity lastPOTA;
    WWFFEntity lastWWFF;
    bool isManualEnterMode;
    LogLocale locale;

};

#endif // NEWCONTACTWIDGET_H
