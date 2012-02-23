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
#include "material.h"
#include "mix.h"
#include "mixfuns.h"
#include "settingsdialog.h"
#include "mixanerror.h"

#include <QString>
#include <QVector>
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
#include <QSpinBox>
#include <QCheckBox>
#include <QSharedPointer>
#include <QFont>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mixanSettings("pa23software", "mixan") {

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
                "</b><br>Analysis of granular material mixes and emulsions." +
                "<br><br><b>" +
                QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") +
                "</b><br><hr><br>"
                );

    //

    settingsDialog = new SettingsDialog();

    spinBox_polyPower = settingsDialog->
            findChild<QSpinBox *>("spinBox_polyPower");
    doubleSpinBox_intersectAccur = settingsDialog->
            findChild<QDoubleSpinBox *>("doubleSpinBox_intersectAccur");
    doubleSpinBox_idealConc = settingsDialog->
            findChild<QDoubleSpinBox *>("doubleSpinBox_idealConc");
    spinBox_imgWidth = settingsDialog->
            findChild<QSpinBox *>("spinBox_imgWidth");
    checkBox_reportReadOnly = settingsDialog->
            findChild<QCheckBox *>("checkBox_reportRO");
    checkBox_createTemporaryGraphics = settingsDialog->
            findChild<QCheckBox *>("checkBox_createTempGraph");

    connect(checkBox_reportReadOnly,
            SIGNAL(clicked()),
            this,
            SLOT(reportReadOnlyChanged())
            );

    //

    readProgramSettings();
}

MainWindow::~MainWindow() {

    writeProgramSettings();

    delete spinBox_polyPower;
    delete doubleSpinBox_intersectAccur;
    delete doubleSpinBox_idealConc;
    delete spinBox_imgWidth;
    delete checkBox_reportReadOnly;
    delete checkBox_createTemporaryGraphics;
    delete settingsDialog;

    delete progressDialog;
    delete futureWatcher;

    delete ui;
}

void MainWindow::forgetSelectedImages() {

    mat1ImageFileName = "";
    mat2ImageFileName = "";
    mixImageFileNames.clear();
}

void MainWindow::runMaterialsAnalysis() {

    try {

        material1->analyze(mat1ImageFileName, spinBox_polyPower->value());
        material2->analyze(mat2ImageFileName, spinBox_polyPower->value());
    }
    catch(MixanError &mixerr) {

        thrmsg += mixerr.mixanErrMsg() + "\n";
        return;
    }
}

void MainWindow::runMixAnalysis() {

    try {

        material1->analyze(mat1ImageFileName, spinBox_polyPower->value());
        material2->analyze(mat2ImageFileName, spinBox_polyPower->value());
    }
    catch(MixanError &mixerr) {

        thrmsg += mixerr.mixanErrMsg() + "\n";
        return;
    }

    //

    probes.clear();

    size_t tcol = defThreshColor(material1.data(),
                                 material2.data(),
                                 doubleSpinBox_intersectAccur->value());

    for ( ptrdiff_t i=0; i<mixImageFileNames.count(); i++ ) {

        try {

            QSharedPointer<Mix> probe(new Mix());
            probe->analyze(mixImageFileNames[i], tcol);
            probes.push_back(probe);
        }
        catch(MixanError &mixerr) {

            thrmsg += mixerr.mixanErrMsg() + "\n";
            continue;
        }
    }
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
    mixanSettings.setValue("/image_width", spinBox_imgWidth->value());
    mixanSettings.setValue("/report_is_read_only",
                           checkBox_reportReadOnly->isChecked());
    mixanSettings.setValue("/create_temporary_graphics",
                           checkBox_createTemporaryGraphics->isChecked());
    mixanSettings.endGroup();
}

