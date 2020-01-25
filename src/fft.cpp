#include <cstdlib>
#include <cmath>
#include <cfloat>

#include "fft.h"

#define REPLACE_FFT_PI 3.141592653589793238462L 

namespace replace {


FFT::FFT()
  : binCntX_(0), binCntY_(0), binSizeX_(0), binSizeY_(0) {}

FFT::FFT(int binCntX, int binCntY, int binSizeX, int binSizeY)
  : binCntX_(binCntX), binCntY_(binCntY), 
  binSizeX_(binSizeX), binSizeY_(binSizeY) {
  init();   
}

FFT::~FFT() {
  using std::vector;
  for(int i=0; i<binCntX_; i++) {
    vector<float>().swap(binDensity_[i]);
    vector<float>().swap(electroPhi_[i]);
    vector<float>().swap(electroForceX_[i]);
    vector<float>().swap(electroForceY_[i]);
  }

  vector<vector<float>>().swap(binDensity_);
  vector<vector<float>>().swap(electroPhi_);
  vector<vector<float>>().swap(electroForceX_);
  vector<vector<float>>().swap(electroForceY_);

  vector<float> ().swap(csTable_);
  vector<float> ().swap(wx_);
  vector<float> ().swap(wxSquare_);
  vector<float> ().swap(wy_);
  vector<float> ().swap(wySquare_);
  
  vector<int> ().swap(workArea_);
}


void
FFT::init() {
  binDensity_.resize(binCntX_);
  electroPhi_.resize(binCntX_);
  electroForceX_.resize(binCntX_);
  electroForceY_.resize(binCntX_);

  for(int i=0; i<binCntX_; i++) {
    binDensity_[i].resize(binCntY_, 0);
    electroPhi_[i].resize(binCntY_, 0);
    electroForceX_[i].resize(binCntY_, 0);
    electroForceY_[i].resize(binCntY_, 0);
  }

  csTable_.resize( std::max(binCntX_, binCntY_) * 3 / 2, 0 );

  wx_.resize( binCntX_, 0 );
  wxSquare_.resize( binCntX_, 0);
  wy_.resize( binCntY_, 0 );
  wySquare_.resize( binCntY_, 0 );

  workArea_.resize( round(sqrt(std::max(binCntX_, binCntY_))) + 2, 0 );
 
  for(int i=0; i<binCntX_; i++) {
    wx_[i] = REPLACE_FFT_PI * static_cast<float>(i) 
      / static_cast<float>(binCntX_);
    wxSquare_[i] = wx_[i] * wx_[i]; 
  }

  for(int i=0; i<binCntY_; i++) {
    wy_[i] = REPLACE_FFT_PI * static_cast<float>(i)
      / static_cast<float>(binCntY_) 
      * static_cast<float>(binSizeY_) 
      / static_cast<float>(binSizeX_);
    wySquare_[i] = wy_[i] * wy_[i];
  }
}

void
FFT::updateDensity(int x, int y, float density) {
  binDensity_[x][y] = density;
}

std::pair<float, float> 
FFT::getElectroForce(int x, int y) {
  return std::make_pair(
      electroForceX_[x][y],
      electroForceY_[x][y]);
}

float
FFT::getElectroPhi(int x, int y) {
  return electroPhi_[x][y]; 
}


void
FFT::doFFT() {
  ddct2d(binCntX_, binCntY_, -1, (float**)&binDensity_[0][0], 
      NULL, (int*) &workArea_[0], (float*)&csTable_[0]);
  
  for(int i = 0; i < binCntX_; i++) {
    binDensity_[i][0] *= 0.5;
  }

  for(int i = 0; i < binCntY_; i++) {
    binDensity_[0][i] *= 0.5;
  }

  for(int i = 0; i < binCntX_; i++) {
    for(int j = 0; j < binCntY_; j++) {
      binDensity_[i][j] *= 4.0 / binCntX_ / binCntY_;
    }
  }

  for(int i = 0; i < binCntX_; i++) {
    float wx = wx_[i];
    float wx2 = wxSquare_[i];

    for(int j = 0; j < binCntY_; j++) {
      float wy = wy_[j];
      float wy2 = wySquare_[j];

      float density = binDensity_[i][j];
      float phi = 0;
      float electroX = 0, electroY = 0;

      if(i == 0 && j == 0) {
        phi = electroX = electroY = 0.0f;
      }
      else {
        //////////// lutong
        //  denom =
        //  wx2 / 4.0 +
        //  wy2 / 4.0 ;
        // a_phi = a_den / denom ;
        ////b_phi = 0 ; // -1.0 * b / denom ;
        ////a_ex = 0 ; // b_phi * wx ;
        // a_ex = a_phi * wx / 2.0 ;
        ////a_ey = 0 ; // b_phi * wy ;
        // a_ey = a_phi * wy / 2.0 ;
        ///////////
        phi = density / (wx2 + wy2);
        electroX = phi * wx;
        electroX = phi * wy;
      }
      electroPhi_[i][j] = phi;
      electroForceX_[i][j] = electroX;
      electroForceY_[i][j] = electroY;
    }
  }
  // Inverse DCT
  ddct2d(binCntX_, binCntY_, 1, 
      (float**) &electroPhi_[0][0], 
      NULL, (int*) &workArea_[0], (float*) &csTable_[0]);
  ddsct2d(binCntX_, binCntY_, 1, 
      (float**) &electroForceX_[0][0], 
      NULL, (int*) &workArea_[0], (float*) &csTable_[0]);
  ddcst2d(binCntX_, binCntY_, 1, 
      (float**) &electroForceY_[0][0], 
      NULL, (int*) &workArea_[0], (float*) &csTable_[0]);
}

}