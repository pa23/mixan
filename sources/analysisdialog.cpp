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
#include <QtConcurrentMap>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QPixmap>
#include <QDateTime>

const Settings *tmp_settings = 0;
QListWidget *tmp_probesFileNames = 0;
size_t tmp_tcol = 0;
QVector< QSharedPointer<Mix> > *tmp_probes = 0;
size_t tmp_limcol1 = 0;
size_t tmp_limcol2 = 0;
QVector< QSharedPointer<Granules> > *tmp_granules = 0;
QString *tmp_thrmsg = 0;

void runMixAnalysis(ptrdiff_t &iter) {

    try {

        QSharedPointer<Mix>
                probe(new Mix(tmp_probesFileNames->item(iter)->text(),
                              tmp_tcol,
                              tmp_settings));
        probe->analyze();
        tmp_probes->push_back(probe);
    }
    catch(const MixanError &mixerr) {

        *tmp_thrmsg += mixerr.mixanErrMsg() + "\n";
    }
}

void runGranulationAnalysis(ptrdiff_t &iter) {

    try {

        QSharedPointer<Granules>
                grans(new Granules(tmp_probesFileNames->item(iter)->text(),
                                   tmp_limcol1,
                                   tmp_limcol2,
                                   tmp_settings));

        grans->analyze();
        tmp_granules->push_back(grans);
    }
    catch(const MixanError &mixerr) {

        *tmp_thrmsg += mixerr.mixanErrMsg() + "\n";
    }
}

AnalysisDialog::AnalysisDialog(QTextBrowser *txtbrowser,
                               const QSharedPointer<Settings> &sts,
                               QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AnalysisDialog),
    report(txtbrowser),
    settings(sts),
    filters("Images (*.png *.jpg *.jpeg *.bmp);;All files (*.*)"),
    material1(QSharedPointer<Material>(new Material())),
    material2(QSharedPointer<Material>(new Material())),
    progressDialog(QSharedPointer<QProgressDialog>(new QProgressDialog())),
    futureWatcher(QSharedPointer< QFutureWatcher<void> >
                  (new QFutureWatcher<void>)),
    tempPath(QDir::homePath() + QDir::separator() + TMPDIR + QDir::separator()) {

    ui->setupUi(this);
    progressDialog->setWindowTitle("mixan: progress");

    //

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

    //

    tmp_settings = settings.data();
    tmp_probesFileNames = ui->listWidget_probesFileNames;
    tmp_probes = &probes;
    tmp_granules = &granules;
    tmp_thrmsg = &thrmsg;
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

    QString mat1ImageFileName;

    if ( !lastImgDir.isEmpty() ) {

        mat1ImageFileName = QFileDialog::getOpenFileName(
                    this,
                    tr("Select image file..."),
                    lastImgDir,
                    filters,
                    0,
                    0
                    );
    }
    else {

        mat1ImageFileName = QFileDialog::getOpenFileName(
                    this,
                    tr("Select image file..."),
                    QDir::currentPath(),
                    filters,
                    0,
                    0
                    );
    }

    if ( !mat1ImageFileName.isEmpty() ) {

        ui->lineEdit_mat1FileName->setText(mat1ImageFileName);

        QFileInfo fileinfo(mat1ImageFileName);
        lastImgDir = fileinfo.absolutePath();
    }
}

