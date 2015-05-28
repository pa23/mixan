/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: granules.cpp

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

#include "granules.h"
#include "mixanerror.h"
#include "constants.h"

#include <QString>
#include <QImage>
#include <QObject>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <cmath>
#include <memory>

using std::shared_ptr;

Granules::Granules(const QString &fileName,
                   const size_t lim1,
                   const size_t lim2,
                   const shared_ptr<const Settings> &set) :
    imgFileName(fileName),
    limCol1(lim1),
    limCol2(lim2),
    settings(set),
    img(),
    areas(),
    compacts(),
    minosizes() {
}

Granules::~Granules() {
}

void Granules::analyze() {
    findAreas();
}

ptrdiff_t Granules::partNumber() const {

    if (areas.size() == compacts.size()) {
        return areas.size();
    }
    else {
        return 0;
    }
}

void Granules::findAreas() {

    IplImage *origImage = 0;
    IplImage *grayImage = 0;
    IplImage *binImage = 0;
    IplImage *dstImage = 0;

    if ( !(origImage = cvLoadImage(imgFileName.toAscii(), 1)) ) {

        throw MixanError(
                    QObject::tr("Can not load image")
                    + " "
                    + imgFileName
                    + "!\n"
                    );
    }

    if ( !(grayImage = cvCreateImage(cvGetSize(origImage), IPL_DEPTH_8U, 1)) ) {

        cvReleaseImage(&origImage);

        throw MixanError(QObject::tr("Can not allocate memory for grayImage!"));
    }

    if ( !(binImage = cvCreateImage(cvGetSize(origImage), IPL_DEPTH_8U, 1)) ) {

        cvReleaseImage(&origImage);
        cvReleaseImage(&grayImage);

        throw MixanError(QObject::tr("Can not allocate memory for binImage!"));
    }

    if ( !(dstImage = cvCloneImage(origImage)) ) {

        cvReleaseImage(&origImage);
        cvReleaseImage(&grayImage);
        cvReleaseImage(&binImage);

        throw MixanError(QObject::tr("Can not clone origImage!"));
    }

    cvCvtColor(origImage, grayImage, CV_RGB2GRAY); // creation of gray
    cvInRangeS(grayImage,
               cvScalar(limCol1),
               cvScalar(limCol2),
               binImage); // creation of binary

    CvMemStorage *storage = cvCreateMemStorage(0);
    CvSeq *contours = 0;

    ptrdiff_t cc = 0;

    if ( !( cc = cvFindContours(binImage,
                                storage,
                                &contours,
                                sizeof(CvContour),
                                CV_RETR_EXTERNAL,             // only ext areas!
                                CV_CHAIN_APPROX_SIMPLE,
                                cvPoint(0, 0)) ) ) {

        cvReleaseImage(&origImage);
        cvReleaseImage(&grayImage);
        cvReleaseImage(&binImage);
        cvReleaseImage(&dstImage);
        cvReleaseMemStorage(&storage);

        throw MixanError(QObject::tr("Can not find contours!"));
    }

    double area = 0;
    double perim = 0;
    double comp = 0;

    CvBox2D rect;
    const double pxpermm2 = settings->val_pxpermm2();
    const double pxpermm = sqrt(pxpermm2);

    for ( CvSeq *seq = contours; seq != 0; seq = seq->h_next ) {

        area = cvContourArea(seq);
        perim = cvContourPerimeter(seq);
        comp = area / (perim * perim);

        if ( area == 0 ) { continue; }

        //

        compacts.push_back(comp);
        rect = cvMinAreaRect2(seq);

        if ( settings->val_sizeinmm() && (pxpermm2 != 0) ) {

            areas.push_back(area/pxpermm2);

            if ( rect.size.width < rect.size.height ) {
                minosizes.push_back(rect.size.width/pxpermm);
            }
            else {
                minosizes.push_back(rect.size.height/pxpermm);
            }
        }
        else {

            areas.push_back(area);

            if ( rect.size.width < rect.size.height ) {
                minosizes.push_back(rect.size.width);
            }
            else {
                minosizes.push_back(rect.size.height);
            }
        }

        //

        if ( settings->val_showImgInReport() || settings->val_createTmpImg() ) {

            cvDrawContours(dstImage,
                           seq,
                           CV_RGB(255, 0, 0),
                           CV_RGB(0, 0, 255),
                           0, 1, 8);
        }
    }

    if ( settings->val_showImgInReport() || settings->val_createTmpImg() ) {

        try {
            IplImage2QImage(dstImage);
        }
        catch (const MixanError &mixerr) {
            cvReleaseImage(&origImage);
            cvReleaseImage(&grayImage);
            cvReleaseImage(&binImage);
            cvReleaseImage(&dstImage);
            cvReleaseMemStorage(&storage);
            throw;
        }
    }

    cvReleaseImage(&origImage);
    cvReleaseImage(&grayImage);
    cvReleaseImage(&binImage);
    cvReleaseImage(&dstImage);
    cvReleaseMemStorage(&storage);
}

void Granules::IplImage2QImage(const IplImage *iplImg) {

    /* http://www.developer.nokia.com/Community/Wiki/Using_OpenCV_with_Qt */

    const ptrdiff_t height = iplImg->height;
    const ptrdiff_t width = iplImg->width;

    if ( iplImg->depth == IPL_DEPTH_8U && iplImg->nChannels == 3 ) {

        const uchar *qImageBuffer = (const uchar *)iplImg->imageData;
        QImage tmpImg(qImageBuffer, width, height, QImage::Format_RGB888);

        img = tmpImg.rgbSwapped();
    }
    else if ( iplImg->depth == IPL_DEPTH_8U && iplImg->nChannels == 1 ) {

        const uchar *qImageBuffer = (const uchar *)iplImg->imageData;
        QImage tmpImg(qImageBuffer, width, height, QImage::Format_Indexed8);

        QVector<QRgb> colorTable;

        for ( size_t i=0; i<256; i++ ) { colorTable.push_back(qRgb(i, i, i)); }

        tmpImg.setColorTable(colorTable);

        img = tmpImg;
    }
    else {
        throw MixanError(QObject::tr("Can not convert image "
                                     "from IplImage to QImage!"));
    }
}
