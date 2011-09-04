/*
    mixan
    Analyze of granular material mix.

    File: mainwindow.h

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QImage>

namespace Ui {

    class MainWindow;
}

class MainWindow : public QMainWindow {

    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:

    Ui::MainWindow *ui;
    QVector<QImage> images;

    void colorToGrey(QImage *) const;
    void setMaxContrast(QImage *) const;

private slots:

    void on_action_loadImages_activated();
    void on_action_createReport_activated();
    void on_action_cleanReportWindow_activated();
    void on_action_quit_activated();
    void on_action_analyze_activated();
    void on_action_about_mixan_activated();

};

#endif // MAINWINDOW_H