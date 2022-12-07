/**
 * This file contains detector/analysis specific configurables
 * \author Julia Tena Vidal \at Tel Aviv University
 * \date October 2022
 **/
#include "conf/AnalysisConstantsI.h"
#include "conf/ParticleI.h"
#include "conf/AnalysisCutsI.h"
#include "TMath.h"

using namespace e4nu ;

double conf::GetMinMomentumCut( const int particle_pdg, const double EBeam ) { 
  double min_p = 0 ;
  if( particle_pdg == kPdgElectron ) {
    if( EBeam == 1.161 ) min_p = 0.4 ; 
    else if ( EBeam == 2.261 ) min_p = 0.55 ;
    else if ( EBeam == 4.461 ) min_p = 1.1 ; 
  }
  return min_p ; 
}

bool conf::ValidPhiOpeningAngle( double phi /*rad*/, const bool apply ) {
  if( !apply ) return true ;
  //Definition as for data. It is also correct for GENIE simulation data since V3_el is rotated above by 180 degree in phi
  phi *= TMath::RadToDeg() ; 
  phi += 30 ; //Add 30 degree for plotting and photon phi cut
  if ( phi < 0 ) phi += 360 ; //Add 360 so that electron phi is between 0 and 360 degree
  
  if ( !(TMath::Abs( phi - 30 )  < kPhiOpeningAngle ||
	 TMath::Abs( phi - 90 )  < kPhiOpeningAngle || 
	 TMath::Abs( phi - 150 )  < kPhiOpeningAngle ||
	 TMath::Abs( phi - 210 )  < kPhiOpeningAngle ||
	 TMath::Abs( phi - 270 )  < kPhiOpeningAngle ||
	 TMath::Abs( phi - 330)  < kPhiOpeningAngle ) ) return false ; 
  return true ; 
}

bool conf::GoodSectorPhiSlice( double phi /*rad*/, const bool apply ) {
  if ( !apply ) return true ; 

  phi *= TMath::RadToDeg() ; 
  phi += 30 ; //Add 30 degree for plotting and photon phi cut
  if ( phi < 0 ) phi += 360 ; //Add 360 so that electron phi is between 0 and 360 degree

  phi -= kCenterFirstSector ; 
  if( TMath::Abs(phi) < kPhiOpeningAngle ) return false ;
  return true ; 
} 

bool conf::GetQ2Cut( double & Q2cut, const double Ebeam, const bool apply_Q2cut ) {
  if( !apply_Q2cut ) return false ; 

  if ( Ebeam >= 1. && Ebeam < 2. ) {
    Q2cut = 0.1;
    return true ;
  } else if ( Ebeam >= 2. && Ebeam < 3. ) {
    Q2cut = 0.4 ;
    return true ;
  } else if ( Ebeam >= 4. && Ebeam < 5. ) {
    Q2cut= 0.8 ;
    return true ;
  }
  Q2cut = 0 ; 
  return false ;
}

bool conf::GetWCut( double & WCut, const double Ebeam ) {
  if ( Ebeam < 2 /*GeV*/ ) {
    WCut = 2 ; 
    return true ;
  } 
  return false ;
}
