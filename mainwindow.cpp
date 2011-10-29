/*
    mixan
    Analysis of granular material mix and emulsions.

    File: mainwindow.cpp

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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "constants.h"
#include "material.h"
#include "mix.h"
#include "mixfuns.h"
#include "settingsdialog.h"

#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QDir>
#include <QPixmap>
#include <QPen>
#include <QPalette>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextCursor>
#include <QDateTime>
#include <QThread>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QSettings>
#include <QRect>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QSharedPointer>

//#include <omp.h>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_item.h>
#include <qwt_symbol.h>

#include <vector>

using std::vector;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mixanSettings("pa23software", "mixan"),
    polynomPower(10),
    imageWidth(400),
    reportReadOnly(1) {

    ui->setupUi(this);

    setWindowTitle("mixan " + VERSION);

    //

    material1 = QSharedPointer<Material>(new Material());
    material2 = QSharedPointer<Material>(new Material());

    //

    progressDialog = new QProgressDialog;
    progressDialog->setWindowTitle("mixan");

    futureWatcher = new QFutureWatcher<void>;

    connect(futureWatcher,
            SIGNAL(finished()),
            this,
            SLOT(showAnalysisResults())
            );
    connect(progressDialog,
            SIGNAL(canceled()),
            this,
            SLOT(resetResults())
            );

    connect(futureWatcher,
            SIGNAL(finished()),
            progressDialog,
            SLOT(reset())
            );
    connect(progressDialog,
            SIGNAL(canceled()),
            futureWatcher,
            SLOT(cancel())
            );
    connect(futureWatcher,
            SIGNAL(progressRangeChanged(int,int)),
            progressDialog,
            SLOT(setRange(int,int))
            );
    connect(futureWatcher,
            SIGNAL(progressValueChanged(int)),
            progressDialog,
            SLOT(setValue(int))
            );

    //

    ui->textBrowser_report->setUndoRedoEnabled(false);

    ui->textBrowser_report->setHtml(
                "<br><b>mixan " +
                VERSION +
                "</b><br>Analysis of granular material mix and emulsions." +
                "<br><br><b>" +
                QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") +
                "</b><br><hr><br>"
                );

    //

    settingsDialog = new SettingsDialog();

    checkBox_reportRO = settingsDialog->
            findChild<QCheckBox *>("checkBox_reportRO");

    pushButton_settingsOK = settingsDialog->
            findChild<QPushButton *>("pushButton_OK");

    connect(pushButton_settingsOK,
            SIGNAL(clicked()),
            this,
            SLOT(saveSettings())
            );

    //

    readProgramSettings();
}

MainWindow::~MainWindow() {

    writeProgramSettings();

    delete ui;

    delete progressDialog;
    delete futureWatcher;

    delete settingsDialog;
}

void MainWindow::forgetSelectedImages() {

    mat1ImageFileName = "";
    mat2ImageFileName = "";
    mixImageFileNames.clear();
}

void MainWindow::runMaterialsAnalysis() {

    if ( !material1.data()->analyze(mat1ImageFileName, polynomPower) ) {

        return;
    }

    if ( !material2.data()->analyze(mat2ImageFileName, polynomPower) ) {

        return;
    }
}

void MainWindow::runMixAnalysis() {

    if ( !material1.data()->analyze(mat1ImageFileName, polynomPower) ) {

        return;
    }

    if ( !material2.data()->analyze(mat2ImageFileName, polynomPower) ) {

        return;
    }

    //

    size_t lcol = material1.data()->thresholdColor();
    size_t dcol = material2.data()->thresholdColor();

    probes.clear();

    //    ptrdiff_t i = 0;
    //#pragma omp parallel for shared(lcol, dcol) private(i)

    for ( ptrdiff_t i=0; i<mixImageFileNames.count(); i++ ) {

        QSharedPointer<Mix> probe = QSharedPointer<Mix>(new Mix());

        if ( !probe->analyze( mixImageFileNames[i],
                              Mix::defThreshColor(lcol, dcol) ) ) {

            continue;
        }

        probes.push_back(probe);
    }
}

void MainWindow::writeProgramSettings() {

    mixanSettings.beginGroup("/Settings");
    mixanSettings.setValue("/window_geometry", geometry());
    mixanSettings.setValue("/panels_state", QMainWindow::saveState());
    mixanSettings.setValue("/polynom_power", (int)polynomPower);
    mixanSettings.setValue("/image_width", (uint)imageWidth);
    mixanSettings.setValue("/report_is_read_only", reportReadOnly);
    mixanSettings.endGroup();
}

void MainWindow::readProgramSettings() {

    mixanSettings.beginGroup("/Settings");
    setGeometry(mixanSettings.value("/window_geometry",
                                    QRect(20, 40, 0, 0)).toRect());
    restoreState(mixanSettings.value("/panels_state").toByteArray());
    polynomPower = mixanSettings.value("/polynom_power", 10).toInt();
    imageWidth = mixanSettings.value("/image_width", 400).toUInt();
    reportReadOnly = mixanSettings.value("/report_is_read_only", true).toBool();
    mixanSettings.endGroup();
}

void MainWindow::on_action_selectMaterialImages_activated() {

    mat1ImageFileName = "";
    mat2ImageFileName = "";

    QString filters = "Images (*.png *.jpg *.jpeg *.bmp);;All files (*.*)";

    mat1ImageFileName =
            QFileDialog::getOpenFileName(
                this,
                tr("Select first component image file..."),
                QDir::currentPath(),
                filters,
                0,
                0
                );

    mat2ImageFileName =
            QFileDialog::getOpenFileName(
                this,
                tr("Select second component image file..."),
                QDir::currentPath(),
                filters,
                0,
                0
                );

    if ( mat1ImageFileName.isEmpty() ||
         mat2ImageFileName.isEmpty() ) {

        QMessageBox::information(this, "mixan",
                                 "Not enough images for analysis :(");
        return;
    }

    //

    ui->textBrowser_report->moveCursor(QTextCursor::End);

    ui->textBrowser_report->insertHtml(
                "Material images selected.<br>"
                );

    //

    ui->textBrowser_report->moveCursor(QTextCursor::End);
}

void MainWindow::on_action_selectProbeImages_activated() {

    mixImageFileNames.clear();

    mixImageFileNames =
            QFileDialog::getOpenFileNames(
                this,
                tr("Select image files of mix..."),
                QDir::currentPath(),
                "Images (*.png *.jpg *.jpeg *.bmp);;All files (*.*)",
                0,
                0);

    if ( mixImageFileNames.count() == 0 ) {

        QMessageBox::information(this, "mixan",
                                 "Not enough images for analysis :(");
        return;
    }

    //

    ui->textBrowser_report->moveCursor(QTextCursor::End);

    ui->textBrowser_report->insertHtml(
                QString::number(mixImageFileNames.count()) +
                " probe images selected.<br>"
                );

    //

    ui->textBrowser_report->moveCursor(QTextCursor::End);
}

void MainWindow::on_action_saveReport_activated() {

    QString filters = "PDF files (*.pdf);;All files (*.*)";

    QString reportFileName(QFileDialog::getSaveFileName(
                               this,
                               "Save report to PDF...",
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

void MainWindow::on_action_cleanReportWindow_activated() {

    ui->textBrowser_report->clear();

    ui->textBrowser_report->setHtml(
                "<br><b>mixan " +
                VERSION +
                "</b><br>Analysis of granular material mix and emulsions." +
                "<br><br><b>" +
                QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") +
                "</b><br><hr><br>"
                );
}

void MainWindow::on_action_quit_activated() {

    close();
}

void MainWindow::on_action_analyzeMaterials_activated() {

    if ( mat1ImageFileName.isEmpty() ||
         mat2ImageFileName.isEmpty() ) {

        QMessageBox::information(this, "mixan",
                                 "Not enough images for analysis :(");
        return;
    }

    //

    material1.clear();
    material1 = QSharedPointer<Material>(new Material());

    material2.clear();
    material2 = QSharedPointer<Material>(new Material());

    //

    ui->textBrowser_report->moveCursor(QTextCursor::End);

    ui->textBrowser_report->insertHtml(
                "<br><b><u>Results of materials analysis:</u></b><br>"
                );

    //

    progressDialog->setLabelText("Images analysis. Please wait...");
    futureWatcher->setFuture(QtConcurrent::
                             run(this, &MainWindow::runMaterialsAnalysis));
    progressDialog->exec();
    futureWatcher->waitForFinished();

    //

    QMessageBox::information(this, "mixan", "Analysis completed!");
}

void MainWindow::on_action_analyzeMix_activated() {

    if ( mat1ImageFileName.isEmpty() ||
         mat2ImageFileName.isEmpty() ||
         (mixImageFileNames.count() == 0) ) {

        QMessageBox::information(this, "mixan",
                                 "Not enough images for analysis :(");
        return;
    }

    //

    //

    //

    ui->textBrowser_report->moveCursor(QTextCursor::End);

    ui->textBrowser_report->insertHtml(
                "<br><b><u>Results of mix analysis:</u></b><br>"
                );

    //

    progressDialog->setLabelText("Images analysis. Please wait...");
    futureWatcher->setFuture(QtConcurrent::run(this,
                                               &MainWindow::runMixAnalysis));
    progressDialog->exec();
    futureWatcher->waitForFinished();

    //

    QMessageBox::information(this, "mixan", "Analysis completed!");
}

void MainWindow::on_action_settings_activated() {

    settingsDialog->exec();
}

void MainWindow::on_action_about_mixan_activated() {

    QString str = "<b>mixan " + VERSION + "</b>\n"
            "<br><br>Analysis of granular material mix and emulsions."
            "<br><br>Copyright (C) 2011 Artem Petrov "
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

    QMessageBox::about(this, "About mixan", str);
}

void MainWindow::showAnalysisResults() {

    QVector<QImage> graphics = createGraphics();

    //

    ui->textBrowser_report->insertHtml(
                "<br>Image of the first material:<br>"
                );
    ui->textBrowser_report->textCursor().insertImage(
                material1.data()->originalImage().scaledToWidth(imageWidth)
                );

    ui->textBrowser_report->insertHtml(
                "<br><br>Characteristic of the first material:<br>"
                );
    ui->textBrowser_report->textCursor().insertImage(graphics[0]);

    ui->textBrowser_report->insertHtml(
                "<br><br>Image of the second material:<br>"
                );
    ui->textBrowser_report->textCursor().insertImage(
                material2.data()->originalImage().scaledToWidth(imageWidth)
                );

    ui->textBrowser_report->insertHtml(
                "<br><br>Characteristic of the second material:<br>"
                );
    ui->textBrowser_report->textCursor().insertImage(graphics[1]);

    ui->textBrowser_report->insertHtml(
                "<br><br>Visualization of the calculated "
                "gray color threshold:<br>"
                );
    ui->textBrowser_report->textCursor().insertImage(graphics[2]);

    ui->textBrowser_report->insertHtml(
                "<br>"
                );

    //

    if ( probes.size() == 0 ) {

        ui->textBrowser_report->insertHtml(
                    "<br><hr><br>"
                    );

        ui->textBrowser_report->moveCursor(QTextCursor::End);

        resetResults();

        return;
    }

    QString imgname;

    double conc = 0;
    vector<double> concs;

    for ( ptrdiff_t i=0; i<probes.size(); i++ ) {

        imgname = probes[i].data()->imageFileName();

        if ( imgname.isEmpty() ) {

            ui->textBrowser_report->insertHtml(
                        "<br><b>Analysis of image " +
                        imgname +
                        " failed or canceled.</b>"
                        );

            continue;
        }

        conc = probes[i].data()->concentration();
        concs.push_back(conc);

        ui->textBrowser_report->insertHtml(
                    "<br>Mix image:<br>"
                    );

        ui->textBrowser_report->textCursor().insertImage(
                    probes[i].data()->originalImage().scaledToWidth(imageWidth)
                    );

        ui->textBrowser_report->insertHtml(
                    "<br>File path: " +
                    imgname +
                    "<br>Concentration of the first component = <b>" +
                    QString::number(conc) +
                    "</b><br>"
                    );
    }

    ui->textBrowser_report->insertHtml(
                "<br><b>Vc = " +
                QString::number(Vc(&concs)) +
                "</b><br><hr><br>"
                );

    ui->textBrowser_report->moveCursor(QTextCursor::End);

    //

    resetResults();
}

void MainWindow::resetResults() {

    forgetSelectedImages();
    probes.clear();
}

QVector<QImage> MainWindow::createGraphics() {

    QVector<QImage> graphics;

    //

    double *x = new double[256];
    for ( ptrdiff_t i=0; i<256; i++ ) { x[i] = i; }

    //

    QwtPlot *plot1 = new QwtPlot();
    plot1->setAxisAutoScale(QwtPlot::xBottom, true);
    plot1->setAxisAutoScale(QwtPlot::yLeft, true);
    plot1->setAutoFillBackground(true);

    QwtPlotCurve *curve11 = new QwtPlotCurve();
    curve11->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve11->setStyle(QwtPlotCurve::NoCurve);
    curve11->setSymbol( new QwtSymbol(QwtSymbol::Ellipse, Qt::NoBrush,
                                      QPen(Qt::black), QSize(1, 1)) );

    vector<size_t> v11 = material1.data()->histogramValues();
    double *y11 = new double[256];
    for ( ptrdiff_t i=0; i<256; i++ ) { y11[i] = v11[i]; }

    curve11->setRawSamples(x, y11, 256);
    curve11->attach(plot1);

    QwtPlotCurve *curve12 = new QwtPlotCurve();
    curve12->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve12->setStyle(QwtPlotCurve::Lines);
    curve12->setPen(QPen(Qt::red));

    vector<double> v12 = material1.data()->polynomValues();
    double *y12 = new double[256];
    for ( ptrdiff_t i=0; i<256; i++ ) { y12[i] = v12[i]; }

    curve12->setRawSamples(x, y12, 256);
    curve12->attach(plot1);

    plot1->resize(600, 300);
    plot1->replot();

    QPixmap pixmap1(plot1->size());
    plot1->render(&pixmap1);

    graphics.push_back(pixmap1.toImage());

    delete curve11;
    delete curve12;
    delete plot1;

    //

    QwtPlot *plot2 = new QwtPlot();
    plot2->setAxisAutoScale(QwtPlot::xBottom, true);
    plot2->setAxisAutoScale(QwtPlot::yLeft, true);
    plot2->setAutoFillBackground(true);

    QwtPlotCurve *curve21 = new QwtPlotCurve();
    curve21->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve21->setStyle(QwtPlotCurve::NoCurve);
    curve21->setSymbol( new QwtSymbol(QwtSymbol::Ellipse, Qt::NoBrush,
                                      QPen(Qt::black), QSize(1, 1)) );

    vector<size_t> v21 = material2.data()->histogramValues();
    double *y21 = new double[256];
    for ( ptrdiff_t i=0; i<256; i++ ) { y21[i] = v21[i]; }

    curve21->setRawSamples(x, y21, 256);
    curve21->attach(plot2);

    QwtPlotCurve *curve22 = new QwtPlotCurve();
    curve22->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve22->setStyle(QwtPlotCurve::Lines);
    curve22->setPen(QPen(Qt::blue));

    vector<double> v22 = material2.data()->polynomValues();
    double *y22 = new double[256];
    for ( ptrdiff_t i=0; i<256; i++ ) { y22[i] = v22[i]; }

    curve22->setRawSamples(x, y22, 256);
    curve22->attach(plot2);

    plot2->resize(600, 300);
    plot2->replot();

    QPixmap pixmap2(plot2->size());
    plot2->render(&pixmap2);

    graphics.push_back(pixmap2.toImage());

    delete curve21;
    delete curve22;
    delete plot2;

    //

    QwtPlot *plot3 = new QwtPlot();
    plot3->setAxisAutoScale(QwtPlot::xBottom, true);
    plot3->setAxisAutoScale(QwtPlot::yLeft, true);
    plot3->setAutoFillBackground(true);

    QwtPlotCurve *curve31 = new QwtPlotCurve();
    curve31->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve31->setStyle(QwtPlotCurve::Lines);
    curve31->setPen(QPen(Qt::red));
    curve31->setRawSamples(x, y12, 256);
    curve31->attach(plot3);

    QwtPlotCurve *curve32 = new QwtPlotCurve();
    curve32->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve32->setStyle(QwtPlotCurve::Lines);
    curve32->setPen(QPen(Qt::blue));
    curve32->setRawSamples(x, y22, 256);
    curve32->attach(plot3);

    QwtPlotCurve *curve33 = new QwtPlotCurve();
    curve33->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve33->setStyle(QwtPlotCurve::Lines);
    curve33->setPen(QPen(Qt::black));

    size_t lcol = material1.data()->thresholdColor();
    size_t dcol = material2.data()->thresholdColor();
    double tcol = Mix::defThreshColor(lcol, dcol);

    size_t max1 = 0;
    size_t max2 = 0;

    for ( size_t i=0; i<v12.size(); i++ ) {

        if ( v12[i] > max1 ) { max1 = v12[i]; }
    }

    for ( size_t i=0; i<v22.size(); i++ ) {

        if ( v22[i] > max2 ) { max2 = v22[i]; }
    }

    double *x33 = new double[2];
    x33[0] = tcol;
    x33[1] = tcol;

    double *y33 = new double[2];
    y33[0] = 0;

    if ( max1 > max2 ) { y33[1] = max1; }
    else               { y33[1] = max2; }

    curve33->setRawSamples(x33, y33, 2);
    curve33->attach(plot3);

    plot3->resize(600, 300);
    plot3->replot();

    QPixmap pixmap3(plot3->size());
    plot3->render(&pixmap3);

    graphics.push_back(pixmap3.toImage());

    delete curve31;
    delete curve32;
    delete curve33;
    delete plot3;

    //

    delete [] x;

    delete [] y11;
    delete [] y12;

    delete [] y21;
    delete [] y22;

    delete [] x33;
    delete [] y33;

    //

    return graphics;
}

void MainWindow::saveSettings() {

    QSpinBox *spinBox_polyPower = settingsDialog->
            findChild<QSpinBox *>("spinBox_polyPower");
    QSpinBox *spinBox_imgWidth = settingsDialog->
            findChild<QSpinBox *>("spinBox_imgWidth");
    QCheckBox *checkBox_reportReadOnly = settingsDialog->
            findChild<QCheckBox *>("checkBox_reportRO");

    polynomPower = spinBox_polyPower->value();
    imageWidth = spinBox_imgWidth->value();
    reportReadOnly = checkBox_reportReadOnly->isChecked();

    if ( reportReadOnly ) {

        ui->textBrowser_report->setReadOnly(true);
    }
    else {

        ui->textBrowser_report->setReadOnly(false);
    }
}
