/** 
 * \info These parameters are configurable 
 * the default values are set here
 **/
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include "analysis/BackgroundI.h"
#include "conf/FiducialCutI.h"
#include "conf/AnalysisConstantsI.h"
#include "conf/AccpetanceMapsI.h"
#include "conf/AnalysisCutsI.h"
#include "utils/KinematicUtils.h"
#include "utils/Utils.h"

using namespace e4nu; 

BackgroundI::BackgroundI( ) {

  if( kIsConfigured ) kIsConfigured = InitializeFiducial() ; 

  if( kIsConfigured && ApplyFiducial() ) {
    fRotation = new Subtraction();
    fRotation->InitSubtraction( GetConfiguredEBeam(), GetConfiguredTarget(), GetNRotations(), fFiducialCut);
    fRotation->ResetQVector(); //Resets q vector to (0,0,0) 
  }
}

BackgroundI::BackgroundI( const std::string input_file ) : ConfigureI( input_file ) {

  if( kIsConfigured ) kIsConfigured = InitializeFiducial() ; 

  if( kIsConfigured && ApplyFiducial() ) {
    fRotation = new Subtraction();
    fRotation->InitSubtraction( GetConfiguredEBeam(), GetConfiguredTarget(), GetNRotations(), fFiducialCut);
    fRotation->ResetQVector(); //Resets q vector to (0,0,0)
  }
}

BackgroundI::BackgroundI( const double EBeam, const unsigned int TargetPdg ) : ConfigureI( EBeam, TargetPdg ) {

  if( kIsConfigured ) kIsConfigured = InitializeFiducial() ; 

  if( kIsConfigured && ApplyFiducial() ) {
    fRotation = new Subtraction();
    fRotation->InitSubtraction( GetConfiguredEBeam(), GetConfiguredTarget(), GetNRotations(), fFiducialCut);
    fRotation->ResetQVector(); //Resets q vector to (0,0,0)
  }
}    

BackgroundI::~BackgroundI() {
  fBkg.clear() ; 
  delete fRotation ;
  delete fFiducialCut ;
}

bool BackgroundI::InitializeFiducial(void) {
  double EBeam = GetConfiguredEBeam() ; 
  unsigned int Target = GetConfiguredTarget() ;

  if( ApplyFiducial() ) {
    // Initialize fiducial for this run
    fFiducialCut = new Fiducial() ; 
    fFiducialCut -> InitPiMinusFit( EBeam ) ; 
    fFiducialCut -> InitEClimits(); 
    fFiducialCut -> up_lim1_ec -> Eval(60) ;
    fFiducialCut -> SetConstants( conf::GetTorusCurrent( EBeam ), Target , EBeam ) ;
    fFiducialCut -> SetFiducialCutParameters( EBeam ) ;
    if( !fFiducialCut ) return false ; 
  } else { return true ; }

  return true ; 
}

