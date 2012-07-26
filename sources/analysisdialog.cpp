/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: analysisdialog.cpp

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

#include "analysisdialog.h"
#include "ui_analysisdialog.h"

#include "settings.h"
#include "material.h"
#include "mix.h"
#include "granules.h"
#include "mixfuns.h"
#include "mixanerror.h"
#include "constants.h"

#include <QTextBrowser>
#include <QFileDialog>
#include <QTextCursor>
#include <QMessageBox>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QPixmap>
#include <QPen>
#include <QPalette>
#include <QDateTime>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_series_data.h>
#include <qwt_plot_histogram.h>

AnalysisDialog::AnalysisDialog(QTextBrowser *txtbrowser,
                               QSharedPointer<Settings> sts,
                               QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AnalysisDialog) {

    ui->setupUi(this);

    report = txtbrowser;
    settings = sts;

    filters = "Images (*.png *.jpg *.jpeg *.bmp);;All files (*.*)";

    //

    material1 = QSharedPointer<Material>(new Material());
    material2 = QSharedPointer<Material>(new Material());

    //

    progressDialog = QSharedPointer<QProgressDialog>(new QProgressDialog());
    progressDialog->setWindowTitle("mixan: progress");

    futureWatcher = QSharedPointer< QFutureWatcher<void> >
            (new QFutureWatcher<void>);

    connect(futureWatcher.data(),
            SIGNAL(finished()),
            this,
            SLOT(showAnalysisResults())
            );

    connect(futureWatcher.data(),
            SIGNAL(finished()),
            progressDialog.data(),
            SLOT(reset())
            );
    connect(progressDialog.data(),
            SIGNAL(canceled()),
            futureWatcher.data(),
            SLOT(cancel())
            );
    connect(futureWatcher.data(),
            SIGNAL(progressRangeChanged(int,int)),
            progressDialog.data(),
            SLOT(setRange(int,int))
            );
    connect(futureWatcher.data(),
            SIGNAL(progressValueChanged(int)),
            progressDialog.data(),
            SLOT(setValue(int))
            );

    tempPath = QDir::homePath()
            + QDir::separator()
            + TMPDIR
            + QDir::separator();
}

AnalysisDialog::~AnalysisDialog() {

    delete ui;
}

void AnalysisDialog::on_comboBox_analysisType_currentIndexChanged(int index) {

    if ( index == ANALTYPE_MIX || index == ANALTYPE_GRANULATION ) {

        ui->listWidget_probesFileNames->setEnabled(true);
        ui->pushButton_selectProbes->setEnabled(true);
    }
    else {

        ui->listWidget_probesFileNames->setEnabled(false);
        ui->pushButton_selectProbes->setEnabled(false);
    }
}

void AnalysisDialog::on_pushButton_selectMat1_clicked() {

    QString mat1ImageFileName =
            QFileDialog::getOpenFileName( this,
                                          "Select image file...",
                                          QDir::currentPath(),
                                          filters,
                                          0,
                                          0 );

    if ( !mat1ImageFileName.isEmpty() ) {

        ui->lineEdit_mat1FileName->setText(mat1ImageFileName);
    }
}

void AnalysisDialog::on_pushButton_selectMat2_clicked() {

    QString mat2ImageFileName =
            QFileDialog::getOpenFileName( this,
                                          "Select image file...",
                                          QDir::currentPath(),
                                          filters,
                                          0,
                                          0 );

    if ( !mat2ImageFileName.isEmpty() ) {

        ui->lineEdit_mat2FileName->setText(mat2ImageFileName);
    }
}

void AnalysisDialog::on_pushButton_selectProbes_clicked() {

    QStringList mixImageFileNames =
            QFileDialog::getOpenFileNames( this,
                                           tr("Select image files..."),
                                           QDir::currentPath(),
                                           filters,
                                           0,
                                           0 );

    ui->listWidget_probesFileNames->clear();

    for ( ptrdiff_t i=0; i<mixImageFileNames.size(); i++ ) {

        ui->listWidget_probesFileNames->addItem(mixImageFileNames[i]);
    }
}

