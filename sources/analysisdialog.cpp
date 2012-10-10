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
#include "graphics.h"
#include "tmpfiles.h"

#include <QTextBrowser>
#include <QFileDialog>
#include <QTextCursor>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QPixmap>
#include <QDateTime>

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

    lastCalcDateTime = "";
    lastImgDir = "";
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

    QString mat1ImageFileName = "";

    if ( !lastImgDir.isEmpty() ) {

        mat1ImageFileName =
                QFileDialog::getOpenFileName( this,
                                              tr("Select image file..."),
                                              lastImgDir,
                                              filters,
                                              0,
                                              0 );
    }
    else {

        mat1ImageFileName =
                QFileDialog::getOpenFileName( this,
                                              tr("Select image file..."),
                                              QDir::currentPath(),
                                              filters,
                                              0,
                                              0 );
    }

    if ( !mat1ImageFileName.isEmpty() ) {

        ui->lineEdit_mat1FileName->setText(mat1ImageFileName);

        QFileInfo fileinfo(mat1ImageFileName);
        lastImgDir = fileinfo.absolutePath();
    }
}

void AnalysisDialog::on_pushButton_selectMat2_clicked() {

    QString mat2ImageFileName = "";

    if ( !lastImgDir.isEmpty() ) {

        mat2ImageFileName =
                QFileDialog::getOpenFileName( this,
                                              tr("Select image file..."),
                                              lastImgDir,
                                              filters,
                                              0,
                                              0 );
    }
    else {

        mat2ImageFileName =
                QFileDialog::getOpenFileName( this,
                                              tr("Select image file..."),
                                              QDir::currentPath(),
                                              filters,
                                              0,
                                              0 );
    }

    if ( !mat2ImageFileName.isEmpty() ) {

        ui->lineEdit_mat2FileName->setText(mat2ImageFileName);

        QFileInfo fileinfo(mat2ImageFileName);
        lastImgDir = fileinfo.absolutePath();
    }
}

void AnalysisDialog::on_pushButton_selectProbes_clicked() {

    QStringList mixImageFileNames;

    if ( !lastImgDir.isEmpty() ) {

        mixImageFileNames =
                QFileDialog::getOpenFileNames( this,
                                               tr("Select image files..."),
                                               lastImgDir,
                                               filters,
                                               0,
                                               0 );
    }
    else {

        mixImageFileNames =
                QFileDialog::getOpenFileNames( this,
                                               tr("Select image files..."),
                                               QDir::currentPath(),
                                               filters,
                                               0,
                                               0 );
    }

    ui->listWidget_probesFileNames->clear();

    if ( mixImageFileNames.count() != 0 ) {

        for ( ptrdiff_t i=0; i<mixImageFileNames.size(); i++ ) {

            ui->listWidget_probesFileNames->addItem(mixImageFileNames[i]);
        }

        QFileInfo fileinfo(mixImageFileNames[0]);
        lastImgDir = fileinfo.absolutePath();
    }
}

