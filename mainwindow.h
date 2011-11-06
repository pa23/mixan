/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: mainwindow.h

    Copyright (C) 2011 Artem Petrov <pa2311@gmail.com>

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QProgressDialog>
#include <QFutureWatcher>
#include <QImage>
#include <QSettings>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QSharedPointer>

#include "material.h"
#include "mix.h"
#include "settingsdialog.h"

namespace Ui {

class MainWindow;
}

class MainWindow : public QMainWindow {

    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:

    Ui::MainWindow *ui;

    QSettings mixanSettings;

    QString mat1ImageFileName;
    QString mat2ImageFileName;
    QStringList mixImageFileNames;

    QSharedPointer<Material> material1;
    QSharedPointer<Material> material2;
    QVector< QSharedPointer<Mix> > probes;
    QVector<QImage> graphics;

    QProgressDialog *progressDialog;
    QFutureWatcher<void> *futureWatcher;

    SettingsDialog *settingsDialog;
    QSpinBox *spinBox_polyPower;
    QDoubleSpinBox *doubleSpinBox_intersectAccur;
    QDoubleSpinBox *doubleSpinBox_idealConc;
    QSpinBox *spinBox_imgWidth;
    QCheckBox *checkBox_reportReadOnly;

    void forgetSelectedImages();
    void runMaterialsAnalysis();
    void runMixAnalysis();

    void writeProgramSettings();
    void readProgramSettings();

    void createGraphics();

private slots:

    void on_action_selectMaterialImages_activated();
    void on_action_selectProbeImages_activated();
    void on_action_saveReport_activated();
    void on_action_printReport_activated();
    void on_action_cleanReportWindow_activated();
    void on_action_quit_activated();
    void on_action_analyzeMaterials_activated();
    void on_action_analyzeMix_activated();
    void on_action_settings_activated();
    void on_action_about_mixan_activated();

    void showAnalysisResults();
    void resetResults();

    void reportReadOnlyChanged();

};

#endif // MAINWINDOW_H