void AnalysisDialog::on_pushButton_run_clicked() {

    if ( ( ( ui->comboBox_analysisType->currentIndex() == ANALTYPE_MATERIALS )
           &&
           ( ui->lineEdit_mat1FileName->text().isEmpty() ||
             ui->lineEdit_mat2FileName->text().isEmpty() ) )
         ||
         ( ( ui->comboBox_analysisType->currentIndex() == ANALTYPE_MIX ||
             ui->comboBox_analysisType->currentIndex() == ANALTYPE_GRANULATION )
           &&
           ( ui->lineEdit_mat1FileName->text().isEmpty() ||
             ui->lineEdit_mat2FileName->text().isEmpty() ||
             ui->listWidget_probesFileNames->count() == 0 ) ) ) {

        QMessageBox::warning(this,
                             "mixan",
                             "Not enough sources data to analysis.\n\n");
        return;
    }

    progressDialog->setLabelText("Images analysis. Please wait...");
    futureWatcher->setFuture(QtConcurrent::
                             run(this, &AnalysisDialog::runAnalysis));
    progressDialog->exec();
    futureWatcher->waitForFinished();

    //

    if ( !thrmsg.isEmpty() ) {

        QMessageBox::warning(this, "mixan", "Analysis completed, but\n\n" +
                             thrmsg);
    }
    else {

        QMessageBox::information(this, "mixan", "Analysis completed!");
    }
}

void AnalysisDialog::on_pushButton_clear_clicked() {

    ui->lineEdit_mat1FileName->clear();
    ui->lineEdit_mat2FileName->clear();
    ui->listWidget_probesFileNames->clear();
}

void AnalysisDialog::runAnalysis() {

    try {

        material1->analyze( ui->lineEdit_mat1FileName->text(),
                            settings->val_polyPwr() );
        material2->analyze( ui->lineEdit_mat2FileName->text(),
                            settings->val_polyPwr() );
    }
    catch(MixanError &mixerr) {

        thrmsg += mixerr.mixanErrMsg() + "\n";
        return;
    }

    if ( ui->comboBox_analysisType->currentIndex() == ANALTYPE_MIX ) {

        probes.clear();

        size_t tcol = defThreshColor( material1.data(),
                                      material2.data(),
                                      settings->val_thrAccur() );

        for ( ptrdiff_t i=0; i<ui->listWidget_probesFileNames->count(); i++ ) {

            try {

                QSharedPointer<Mix> probe(new Mix(ui->
                                                  listWidget_probesFileNames->
                                                  item(i)->text(),
                                                  tcol));
                probe->analyze();
                probes.push_back(probe);
            }
            catch(MixanError &mixerr) {

                thrmsg += mixerr.mixanErrMsg() + "\n";
                continue;
            }
        }
    }
    else if ( ui->comboBox_analysisType->currentIndex() ==
              ANALTYPE_GRANULATION ) {

        granules.clear();

        size_t tcol = defThreshColor( material1.data(),
                                      material2.data(),
                                      settings->val_thrAccur() );

        size_t limcol1 = 0;
        size_t limcol2 = 0;

        if ( material1->thresholdColor() < tcol ) {

            limcol1 = 0;
            limcol2 = tcol;
        }
        else {

            limcol1 = tcol;
            limcol2 = 255;
        }

        for ( ptrdiff_t i=0; i<ui->listWidget_probesFileNames->count(); i++ ) {

            try {

                QSharedPointer<Granules>
                        grans(new Granules(ui->listWidget_probesFileNames->
                                           item(i)->text(),
                                           limcol1,
                                           limcol2));
                grans->analyze();
                granules.push_back(grans);
            }
            catch(MixanError &mixerr) {

                thrmsg += mixerr.mixanErrMsg() + "\n";
                continue;
            }
        }
    }
}

