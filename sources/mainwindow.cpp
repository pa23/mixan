/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: mainwindow.cpp

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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "constants.h"
#include "settingsdialog.h"
#include "analysisdialog.h"
#include "settings.h"

#include <QString>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextCursor>
#include <QSettings>
#include <QRect>
#include <QSpinBox>
#include <QCheckBox>
#include <QSharedPointer>
#include <QFont>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mixanSettings("pa23software", "mixan") {

    ui->setupUi(this);

    setWindowTitle("mixan " + VERSION);

    //

    reportCaption = "<br><b>mixan "
            + VERSION
            + "</b><br>"
            + tr("Analysis of granular material mixes and emulsions.")
            + "<br><hr><br>";

    ui->textBrowser_report->setUndoRedoEnabled(false);
    ui->textBrowser_report->setHtml(reportCaption);

    //

    settingsDialog = QSharedPointer<SettingsDialog>(new SettingsDialog());

    analysisDialog = QSharedPointer<AnalysisDialog>
            (new AnalysisDialog(ui->textBrowser_report, calcSettings));

    //

    spinBox_polyPower = settingsDialog->
            findChild<QSpinBox *>("spinBox_polyPower");
    doubleSpinBox_intersectAccur = settingsDialog->
            findChild<QDoubleSpinBox *>("doubleSpinBox_intersectAccur");
    doubleSpinBox_idealConc = settingsDialog->
            findChild<QDoubleSpinBox *>("doubleSpinBox_idealConc");
    checkBox_reportReadOnly = settingsDialog->
            findChild<QCheckBox *>("checkBox_reportRO");
    checkBox_imagesInReport = settingsDialog->
            findChild<QCheckBox *>("checkBox_imagesInReport");
    checkBox_createTemporaryGraphics = settingsDialog->
            findChild<QCheckBox *>("checkBox_createTempGraph");
    spinBox_imgWidth = settingsDialog->
            findChild<QSpinBox *>("spinBox_imgWidth");
    comboBox_analysisType = analysisDialog->
            findChild<QComboBox *>("comboBox_analysisType");

    connect(checkBox_reportReadOnly,
            SIGNAL(clicked()),
            this,
            SLOT(reportReadOnlyChanged())
            );

    //

    calcSettings = QSharedPointer<Settings>(new Settings());

    readProgramSettings();
}

