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
#include <QObject>

#include <opencv/cv.h>
#include <opencv/highgui.h>

Granules::Granules(const QString &fileName,
                   const size_t &lim1,
                   const size_t &lim2) :
    limCol1(0),
    limCol2(0),
    pxpermm(0),
    meanSizePart(0),
    meanCompPart(0) {

    imgFileName = fileName;
    limCol1 = lim1;
    limCol2 = lim2;

    init();
}

Granules::Granules(const QString &fileName,
                   const size_t &lim1,
                   const size_t &lim2,
                   const double &k) :
    limCol1(0),
    limCol2(0),
    pxpermm(0),
    meanSizePart(0),
    meanCompPart(0) {

    imgFileName = fileName;
    limCol1 = lim1;
    limCol2 = lim2;

    if ( k != 0 ) { pxpermm = k; }

    init();
}

Granules::~Granules() {
}

void Granules::init() {

    hist1XSet.minval = 0;
    hist1XSet.maxval = 0;
    hist1XSet.step = 0;

    hist1Vls.clear();
    hist1Vls.resize(HISTDIMENSION);

    hist2XSet.minval = 0;
    hist2XSet.maxval = 0;
    hist2XSet.step = 0;

    hist2Vls.clear();
    hist2Vls.resize(HISTDIMENSION);
}

void Granules::analyze() {

    try {

        findAreas();
        defHistsData();
    }
    catch(MixanError &mixerr) {

        throw;
    }
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

        throw MixanError(QObject::tr("Can not allocate memory for grayImage!"));
    }

    if ( !(binImage = cvCreateImage(cvGetSize(origImage), IPL_DEPTH_8U, 1)) ) {

        throw MixanError(QObject::tr("Can not allocate memory for binImage!"));
    }

    if ( !(dstImage = cvCloneImage(origImage)) ) {

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

        throw MixanError(QObject::tr("Can not find contours!"));
    }

    double area = 0;
    double areamm = 0;
    double perim = 0;
    double comp = 0;

    for ( CvSeq *seq = contours; seq != 0; seq = seq->h_next ) {

        area = cvContourArea(seq);
        perim = cvContourPerimeter(seq);
        comp = area / (perim * perim);

        if ( area == 0 ) { continue; }

        //

        if ( pxpermm == 0 ) {

            areas.push_back(area);
            meanSizePart += area;
        }
        else {

            areamm = area/pxpermm;

            areas.push_back(areamm);
            meanSizePart += areamm;
        }

        compacts.push_back(comp);
        meanCompPart += comp;

        //

        cvDrawContours(dstImage,
                       seq,
                       CV_RGB(255, 0, 0),
                       CV_RGB(0, 0, 255),
                       0, 1, 8);
    }

    meanSizePart /= areas.size();
    meanCompPart /= compacts.size();

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

        throw MixanError(QObject::tr("Can not convert image "
                                     "from IplImage to QImage!"));
    }
}

void Granules::defHistsData() {

    if ( areas.isEmpty()    ||
         compacts.isEmpty() ||
         (areas.size() != compacts.size()) ) { return; }

    hist1XSet.minval = areas[0];
    hist1XSet.maxval = areas[0];

    for ( ptrdiff_t i=1; i<areas.size(); i++ ) {

        if ( areas[i] < hist1XSet.minval ) { hist1XSet.minval = areas[i]; }
        if ( areas[i] > hist1XSet.maxval ) { hist1XSet.maxval = areas[i]; }
    }

    hist1XSet.step = (hist1XSet.maxval - hist1XSet.minval) / HISTDIMENSION;

    //

    double tmpmin1 = hist1XSet.minval;
    double tmpmax1 = tmpmin1 + hist1XSet.step;

    for ( ptrdiff_t n=0; n<areas.size(); n++ ) {

        for ( ptrdiff_t i=0; i<HISTDIMENSION; i++ ) {

            if ( i == (HISTDIMENSION-1) ) {

                if ( areas[n]>=tmpmin1 && areas[n]<(tmpmax1+hist1XSet.step) ) {

                    hist1Vls[i]++;
                    break;
                }
            }
            else {

                if ( areas[n]>=tmpmin1 && areas[n]<tmpmax1 ) {

                    hist1Vls[i]++;
                    break;
                }
            }

            tmpmin1 += hist1XSet.step;
            tmpmax1 += hist1XSet.step;
        }

        tmpmin1 = hist1XSet.minval;
        tmpmax1 = tmpmin1 + hist1XSet.step;
    }

    for ( ptrdiff_t i=0; i<hist1Vls.size(); i++ ) {

        hist1Vls[i] /= areas.size();
    }

    //

    hist2XSet.minval = compacts[0];
    hist2XSet.maxval = compacts[0];

    for ( ptrdiff_t i=1; i<compacts.size(); i++ ) {

        if ( compacts[i] < hist2XSet.minval ) {

            hist2XSet.minval = compacts[i];
        }

        if ( compacts[i] > hist2XSet.maxval ) {

            hist2XSet.maxval = compacts[i];
        }
    }

    hist2XSet.step = (hist2XSet.maxval - hist2XSet.minval) / HISTDIMENSION;

    //

    double tmpmin2 = hist2XSet.minval;
    double tmpmax2 = tmpmin2 + hist2XSet.step;

    for ( ptrdiff_t n=0; n<compacts.size(); n++ ) {

        for ( ptrdiff_t i=0; i<HISTDIMENSION; i++ ) {

            if ( i == (HISTDIMENSION-1) ) {

                if ( compacts[n]>=tmpmin2 &&
                     compacts[n]<(tmpmax2+hist2XSet.step) ) {

                    hist2Vls[i]++;
                    break;
                }
            }
            else {

                if ( compacts[n]>=tmpmin2 && compacts[n]<tmpmax2 ) {

                    hist2Vls[i]++;
                    break;
                }
            }

            tmpmin2 += hist2XSet.step;
            tmpmax2 += hist2XSet.step;
        }

        tmpmin2 = hist2XSet.minval;
        tmpmax2 = tmpmin2 + hist2XSet.step;
    }

    for ( ptrdiff_t i=0; i<hist2Vls.size(); i++ ) {

        hist2Vls[i] /= compacts.size();
    }
}
