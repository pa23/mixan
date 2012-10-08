/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: tmpfiles.cpp

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

#include <QPixmap>
#include <QVector>
#include <QSharedPointer>
#include <QDir>
#include <QMessageBox>

#include "granules.h"

void saveGraphics(const QPixmap &pixmap1,
                  const QPixmap &pixmap2,
                  const QPixmap &pixmap3,
                  const QString &path) {

    QDir tempDir;

    if ( !tempDir.exists(path) ) {

        if ( !tempDir.mkpath(path) ) {

            QMessageBox::warning(0, "mixan",
                                 QObject::tr("Can not create temporary "
                                             "directory!"));
        }
    }

    if ( !pixmap1.save(path + QDir::separator() + "graphic_0.png") ) {

        QMessageBox::warning(0, "mixan",
                             QObject::tr("Can not save pixmap to file!"));
    }

    if ( !pixmap2.save(path + QDir::separator() + "graphic_1.png") ) {

        QMessageBox::warning(0, "mixan",
                             QObject::tr("Can not save pixmap to file!"));
    }

    if ( !pixmap3.save(path + QDir::separator() + "graphic_2.png") ) {

        QMessageBox::warning(0, "mixan",
                             QObject::tr("Can not save pixmap to file!"));
    }
}

void saveHistograms(const QVector<QImage> &histograms_area,
                    const QVector<QImage> &histograms_circul,
                    const QString &path) {

    if ( histograms_area.size() != histograms_circul.size() ) { return; }

    QDir tempDir;

    if ( !tempDir.exists(path) ) {

        if ( !tempDir.mkpath(path) ) {

            QMessageBox::warning(
                        0,
                        "mixan",
                        QObject::tr("Can not create temporary "
                                    "directory!"));
        }
    }

    for ( ptrdiff_t n=0; n<histograms_area.size(); n++ ) {

        if ( !histograms_area[n].save(path
                                      + QDir::separator()
                                      + "histogram_"
                                      + QString::number(n)
                                      + ".1.png") ) {

            QMessageBox::warning(
                        0,
                        "mixan",
                        QObject::tr("Can not save image to file!"));
        }

        if ( !histograms_circul[n].save(path
                                        + QDir::separator()
                                        + "histogram_"
                                        + QString::number(n)
                                        + ".2.png") ) {

            QMessageBox::warning(
                        0,
                        "mixan",
                        QObject::tr("Can not save image to file!"));
        }
    }
}

void saveImages(const QVector< QSharedPointer<Granules> > &granules,
                const QString &path) {

    QDir tempDir;

    if ( !tempDir.exists(path) ) {

        if ( !tempDir.mkpath(path) ) {

            QMessageBox::warning(
                        0,
                        "mixan",
                        QObject::tr("Can not create temporary directory!")
                        );
        }
    }

    for ( ptrdiff_t n=0; n<granules.size(); n++ ) {

        if ( !granules[n]->resImage().save(path
                                           + QDir::separator()
                                           + "granules_image_"
                                           + QString::number(n)
                                           + ".png") ) {

            QMessageBox::warning(
                        0,
                        "mixan",
                        QObject::tr("Can not save pixmap to file!")
                        );
        }
    }
}
