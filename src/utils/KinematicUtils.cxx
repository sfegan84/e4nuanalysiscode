
#include <iostream>
#include <cmath>
#include "TVector3.h"
#include "TLorentzVector.h"

#include "utils/KinematicUtils.h"
#include "conf/ParticleI.h"
#include "utils/TargetUtils.h"
#include "utils/ParticleUtils.h"

using namespace e4nu ;

double utils::GetECal( const double Ef, const std::map<int,std::vector<TLorentzVector>> particle_map, const int tgt ) {
  double ECal = Ef ; // Add energy of outgoing lepton
  for( auto it = particle_map.begin() ; it != particle_map.end() ; ++it ) {
    // Calculate ECal for visible particles
    for( unsigned int i = 0 ; i < (it->second).size() ; ++i ) {
      ECal += (it->second)[i].E() ; // Add Kinetic energy of hadrons
      if( it->first == conf::kPdgProton ) ECal += utils::GetBindingEnergy( tgt ) - utils::GetParticleMass( it->first ) ; // Correct for proton binding energy
    }
  }

  return ECal ;
}

double utils::GetRecoEnu( const TLorentzVector & leptonf, const unsigned int target_pdg ) {
  return (conf::kProtonMass * utils::GetBindingEnergy( target_pdg ) +conf::kProtonMass * leptonf.E() ) / ( conf::kProtonMass - leptonf.E() + leptonf.Rho() * TMath::Cos( leptonf.Theta() ));
}

double utils::GetQELRecoEnu( const TLorentzVector & leptonf, const unsigned int target_pdg ) {
  double Ereco = 0 ; 
  double E = leptonf.E();
  double P = leptonf.P();
  double CosTh = TMath::Cos( leptonf.Theta() ); 
  double BE = utils::GetBindingEnergy( target_pdg ) ;

  Ereco = std::pow( conf::kProtonMass,2) - pow( conf::kElectronMass,2) + 2*E*( conf::kNeutronMass - BE ) - std::pow( conf::kNeutronMass -  BE ,2) ;
  Ereco /= ( conf::kNeutronMass - BE ) - E + P * CosTh ; 
  Ereco /= 2. ;

  return Ereco ; 
} 

double utils::GetEnergyTransfer( const TLorentzVector & leptonf, const double Ebeam ) {
  return Ebeam - leptonf.E() ; 
}

double utils::GetNuECal( const TLorentzVector & leptonf, const double ECal ) {
  TLorentzVector nu(0,0,ECal,ECal) ; 
  return -(leptonf-nu).E(); 
}

TVector3 utils::GetRecoq3( const TLorentzVector & leptonf, const double EBeam ) {
  TLorentzVector beam ( 0,0,EBeam,EBeam) ;
  return ( beam - leptonf ).Vect() ;
}

double utils::GetRecoQ2( const TLorentzVector & leptonf, const double EBeam ) {
  TLorentzVector beam ( 0,0,EBeam,EBeam) ;
  return -( leptonf - beam ).Mag2() ; 
}

double utils::GetRecoXBJK( const TLorentzVector & leptonf, const double EBeam ) {
  double nu = utils::GetEnergyTransfer( leptonf, EBeam ) ; 
  double Q2 = utils::GetRecoQ2( leptonf, EBeam ) ; 
  return Q2 / ( 2 * conf::kProtonMass * nu ) ;
}

double utils::GetRecoW( const TLorentzVector & leptonf, const double EBeam ) {
  double mp = conf::kProtonMass ; 
  double nu = utils::GetEnergyTransfer( leptonf, EBeam ) ; 
  TVector3 q3 = utils::GetRecoq3( leptonf, EBeam ) ; 
  double W2 = std::pow( mp + nu, 2 ) - q3.Mag2() ; 
  if ( W2 < 0 ) return 0 ;
  return TMath::Sqrt( W2 ) ; 
} 

TVector3 utils::GetPT( const TVector3 p ) {
  TVector3 beam_dir (0,0,1);
  double vect_parallel = p.Dot(beam_dir);

  // Calculate transverse vector:
  TVector3 vect_T = p - (vect_parallel * beam_dir ) ; 
  return vect_T ; 
}

double utils::DeltaAlphaT( const TVector3 p1 /*out electron*/, const TVector3 p2/*proton*/ ) {
  TVector3 P1T_dir = utils::GetPT(p1).Unit();
  TVector3 DeltaPT_dir = utils::DeltaPT(p1, p2).Unit();

  return acos(-P1T_dir.Dot(DeltaPT_dir)) * 180. / TMath::Pi();
}

double utils::DeltaAlphaT( const TLorentzVector out_electron , const std::map<int,std::vector<TLorentzVector>> hadrons ) {
  TVector3 P1T_dir = utils::GetPT(out_electron.Vect()).Unit();
  TVector3 DeltaPT_dir = utils::DeltaPT(out_electron, hadrons).Unit();

  return acos(-P1T_dir.Dot(DeltaPT_dir)) * 180. / TMath::Pi();
}

TVector3 utils::DeltaPT( const TVector3 p1 , const TVector3 p2 ) {
  TVector3 P1_T = utils::GetPT(p1);
  TVector3 P2_T = utils::GetPT(p2);

  return P1_T + P2_T;
}

TVector3 utils::DeltaPT( const TLorentzVector out_electron , const std::map<int,std::vector<TLorentzVector>> hadrons ) {
  TVector3 P1_T = utils::GetPT(out_electron.Vect());
  TLorentzVector tot_hadron ; 
  for( auto it = hadrons.begin() ; it!=hadrons.end() ; ++it ) {
    for( unsigned int i = 0 ; i < (it->second).size() ; ++i ) { 
      tot_hadron += (it->second)[i] ; 
    }
  }
  TVector3 P2_T = utils::GetPT(tot_hadron.Vect());

  return P1_T + P2_T;
}

double utils::DeltaPhiT( const TVector3 p1 , const TVector3 p2 ) {
  TVector3 P1T_dir = utils::GetPT(p1).Unit();
  TVector3 P2T_dir = utils::GetPT(p2).Unit();
  return acos(-P1T_dir.Dot(P2T_dir)) * 180. / TMath::Pi() ; 
}

double utils::DeltaPhiT( const TLorentzVector out_electron , const std::map<int,std::vector<TLorentzVector>> hadrons ) {
  TVector3 P1T_dir = utils::GetPT(out_electron.Vect()).Unit();
  TLorentzVector tot_hadron ;
  for( auto it = hadrons.begin() ; it!=hadrons.end() ; ++it ) {
    for( unsigned int i= 0 ; i< (it->second).size() ; ++i ) {
      tot_hadron += (it->second)[i] ;
    }
  }
  TVector3 P2T_dir = utils::GetPT(tot_hadron.Vect()).Unit();
  return acos(-P1T_dir.Dot(P2T_dir)) * 180. / TMath::Pi() ; 
}




