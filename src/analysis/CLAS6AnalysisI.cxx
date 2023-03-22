/*
  * Analysis Interface base class
  * 
  */
 #include <iostream>
 #include "TFile.h"
 #include "TDirectoryFile.h"
 #include "analysis/CLAS6AnalysisI.h"
 #include "utils/ParticleUtils.h"
 #include "utils/KinematicUtils.h"
 #include "conf/ParticleI.h"
 #include "conf/TargetI.h"
 #include "utils/DetectorUtils.h"
 #include "conf/FiducialCutI.h"
 #include "conf/AccpetanceMapsI.h"
 #include "conf/AnalysisCutsI.h"
 #include "conf/AnalysisConstantsI.h"

 using namespace e4nu ; 

 CLAS6AnalysisI::CLAS6AnalysisI() {
   kAnalysisTree = std::unique_ptr<TTree>( new TTree("CLAS6Tree","CLAS6 Tree") ) ; 
   kMult_signal = GetNTopologyParticles() ; 
   this->Initialize() ;
 }

 CLAS6AnalysisI::~CLAS6AnalysisI() {
   delete fData;
   kAnalysedEventHolder.clear();
 }

 bool CLAS6AnalysisI::LoadData( void ) {
   if( ! IsConfigured() ) return false ; 

   std::string file = GetInputFile() ; 
   double nevents = GetNEventsToRun() ; 
   double first_event = GetFirstEventToRun() ; 

   if( ! kIsDataLoaded ) { 
     fData = new CLAS6EventHolder( file, first_event, nevents ) ;
     kNEvents = fData->GetNEvents() ; 
     kIsDataLoaded = true ;
   }
   return kIsDataLoaded ; 
 }

 EventI * CLAS6AnalysisI::GetEvent( const unsigned int event_id ) {
   return fData -> GetEvent(event_id) ; 
 }

 EventI * CLAS6AnalysisI::GetValidEvent( const unsigned int event_id ) {

   CLAS6Event * event = (CLAS6Event*) fData -> GetEvent(event_id) ; 
   if( !event ) {
     delete event ; 
     return nullptr ; 
   }

   ++kNEventsBeforeCuts ;

   TLorentzVector in_mom = event -> GetInLepton4Mom() ; 
   TLorentzVector out_mom = event -> GetOutLepton4Mom() ; 

   // Check run is correct
   double EBeam = GetConfiguredEBeam() ; 
   if ( in_mom.E() != EBeam ) {
     std::cout << " Electron energy is " << in_mom.E() << " instead of " << EBeam << "GeV. Configuration failed. Exit" << std::endl;
     delete event ;
     exit(11); 
   }

   if ( (unsigned int) event -> GetTargetPdg() != GetConfiguredTarget() ) {
     std::cout << "Target is " << event -> GetTargetPdg() << " instead of " << GetConfiguredTarget() << ". Configuration failed. Exit" << std::endl;
     delete event ;
     exit(11); 
   }

   // Check weight is physical
   double wght = event->GetEventWeight() ; 
   if ( wght < 0 || wght > 10 || wght == 0 ) {
     delete event ;
     return nullptr ; 
   }

  // Apply Generic analysis cuts
  if ( ! AnalysisI::Analyse( event ) ) {
    delete event ; 
    return nullptr ; 
  }

  // Get Topology Definition
  std::map<int,unsigned int> Topology = GetTopology(); 
  std::map<int,std::vector<TLorentzVector>> part_map = event -> GetFinalParticles4Mom() ;
  std::map<int,std::vector<TLorentzVector>> part_map_uncorr = event -> GetFinalParticlesUnCorr4Mom() ;

  // Signal event
  bool is_signal = true ;
  //Topology ID
  for( auto it = Topology.begin() ; it != Topology.end() ; ++it ) {
    if( it->first == conf::kPdgElectron ) continue ; 
    else if( part_map[it->first].size() != it->second ) {
      is_signal = false ; 
      break ; 
    } 
  }

  unsigned int signal_mult = GetMinBkgMult() ;
  
  if( is_signal ) {
    // Storing in background the signal events
    if( kAnalysedEventHolder.find(signal_mult) == kAnalysedEventHolder.end() ) {
      std::vector<CLAS6Event> temp ( 1, *event ) ;
      kAnalysedEventHolder[signal_mult] = temp ; 
    } else { 
      kAnalysedEventHolder[signal_mult].push_back( *event ) ; 
    }

    ++kNEventsAfterTopologyCut ;
  } else { // BACKGROUND 
    event->SetIsBkg(true); 
    ++kNBkgEvents ;

    // Get Number of signal particles, "multiplicity"
    unsigned int mult_bkg = event->GetNSignalParticles( part_map, Topology ) ; 

    // Check events are above minumum particle multiplicity 
    bool is_signal_bkg = true ; 
    std::map<int,std::vector<TLorentzVector>> hadrons = event->GetFinalParticles4Mom() ;
    
    for( auto part = hadrons.begin() ; part != hadrons.end() ; ++part ) {
      is_signal_bkg *= ChechMinParticleMultiplicity( part->first, (part->second).size() ) ;
    }

    if( !is_signal_bkg ) {
      delete event ;
      return nullptr;
    }

    // Only store background events with multiplicity > mult_signal
    // Also ignore background events above the maximum multiplicity
    if( mult_bkg > signal_mult && mult_bkg <= GetMaxBkgMult() ) {
      if( kAnalysedEventHolder.find(mult_bkg) == kAnalysedEventHolder.end() ) {
	std::vector<CLAS6Event> temp ( 1, *event ) ;
	kAnalysedEventHolder[mult_bkg] = temp ; 
      } else { 
	kAnalysedEventHolder[mult_bkg].push_back( *event ) ; 
      }
    }
    delete event ; 
    return nullptr; 
  }
  
  return event ; 
    
}