void MainWindow::readProgramSettings() {

    mixanSettings.beginGroup("/Settings");
    setGeometry(mixanSettings.value("/window_geometry",
                                    QRect(20, 40, 0, 0)).toRect());
    restoreState(mixanSettings.value("/panels_state").toByteArray());
    spinBox_polyPower->setValue(
                mixanSettings.value("/polynom_power", 6).toInt()
                );
    doubleSpinBox_intersectAccur->setValue(
                mixanSettings.value("/intersection_accuracy", 0.0005).toDouble()
                );
    doubleSpinBox_idealConc->setValue(
                mixanSettings.value("/ideal_concentration", 0.5).toDouble()
                );
    spinBox_imgWidth->setValue(
                mixanSettings.value("/image_width", 400).toInt()
                );
    checkBox_reportReadOnly->setChecked(
                mixanSettings.value("/report_is_read_only", true).toBool()
                );
    checkBox_createTemporaryGraphics->setChecked(
                mixanSettings.value("/create_temporary_graphics").toBool()
                );
    mixanSettings.endGroup();

    //

    reportReadOnlyChanged();
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
                "</b><br>Analysis of granular material mixes and emulsions." +
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

    if ( !thrmsg.isEmpty() ) {

        QMessageBox::warning(this, "mixan", "Analysis completed, but\n\n" +
                             thrmsg);
        thrmsg.clear();
    }
    else {

        QMessageBox::information(this, "mixan", "Analysis completed!");
    }
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

    if ( !thrmsg.isEmpty() ) {

        QMessageBox::warning(this, "mixan", "Analysis completed, but\n\n" +
                             thrmsg);
        thrmsg.clear();
    }
    else {

        QMessageBox::information(this, "mixan", "Analysis completed!");
    }
}

void MainWindow::on_action_settings_activated() {

    settingsDialog->exec();
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

    QMessageBox::about(this, "About mixan", str);
}

