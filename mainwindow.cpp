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

#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QDir>
#include <QPixmap>
#include <QMessageBox>
#include <QDateTime>
#include <QPrinter>
#include <QTextCursor>

#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    ui->setupUi(this);

    setWindowTitle("mixan " + version);
    ui->textBrowser_report->setHtml("<br><b>mixan " + version + "</b><br>Analyze of granular material mix.<br>");
}

MainWindow::~MainWindow() {

    delete ui;
}

void MainWindow::on_action_loadImages_activated() {

    QStringList imageFiles(QFileDialog::getOpenFileNames(
                               this,
                               tr("Open Image File..."),
                               QDir::currentPath(),
                               QString::fromAscii("JPEG files (*.jpg);;JPEG files (*.jpeg);;PNG files (*.png);;Bitmap files (*.bmp);;All files (*.*)"),
                               0,
                               0));

    if ( imageFiles.count() == 0 ) {

        QMessageBox::warning(this, "mixan", "Please, select images files.");
        return;
    }

    //

    ui->textBrowser_report->moveCursor(QTextCursor::End);

    ui->textBrowser_report->insertHtml(
                "<hr><br><b>" +
                QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") +
                "</b><br>"
                );

    images.clear();

    QImage img;
    size_t width = 0;

    //

    for ( ptrdiff_t i=0; i<imageFiles.count(); i++ ) {

        img.load(imageFiles[i]);

        if ( img.isNull() ) {

            QMessageBox::critical(this, "mixan", "Image is empty or this is not image!");
            continue;
        }

        images.push_back(img);
        width = img.width();

        if ( width > 600 ) {

            width = 600;
        }

        ui->textBrowser_report->insertHtml(
                    "<br>File for analysis: " +
                    imageFiles[i] +
                    "<br><img src=\"" +
                    imageFiles[i] +
                    "\" width=\"" +
                    QString::number(width) +
                    "\" /><br>"
                    );
    }

    ui->textBrowser_report->moveCursor(QTextCursor::End);
}

void MainWindow::on_action_createReport_activated() {

    QString reportFileName(QFileDialog::getSaveFileName(
                               this,
                               tr("Save report to PDF..."),
                               "noname.pdf",
                               QString::fromAscii("PDF files (*.pdf);;All files (*.*)"),
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

    ui->textBrowser_report->setHtml("<br><b>mixan " + version + "</b><br>Analyze of granular material mix.<br>");
}

void MainWindow::on_action_quit_activated() {

    close();
}

void MainWindow::on_action_analyze_activated() {

    if ( images.isEmpty() ) {

        QMessageBox::critical(this, "mixan", "No images for analysis!");
        return;
    }

    size_t curColor = 0;
    size_t part1 = 0; // light
    size_t part2 = 0; // dark

    QVector<double> concentrations;

    double conc = 0;
    double summ_concdiff = 0;

    for ( ptrdiff_t i=0; i<images.count(); i++ ) {

        for (ptrdiff_t j=0; j<images[i].width(); j++) {

            for (ptrdiff_t k=0; k<images[i].height(); k++) {

                curColor = (size_t)images[i].pixel(j, k);

                if ( curColor < baseColor ) { part2++; }
                else                        { part1++; }
            }
        }

        conc = (double)part1 / ( (double)part1 + (double)part2 );
        summ_concdiff += pow( (conc - idealConcentration), 2 );

        concentrations.push_back(conc);

        part1 = 0;
        part2 = 0;
    }

    //

    ui->textBrowser_report->moveCursor(QTextCursor::End);

    ui->textBrowser_report->insertHtml(
                "<br>Analysis results: \"Light in dark\" concentrations: "
                );

    for ( ptrdiff_t i=0; i<concentrations.count(); i++ ) {

        ui->textBrowser_report->insertHtml(
                    "<b>" + QString::number(concentrations[i]) + "; </b>"
                    );
    }

    ui->textBrowser_report->insertHtml(
                "<br>"
                );

    if ( images.count() > 1 ) {

        double Vc = 100.0 / idealConcentration * pow( 1.0 / ( concentrations.count() - 1.0 ) * summ_concdiff, 0.5 );

        ui->textBrowser_report->insertHtml(
                    "Analysis results: Vc = <b>" +
                    QString::number(Vc) + "</b><br>"
                    );
    }

    ui->textBrowser_report->moveCursor(QTextCursor::End);

    //

    concentrations.clear();
    images.clear();

    //

    QMessageBox::information(this, "mixan", "Analyze completed!");
}

void MainWindow::on_action_about_mixan_activated() {

    QString str = "<b>mixan v0.1</b>\n"
            "<br><br>Analyze of granular material mix."
            "<br><br>Copyright (C) 2011 Artem Petrov <a href= \"mailto:pa2311@gmail.com\" >pa2311@gmail.com</a>"
            "<br><br>Web site: <a href= \"https://github.com/pa23/mixan\">https://github.com/pa23/mixan</a>"
            "<br><br>This program is free software: you can redistribute it and/or modify "
            "it under the terms of the GNU General Public License as published by "
            "the Free Software Foundation, either version 3 of the License, or "
            "(at your option) any later version. "
            "<br>This program is distributed in the hope that it will be useful, "
            "but WITHOUT ANY WARRANTY; without even the implied warranty of "
            "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
            "GNU General Public License for more details. "
            "<br>You should have received a copy of the GNU General Public License "
            "along with this program. If not, see <a href= \"http://www.gnu.org/licenses/\" >http://www.gnu.org/licenses/</a>.<br>";

    QMessageBox::about(this, "About mixan", str);
}
