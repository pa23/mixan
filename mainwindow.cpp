/*
    mixan
    Analyze of granular material mix.

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
#include "granularmaterial.h"
#include "granularmix.h"

#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QDir>
#include <QPixmap>
#include <QMessageBox>
#include <QPrinter>
#include <QTextCursor>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    ui->setupUi(this);

    setWindowTitle("mixan " + VERSION);

    ui->textBrowser_report->setHtml(
                "<br><b>mixan " +
                VERSION +
                "</b><br>Analyze of granular material mix.<br>"
                );

    //

    lightMaterial = new GranularMaterial();
    darkMaterial = new GranularMaterial();
}

MainWindow::~MainWindow() {

    delete ui;

    delete lightMaterial;
    delete darkMaterial;

    probes.clear();

//    for (ptrdiff_t i=0; i<probes.count(); i++) {

//        delete probes[i];
//    }
}

void MainWindow::on_action_selectImages_activated() {

    QString filters = "Images (*.jpg *.png *.bmp);;All files (*.*)";

    lightMaterialImageFileName =
            QFileDialog::getOpenFileName(
                this,
                tr("Select light component image file..."),
                QDir::currentPath(),
                filters,
                0,
                0);

    darkMaterialImageFileName =
            QFileDialog::getOpenFileName(
                this,
                tr("Select dark component image file..."),
                QDir::currentPath(),
                filters,
                0,
                0);

    mixImageFileNames =
            QFileDialog::getOpenFileNames(
                this,
                tr("Select image files of mix..."),
                QDir::currentPath(),
                filters,
                0,
                0);

    if ( lightMaterialImageFileName.isEmpty() ||
         darkMaterialImageFileName.isEmpty()  ||
         (mixImageFileNames.count() == 0) ) {

        QMessageBox::information(this, "mixan", "You do not select any file (");
        return;
    }

    //

    ui->textBrowser_report->moveCursor(QTextCursor::End);

    ui->textBrowser_report->insertHtml(
                "<hr><br><b>" +
                QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") +
                "</b><br><br><u>Selected files:</u><br>"
                );

    //

    ptrdiff_t filenum = 1;

    ui->textBrowser_report->insertHtml(
                QString::number(filenum) +
                ". " +
                lightMaterialImageFileName +
                "<br>"
                );

    filenum++;

    ui->textBrowser_report->insertHtml(
                QString::number(filenum) +
                ". " +
                darkMaterialImageFileName +
                "<br>"
                );

    filenum++;

    probes.clear();

    for ( ptrdiff_t i=0; i<mixImageFileNames.count(); i++ ) {

        ui->textBrowser_report->insertHtml(
                    QString::number(filenum) +
                    ". " +
                    mixImageFileNames[i] +
                    "<br>"
                    );

        probes.push_back(new GranularMix());

        filenum++;
    }

    //

    ui->textBrowser_report->moveCursor(QTextCursor::End);
}

void MainWindow::on_action_createReport_activated() {

    QString filters = "PDF files (*.pdf);;All files (*.*)";

    QString reportFileName(QFileDialog::getSaveFileName(
                               this,
                               "Save report to PDF...",
                               "noname.pdf",
                               filters,
                               0,
                               0));

    QPrinter printer;

    printer.setOrientation(QPrinter::Portrait);
    printer.setPageMargins(20, 15, 15, 15, QPrinter::Millimeter);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(reportFileName);
    printer.setFontEmbeddingEnabled(true);

    ui->textBrowser_report->print(&printer);
}

void MainWindow::on_action_cleanReportWindow_activated() {

    ui->textBrowser_report->setHtml(
                "<br><b>mixan " +
                VERSION +
                "</b><br>Analyze of granular material mix.<br>"
                );
}

void MainWindow::on_action_quit_activated() {

    close();
}

void MainWindow::on_action_analyze_activated() {

    ui->textBrowser_report->moveCursor(QTextCursor::End);

    ui->textBrowser_report->insertHtml(
                "<br><u>Analysis results:</u><br>"
                );

    //

    if ( !lightMaterial->analyze(lightMaterialImageFileName) ) {

        QMessageBox::critical(this, "mixan", "Analysis of image \"" +
                              lightMaterialImageFileName + "\" fails!");
        return;
    }

    if ( !darkMaterial->analyze(darkMaterialImageFileName) ) {

        QMessageBox::critical(this, "mixan", "Analysis of image \"" +
                              darkMaterialImageFileName + "\" fails!");
        return;
    }

    QString imgname;

    size_t lcol = lightMaterial->thresholdColor();
    size_t dcol = darkMaterial->thresholdColor();

    for ( ptrdiff_t i=0; i<probes.size(); i++ ) {

        imgname = mixImageFileNames[i];

        if ( !probes[i]->analyze(imgname, lcol, dcol) ) {

            QMessageBox::critical(this, "mixan", "Analysis of image \"" +
                                  imgname + "\" fails!");
            continue;
        }

        ui->textBrowser_report->insertHtml(
                    "Image: " +
                    imgname +
                    "<br>Light component concentration = <b>" +
                    QString::number(probes[i]->concentration()) +
                    "</b><br>"
                    );
    }

    //

    ui->textBrowser_report->moveCursor(QTextCursor::End);

    //

    QMessageBox::information(this, "mixan", "Analyze completed!");
}

void MainWindow::on_action_about_mixan_activated() {

    QString str = "<b>mixan " + VERSION + "</b>\n"
            "<br><br>Analyze of granular material mix."
            "<br><br>Copyright (C) 2011 Artem Petrov "
            "<a href= \"mailto:pa2311@gmail.com\" >pa2311@gmail.com</a>"
            "<br><br>Web site: <a href= \"https://github.com/pa23/mixan\">"
            "https://github.com/pa23/mixan</a>"
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
