/*
 * Analysis Interface base class
 * 
 */
#include <iostream>
#include "analysis/CLAS6StandardAnalysis.h"

using namespace e4nu ; 

CLAS6StandardAnalysis::CLAS6StandardAnalysis() {}

CLAS6StandardAnalysis::~CLAS6StandardAnalysis() {}

EventI * CLAS6StandardAnalysis::GetValidEvent( const unsigned int event_id ) {
  CLAS6Event * event = (CLAS6Event*) CLAS6AnalysisI::GetValidEvent(event_id) ;

  // Add additional constrains here ...                                                                                                                                                        
  // Operations ...                                                                                                                                                                            

  return event;
}
