/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: mainwindow.h

    Copyright (C) 2011-2012 Artem Petrov <pa2311@gmail.com>

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
#include <QSharedPointer>
#include <QSettings>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QCloseEvent>

#include "settingsdialog.h"
#include "analysisdialog.h"
#include "settings.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {

    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:

     void closeEvent(QCloseEvent *event);

private slots:

    void on_action_saveReport_activated();
    void on_action_printReport_activated();
    void on_action_quit_activated();
    void on_action_cleanReportWindow_activated();
    void on_action_settings_activated();
    void on_action_analysis_activated();
    void on_action_userManual_activated();
    void on_action_about_mixan_activated();

    void reportReadOnlyChanged(int);
    void sizeinmmChanged(int);

private:

    Ui::MainWindow *ui;

    QSettings mixanSettings;

    QSharedPointer<Settings> calcSettings;

    SettingsDialog *settingsDialog;
    AnalysisDialog *analysisDialog;

    QSpinBox *spinBox_polyPower;
    QDoubleSpinBox *doubleSpinBox_intersectAccur;
    QComboBox *comboBox_thrColDefMethod;
    QDoubleSpinBox *doubleSpinBox_idealConc;
    QCheckBox *checkBox_sizeinmm;
    QDoubleSpinBox *doubleSpinBox_pxpermm2;
    QLineEdit *lineEdit_sievesCellDiameter;
    QLineEdit *lineEdit_sievesCellDimension;
    QCheckBox *checkBox_reportReadOnly;
    QCheckBox *checkBox_imagesInReport;
    QCheckBox *checkBox_createTemporaryGraphics;
    QSpinBox *spinBox_imgWidth;
    QComboBox *comboBox_analysisType;

    QString reportCaption;

    void writeProgramSettings();
    void readProgramSettings();
    void initCalcSettings();
    void saveIfNecessary();

};

#endif // MAINWINDOW_H
