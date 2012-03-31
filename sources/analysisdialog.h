/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: analysisdialog.h

    Copyright (C) 2012 Artem Petrov <pa2311@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ANALYSISDIALOG_H
#define ANALYSISDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTextBrowser>
#include <QString>
#include <QSharedPointer>
#include <QVector>
#include <QImage>
#include <QProgressDialog>
#include <QFutureWatcher>

#include "settings.h"
#include "material.h"
#include "mix.h"
#include "granules.h"

namespace Ui {
class AnalysisDialog;
}

class AnalysisDialog : public QDialog {

    Q_OBJECT

public:

    explicit AnalysisDialog(QTextBrowser *, // report widget
                            QSharedPointer<Settings>,
                            QWidget *parent = 0);
    ~AnalysisDialog();

private slots:

    void on_comboBox_analysisType_currentIndexChanged(int index);
    void on_pushButton_selectMat1_clicked();
    void on_pushButton_selectMat2_clicked();
    void on_pushButton_selectProbes_clicked();
    void on_pushButton_clear_clicked();
    void on_pushButton_run_clicked();

    void showAnalysisResults();

private:

    Ui::AnalysisDialog *ui;

    QTextBrowser *report;
    QSharedPointer<Settings> settings;

    QString filters;

    QSharedPointer<Material> material1;
    QSharedPointer<Material> material2;
    QVector< QSharedPointer<Mix> > probes;
    QVector< QSharedPointer<Granules> > granules;

    QVector<QImage> graphics;
    QVector<QImage> histograms;

    QSharedPointer<QProgressDialog> progressDialog;
    QSharedPointer< QFutureWatcher<void> > futureWatcher;

    QString thrmsg;

    enum { ANALTYPE_MATERIALS,
           ANALTYPE_MIX,
           ANALTYPE_GRANULATION };

    void runAnalysis();
    void createGraphics();
    void createHistograms();

};

#endif // ANALYSISDIALOG_H