void AnalysisDialog::showAnalysisResults() {

    bool showImg = settings->val_showImgInReport();
    bool createTmpImgFiles = settings->val_createTmpImg();
    ptrdiff_t imgWidth = settings->val_imgWidth();

    createGraphics();

    //

    report->moveCursor(QTextCursor::End);

    report->insertHtml(
                "<b>"
                + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss")
                + "</b><br><br>"
                "Settings"
                "<br>* Type of analysis: "
                + ui->comboBox_analysisType->currentText()
                + "<br>* Power of the approximate polynom: "
                + QString::number(settings->val_polyPwr())
                + "<br>* Accuracy of color threshold determining: "
                + QString::number(settings->val_thrAccur())
                );

    if ( ui->comboBox_analysisType->currentIndex() == ANALTYPE_MIX ) {

        report->insertHtml(
                    "<br>* Ideal concentration: " +
                    QString::number(settings->val_idealConc())
                    );
    }

    if ( !thrmsg.isEmpty() ) {

        report->insertHtml("<br><br><b>" + thrmsg + "</b><br><hr><br>");
        report->moveCursor(QTextCursor::End);

        thrmsg.clear();
        return;
    }

    report->insertHtml("<br><br>First material image file: "
                       + ui->lineEdit_mat1FileName->text());

    if ( showImg ) {

        report->insertHtml("<br><br>Image of the first material:<br>");

        if ( material1->originalImage().width() > imgWidth ) {

            report->textCursor().insertImage(
                        material1->
                        originalImage().scaledToWidth(imgWidth)
                        );
        }
        else {

            report->textCursor().insertImage(material1->originalImage());
        }
    }

    report->insertHtml(
                "<br><br>Characteristic of the first material "
                "(histogram and polynomial approximant):<br>"
                );
    report->textCursor().insertImage(graphics[0]);

    report->insertHtml("<br><br>Second material image file: "
                       + ui->lineEdit_mat2FileName->text());

    if ( showImg ) {

        report->insertHtml("<br><br>Image of the second material:<br>");

        if ( material2->originalImage().width() > imgWidth ) {

            report->textCursor().insertImage(
                        material2->
                        originalImage().scaledToWidth(imgWidth)
                        );
        }
        else {

            report->textCursor().insertImage(material2->originalImage());
        }
    }

    report->insertHtml(
                "<br><br>Characteristic of the second material "
                "(histogram and polynomial approximant):<br>"
                );
    report->textCursor().insertImage(graphics[1]);

    report->insertHtml(
                "<br><br>Visualization of the calculated "
                "gray color threshold "
                "(polynoms and threshold color of mix):<br>"
                );
    report->textCursor().insertImage(graphics[2]);

    report->insertHtml("<br>");

    //

    if ( ui->comboBox_analysisType->currentIndex() == ANALTYPE_MIX &&
         probes.size() != 0 ) {

        QString imgname;

        double conc = 0;
        QVector<double> concs;

        for ( ptrdiff_t i=0; i<probes.size(); i++ ) {

            imgname = probes[i]->imageFileName();

            if ( imgname.isEmpty() ) {

                report->insertHtml(
                            "<br><b>Analysis of image " +
                            imgname +
                            " failed or canceled.</b>"
                            );

                continue;
            }

            conc = probes[i]->concentration();
            concs.push_back(conc);

            if ( showImg ) {

                report->insertHtml("<br>Mix image:<br>");

                if ( probes[i]->originalImage().width() > imgWidth ) {

                    report->textCursor().insertImage(
                                probes[i]->originalImage().
                                scaledToWidth(settings->val_imgWidth())
                                );
                }
                else {

                    report->textCursor().insertImage(
                                probes[i]->originalImage()
                                );
                }
            }

            report->insertHtml(
                        "<br>Mix image file: " +
                        imgname +
                        "<br>Concentration of the first component = <b>" +
                        QString::number(conc) +
                        "</b><br>"
                        );
        }

        if ( probes.size() < 2 ) {

            report->insertHtml("<hr><br>");
        }
        else {

            report->insertHtml(
                        "<br><b>Vc = " +
                        QString::number(Vc(concs, settings->val_idealConc())) +
                        "</b><br><hr><br>"
                        );
        }

        report->moveCursor(QTextCursor::End);
    }
    else if ( ui->comboBox_analysisType->currentIndex() ==
              ANALTYPE_GRANULATION &&
              granules.size() != 0 ) {

        if ( settings->val_createTmpImg() ) {

            QDir tempDir;

            if ( !tempDir.exists(tempPath) ) {

                if ( !tempDir.mkpath(tempPath) ) {

                    QMessageBox::warning(this, "mixan",
                                         "Can not create temporary directory!");
                }
            }
        }

        //

        createHistograms();

        QString imgname;

        for ( ptrdiff_t i=0; i<granules.size(); i++ ) {

            imgname = granules[i]->imageFileName();

            if ( imgname.isEmpty() ) {

                report->insertHtml(
                            "<br><b>Analysis of image " +
                            imgname +
                            " failed or canceled.</b>"
                            );

                continue;
            }

            if ( showImg ) {

                report->insertHtml("<br>Granules image:<br>");

                if ( granules[i]->resImage().width() > imgWidth ) {

                    report->textCursor().insertImage(
                                granules[i]->resImage().
                                scaledToWidth(settings->val_imgWidth())
                                );
                }
                else {

                    report->textCursor().insertImage(
                                granules[i]->resImage()
                                );
                }
            }

            if ( createTmpImgFiles ) {

                QPixmap tmpPxp = QPixmap::fromImage(granules[i]->resImage());

                if ( !tmpPxp.save(tempPath
                                  + "granules_image_"
                                  + QString::number(i)
                                  + "__"
                                  + QDateTime::currentDateTime().
                                    toString("dd-MM-yyyy_hh-mm-ss")
                                  + ".png") ) {

                    QMessageBox::warning(this,
                                         "mixan",
                                         "Can not save pixmap to file!");
                }
            }

            report->insertHtml(
                        "<br><br>Image file: "
                        + imgname
                        + "<br>"
                        );

            report->insertHtml("<br>Particle-size distribution:<br>");
            report->textCursor().insertImage(histograms[i]);
        }

        report->insertHtml("<br><hr><br>");
        report->moveCursor(QTextCursor::End);
    }
    else {

        report->insertHtml("<br><hr><br>");
        report->moveCursor(QTextCursor::End);
    }

    //

    graphics.clear();
    histograms.clear();
    material1->clear();
    material2->clear();
    probes.clear();
    granules.clear();
}

