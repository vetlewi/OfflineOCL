// -*- c++ -*-

#ifndef RootWriter_H_
#define RootWriter_H_ 1

#include <string>
#include <memory>

class TH1;
class TH2;

typedef TH1* TH1p;
typedef TH2* TH2p;

class Histogram1D;
class Histogram2D;

typedef std::shared_ptr<Histogram1D> Histogram1Dp;
typedef std::shared_ptr<Histogram2D> Histogram2Dp;

class Histograms;

#define ROOT1D_YTITLE 1 // 0=No title on y-axis, 1=Counts/binwidth on y-axis.

//! Functions to write histograms into ROOT files.
//!
/*!
 * \class RootWriter
 * \brief Class to write hitograms in ROOT files.
 * \details This class recives a list of histograms and writes them to a root file.
 * \author unknown
 * \copyright GNU Public License v. 3
 */
class RootWriter {
public:
    //! Write many histograms at once.
    /*! All of the histograms in the list will be written. The output
     *  file will be overwritten if it exists.
     */
    static void Write( Histograms& histograms,     /*!< The histogram list. */
                       const std::string& filename /*!< The output filename. */);

    //! Create a ROOT histogram from a Histogram1D.
    /*! \return the ROOT 1D histogram.
     */
    static TH1p CreateTH1(Histogram1Dp h /*!< The Histogram1D to be cpoied. */);

    //! Create a ROOT histogram from a Histogram2D.
    /*! \return the ROOT 2D histogram.
     */
    static TH2p CreateTH2(Histogram2Dp h /*!< The Histogram2D to be cpoied. */);
};

#endif /* RootWriter_H_ */
