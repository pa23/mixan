/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: settingsdialog.cpp

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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog) {

    ui->setupUi(this);
}

SettingsDialog::~SettingsDialog() {

    delete ui;
}

void SettingsDialog::on_pushButton_OK_clicked() {

    close();
}

void SettingsDialog::on_pushButton_defaults_clicked() {

    ui->spinBox_polyPower->setValue(9);
    ui->doubleSpinBox_intersectAccur->setValue(0.00025);
    ui->doubleSpinBox_idealConc->setValue(0.5);
    ui->checkBox_sizeinmm->setChecked(false);
    ui->doubleSpinBox_pxpermm2->setValue(0);
    ui->lineEdit_sieveHoleDiameters->setText("");
    ui->checkBox_reportRO->setChecked(true);
    ui->checkBox_imagesInReport->setChecked(true);
    ui->checkBox_createTempGraph->setChecked(false);
    ui->spinBox_imgWidth->setValue(600);
}