void MainWindow::showAnalysisResults() {

    createGraphics();

    //

    ui->textBrowser_report->insertHtml(
                "<br>File of the first material: " +
                mat1ImageFileName +
                "<br>Image of the first material:<br>"
                );
    ui->textBrowser_report->textCursor().insertImage(
                material1->
                originalImage().scaledToWidth(spinBox_imgWidth->value())
                );

    ui->textBrowser_report->insertHtml(
                "<br><br>Characteristic of the first material "
                "(histogram and polynomial approximant):<br>"
                );
    ui->textBrowser_report->textCursor().insertImage(graphics[0]);

    ui->textBrowser_report->insertHtml(
                "<br><br>File of the second material: " +
                mat2ImageFileName +
                "<br>Image of the second material:<br>"
                );
    ui->textBrowser_report->textCursor().insertImage(
                material2->
                originalImage().scaledToWidth(spinBox_imgWidth->value())
                );

    ui->textBrowser_report->insertHtml(
                "<br><br>Characteristic of the second material "
                "(histogram and polynomial approximant):<br>"
                );
    ui->textBrowser_report->textCursor().insertImage(graphics[1]);

    ui->textBrowser_report->insertHtml(
                "<br><br>Visualization of the calculated "
                "gray color threshold "
                "(polynoms and threshold color of mix):<br>"
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
    QVector<double> concs;

    for ( ptrdiff_t i=0; i<probes.size(); i++ ) {

        imgname = probes[i]->imageFileName();

        if ( imgname.isEmpty() ) {

            ui->textBrowser_report->insertHtml(
                        "<br><b>Analysis of image " +
                        imgname +
                        " failed or canceled.</b>"
                        );

            continue;
        }

        conc = probes[i]->concentration();
        concs.push_back(conc);

        ui->textBrowser_report->insertHtml(
                    "<br>Mix image:<br>"
                    );

        ui->textBrowser_report->textCursor().insertImage(
                    probes[i]->
                    originalImage().scaledToWidth(spinBox_imgWidth->value())
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
                QString::number(Vc(concs, doubleSpinBox_idealConc->value())) +
                "</b><br><hr><br>"
                );

    ui->textBrowser_report->moveCursor(QTextCursor::End);

    //

    graphics.clear();
    resetResults();
}

void MainWindow::resetResults() {

    forgetSelectedImages();
    probes.clear();
}

void MainWindow::createGraphics() {

    graphics.clear();

    //

    QVector<double> x(256);
    for ( ptrdiff_t i=0; i<x.size(); i++ ) { x[i] = i; }

    //

    QwtText xAxisTitle("gray color");
    xAxisTitle.setFont(QFont("Liberation Sans", 12));

    QwtText yAxisTitle("n_i / N");
    yAxisTitle.setFont(QFont("Liberation Sans", 12));

    //

    QSharedPointer<QwtPlot> plot1(new QwtPlot());
    plot1->setPalette(QPalette(QColor(Qt::white)));
    plot1->setFrameShape(QFrame::NoFrame);
    plot1->setFrameShadow(QFrame::Plain);
    plot1->setCanvasLineWidth(0);
    plot1->setAxisAutoScale(QwtPlot::xBottom, true);
    plot1->setAxisTitle(QwtPlot::xBottom, xAxisTitle);
    plot1->setAxisAutoScale(QwtPlot::yLeft, true);
    plot1->setAxisTitle(QwtPlot::yLeft, yAxisTitle);

    QSharedPointer<QwtPlotCurve> curve11(new QwtPlotCurve());
    curve11->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve11->setStyle(QwtPlotCurve::NoCurve);
    curve11->setSymbol( new QwtSymbol(QwtSymbol::Ellipse, Qt::NoBrush,
                                      QPen(Qt::black), QSize(1, 1)) );

    QVector<double> y11 = material1->histogramValues();

    curve11->setRawSamples(x.data(), y11.data(), x.size());
    curve11->attach(plot1.data());

    QSharedPointer<QwtPlotCurve> curve12(new QwtPlotCurve());
    curve12->setRenderHint(QwtPlotItem::RenderAntialiased);

    QVector<double> y12 = material1->polynomValues();

    curve12->setRawSamples(x.data(), y12.data(), x.size());
    curve12->attach(plot1.data());

    plot1->resize(600, 400);
    plot1->replot();

    QPixmap pixmap1(plot1->size());
    plot1->render(&pixmap1);

    graphics.push_back(pixmap1.toImage());

    //

    QSharedPointer<QwtPlot> plot2(new QwtPlot());
    plot2->setPalette(QPalette(QColor(Qt::white)));
    plot2->setFrameShape(QFrame::NoFrame);
    plot2->setFrameShadow(QFrame::Plain);
    plot2->setCanvasLineWidth(0);
    plot2->setAxisAutoScale(QwtPlot::xBottom, true);
    plot2->setAxisTitle(QwtPlot::xBottom, xAxisTitle);
    plot2->setAxisAutoScale(QwtPlot::yLeft, true);
    plot2->setAxisTitle(QwtPlot::yLeft, yAxisTitle);

    QSharedPointer<QwtPlotCurve> curve21(new QwtPlotCurve());
    curve21->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve21->setStyle(QwtPlotCurve::NoCurve);
    curve21->setSymbol( new QwtSymbol(QwtSymbol::Ellipse, Qt::NoBrush,
                                      QPen(Qt::black), QSize(1, 1)) );

    QVector<double> y21 = material2->histogramValues();

    curve21->setRawSamples(x.data(), y21.data(), x.size());
    curve21->attach(plot2.data());

    QSharedPointer<QwtPlotCurve> curve22(new QwtPlotCurve());
    curve22->setRenderHint(QwtPlotItem::RenderAntialiased);

    QVector<double> y22 = material2->polynomValues();

    curve22->setRawSamples(x.data(), y22.data(), x.size());
    curve22->attach(plot2.data());

    plot2->resize(600, 400);
    plot2->replot();

    QPixmap pixmap2(plot2->size());
    plot2->render(&pixmap2);

    graphics.push_back(pixmap2.toImage());

    //

    QSharedPointer<QwtPlot> plot3(new QwtPlot());
    plot3->setPalette(QPalette(QColor(Qt::white)));
    plot3->setFrameShape(QFrame::NoFrame);
    plot3->setFrameShadow(QFrame::Plain);
    plot3->setCanvasLineWidth(0);
    plot3->setAxisAutoScale(QwtPlot::xBottom, true);
    plot3->setAxisTitle(QwtPlot::xBottom, xAxisTitle);
    plot3->setAxisAutoScale(QwtPlot::yLeft, true);
    plot3->setAxisTitle(QwtPlot::yLeft, yAxisTitle);

    QSharedPointer<QwtPlotCurve> curve31(new QwtPlotCurve());
    curve31->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve31->setRawSamples(x.data(), y12.data(), x.size());
    curve31->attach(plot3.data());

    QSharedPointer<QwtPlotCurve> curve31_s(new QwtPlotCurve());
    curve31_s->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve31_s->setStyle(QwtPlotCurve::NoCurve);
    curve31_s->setSymbol( new QwtSymbol(QwtSymbol::Ellipse, Qt::NoBrush,
                                        QPen(Qt::black), QSize(7, 7)) );

    QVector<double> x_s;
    QVector<double> y12_s;

    for ( ptrdiff_t i=0; i<x.size(); i+=10 ) {

        x_s.push_back(x[i]);
        y12_s.push_back(y12[i]);
    }

    curve31_s->setRawSamples(x_s.data(), y12_s.data(), x_s.size());
    curve31_s->attach(plot3.data());

    QSharedPointer<QwtPlotCurve> curve32(new QwtPlotCurve());
    curve32->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve32->setRawSamples(x.data(), y22.data(), x.size());
    curve32->attach(plot3.data());

    QSharedPointer<QwtPlotCurve> curve32_s(new QwtPlotCurve());
    curve32_s->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve32_s->setStyle(QwtPlotCurve::NoCurve);
    curve32_s->setSymbol( new QwtSymbol(QwtSymbol::Triangle, Qt::NoBrush,
                                        QPen(Qt::black), QSize(7, 7)) );

    QVector<double> y22_s;

    for ( ptrdiff_t i=0; i<x.size(); i+=10 ) {

        y22_s.push_back(y22[i]);
    }

    curve32_s->setRawSamples(x_s.data(), y22_s.data(), x_s.size());
    curve32_s->attach(plot3.data());

    QSharedPointer<QwtPlotCurve> curve33(new QwtPlotCurve());
    curve33->setRenderHint(QwtPlotItem::RenderAntialiased);

    double tcolm = defThreshColor(material1.data(),
                                  material2.data(),
                                  doubleSpinBox_intersectAccur->value());

    double max1 = y12[material1->thresholdColor()];
    double max2 = y22[material2->thresholdColor()];

    QVector<double> x33(2, tcolm);
    QVector<double> y33(2);

    if ( max1 > max2 ) { y33[1] = max1; }
    else               { y33[1] = max2; }

    curve33->setRawSamples(x33.data(), y33.data(), x33.size());
    curve33->attach(plot3.data());

    plot3->resize(600, 400);
    plot3->replot();

    QPixmap pixmap3(plot3->size());
    plot3->render(&pixmap3);

    graphics.push_back(pixmap3.toImage());

    //

    if ( checkBox_createTemporaryGraphics->isChecked() ) {

        QDir tempDir;

        if ( !tempDir.exists("temp") ) {

            if ( !tempDir.mkdir("temp") ) {

                QMessageBox::warning(this, "mixan",
                                     "Can not create temporary directory!");
            }
        }

        if ( !pixmap1.save("temp/graphic1.png") ) {

            QMessageBox::warning(this, "mixan", "Can not save pixmap to file!");
        }

        if ( !pixmap2.save("temp/graphic2.png") ) {

            QMessageBox::warning(this, "mixan", "Can not save pixmap to file!");
        }

        if ( !pixmap3.save("temp/graphic3.png") ) {

            QMessageBox::warning(this, "mixan", "Can not save pixmap to file!");
        }
    }
}

void MainWindow::reportReadOnlyChanged() {

    if ( checkBox_reportReadOnly->isChecked() ) {

        ui->textBrowser_report->setReadOnly(true);
    }
    else {

        ui->textBrowser_report->setReadOnly(false);
    }
}