void AnalysisDialog::createGraphics() {

    graphics.clear();

    //

    if ( material1->isEmpty() || material2->isEmpty() ) { return; }

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
                                  settings->val_thrAccur());

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

    if ( settings->val_createTmpImg() ) {

        QDir tempDir;

        if ( !tempDir.exists(tempPath) ) {

            if ( !tempDir.mkpath(tempPath) ) {

                QMessageBox::warning(this, "mixan",
                                     "Can not create temporary directory!");
            }
        }

        if ( !pixmap1.save(tempPath
                           + "graphic_0"
                           + "__"
                           + QDateTime::currentDateTime().
                             toString("dd-MM-yyyy_hh-mm-ss")
                           + ".png") ) {

            QMessageBox::warning(this, "mixan", "Can not save pixmap to file!");
        }

        if ( !pixmap2.save(tempPath
                           + "graphic_1"
                           + "__"
                           + QDateTime::currentDateTime().
                             toString("dd-MM-yyyy_hh-mm-ss")
                           + ".png") ) {

            QMessageBox::warning(this, "mixan", "Can not save pixmap to file!");
        }

        if ( !pixmap3.save(tempPath
                           + "graphic_2"
                           + "__"
                           + QDateTime::currentDateTime().
                             toString("dd-MM-yyyy_hh-mm-ss")
                           + ".png") ) {

            QMessageBox::warning(this, "mixan", "Can not save pixmap to file!");
        }
    }
}

void AnalysisDialog::createHistograms() {

    histograms.clear();

    //

    if ( granules.size() == 0 ) { return; }

    //

    for ( ptrdiff_t n=0; n<granules.size(); n++ ) {

        double minval = granules[n]->histXSetup().minval;
        double step = granules[n]->histXSetup().step;

        QVector<double> histvls = granules[n]->histValues();

        //

        QVector<QwtIntervalSample> histdata;

        double tmpmin = minval;
        double tmpmax = tmpmin + step;

        for ( ptrdiff_t i=0; i<HISTDIMENSION; i++ ) {

            histdata.push_back(QwtIntervalSample(histvls[i], tmpmin, tmpmax));

            tmpmin += step;
            tmpmax += step;
        }

        //

        QwtText xAxisTitle("Granule area");
        xAxisTitle.setFont(QFont("Liberation Sans", 12));

        QwtText yAxisTitle("n_i / N");
        yAxisTitle.setFont(QFont("Liberation Sans", 12));

        QSharedPointer<QwtPlot> histogram(new QwtPlot());
        histogram->setPalette(QPalette(QColor(Qt::white)));
        histogram->setFrameShape(QFrame::NoFrame);
        histogram->setFrameShadow(QFrame::Plain);
        histogram->setCanvasLineWidth(0);
        histogram->setAxisTitle(QwtPlot::xBottom, xAxisTitle);
        histogram->setAxisTitle(QwtPlot::yLeft, yAxisTitle);

        QSharedPointer<QwtPlotHistogram> hist(new QwtPlotHistogram());
        hist->setStyle(QwtPlotHistogram::Columns);
        hist->setRenderHint(QwtPlotItem::RenderAntialiased);

        hist->setSamples(histdata);
        hist->attach(histogram.data());

        histogram->resize(600, 400);
        histogram->replot();

        QPixmap pixmap(histogram->size());
        histogram->render(&pixmap);

        histograms.push_back(pixmap.toImage());

        if ( settings->val_createTmpImg() ) {

            QDir tempDir;

            if ( !tempDir.exists(tempPath) ) {

                if ( !tempDir.mkpath(tempPath) ) {

                    QMessageBox::warning(this, "mixan",
                                         "Can not create temporary directory!");
                }
            }

            if ( !pixmap.save(tempPath
                              + "histogram_"
                              + QString::number(n)
                              + "__"
                              + QDateTime::currentDateTime().
                                toString("dd-MM-yyyy_hh-mm-ss")
                              + ".png") ) {

                QMessageBox::warning(this, "mixan",
                                     "Can not save pixmap to file!");
            }
        }
    }
}