bool BackgroundI::SubstractBackground(void) {

  /*
  if( !GetSubstractBkg() ) return true ;

  unsigned int max_mult = GetMaxBkgMult(); // Max multiplicity specified in conf file
  unsigned int min_mult = GetMinBkgMult(); // Signal multiplicity
  std::map<int,unsigned int> Topology = GetTopology();
  unsigned int m = max_mult ;
  while ( m >= min_mult ) {
    if( fBkg.find(m) != fBkg.end() ) {
      std::cout<< " Number of events with multiplicity " << m << " = " << fBkg[m].size() <<std::endl; 
    }
    --m; 
  }

  std::map<int,std::vector<TLorentzVector>> particles ;
  std::map<int,std::vector<TLorentzVector>> particles_uncorr ; 
  TLorentzVector V4_el ;
  unsigned int bkg_mult = min_mult + 1 ;
  if( bkg_mult > max_mult ) return true ; 

  // Create new map for event particles
  std::map<int,std::vector<TLorentzVector>> t_particles ;
  std::map<int,std::vector<TLorentzVector>> t_particles_uncorr ;
  
  if ( fBkg.find(bkg_mult) != fBkg.end() ) {
    for ( unsigned int i = 0 ; i < fBkg[bkg_mult].size() ; ++i ) {
      particles = fBkg[bkg_mult][i].GetFinalParticles4Mom();
      particles_uncorr = fBkg[bkg_mult][i].GetFinalParticlesUnCorr4Mom(); // This map needs to change with the cuts as well...
      V4_el = fBkg[bkg_mult][i].GetOutLepton4Mom();
      double event_wgt = fBkg[bkg_mult][i].GetTotalWeight() ;

      // remove multiplicity 2 contribution to signal...
      // 2p0pi -> 1p0pi ( multiplicity 2 -> multiplicity 1 ) 
      if( particles[conf::kPdgProton].size() == 2 
	  && particles[conf::kPdgPiP].size() == 0 
	  && particles[conf::kPdgPiM].size() == 0 
	  && particles[conf::kPdgPhoton].size() == 0 ) {
	  
	double E_tot_2p[bkg_mult]={0};
	double p_perp_tot_2p[bkg_mult]={0};
	double N_prot_both = 0;
	double P_N_2p[bkg_mult]={0};
	  
	TVector3 V3_2prot_corr[bkg_mult];
	for ( unsigned k = 0 ; k < bkg_mult ; ++k ) {
	  V3_2prot_corr[k] = particles[conf::kPdgProton][k].Vect() ; 
	}
	
	TVector3 V3_2prot_uncorr[bkg_mult];
	for ( unsigned k = 0 ; k < bkg_mult ; ++k ) {
	  V3_2prot_uncorr[k] = particles_uncorr[conf::kPdgProton][k].Vect() ; 
	}

	// Set rotation around q3 vector
	fRotation->SetQVector( fBkg[bkg_mult][i].GetRecoq3() );	
	fRotation->prot2_rot_func( V3_2prot_corr, V3_2prot_uncorr, V4_el, E_tot_2p, p_perp_tot_2p, P_N_2p , &N_prot_both);

	for( unsigned int j = 0 ; j < bkg_mult ; ++j ) {
	  std::vector<TLorentzVector> corr_mom = { particles[conf::kPdgProton][j] } ;
	  std::vector<TLorentzVector> uncorr_mom = { particles_uncorr[conf::kPdgProton][j] } ;
	  t_particles[conf::kPdgProton] = corr_mom ;
	  t_particles_uncorr[conf::kPdgProton] = uncorr_mom ;
	  // Store background event with the detected proton
	  fBkg[bkg_mult][i].SetFinalParticlesKinematics( t_particles ) ; 
	  fBkg[bkg_mult][i].SetFinalParticlesUnCorrKinematics( t_particles_uncorr ) ; 
	  fBkg[bkg_mult][i].SetEventWeight( -P_N_2p[j] * event_wgt ) ; 
	  if ( fBkg.find(min_mult) != fBkg.end() ) {
	    fBkg[min_mult].push_back( fBkg[bkg_mult][i] ) ; 
	  } else {
	    std::vector<e4nu::MCEvent> temp = { fBkg[bkg_mult][i] } ;
	    fBkg[min_mult] = temp ; 
	  }
	}
      }
      particles.clear() ;
      particles_uncorr.clear() ;
    }
  }

  bkg_mult = min_mult + 2 ;
  if( bkg_mult > max_mult ) return true ; 
  // remove multiplicity 3 contribution to signal...
  if ( fBkg.find(bkg_mult) != fBkg.end() ) {
    for ( unsigned int i = 0 ; i < fBkg[bkg_mult].size() ; ++i ) {
      particles = fBkg[bkg_mult][i].GetFinalParticles4Mom();
      particles_uncorr = fBkg[bkg_mult][i].GetFinalParticlesUnCorr4Mom();
      V4_el = fBkg[bkg_mult][i].GetOutLepton4Mom();
      fRotation->SetQVector( fBkg[bkg_mult][i].GetRecoq3() );	
      double event_wgt = fBkg[bkg_mult][i].GetTotalWeight() ;

      // 2p1pi 
      unsigned int n_pions = particles[conf::kPdgPiP].size() + particles[conf::kPdgPiM].size() + particles[conf::kPdgPhoton].size() ;  
      if( particles[conf::kPdgProton].size() == 2 && n_pions == 1 ) {

	TVector3 V3_2prot_corr[2], V3_2prot_uncorr[2];
	for ( unsigned k = 0 ; k < 2 ; ++k ) {
	  V3_2prot_corr[k] = particles[conf::kPdgProton][k].Vect() ; 
	  V3_2prot_uncorr[k] = particles_uncorr[conf::kPdgProton][k].Vect() ; 
	}

	TVector3 V3_1pi_corr(0,0,0) ; 
	int pi_charge = 0 ; 
	if( particles[conf::kPdgPiP].size() == 1 ) {
	  V3_1pi_corr = particles[conf::kPdgPiP][0].Vect() ; 
	  pi_charge = 1 ; 
	} else if( particles[conf::kPdgPiM].size() == 1 ) {
	  V3_1pi_corr = particles[conf::kPdgPiM][0].Vect() ;
	  pi_charge = -1 ; 
	else if( particles[conf::kPdgPhoton].size() == 1 ) V3_1pi_corr = particles[conf::kPdgPhoton][0].Vect() ; 

	double Ecal_2p1pi_to2p0pi[bkg_mult-1] = {0};
	double p_miss_perp_2p1pi_to2p0pi[bkg_mult-1]={0}; 
	double P_2p1pito2p0pi[2] = {0};
	double P_2p1pito1p1pi[2] = {0};
	double P_2p1pito1p0pi[2] = {0};
	double Ptot = 0;

	fRotation->prot2_pi1_rot_func(V3_2prot_corr,V3_2prot_uncorr,V3_1pi_corr, pi_charge, 
				      V4_el,Ecal_2p1pi_to2p0pi,p_miss_perp_2p1pi_to2p0pi,
				      P_2p1pito2p0pi, P_2p1pito1p1pi, P_2p1pito1p0pi,&Ptot);
	// 2p 1pi -> 2p 0pi
	// Store one proton only so ECal calculation is consistent with master
	for( unsigned int j = 0 ; j < particles[conf::kPdgProton].size() ; ++j ) {
	  std::vector<TLorentzVector> corr_mom = { particles[conf::kPdgProton][j] } ;
	  std::vector<TLorentzVector> uncorr_mom = { particles_uncorr[conf::kPdgProton][j] } ;
	  t_particles[conf::kPdgProton] = corr_mom ;
	  t_particles_uncorr[conf::kPdgProton] = uncorr_mom ;
	  // Store information
	  fBkg[bkg_mult][i].SetFinalParticlesKinematics( t_particles ) ; 
	  fBkg[bkg_mult][i].SetFinalParticlesUnCorrKinematics( t_particles_uncorr ) ; 
	  fBkg[bkg_mult][i].SetEventWeight( P_2p1pito2p0pi[j] * event_wgt ) ; 
	  if ( fBkg.find(min_mult) != fBkg.end() ) {
	    fBkg[min_mult].push_back( fBkg[bkg_mult][i] ) ; 
	  } else {
	    std::vector<e4nu::MCEvent> temp = { fBkg[bkg_mult][i] } ;
	    fBkg[min_mult] = temp ; 
	  }
	}

	// 2p 1pi -> 1p 1pi
	// Store one proton only for a consistent ECal calculation
	for( unsigned int j = 0 ; j < particles[conf::kPdgProton].size() ; ++j ) {
	  std::vector<TLorentzVector> corr_mom = { particles[conf::kPdgProton][j] } ;
	  std::vector<TLorentzVector> uncorr_mom = { particles_uncorr[conf::kPdgProton][j] } ;
	  t_particles[conf::kPdgProton] = corr_mom ;
	  t_particles_uncorr[conf::kPdgProton] = uncorr_mom ;
	  // Store information
	  fBkg[bkg_mult][i].SetFinalParticlesKinematics( t_particles ) ; 
	  fBkg[bkg_mult][i].SetFinalParticlesUnCorrKinematics( t_particles_uncorr ) ; 
	  fBkg[bkg_mult][i].SetEventWeight( P_2p1pito1p1pi[j] * event_wgt ) ; 
	  if ( fBkg.find(min_mult) != fBkg.end() ) {
	    fBkg[min_mult].push_back( fBkg[bkg_mult][i] ) ; 
	  } else {
	    std::vector<e4nu::MCEvent> temp = { fBkg[bkg_mult][i] } ;
	    fBkg[min_mult] = temp ; 
	  }
	}
	// 2p 1pi -> 1p 0pi
	// Store one proton only so ECal calculation is consistent with master
	for( unsigned int j = 0 ; j < particles[conf::kPdgProton].size() ; ++j ) {
	  std::vector<TLorentzVector> corr_mom = { particles[conf::kPdgProton][j] } ;
	  std::vector<TLorentzVector> uncorr_mom = { particles_uncorr[conf::kPdgProton][j] } ;
	  t_particles[conf::kPdgProton] = corr_mom ;
	  t_particles_uncorr[conf::kPdgProton] = uncorr_mom ;
	  // Store information
	  fBkg[bkg_mult][i].SetFinalParticlesKinematics( t_particles ) ; 
	  fBkg[bkg_mult][i].SetFinalParticlesUnCorrKinematics( t_particles_uncorr ) ; 
	  fBkg[bkg_mult][i].SetEventWeight( -P_2p1pito1p0pi[j] * event_wgt ) ; 
	  if ( fBkg.find(min_mult) != fBkg.end() ) {
	    fBkg[min_mult].push_back( fBkg[bkg_mult][i] ) ; 
	  } else {
	    std::vector<e4nu::MCEvent> temp = { fBkg[bkg_mult][i] } ;
	    fBkg[min_mult] = temp ; 
	  }
	}
      }

      // 3p0pi ->2p -> 1p
      if( particles[conf::kPdgProton].size() == 3 && n_pions == 0 ) {
	const int N_3p = 3;
	const int N_comb = 3; // N_2p!/(N_2p!*(N_3p-N_2p)!
	const int N_2p = 2;
	TVector3 V3_prot_uncorr[N_3p],V3_prot_corr[N_3p];
	for ( unsigned k = 0 ; k < N_3p ; ++k ) {
	  V3_prot_corr[k] = particles[conf::kPdgProton][k].Vect() ; 
	  V3_prot_uncorr[k] = particles_uncorr[conf::kPdgProton][k].Vect() ; 
	}

	// Store all possible combinations
	TLorentzVector TLV_prot_corr[N_comb][N_2p] ; 
	TLorentzVector TLV_prot_uncorr[N_comb][N_2p] ; 
	unsigned int count = 0 ; 
	for( unsigned int i1 = 0; i1 < N_comb; ++i1) {
	  for( unsigned int i2 = 0; i2 < N_comb; ++i2) {
	    if( ( i1 == i2 ) || ( i1 > i2 ) ) continue ; 
	    TLV_prot_corr[count][0] = particles[conf::kPdgProton][i1];
	    TLV_prot_corr[count][1] = particles[conf::kPdgProton][i2];
	    ++count ; 
	  }
	}

	double P_3pto1p[N_3p];
	double N_p1[N_3p]={0};
	double N_p_three=0;
	double E_cal_3pto1p[3]={0};
	double p_miss_perp_3pto1p[3]={0};
	double E_cal_3pto2p[3][N_2p]={0};
	double p_miss_perp_3pto2p[3][N_2p]={0};
	double P_3pto2p[3][N_2p]={0};

	fRotation->prot3_rot_func( V3_prot_corr,V3_prot_uncorr,V4_el,E_cal_3pto2p,p_miss_perp_3pto2p, P_3pto2p,N_p1, E_cal_3pto1p,p_miss_perp_3pto1p,&N_p_three);
	//3p to 2p->1p
	for(int c = 0; c < N_comb; c++) { // Loop over number of combinations
	    for(int j = 0; j < N_2p; j++) { // Loop over two protons
	      std::vector<TLorentzVector> corr_mom = { TLV_prot_corr[c][j] };
	      std::vector<TLorentzVector> uncorr_mom = {  TLV_prot_uncorr[c][j] };
	      t_particles[conf::kPdgProton] = corr_mom ;
	      t_particles_uncorr[conf::kPdgProton] = uncorr_mom; 
	      // Store information
	      fBkg[bkg_mult][i].SetFinalParticlesKinematics( t_particles ) ; 
	      fBkg[bkg_mult][i].SetFinalParticlesUnCorrKinematics( t_particles_uncorr ) ; 
	      fBkg[bkg_mult][i].SetEventWeight( P_3pto2p[c][j] * event_wgt ) ; 
	      if ( fBkg.find(min_mult) != fBkg.end() ) {
		fBkg[min_mult].push_back( fBkg[bkg_mult][i] ) ; 
	      } else {
		std::vector<e4nu::MCEvent> temp = { fBkg[bkg_mult][i] } ;
		fBkg[min_mult] = temp ; 
	      }
	    }
	}
	// 3p -> 1p 
	for(int j = 0; j < N_3p; j++) {
	  P_3pto1p[j]= N_p1[j] / N_p_three;

	  std::vector<TLorentzVector> corr_pmom = { particles[conf::kPdgProton][j] } ; 
	  std::vector<TLorentzVector> uncorr_pmom = {particles_uncorr[conf::kPdgProton][j] }; 
	  t_particles[conf::kPdgProton] = corr_pmom ;
	  t_particles_uncorr[conf::kPdgProton] = uncorr_pmom ;
	  // Store information
	  fBkg[bkg_mult][i].SetFinalParticlesKinematics( t_particles ) ; 
	  fBkg[bkg_mult][i].SetFinalParticlesUnCorrKinematics( t_particles_uncorr ) ; 
	  fBkg[bkg_mult][i].SetEventWeight( -P_3pto1p[j] * event_wgt ) ; 
	  if ( fBkg.find(min_mult) != fBkg.end() ) {
	    fBkg[min_mult].push_back( fBkg[bkg_mult][i] ) ; 
	  } else {
	    std::vector<e4nu::MCEvent> temp = { fBkg[bkg_mult][i] } ;
	    fBkg[min_mult] = temp ; 
	  }
	}
      }
    }
  }

  bkg_mult = min_mult + 3 ;
  if( bkg_mult > max_mult ) return true ; 
  // remove multiplicity 4 contribution to signal...
  if ( fBkg.find(bkg_mult) != fBkg.end() ) {
    for ( unsigned int i = 0 ; i < fBkg[bkg_mult].size() ; ++i ) {
      particles = fBkg[bkg_mult][i].GetFinalParticles4Mom();
      particles_uncorr = fBkg[bkg_mult][i].GetFinalParticlesUnCorr4Mom(); // This map needs to change with the cuts as well...
      double event_wgt = fBkg[bkg_mult][i].GetTotalWeight() ;
      unsigned int n_pions = particles[conf::kPdgPiP].size() + particles[conf::kPdgPiM].size() + particles[conf::kPdgPhoton].size() ;  

      // 2p2pi 

      // 3p1pi
      if( particles[conf::kPdgProton].size() == 3 && n_pions == 1 ) {
	double P_tot_3p[3]={0};
	double Ecal_3p1pi[3]={0};
	double p_miss_perp_3p1pi[3]={0};
	TVector3 V3_prot_uncorr[3],V3_prot_corr[3];
	for ( unsigned k = 0 ; k < 3 ; ++k ) {
	  V3_prot_corr[k] = particles[conf::kPdgProton][k].Vect() ; 
	  V3_prot_uncorr[k] = particles_uncorr[conf::kPdgProton][k].Vect() ; 
	}

	TVector3 V3_pi_corr(0,0,0) ; 
	int pi_charge = 0 ; 
	if( particles[conf::kPdgPiP].size() == 1 ) {
	  V3_pi_corr = particles[conf::kPdgPiP][0].Vect() ; 
	  pi_charge = 1 ; 
	} else if( particles[conf::kPdgPiM].size() == 1 ) {
	  V3_pi_corr = particles[conf::kPdgPiM][0].Vect() ;
	  pi_charge = -1 ; 
	else if( particles[conf::kPdgPhoton].size() == 1 ) V3_pi_corr = particles[conf::kPdgPhoton][0].Vect() ; 

	fRotation->prot3_pi1_rot_func(V3_prot_corr, V3_prot_uncorr, V3_pi_corr, pi_charge , V4_el, Ecal_3p1pi, p_miss_perp_3p1pi, P_tot_3p);

	for(int j = 0; j < 3; j++) {
	  std::vector<TLorentzVector> corr_pmom = { particles[conf::kPdgProton][j] } ; 
	  std::vector<TLorentzVector> uncorr_pmom = {particles_uncorr[conf::kPdgProton][j] }; 
	  t_particles[conf::kPdgProton] = corr_pmom ;
	  t_particles_uncorr[conf::kPdgProton] = uncorr_pmom ;
	  // Store information
	  fBkg[bkg_mult][i].SetFinalParticlesKinematics( t_particles ) ; 
	  fBkg[bkg_mult][i].SetFinalParticlesUnCorrKinematics( t_particles_uncorr ) ; 
	  fBkg[bkg_mult][i].SetEventWeight( P_tot_3p[j] * event_wgt ) ; 
	  if ( fBkg.find(min_mult) != fBkg.end() ) {
	    fBkg[min_mult].push_back( fBkg[bkg_mult][i] ) ; 
	  } else {
	    std::vector<e4nu::MCEvent> temp = { fBkg[bkg_mult][i] } ;
	    fBkg[min_mult] = temp ; 
	  }
	}
       
      }
    }
  }
  */
  return true ; 
} 
