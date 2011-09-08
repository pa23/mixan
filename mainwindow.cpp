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
#include "probe.h"

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
    ui->textBrowser_report->setHtml("<br><b>mixan " + VERSION + "</b><br>Analyze of granular material mix.<br>");
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

        QMessageBox::information(this, "mixan", "You do not select any file (");
        return;
    }

    //

    ui->textBrowser_report->moveCursor(QTextCursor::End);

    ui->textBrowser_report->insertHtml(
                "<hr><br><b>" +
                QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") +
                "</b><br>"
                );

    //

    probes.clear();

    ptrdiff_t filenum = 1;

    ui->textBrowser_report->insertHtml(
                "<br><u>Selected files:</u><br>"
                );

    //

    for ( ptrdiff_t i=0; i<imageFiles.count(); i++ ) {

        ui->textBrowser_report->insertHtml(
                    QString::number(filenum) +
                    ". " +
                    imageFiles[i] +
                    "<br>"
                    );

        probes.push_back(new Probe(imageFiles[i]));

        filenum++;
    }

    //

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

    ui->textBrowser_report->setHtml("<br><b>mixan " + VERSION + "</b><br>Analyze of granular material mix.<br>");
}

void MainWindow::on_action_quit_activated() {

    close();
}

void MainWindow::on_action_analyze_activated() {

    if ( probes.isEmpty() ) {

        QMessageBox::critical(this, "mixan", "No images for analysis!");
        return;
    }

    ui->textBrowser_report->moveCursor(QTextCursor::End);

    //

    ui->textBrowser_report->insertHtml(
                "<br><u>Analysis results:</u><br>"
                );

    for ( ptrdiff_t i=0; i<probes.size(); i++ ) {

        if ( !probes[i]->analyze() ) {

            QMessageBox::warning(this, "mixan", "Analysis of image " + QString::number(i) + " fails!");
            continue;
        }

        ui->textBrowser_report->insertHtml(
                    "Image <b>" +
                    QString::number(i+1) +
                    "</b><br>Threshold color = 0x" +
                    QString::number(probes[i]->thresholdColor(), 16).toUpper() +
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