bool CLAS6AnalysisI::SubtractBackground() {
  //  return BackgroundI::SubtractBackground<CLAS6Event>( kAnalysedEventHolder ) ; 
  //  bool is_ok = BackgroundI::NewBackgroundSubstraction( kAnalysedEventHolder ) ; 
  //is_ok *= BackgroundI::AcceptanceCorrection( kAnalysedEventHolder ) ; 
  return true ;//is_ok ; 
} 

unsigned int CLAS6AnalysisI::GetNEvents( void ) const {
  return (unsigned int) fData ->GetNEvents() ; 
}

void CLAS6AnalysisI::Initialize() { 

  fData = nullptr ; 
}

bool CLAS6AnalysisI::Finalise( void ) {

  if( !AnalysisI::Finalise() ) return false ; 

  // Store corrected background in event sample
  unsigned int min_mult = GetMinBkgMult() ; 
  for( unsigned int k = 0 ; k < kAnalysedEventHolder[min_mult].size() ; ++k ) {
    // if( IsData() ) 
    StoreTree( static_cast<CLAS6Event*>( &kAnalysedEventHolder[min_mult][k] ) );

    double norm_weight = 1 ; 
    if( ApplyCorrWeights() ) { 
      norm_weight = kAnalysedEventHolder[min_mult][k].GetTotalWeight() ;
    }

    // Store in histogram(s)
    for( unsigned int j = 0 ; j < kHistograms.size() ; ++j ) {
      kHistograms[j]-> Fill( kAnalysedEventHolder[min_mult][k].GetObservable( GetObservablesTag()[j] ), norm_weight ) ; 
    }
  }

  // Normalize
  double domega = 0.01; // sr
  double ConversionFactorCm2ToMicroBarn = TMath::Power(10.,30.); // cm^2 to μbarn

  if ( NormalizeHist() ) {
    for( unsigned int j = 0 ; j < kHistograms.size() ; ++j ) {
      double NBins = kHistograms[j]->GetNbinsX(); 
    
      for (int k = 1; k <= NBins; k++) { 
	double content = kHistograms[j]->GetBinContent(k);
	double error = kHistograms[j]->GetBinError(k);
	double width = kHistograms[j]->GetBinWidth(k);
	double newcontent = content / width;
	double newerror = error / width;
	kHistograms[j]->SetBinContent(k,newcontent);
	kHistograms[j]->SetBinError(k,newerror);
      }

      //      kHistograms[j]->Scale( kXSec * ConversionFactorCm2ToMicroBarn  * TMath::Power(10.,-38.) / ( GetNEventsToRun() * domega ) );
    }
  }
  std::cout << " Total Number of Events Processed = " << kNEventsBeforeCuts << std::endl;
  std::cout << " Total number of true signal events = " << kNEventsAfterTopologyCut << std::endl;

  return true ; 
}