void AnalysisDialog::on_pushButton_run_clicked() {

    hide();

    //

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
                             tr("Not enough sources data to analysis.")
                             + "\n\n");
        return;
    }

    //

    progressDialog->setLabelText(tr("Images analysis. Please wait..."));
    futureWatcher->setFuture(QtConcurrent::
                             run(this, &AnalysisDialog::runAnalysis));
    progressDialog->exec();
    futureWatcher->waitForFinished();

    //

    if ( !thrmsg.isEmpty() ) {

        QMessageBox::warning(this, "mixan", tr("Analysis completed, but")
                             + "\n\n"
                             + thrmsg);
    }
    else {

        QMessageBox::information(this, "mixan", tr("Analysis completed!"));
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

    lastCalcDateTime =
            QDateTime::currentDateTime().toString("dd-MM-yyyy_hh-mm-ss");

    bool showImg = settings->val_showImgInReport();
    bool createTmpImgFiles = settings->val_createTmpImg();
    ptrdiff_t imgWidth = settings->val_imgWidth();

    createGraphics(graphics,
                   material1,
                   material2,
                   settings,
                   tempPath + QDir::separator() + lastCalcDateTime);

    //

    report->moveCursor(QTextCursor::End);

    report->insertHtml(
                "<b>"
                + lastCalcDateTime
                + "</b><br><br>"
                + tr("Settings")
                + "<br>* "
                + tr("Type of analysis")
                + ": "
                + ui->comboBox_analysisType->currentText()
                + "<br>* "
                + tr("Power of the approximate polynom")
                + ": "
                + QString::number(settings->val_polyPwr())
                + "<br>* "
                + tr("Accuracy of color threshold determining")
                + ": "
                + QString::number(settings->val_thrAccur())
                );

    if ( ui->comboBox_analysisType->currentIndex() == ANALTYPE_MIX ) {

        report->insertHtml(
                    "<br>* "
                    + tr("Ideal concentration")
                    + ": "
                    + QString::number(settings->val_idealConc())
                    );
    }

    if ( !thrmsg.isEmpty() ) {

        report->insertHtml("<br><br><b>" + thrmsg + "</b><br><hr><br>");
        report->moveCursor(QTextCursor::End);

        thrmsg.clear();
        return;
    }

    report->insertHtml(
                "<br><br>"
                + tr("First material image file")
                + ": "
                + ui->lineEdit_mat1FileName->text()
                );

    if ( showImg ) {

        report->insertHtml(
                    "<br><br>"
                    + tr("Image of the first material")
                    + ":<br>"
                    );

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
                "<br><br>"
                + tr("Characteristic of the first material "
                     "(histogram and polynomial approximant)")
                +":<br>"
                );
    report->textCursor().insertImage(graphics[0]);

    report->insertHtml(
                "<br><br>"
                + tr("Second material image file")
                + ": "
                + ui->lineEdit_mat2FileName->text());

    if ( showImg ) {

        report->insertHtml(
                    "<br><br>"
                    + tr("Image of the second material")
                    + ":<br>"
                    );

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
                "<br><br>"
                + tr("Characteristic of the second material "
                     "(histogram and polynomial approximant)")
                + ":<br>"
                );
    report->textCursor().insertImage(graphics[1]);

    report->insertHtml(
                "<br><br>"
                + tr("Visualization of the calculated "
                     "gray color threshold "
                     "(polynoms and threshold color)")
                + ":<br>"
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
                            "<br><b>"
                            + tr("Analysis of image")
                            + " "
                            + imgname
                            + " "
                            + tr("failed or canceled")
                            + ".</b>"
                            );

                continue;
            }

            conc = probes[i]->concentration();
            concs.push_back(conc);

            if ( showImg ) {

                report->insertHtml(
                            "<br>"
                            + tr("Mix image")
                            + ":<br>"
                            );

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
                        "<br>"
                        + tr("Mix image file")
                        + ": "
                        + imgname
                        + "<br>"
                        + tr("Concentration of the first component")
                        + " = <b>"
                        + QString::number(conc)
                        + "</b><br>"
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

        createHistograms(histograms_area,
                         histograms_circul,
                         granules,
                         settings,
                         tempPath + QDir::separator() + lastCalcDateTime);

        QString imgname;

        for ( ptrdiff_t i=0; i<granules.size(); i++ ) {

            imgname = granules[i]->imageFileName();

            if ( imgname.isEmpty() ) {

                report->insertHtml(
                            "<br><b>"
                            + tr("Analysis of image")
                            + " "
                            + imgname
                            + " "
                            + tr("failed or canceled")
                            + ".</b>"
                            );

                continue;
            }

            if ( showImg ) {

                report->insertHtml(
                            "<br>"
                            + tr("Granules image")
                            + ":<br>"
                            );

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

            report->insertHtml(
                        "<br><br>"
                        + tr("Image file")
                        + ": "
                        + imgname
                        + "<br>"
                        );

            report->insertHtml(
                        "<br>"
                        + tr("Particles found")
                        + ": <b>"
                        + QString::number(granules[i]->partNumber())
                        + "</b><br>"
                        );

            report->insertHtml(
                        "<br>"
                        + tr("Particle-size distribution")
                        + ":<br>"
                        );
            report->textCursor().insertImage(histograms_area[i]);

            report->insertHtml(
                        "<br><br>"
                        + tr("Mean size particles")
                        + ": <b>"
                        + QString::number(granules[i]->meanSizeParticles())
                        + "</b>"
                        );

            report->insertHtml(
                        "<br><br>"
                        + tr("Particle-circularity distribution")
                        + ":<br>"
                        );
            report->textCursor().insertImage(histograms_circul[i]);

            report->insertHtml(
                        "<br><br>"
                        + tr("Mean compact particles")
                        + ": <b>"
                        + QString::number(granules[i]->meanCompactParticles())
                        + "</b>"
                        );

            report->insertHtml("<br>");
        }

        report->insertHtml("<br><hr><br>");
        report->moveCursor(QTextCursor::End);

        //

        if ( createTmpImgFiles ) {

            saveImages(granules,
                       tempPath
                       + QDir::separator()
                       + lastCalcDateTime
                       );
        }
    }
    else {

        report->insertHtml("<br><hr><br>");
        report->moveCursor(QTextCursor::End);
    }

    //

    graphics.clear();
    histograms_area.clear();
    histograms_circul.clear();
    material1->clear();
    material2->clear();
    probes.clear();
    granules.clear();
}
