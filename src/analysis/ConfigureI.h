/**
 * \info These parameters are configurable 
 * the default values are set here
 **/

#ifndef _CONFIGURE_I_H_
#define _CONFIGURE_I_H_

#include <vector>
#include <map>
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

namespace e4nu { 

  class ConfigureI {

  public: 
    // Default constructor
    ConfigureI() ;

    // Configure with input file 
    ConfigureI( const std::string input_file ) ;
    ConfigureI( const double EBeam, const unsigned int TargetPdg ) ;
      
    void PrintConfiguration(void) const ; 
    double GetConfiguredEBeam(void) const { return kEBeam ; }
    unsigned int GetConfiguredTarget(void) const { return kTargetPdg ; }
    bool UseAllSectors(void) const { return kUseAllSectors ; } 
    bool ApplyFiducial(void) const { return kApplyFiducial ; }
    bool ApplyCorrWeights(void) const { return kApplyAccWeights ; }
    bool ApplyAccWeights(void) const { return kApplyAccWeights ; }
    bool ApplyMottScaling(void) const { return kApplyMottWeight ; }
    bool ApplyReso(void) const { return kApplyReso ; }
    bool ApplyPhiOpeningAngle(void) const { return kApplyPhiOpeningAngle ; }
    bool UsePhiThetaBand(void) const { return kUsePhiThetaBand ; } 
    bool ApplyThetaSlice(void) const { return kApplyThetaSlice ; } 
    bool ApplyGoodSectorPhiSlice(void) const { return kApplyGoodSectorPhiSlice ; }
    bool IsData(void) const { return kIsData ; } 
    bool GetOffSet(void) const { return koffset ; } 
    bool ApplyQ2Cut(void) const{ return kQ2Cut ; }
    bool ApplyWCut(void) const{ return kWCut ; }
    bool ApplyMomCut(void) const { return kApplyMomCut ; } 
    bool ApplyOutElectronCut(void) const { return fOutMomCut ; }      
    bool IsElectronData(void) const { return kIsElectron ; }
    bool IsConfigured(void) const { return kIsConfigured ; }

    unsigned int GetNEventsToRun(void) const { return kNEvents ; } 
    unsigned int GetFirstEventToRun(void) const { return kFirstEvent ; } 
    unsigned int GetMaxBkgMult(void) const { return kMaxBkgMult ; }
    unsigned int GetMinBkgMult(void) const { return kMult_signal ; }
    unsigned int GetNRotations(void) const { return kNRotations ; } 
    bool GetSubstractBkg(void) const { return kSubstractBkg ; }

    std::map<int,unsigned int> GetTopology(void) const{ return kTopology_map ; } 
    unsigned int GetNTopologyParticles(void) ;    
      
    // Histogram Configurables
    std::vector<std::string> GetObservablesTag(void) const { return kObservables ; }
    std::vector<unsigned int> GetNBins(void) const { return kNBins ; }
    std::vector<std::vector<double>> GetRange(void) const { return kRanges ; } 
    bool NormalizeHist(void) { return kNormalize ; }

    std::string GetOutputFile(void) const { return kOutputFile ; }
    std::string GetInputFile(void) const { return kInputFile ; }
    std::string GetXSecFile(void) const { return kXSecFile ; }

    virtual ~ConfigureI();
      
  protected: 
    bool kUseAllSectors = false ; 
    bool kApplyFiducial = true ; 
    bool kApplyAccWeights = true ; 
    bool kApplyCorrWeights = true ; 
    bool kApplyMottWeight = true ; 
    bool kApplyReso = true ; 
    bool kApplyPhiOpeningAngle = false ; 
    bool kUsePhiThetaBand = false ;
    bool kApplyThetaSlice = false; 
    bool kApplyGoodSectorPhiSlice = false ; 
    bool kApplyMomCut = true ; 
    bool kIsData = false ; 
    bool kQ2Cut = true ; 
    bool kWCut = true ; 
    bool fOutMomCut = true ;
    bool kIsElectron = true ; 
    // ofset for oscillation studies
    double koffset = 0 ; 

    // Physics 
    double kEBeam = 1.161 ; 
    unsigned int kTargetPdg = 1000060120 ;

    // Number of events
    unsigned int kNEvents = 0;
    unsigned int kFirstEvent = 0 ; 
      
    // Store "multiplicty" of singal events 
    unsigned int kMult_signal = 0;
    bool kSubstractBkg = false ;

    // Histogram configurables
    std::vector< std::string > kObservables ;
    std::vector< unsigned int > kNBins ;
    std::vector<std::vector<double>> kRanges ; 
    std::string kInputFile ;
    std::string kOutputFile = "";
    std::string kXSecFile = "";
    bool kNormalize = true ; 

    // Topology
    std::map<int,unsigned int> kTopology_map ; // Pdg, multiplicity
    unsigned int kMaxBkgMult = 2 ; 
    unsigned int kNRotations = 100; 

    bool kIsDataLoaded = false ;
    bool kIsConfigured = true ; 

    // Information for output file
    std::unique_ptr<TFile> kOutFile ;
    std::unique_ptr<TTree> kAnalysisTree ; 
    std::vector<TH1D*> kHistograms ; 

  };
}

#endif