bool CLAS6AnalysisI::StoreTree(CLAS6Event * event){
  static bool n = true ; 
  int ID = event->GetEventID() ; 
  int TargetPdg = event->GetTargetPdg() ;
  int InLeptonPdg = event->GetInLeptPdg() ; 
  int OutLeptonPdg = event->GetOutLeptPdg() ; 
  double TotWeight = event->GetTotalWeight() ; 
  double EventWght = event->GetEventWeight() ; 
  double BeamE = event->GetInLepton4Mom().E() ; 

  unsigned int RecoNProtons = event->GetRecoNProtons() ; 
  unsigned int RecoNNeutrons = event->GetRecoNNeutrons();
  unsigned int RecoNPiP = event->GetRecoNPiP();
  unsigned int RecoNPiM = event->GetRecoNPiM();
  unsigned int RecoNPi0 = event->GetRecoNPi0();
  unsigned int RecoNKP = event->GetRecoNKP();
  unsigned int RecoNKM = event->GetRecoNKM(); 
  unsigned int RecoNK0 = event->GetRecoNK0();
  unsigned int RecoNEM = event->GetRecoNEM();

  TLorentzVector out_mom = event->GetOutLepton4Mom();
  double Efl = out_mom.E();
  double pfl = out_mom.P();
  double pflx = out_mom.Px();
  double pfly = out_mom.Py();
  double pflz = out_mom.Pz();
  double pfl_theta = out_mom.Theta();
  double pfl_phi = out_mom.Phi() + TMath::Pi();
  unsigned int ElectronSector = utils::GetSector( pfl_phi ) ; 

  double RecoQELEnu = utils::GetQELRecoEnu( out_mom, TargetPdg ) ; 
  double RecoEnergyTransfer = utils::GetEnergyTransfer( out_mom, TargetPdg ) ; 
  double Recoq3 = utils::GetRecoq3( out_mom, BeamE ).Mag() ; 
  double RecoQ2 = utils::GetRecoQ2( out_mom, BeamE ) ; 
  double RecoXBJK = utils::GetRecoXBJK( out_mom, BeamE ) ; 
  double RecoW = utils::GetRecoW(out_mom, BeamE ) ;

  std::map<int,unsigned int> topology = GetTopology() ;
  static bool topology_has_protons = false ; 
  if( topology.count(conf::kPdgProton) != 0 ) {
    if( topology[conf::kPdgProton] != 0 ) topology_has_protons = true ; 
  }
  static bool topology_has_pip = false ; 
  if( topology.count(conf::kPdgPiP) != 0 ) {
    if( topology[conf::kPdgPiP] != 0 ) topology_has_pip = true ; 
  }

  static bool topology_has_pim = false ; 
  if( topology.count(conf::kPdgPiM) != 0 ) {
    if( topology[conf::kPdgPiM] != 0 ) topology_has_pim = true ; 
  }

  unsigned int TopMult = GetNTopologyParticles();
  std::map<int,std::vector<TLorentzVector>> hadron_map = event->GetFinalParticles4Mom();
  TLorentzVector p_max(0,0,0,0) ;
  if( topology_has_protons ) {
    double max_mom = 0 ; 
    for( unsigned int i = 0 ; i < hadron_map[conf::kPdgProton].size() ; ++i ) {
      if( hadron_map[conf::kPdgProton][i].P() > max_mom ) {
	max_mom = hadron_map[conf::kPdgProton][i].P() ; 
	p_max = hadron_map[conf::kPdgProton][i] ; 
      }
    }
  }
  double proton_mom = p_max.P() ; 
  double proton_momx = p_max.Px() ; 
  double proton_momy = p_max.Py() ; 
  double proton_momz = p_max.Pz() ; 
  double proton_theta = p_max.Theta() ; 
  double proton_phi = p_max.Phi() + TMath::Pi() ; 
  double ECal = utils::GetECal( out_mom.E(), event->GetFinalParticles4Mom(), TargetPdg ) ; 
  double AlphaT = utils::DeltaAlphaT( out_mom.Vect(), p_max.Vect() ) ; 
  double DeltaPT = utils::DeltaPT( out_mom.Vect(), p_max.Vect() ).Mag() ; 
  double DeltaPhiT = utils::DeltaPhiT( out_mom.Vect(), p_max.Vect() ) ; 

  TLorentzVector pip_max(0,0,0,0) ;
  if( topology_has_pip ) {
    double max_mom = 0 ; 
    for( unsigned int i = 0 ; i < hadron_map[conf::kPdgPiP].size() ; ++i ) {
      if( hadron_map[conf::kPdgPiP][i].P() > max_mom ) {
	max_mom = hadron_map[conf::kPdgPiP][i].P() ; 
	pip_max = hadron_map[conf::kPdgPiP][i] ; 
      }
    }
  }
  double pip_mom = pip_max.P() ;
  double pip_momx = pip_max.Px() ;
  double pip_momy = pip_max.Py() ;
  double pip_momz = pip_max.Pz() ;
  double pip_theta = pip_max.Theta() ;
  double pip_phi = pip_max.Phi() + TMath::Pi();

  TLorentzVector pim_max(0,0,0,0) ;
  if( topology_has_pim ) {
    double max_mom = 0 ; 
    for( unsigned int i = 0 ; i < hadron_map[conf::kPdgPiM].size() ; ++i ) {
      if( hadron_map[conf::kPdgPiM][i].P() > max_mom ) {
	max_mom = hadron_map[conf::kPdgPiM][i].P() ; 
	pim_max = hadron_map[conf::kPdgPiM][i] ; 
      }
    }
  }

  double pim_mom = pim_max.P() ;
  double pim_momx = pim_max.Px() ;
  double pim_momy = pim_max.Py() ;
  double pim_momz = pim_max.Pz() ;
  double pim_theta = pim_max.Theta() ;
  double pim_phi = pim_max.Phi() + TMath::Pi() ;

  bool IsBkg = event->IsBkg() ; 
  if( n == true ) {
    kAnalysisTree -> Branch( "ID", &ID, "ID/I"); 
    kAnalysisTree -> Branch( "TargetPdg", &TargetPdg, "TargetPdg/I");
    kAnalysisTree -> Branch( "InLeptonPdg", &InLeptonPdg, "InLeptonPdg/I");
    kAnalysisTree -> Branch( "OutLeptonPdg", &OutLeptonPdg, "OutLeptonPdg/I");
    kAnalysisTree -> Branch( "BeamE", &BeamE, "BeamE/D");
    kAnalysisTree -> Branch( "TotWeight", &TotWeight, "TotWeight/D");
    kAnalysisTree -> Branch( "EventWght", &EventWght, "EventWght/D");
    kAnalysisTree -> Branch( "IsBkg", &IsBkg, "IsBkg/B");
    kAnalysisTree -> Branch( "RecoNProtons", &RecoNProtons, "RecoNProtons/I");
    kAnalysisTree -> Branch( "RecoNNeutrons", &RecoNNeutrons, "RecoNNeutrons/I");
    kAnalysisTree -> Branch( "RecoNPiP", &RecoNPiP, "RecoNPiP/I");
    kAnalysisTree -> Branch( "RecoNPiM", &RecoNPiM, "RecoNPiM/I");
    kAnalysisTree -> Branch( "RecoNPi0", &RecoNPi0, "RecoNPi0/I");
    kAnalysisTree -> Branch( "RecoNKP", &RecoNKP, "RecoNKP/I");
    kAnalysisTree -> Branch( "RecoNKM", &RecoNKM, "RecoNKM/I");
    kAnalysisTree -> Branch( "RecoNK0", &RecoNK0, "RecoNK0/I");
    kAnalysisTree -> Branch( "RecoNEM", &RecoNEM, "RecoNEM/I");
    kAnalysisTree -> Branch( "TopMult", &TopMult, "TopMult/I");
    kAnalysisTree -> Branch( "Efl", &Efl, "Efl/D");
    kAnalysisTree -> Branch( "pfl", &pfl, "pfl/D");
    kAnalysisTree -> Branch( "pflx", &pflx, "pflx/D");
    kAnalysisTree -> Branch( "pfly", &pfly, "pfly/D");
    kAnalysisTree -> Branch( "pflz", &pflz, "pflz/D");
    kAnalysisTree -> Branch( "pfl_theta", &pfl_theta, "pfl_theta/D");
    kAnalysisTree -> Branch( "pfl_phi", &pfl_phi, "pfl_phi/D");
    kAnalysisTree -> Branch( "RecoQELEnu", &RecoQELEnu, "RecoQELEnu/D");
    kAnalysisTree -> Branch( "RecoEnergyTransfer", &RecoEnergyTransfer, "RecoEnergyTransfer/D");
    kAnalysisTree -> Branch( "Recoq3", &Recoq3, "Recoq3/D");
    kAnalysisTree -> Branch( "RecoQ2", &RecoQ2, "RecoQ2/D");
    kAnalysisTree -> Branch( "RecoW", &RecoW, "RecoW/D");
    kAnalysisTree -> Branch( "RecoXBJK", &RecoXBJK, "RecoXBJK/D");
    kAnalysisTree -> Branch( "ElectronSector", &ElectronSector, "ElectronSector/I");
    if( topology_has_protons ) {
      kAnalysisTree -> Branch( "proton_mom", &proton_mom, "proton_mom/D");
      kAnalysisTree -> Branch( "proton_momx", &proton_momx, "proton_momx/D");
      kAnalysisTree -> Branch( "proton_momy", &proton_momy, "proton_momy/D");
      kAnalysisTree -> Branch( "proton_momz", &proton_momz, "proton_momz/D");
      kAnalysisTree -> Branch( "proton_theta", &proton_theta, "proton_theta/D");
      kAnalysisTree -> Branch( "proton_phi", &proton_phi, "proton_phi/D");
      kAnalysisTree -> Branch( "ECal", &ECal, "ECal/D");
      kAnalysisTree -> Branch( "AlphaT", &AlphaT, "AlphaT/D");
      kAnalysisTree -> Branch( "DeltaPT", &DeltaPT, "DeltaPT/D");
      kAnalysisTree -> Branch( "DeltaPhiT", &DeltaPhiT, "DeltaPhiT/D");
    }

    if( topology_has_pip ) {
      kAnalysisTree -> Branch( "pip_mom", &pip_mom, "pip_mom/D");
      kAnalysisTree -> Branch( "pip_momx", &pip_momx, "pip_momx/D");
      kAnalysisTree -> Branch( "pip_momy", &pip_momy, "pip_momy/D");
      kAnalysisTree -> Branch( "pip_momz", &pip_momz, "pip_momz/D");
      kAnalysisTree -> Branch( "pip_theta", &pip_theta, "pip_theta/D");
      kAnalysisTree -> Branch( "pip_phi", &pip_phi, "pip_phi/D");
    }

    if( topology_has_pim ) {
      kAnalysisTree -> Branch( "pim_mom", &pim_mom, "pim_mom/D");
      kAnalysisTree -> Branch( "pim_momx", &pim_momx, "pim_momx/D");
      kAnalysisTree -> Branch( "pim_momy", &pim_momy, "pim_momy/D");
      kAnalysisTree -> Branch( "pim_momz", &pim_momz, "pim_momz/D");
      kAnalysisTree -> Branch( "pim_theta", &pim_theta, "pim_theta/D");
      kAnalysisTree -> Branch( "pim_phi", &pim_phi, "pim_phi/D");
    }

    n = false ; 
  }
  
  kAnalysisTree -> Fill();
  return true ; 
}
