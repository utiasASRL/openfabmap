/*//////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this
//  license. If you do not agree to this license, do not download, install,
//  copy or use the software.
//
// This file originates from the openFABMAP project:
// [http://code.google.com/p/openfabmap/] -or-
// [https://github.com/arrenglover/openfabmap]
//
// For published work which uses all or part of OpenFABMAP, please cite:
// [http://ieeexplore.ieee.org/xpl/articleDetails.jsp?arnumber=6224843]
//
// Original Algorithm by Mark Cummins and Paul Newman:
// [http://ijr.sagepub.com/content/27/6/647.short]
// [http://ieeexplore.ieee.org/xpl/articleDetails.jsp?arnumber=5613942]
// [http://ijr.sagepub.com/content/30/9/1100.abstract]
//
//                           License Agreement
//
// Copyright (C) 2012 Arren Glover [aj.glover@qut.edu.au] and
//                    Will Maddern [w.maddern@qut.edu.au], all rights reserved.
//
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  * Redistribution's of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
//  * Redistribution's in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
//  * The name of the copyright holders may not be used to endorse or promote
//    products derived from this software without specific prior written
///   permission.
//
// This software is provided by the copyright holders and contributors "as is"
// and any express or implied warranties, including, but not limited to, the
// implied warranties of merchantability and fitness for a particular purpose
// are disclaimed. In no event shall the Intel Corporation or contributors be
// liable for any direct, indirect, incidental, special, exemplary, or
// consequential damages (including, but not limited to, procurement of
// substitute goods or services; loss of use, data, or profits; or business
// interruption) however caused and on any theory of liability, whether in
// contract, strict liability,or tort (including negligence or otherwise)
// arising in any way out of the use of this software, even if advised of the
// possibility of such damage.
//////////////////////////////////////////////////////////////////////////////*/

#include "../include/openfabmap.hpp"

namespace of2 {

BOWMSCTrainer::BOWMSCTrainer(double _clusterSize) :
    clusterSize(_clusterSize) {
}

BOWMSCTrainer::~BOWMSCTrainer() {
}

cv::Mat BOWMSCTrainer::cluster() const {
    CV_Assert(!descriptors.empty());
    int descCount = 0;
    for(size_t i = 0; i < descriptors.size(); i++)
    descCount += descriptors[i].rows;

    cv::Mat mergedDescriptors(descCount, descriptors[0].cols,
        descriptors[0].type());
    for(size_t i = 0, start = 0; i < descriptors.size(); i++)
    {
        cv::Mat submut = mergedDescriptors.rowRange((int)start,
            (int)(start + descriptors[i].rows));
        descriptors[i].copyTo(submut);
        start += descriptors[i].rows;
    }
    return cluster(mergedDescriptors);
}

cv::Mat BOWMSCTrainer::cluster(const cv::Mat& descriptors) const {

    CV_Assert(!descriptors.empty());

    // TODO: sort the descriptors before clustering.


    cv::Mat icovar = cv::Mat::eye(descriptors.cols,descriptors.cols,descriptors.type());

    std::vector<cv::Mat> initialCentres;
    initialCentres.push_back(descriptors.row(0));
    for (int i = 1; i < descriptors.rows; i++) {
        double minDist = DBL_MAX;
        for (size_t j = 0; j < initialCentres.size(); j++) {
            minDist = std::min(minDist,
                cv::Mahalanobis(descriptors.row(i),initialCentres[j],
                icovar));
        }
        if (minDist > clusterSize)
            initialCentres.push_back(descriptors.row(i));
    }

    std::vector<std::list<cv::Mat> > clusters;
    clusters.resize(initialCentres.size());
    for (int i = 0; i < descriptors.rows; i++) {
        int index; double dist, minDist = DBL_MAX;
        for (size_t j = 0; j < initialCentres.size(); j++) {
            dist = cv::Mahalanobis(descriptors.row(i),initialCentres[j],icovar);
            if (dist < minDist) {
                minDist = dist;
                index = (int)j;
            }
        }
        clusters[index].push_back(descriptors.row(i));
    }

    // TODO: throw away small clusters.

    cv::Mat vocabulary;
    cv::Mat centre = cv::Mat::zeros(1,descriptors.cols,descriptors.type());
    for (size_t i = 0; i < clusters.size(); i++) {
        centre.setTo(0);
        for (std::list<cv::Mat>::iterator Ci = clusters[i].begin(); Ci != clusters[i].end(); Ci++) {
            centre += *Ci;
        }
        centre /= (double)clusters[i].size();
        vocabulary.push_back(centre);
    }

    return vocabulary;
}

}
