/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: tmpfiles.cpp

    Copyright (C) 2012-2015 Artem Petrov <pa2311@gmail.com>

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
#include <QDir>
#include <QMessageBox>
#include <QThread>
#include <QtConcurrentMap>
#include <QFutureWatcher>
#include <QProgressDialog>

#include "tmpfiles.h"
#include "granules.h"

#include <memory>

using std::shared_ptr;

void saveGraphics(const QPixmap &pixmap1,
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
}

void saveHistograms(const QVector<QImage> &histograms,
                    const QString &path) {

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

    for ( ptrdiff_t n=0; n<histograms.size(); n++ ) {

        if ( !histograms[n].save(path
                                 + QDir::separator()
                                 + "histogram_"
                                 + QString::number(n)
                                 + ".png") ) {
            QMessageBox::warning(
                        0,
                        "mixan",
                        QObject::tr("Can not save image to file!"));
        }
    }
}

const QVector< shared_ptr<Granules> > *tmp_granules_tf = 0;
const QString *tmp_path_tf = 0;

void realSavingImages(ptrdiff_t &iter) {

    if ( !tmp_granules_tf->at(iter)->resImage().save(
             *tmp_path_tf
             + QDir::separator()
             + "granules_image_"
             + QString::number(iter)
             + ".png"
             )
         ) {
        QMessageBox::warning(
                    0,
                    "mixan",
                    QObject::tr("Can not save pixmap to file!")
                    );
    }
}

void saveImages(const QVector< shared_ptr<Granules> > &granules,
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

        return;
    }

    //

    tmp_granules_tf = &granules;
    tmp_path_tf = &path;

    //

    shared_ptr<QProgressDialog> progressDialog =
            shared_ptr<QProgressDialog>(new QProgressDialog());
    progressDialog->setWindowTitle("mixan: progress");

    shared_ptr< QFutureWatcher<void> > futureWatcher =
            shared_ptr< QFutureWatcher<void> >(new QFutureWatcher<void>);

    QObject::connect(futureWatcher.get(),
                     SIGNAL(finished()),
                     progressDialog.get(),
                     SLOT(reset())
                     );
    QObject::connect(progressDialog.get(),
                     SIGNAL(canceled()),
                     futureWatcher.get(),
                     SLOT(cancel())
                     );
    QObject::connect(futureWatcher.get(),
                     SIGNAL(progressRangeChanged(int,int)),
                     progressDialog.get(),
                     SLOT(setRange(int,int))
                     );
    QObject::connect(futureWatcher.get(),
                     SIGNAL(progressValueChanged(int)),
                     progressDialog.get(),
                     SLOT(setValue(int))
                     );

    QVector<ptrdiff_t> iterations;
    for ( ptrdiff_t n=0; n<tmp_granules_tf->size(); n++ ) {
        iterations.push_back(n);
    }

    progressDialog->setLabelText(QObject::tr("Saving temporary image files. "
                                             "Please wait..."));
    futureWatcher->setFuture(QtConcurrent::map(iterations, &realSavingImages));
    progressDialog->exec();
    futureWatcher->waitForFinished();
}
