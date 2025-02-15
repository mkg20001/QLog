#ifndef EQSLDIALOG_H
#define EQSLDIALOG_H

#include <QDialog>
#include "core/LogLocale.h"

namespace Ui {
class EqslDialog;
}

class EqslDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EqslDialog(QWidget *parent = nullptr);
    ~EqslDialog();

public slots:
    void download();
    void upload();
    void uploadCallsignChanged(const QString&);

private:
    Ui::EqslDialog *ui;

    void saveDialogState();
    void loadDialogState();
    LogLocale locale;
};

#endif // EQSLDIALOG_H
