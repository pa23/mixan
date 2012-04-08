/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: granules.cpp

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

#include "granules.h"
#include "mixanerror.h"
#include "constants.h"

#include <QString>
#include <QImage>

#include <opencv/cv.h>
#include <opencv/highgui.h>

Granules::Granules(const QString &fileName,
                   const size_t &lim1,
                   const size_t &lim2) :
    limCol1(0),
    limCol2(0) {

    imgFileName = fileName;
    limCol1 = lim1;
    limCol2 = lim2;

    histVls.clear();
    histVls.resize(HISTDIMENSION);
}

Granules::~Granules() {
}

void Granules::analyze() {

    try {

        findAreas();
        defHistData();
    }
    catch(MixanError &mixerr) {

        throw;
    }
}

void Granules::findAreas() {

    IplImage *origImage = 0;
    IplImage *grayImage = 0;
    IplImage *binImage = 0;
    IplImage *dstImage = 0;

    if ( !(origImage = cvLoadImage(imgFileName.toAscii(), 1)) ) {

        throw MixanError("Can not load image " + imgFileName + "!");
    }

    if ( !(grayImage = cvCreateImage(cvGetSize(origImage), IPL_DEPTH_8U, 1)) ) {

        throw MixanError("Can not allocate memory for grayImage!");
    }

    if ( !(binImage = cvCreateImage(cvGetSize(origImage), IPL_DEPTH_8U, 1)) ) {

        throw MixanError("Can not allocate memory for binImage!");
    }

    if ( !(dstImage = cvCloneImage(origImage)) ) {

        throw MixanError("Can not clone origImage!");
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
                                CV_RETR_EXTERNAL,
                                CV_CHAIN_APPROX_SIMPLE,
                                cvPoint(0, 0)) ) ) {

        throw MixanError("Can not find contours!");
    }

    for ( CvSeq *seq0 = contours; seq0 != 0; seq0 = seq0->h_next ) {

        areas.push_back(cvContourArea(seq0));

        cvDrawContours(dstImage,
                       seq0,
                       CV_RGB(255, 0, 0),
                       CV_RGB(0, 0, 255),
                       0, 1, 8);
    }

    cvReleaseImage(&origImage);
    cvReleaseImage(&grayImage);
    cvReleaseImage(&binImage);

    try {

        IplImage2QImage(dstImage);
    }
    catch (MixanError &mixerr) {

        cvReleaseImage(&dstImage);
        throw;
    }

    cvReleaseImage(&dstImage);
}

void Granules::IplImage2QImage(const IplImage *iplImg) {

    /* http://www.developer.nokia.com/Community/Wiki/Using_OpenCV_with_Qt */

    ptrdiff_t height = iplImg->height;
    ptrdiff_t width = iplImg->width;

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

        throw MixanError("Can not convert image from IplImage to QImage!");
    }
}

void Granules::defHistData() {

    histXSet.minval = areas[0];
    histXSet.maxval = areas[0];

    for ( ptrdiff_t i=1; i<areas.size(); i++ ) {

        if ( areas[i] < histXSet.minval ) { histXSet.minval = areas[i]; }
        if ( areas[i] > histXSet.maxval ) { histXSet.maxval = areas[i]; }
    }

    histXSet.step = (histXSet.maxval - histXSet.minval) / HISTDIMENSION;

    //

    double tmpmin = histXSet.minval;
    double tmpmax = tmpmin + histXSet.step;

    for ( ptrdiff_t n=0; n<areas.size(); n++ ) {

        for ( ptrdiff_t i=0; i<HISTDIMENSION; i++ ) {

            if ( areas[n]>=tmpmin && areas[n]<=tmpmax ) {

                histVls[i]++;
                break;
            }

            tmpmin += histXSet.step;
            tmpmax += histXSet.step;
        }

        tmpmin = histXSet.minval;
        tmpmax = tmpmin + histXSet.step;
    }

    for ( ptrdiff_t i=0; i<histVls.size(); i++ ) {

        histVls[i] /= areas.size();
    }
}