void AnalysisDialog::on_pushButton_selectMat2_clicked() {

    QString mat2ImageFileName;

    if ( !lastImgDir.isEmpty() ) {

        mat2ImageFileName = QFileDialog::getOpenFileName(
                    this,
                    tr("Select image file..."),
                    lastImgDir,
                    filters,
                    0,
                    0
                    );
    }
    else {

        mat2ImageFileName = QFileDialog::getOpenFileName(
                    this,
                    tr("Select image file..."),
                    QDir::currentPath(),
                    filters,
                    0,
                    0
                    );
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

        mixImageFileNames = QFileDialog::getOpenFileNames(
                    this,
                    tr("Select image files..."),
                    lastImgDir,
                    filters,
                    0,
                    0
                    );
    }
    else {

        mixImageFileNames = QFileDialog::getOpenFileNames(
                    this,
                    tr("Select image files..."),
                    QDir::currentPath(),
                    filters,
                    0,
                    0
                    );
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

    try {

        material1->analyze(ui->lineEdit_mat1FileName->text(), settings.data());
        material2->analyze(ui->lineEdit_mat2FileName->text(), settings.data());
    }
    catch(const MixanError &mixerr) {

        thrmsg += mixerr.mixanErrMsg() + "\n";
        return;
    }

    if ( ui->comboBox_analysisType->currentIndex() == ANALTYPE_MATERIALS ) {

        showAnalysisResults();
    }
    else if ( ui->comboBox_analysisType->currentIndex() == ANALTYPE_MIX ) {

        probes.clear();

        tmp_tcol = defThreshColor(material1.data(),
                                  material2.data(),
                                  settings->val_thrAccur());

        QVector<ptrdiff_t> iterations;
        for ( ptrdiff_t i=0; i<ui->listWidget_probesFileNames->count(); i++ ) {

            iterations.push_back(i);
        }

        progressDialog->setLabelText(tr("Images analysis. Please wait..."));
        futureWatcher->setFuture(QtConcurrent::map(iterations, &runMixAnalysis));
        progressDialog->exec();
        futureWatcher->waitForFinished();
    }
    else if ( ui->comboBox_analysisType->currentIndex() ==
              ANALTYPE_GRANULATION ) {

        granules.clear();

        tmp_tcol = defThreshColor(material1.data(),
                                  material2.data(),
                                  settings->val_thrAccur());

        if ( material1->thresholdColor() < tmp_tcol ) {

            tmp_limcol1 = 0;
            tmp_limcol2 = tmp_tcol;
        }
        else {

            tmp_limcol1 = tmp_tcol;
            tmp_limcol2 = 255;
        }

        QVector<ptrdiff_t> iterations;
        for ( ptrdiff_t i=0; i<ui->listWidget_probesFileNames->count(); i++ ) {

            iterations.push_back(i);
        }

        progressDialog->setLabelText(tr("Images analysis. Please wait..."));
        futureWatcher->setFuture(QtConcurrent::map(iterations,
                                                   &runGranulationAnalysis));
        progressDialog->exec();
        futureWatcher->waitForFinished();
    }

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

    if ( graphics.size() != 3 ) {

        QMessageBox::warning(this, "mixan", tr("Can not create graphics!"));
        freeMemory();

        return;
    }

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
    else if ( ui->comboBox_analysisType->currentIndex() ==
              ANALTYPE_GRANULATION ) {

        report->insertHtml(
                    "<br>* "
                    + tr("Pixels per square millimeter")
                    + ": "
                    + QString::number(settings->val_pxpermm2())
                    );

        if ( settings->val_sievesCellDiameter().isEmpty() ) {

            report->insertHtml(
                        "<br>* "
                        + tr("Sieves cell diameter, mm")
                        + ": "
                        + tr("no values")
                        );
        }
        else {

            report->insertHtml(
                        "<br>* "
                        + tr("Sieves cell diameter, mm")
                        + ": "
                        + settings->val_sievesCellDiameter()
                        );
        }

        if ( settings->val_sievesCellDimension().isEmpty() ) {

            report->insertHtml(
                        "<br>* "
                        + tr("Sieves cell dimension, mm")
                        + ": "
                        + tr("no values")
                        );
        }
        else {

            report->insertHtml(
                        "<br>* "
                        + tr("Sieves cell dimension, mm")
                        + ": "
                        + settings->val_sievesCellDimension()
                        );
        }
    }

    if ( !thrmsg.isEmpty() ) {

        report->insertHtml("<br><br><b>" + thrmsg + "</b><br><hr><br>");
        report->moveCursor(QTextCursor::End);

        thrmsg.clear();
        freeMemory();

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

        double minArea = 0;
        double maxArea = 0;
        double meanArea = 0;
        double meanCompact = 0;

        createHistograms(histograms,
                         granules,
                         settings,
                         tempPath + QDir::separator() + lastCalcDateTime,
                         minArea,
                         maxArea,
                         meanArea,
                         meanCompact);

        if ( histograms.size() != 2 ) {

            QMessageBox::warning(this,
                                 "mixan",
                                 tr("Can not create histograms!"));
            freeMemory();

            return;
        }

        //

        QString imgname;
        ptrdiff_t totalParticlesNumber = 0;

        for ( ptrdiff_t i=0; i<granules.size(); i++ ) {

            totalParticlesNumber += granules[i]->partNumber();

            //

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
                        "<br>"
                        + tr("Image file")
                        + " #"
                        + QString::number(i)
                        + ": "
                        + imgname
                        + "<br>"
                        );

            report->insertHtml(
                        tr("Particles found")
                        + ": <b>"
                        + QString::number(granules[i]->partNumber())
                        + "</b><br>"
                        );
        }

        report->insertHtml(
                    "<br>"
                    + tr("Total particles number (on all images)")
                    + ": <b>"
                    + QString::number(totalParticlesNumber)
                    + "</b><br>"
                    );

        report->insertHtml(
                    "<br>"
                    + tr("Total particle-size distribution")
                    + ":<br>"
                    );
        report->textCursor().insertImage(histograms[0]);

        report->insertHtml(
                    "<br><br>"
                    + tr("Size of particles")
                    + " ["
                    );

        if ( settings->val_sizeinmm() ) {

            report->insertHtml(tr("mm2"));
        }
        else {

            report->insertHtml(tr("px"));
        }

        report->insertHtml(
                    "]:<br>"
                    + tr("minimum_size")
                    + " = <b>"
                    + QString::number(minArea)
                    + "</b><br>"
                    + tr("maximum_size")
                    + " = <b>"
                    + QString::number(maxArea)
                    + "</b><br>"
                    + tr("mean_size")
                    + " = <b>"
                    + QString::number(meanArea)
                    + "</b>"
                    );

        report->insertHtml(
                    "<br><br>"
                    + tr("Total particle-circularity distribution")
                    + ":<br>"
                    );
        report->textCursor().insertImage(histograms[1]);

        report->insertHtml(
                    "<br><br>"
                    + tr("Mean compact particles")
                    + ": <b>"
                    + QString::number(meanCompact)
                    + "</b>"
                    );

        //

        if ( settings->val_sizeinmm() &&
             (!(settings->val_sievesCellDiameter().isEmpty()) ||
              !(settings->val_sievesCellDimension().isEmpty())) ) {

            QStringList diameters =
                    settings->val_sievesCellDiameter().
                    split(";", QString::SkipEmptyParts);
            QStringList dimensions =
                    settings->val_sievesCellDimension().
                    split(";", QString::SkipEmptyParts);

            QHash<QString, double> cells;
            QVector<double> sieveCells;

            if ( !diameters.isEmpty() ) {

                double diam = 0;

                for ( ptrdiff_t i=0; i<diameters.size(); i++ ) {

                    diam = diameters[i].toDouble();

                    cells.insert(diameters[i], diam);
                    sieveCells.push_back(diam);
                }
            }

            if ( !dimensions.isEmpty() ) {

                double reducedDim = 0;

                for ( ptrdiff_t i=0; i<dimensions.size(); i++ ) {

                    reducedDim = dimensions[i].toDouble() * CELLKOEFF;

                    cells.insert(dimensions[i], reducedDim);
                    sieveCells.push_back(reducedDim);
                }
            }

            QVector<double> partRemainders;
            QVector<double> totalRemainders;

            defRemainders(granules, sieveCells, partRemainders, totalRemainders);

            QString str = "<br><br>"
                    + tr("Remainders on sieves")
                    + "<table><tr><td align=\"right\" colspan=\"25\">"
                    + tr("Particular")
                    + "</td><td align=\"right\" colspan=\"11\">"
                    + tr("Total")
                    +"</td></tr>";

            ptrdiff_t i_fm = 0;

            for ( ptrdiff_t i=(sieveCells.size()-1); i>=0; i-- ) {

                if ( cells.key(sieveCells[i]) == "2.5" ) { i_fm = i; }

                str += "<tr><td colspan=\"5\">"
                        + tr("Sieve")
                        + "</td><td colspan=\"5\">#"
                        + QString::number(i)
                        + "</td><td>(</td><td align=\"right\">"
                        + cells.key(sieveCells[i])
                        + "<td>"
                        + tr("mm")
                        + "</td><td>)</td><td align=\"right\" colspan=\"10\">"
                        + QString::number(partRemainders[i]*100, 'f', 2)
                        + "</td><td>%</td>"
                        + "<td align=\"right\" colspan=\"10\">"
                        + QString::number(totalRemainders[i]*100, 'f', 2)
                        + "</td><td>%</td></tr>";
            }

            str += "</table>";

            report->insertHtml(str);

            //

            if ( cells.contains("2.5") &&
                 cells.contains("1.25") &&
                 cells.contains("0.63") &&
                 cells.contains("0.315") &&
                 (cells.contains("0.14") || cells.contains("0.16")) ) {

                double summFullRem = 0;

                for ( ptrdiff_t i=i_fm; i>(i_fm-5); i-- ) {

                    summFullRem += totalRemainders[i]*100;
                }

                report->insertHtml(
                            "<br><br>"
                            + tr("Fineness modulus")
                            + ": <b>"
                            + QString::number(summFullRem/100, 'f', 3)
                            + "</b>"
                            );
            }
        }

        //

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

    freeMemory();
}

void AnalysisDialog::freeMemory() {

    graphics.clear();
    histograms.clear();
    material1->clear();
    material2->clear();
    probes.clear();
    granules.clear();
}
