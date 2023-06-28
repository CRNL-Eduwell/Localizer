#ifndef FREQUENCYWINDOW_H
#define FREQUENCYWINDOW_H

#include "ui_FrequencyWindow.h"
#include <QtWidgets/QDialog>
#include <QLineEdit>
#include "FrequencyBand.h"

class FrequencyWindow : public QDialog
{
    Q_OBJECT

public:
    FrequencyWindow(InsermLibrary::FrequencyBand& f, QWidget *parent = nullptr);
    ~FrequencyWindow();

private slots:
    void ValidateAndSave();

private:
    Ui::FrequencyWindow ui;
    InsermLibrary::FrequencyBand *m_frequencyBand = nullptr;
};

#endif // FREQUENCIESWINDOW_H