MainWindow::~MainWindow() {

    writeProgramSettings();
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {

    saveIfNecessary();
    event->accept();
}

void MainWindow::writeProgramSettings() {

    mixanSettings.beginGroup("/Settings");
    mixanSettings.setValue("/window_geometry", geometry());
    mixanSettings.setValue("/panels_state", QMainWindow::saveState());
    mixanSettings.setValue("/polynom_power", spinBox_polyPower->value());
    mixanSettings.setValue("/intersection_accuracy",
                           doubleSpinBox_intersectAccur->value());
    mixanSettings.setValue("/ideal_concentration",
                           doubleSpinBox_idealConc->value());
    mixanSettings.setValue("/report_is_read_only",
                           checkBox_reportReadOnly->isChecked());
    mixanSettings.setValue("/show_images_in_report",
                           checkBox_imagesInReport->isChecked());
    mixanSettings.setValue("/create_temporary_graphics",
                           checkBox_createTemporaryGraphics->isChecked());
    mixanSettings.setValue("/image_width", spinBox_imgWidth->value());
    mixanSettings.setValue("/comboBox_analysisType",
                           comboBox_analysisType->currentIndex());
    mixanSettings.endGroup();
}

void MainWindow::readProgramSettings() {

    mixanSettings.beginGroup("/Settings");
    setGeometry(mixanSettings.value("/window_geometry",
                                    QRect(20, 40, 0, 0)).toRect());
    restoreState(mixanSettings.value("/panels_state").toByteArray());
    spinBox_polyPower->setValue(
                mixanSettings.value("/polynom_power", 9).toInt()
                );
    doubleSpinBox_intersectAccur->setValue(
                mixanSettings.
                value("/intersection_accuracy", 0.00025).toDouble()
                );
    doubleSpinBox_idealConc->setValue(
                mixanSettings.value("/ideal_concentration", 0.5).toDouble()
                );
    checkBox_reportReadOnly->setChecked(
                mixanSettings.value("/report_is_read_only", true).toBool()
                );
    checkBox_imagesInReport->setChecked(
                mixanSettings.value("/show_images_in_report", true).toBool()
                );
    checkBox_createTemporaryGraphics->setChecked(
                mixanSettings.
                value("/create_temporary_graphics", false).toBool()
                );
    spinBox_imgWidth->setValue(
                mixanSettings.value("/image_width", 600).toInt()
                );
    comboBox_analysisType->setCurrentIndex(
                mixanSettings.value("/comboBox_analysisType", 1).toInt()
                );
    mixanSettings.endGroup();

    //

    reportReadOnlyChanged();
}

void MainWindow::initCalcSettings() {

    calcSettings->setPolyPwr(spinBox_polyPower->value());
    calcSettings->setThrAccur(doubleSpinBox_intersectAccur->value());
    calcSettings->setIdealConc(doubleSpinBox_idealConc->value());
    calcSettings->setShowImgInReport(checkBox_imagesInReport->isChecked());
    calcSettings->
            setCreateTmpImg(checkBox_createTemporaryGraphics->isChecked());
    calcSettings->setImgWidth(spinBox_imgWidth->value());
}

void MainWindow::on_action_saveReport_activated() {

    QString filters = "PDF files (*.pdf);;All files (*.*)";

    QString reportFileName(QFileDialog::getSaveFileName(
                               this,
                               tr("Save report to PDF..."),
                               "noname.pdf",
                               filters,
                               0,
                               0));

    if ( reportFileName.isEmpty() ) { return; }

    QPrinter printer;

    printer.setOrientation(QPrinter::Portrait);
    printer.setPageMargins(20, 15, 15, 15, QPrinter::Millimeter);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(reportFileName);
    printer.setFontEmbeddingEnabled(true);

    ui->textBrowser_report->print(&printer);
}

void MainWindow::on_action_printReport_activated() {

    QPrinter printer;

    printer.setOrientation(QPrinter::Portrait);
    printer.setPageMargins(25, 15, 15, 15, QPrinter::Millimeter);

    QPrintDialog printDialog(&printer, this);

    if (printDialog.exec() == QDialog::Accepted) {

        ui->textBrowser_report->print(&printer);
    }
}

void MainWindow::on_action_quit_activated() {

    close();
}

void MainWindow::on_action_cleanReportWindow_activated() {

    ui->textBrowser_report->clear();
    ui->textBrowser_report->setHtml(reportCaption);
}

void MainWindow::on_action_settings_activated() {

    settingsDialog->exec();
}

void MainWindow::on_action_analysis_activated() {

    initCalcSettings();

    analysisDialog->exec();
}

void MainWindow::on_action_about_mixan_activated() {

    QString str = "<b>mixan " + VERSION + "</b>\n"
            "<br><br>Analysis of granular material mixes and emulsions."
            "<br><br>Copyright (C) 2011-2012 Artem Petrov "
            "<a href= \"mailto:pa2311@gmail.com\" >pa2311@gmail.com</a>"
            "<br><br>Web site: <a href= \"https://github.com/pa23/mixan\">"
            "https://github.com/pa23/mixan</a>"
            "<br>Author's blog (RU): "
            "<a href= \"http://pa2311.blogspot.com\">"
            "http://pa2311.blogspot.com</a>"
            "<br><br>This program is free software: you can redistribute it "
            "and/or modify "
            "it under the terms of the GNU General Public License as published "
            "by "
            "the Free Software Foundation, either version 3 of the License, or "
            "(at your option) any later version. "
            "<br>This program is distributed in the hope that it will be "
            "useful, "
            "but WITHOUT ANY WARRANTY; without even the implied warranty of "
            "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
            "GNU General Public License for more details. "
            "<br>You should have received a copy of the GNU General Public "
            "License "
            "along with this program. If not, see <a href= "
            "\"http://www.gnu.org/licenses/\" >"
            "http://www.gnu.org/licenses/</a>.<br>";

    QMessageBox::about(this, tr("About mixan"), str);
}

void MainWindow::reportReadOnlyChanged() {

    if ( checkBox_reportReadOnly->isChecked() ) {

        ui->textBrowser_report->setReadOnly(true);
    }
    else {

        ui->textBrowser_report->setReadOnly(false);
    }
}

void MainWindow::saveIfNecessary() {

    if ( !ui->textBrowser_report->document()->isEmpty() ) {

        ptrdiff_t ret = QMessageBox::question(
                    0,
                    "mixan",
                    tr("Report contains data. Save it?"),
                    QMessageBox::Yes,
                    QMessageBox::No);

        if ( ret == QMessageBox::Yes ) { on_action_saveReport_activated(); }
    }
}
