// -*- C++ -*-
//
// Package:    MuonHIPOverlapsOneSideRefitter
// Class:      MuonHIPOverlapsOneSideRefitter
// 
/**\class MuonHIPOverlapsOneSideRefitter MuonHIPOverlapsOneSideRefitter.cc Alignment/MuonHIPOverlapsOneSideRefitter/src/MuonHIPOverlapsOneSideRefitter.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Jim Pivarski
//         Created:  Wed Dec 12 13:31:55 CST 2007
// $Id: MuonHIPOverlapsOneSideRefitter.cc,v 1.1 2008/06/16 15:46:41 pivarski Exp $
//
//

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

// references
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "Geometry/DTGeometry/interface/DTGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "DataFormats/MuonDetId/interface/MuonSubdetId.h"
#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/MuonDetId/interface/DTLayerId.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHitFwd.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include "RecoMuon/TransientTrackingRecHit/interface/MuonTransientTrackingRecHitBuilder.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "Geometry/Records/interface/GlobalTrackingGeometryRecord.h"
#include "CondFormats/Alignment/interface/Definitions.h"
#include "DataFormats/GeometrySurface/interface/Surface.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "PhysicsTools/UtilAlgos/interface/TFileService.h"
#include "TH1F.h"

// products
#include "TrackingTools/PatternTools/interface/Trajectory.h"
#include "TrackingTools/PatternTools/interface/TrajTrackAssociation.h"

//
// class decleration
//

class MuonHIPOverlapsOneSideRefitter : public edm::EDFilter {
   public:
      explicit MuonHIPOverlapsOneSideRefitter(const edm::ParameterSet&);
      ~MuonHIPOverlapsOneSideRefitter();

   private:
      virtual void beginJob(const edm::EventSetup&) ;
      virtual bool filter(edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;
      
      bool onPlusSide(const TrackingRecHit *hit, const DTGeometry *dtGeometry, const CSCGeometry *cscGeometry);

      // ----------member data ---------------------------
      edm::InputTag m_input;
      int m_minDOF;
      bool m_plusIsReference;
      bool m_filterMode;
      double m_maxRedChi2;
      bool m_debuggingHistograms;
      TH1F *th1f_redchi2_10, *th1f_redchi2_100;

      unsigned long m_total_events, m_passing_cuts;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
MuonHIPOverlapsOneSideRefitter::MuonHIPOverlapsOneSideRefitter(const edm::ParameterSet& iConfig)
   : m_input(iConfig.getParameter<edm::InputTag>("input"))
   , m_minDOF(iConfig.getParameter<int>("minDOF"))
   , m_plusIsReference(iConfig.getParameter<bool>("plusIsReference"))
   , m_filterMode(iConfig.getParameter<bool>("filterMode"))
   , m_maxRedChi2(iConfig.getParameter<double>("maxRedChi2"))
   , m_debuggingHistograms(iConfig.getUntrackedParameter<bool>("debuggingHistograms", false))
{
   if (m_debuggingHistograms) {
      edm::Service<TFileService> tfile;
      th1f_redchi2_10 = tfile->make<TH1F>("redchi2_10", "redchi2_10", 100, 0., 10.);
      th1f_redchi2_100 = tfile->make<TH1F>("redchi2_100", "redchi2_100", 100, 0., 100.);
   }

   produces<std::vector<Trajectory> >();
   produces<TrajTrackAssociationCollection>();

   m_total_events = 0;
   m_passing_cuts = 0;
}


MuonHIPOverlapsOneSideRefitter::~MuonHIPOverlapsOneSideRefitter()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
bool MuonHIPOverlapsOneSideRefitter::onPlusSide(const TrackingRecHit *hit, const DTGeometry *dtGeometry, const CSCGeometry *cscGeometry) {
   DetId id = hit->geographicalId();
   if (id.det() == DetId::Muon  &&  id.subdetId() == MuonSubdetId::DT) {
      GlobalPoint zero = dtGeometry->idToDet(id)->toGlobal(LocalPoint(0., 0., 0.));
      GlobalPoint hitpos = dtGeometry->idToDet(id)->toGlobal(hit->localPosition());
      double phiZero = atan2(zero.y(), zero.x());
      double phiHit = atan2(hitpos.y(), hitpos.x());
      double phiDiff = phiHit - phiZero;
      while (phiDiff > M_PI) phiDiff -= 2.*M_PI;
      while (phiDiff < -M_PI) phiDiff += 2.*M_PI;
      return (phiDiff > 0.);
   }
   else if (id.det() == DetId::Muon  &&  id.subdetId() == MuonSubdetId::CSC) {
      GlobalPoint zero = cscGeometry->idToDet(id)->toGlobal(LocalPoint(0., 0., 0.));
      GlobalPoint hitpos = cscGeometry->idToDet(id)->toGlobal(hit->localPosition());
      double phiZero = atan2(zero.y(), zero.x());
      double phiHit = atan2(hitpos.y(), hitpos.x());
      double phiDiff = phiHit - phiZero;
      while (phiDiff > M_PI) phiDiff -= 2.*M_PI;
      while (phiDiff < -M_PI) phiDiff += 2.*M_PI;
      return (phiDiff > 0.);
   }
   else assert(false);  // this shouldn't be used on hits that might not be DT/CSC; not an exception that you can recover from
}

bool
MuonHIPOverlapsOneSideRefitter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   m_total_events++;

   edm::Handle<reco::TrackCollection> tracks;
   iEvent.getByLabel(m_input, tracks);

   edm::ESHandle<DTGeometry> dtGeometry;
   iSetup.get<MuonGeometryRecord>().get(dtGeometry);

   edm::ESHandle<CSCGeometry> cscGeometry;
   iSetup.get<MuonGeometryRecord>().get(cscGeometry);

   // only a formal requirement; not used
   edm::ESHandle<MagneticField> magneticField;
   iSetup.get<IdealMagneticFieldRecord>().get(magneticField);

   edm::ESHandle<GlobalTrackingGeometry> globalGeometry;
   iSetup.get<GlobalTrackingGeometryRecord>().get(globalGeometry);

   // Create these factories once per event
   TrajectoryStateTransform transformer;
   MuonTransientTrackingRecHitBuilder muonTransBuilder;

   // Create a collection of Trajectories, to put in the Event
   std::auto_ptr<std::vector<Trajectory> > trajectoryCollection(new std::vector<Trajectory>);

   // Remember which trajectory is associated with which track
   std::map<edm::Ref<std::vector<Trajectory> >::key_type, edm::Ref<reco::TrackCollection>::key_type> reference_map;
   edm::Ref<std::vector<Trajectory> >::key_type trajCounter = 0;
   edm::Ref<reco::TrackCollection>::key_type trackCounter = 0;

   int tracklets = 0;
   for (reco::TrackCollection::const_iterator track = tracks->begin();  track != tracks->end();  ++track) {
      trackCounter++;
      
      std::vector<std::vector<const TrackingRecHit*> > hits_by_station;
      std::vector<const TrackingRecHit*> current_station;
      int last_station = 0;
      DetId last_id;

      for (trackingRecHit_iterator hit = track->recHitsBegin();  hit != track->recHitsEnd();  ++hit) {
	 DetId id = (*hit)->geographicalId();

	 if (id.det() == DetId::Muon) {
	    if (id.subdetId() == MuonSubdetId::DT) {
	       DTChamberId chamberId(id.rawId());

	       if (last_station == 0) last_station = chamberId.station();
	       if (last_station != chamberId.station()) {

		  if (int(current_station.size()) - 4 >= m_minDOF  &&  chamberId.station() != 4) {
		     hits_by_station.push_back(current_station);
		  }
		  current_station.clear();
	       }

	       current_station.push_back(&**hit);
	       last_station = chamberId.station();
	       last_id = id;
	    } // end if DT

	    else if (id.subdetId() == MuonSubdetId::CSC) {
	       CSCDetId cscId(id.rawId());
	       CSCDetId chamberId(cscId.endcap(), cscId.station(), cscId.ring(), cscId.chamber(), 0);
	       int station = (chamberId.endcap() == 1 ? 1 : -1) * chamberId.station();

	       if (last_station == 0) last_station = station;
	       if (last_station != station) {

		  if (2*int(current_station.size()) - 4 >= m_minDOF) {
		     hits_by_station.push_back(current_station);
		  }
		  current_station.clear();
	       }

	       current_station.push_back(&**hit);
	       last_station = station;
	       last_id = id;
	    } // end if CSC

	 } // end if in the muon system
      } // end loop over hits

      // add the last station
      if (last_id.subdetId() == MuonSubdetId::DT) {
	 DTChamberId chamberId(last_id.rawId());
	 if (int(current_station.size()) - 4 >= m_minDOF  &&  chamberId.station() != 4) {
	    hits_by_station.push_back(current_station);
	 }
      }
      else if (2*int(current_station.size()) - 4 >= m_minDOF) {
	 hits_by_station.push_back(current_station);
      }

      edm::OwnVector<TrackingRecHit> clonedHits, clonedHits2;
      std::vector<TrajectoryMeasurement::ConstRecHitPointer> transHits;
      std::vector<TrajectoryStateOnSurface> TSOSes;

      for (std::vector<std::vector<const TrackingRecHit*> >::const_iterator station = hits_by_station.begin();  station != hits_by_station.end();  ++station) {
	 if (station->size() > 0) {

	    DetId firstId = (*(station->begin()))->geographicalId();
	    const Surface* chamberSurface;
	    if (firstId.subdetId() == MuonSubdetId::DT) {
	       chamberSurface = &(dtGeometry->idToDet(DTChamberId(firstId.rawId()))->surface());
	    }
	    else {
	       CSCDetId cscId(firstId.rawId());
	       CSCDetId chamberId(cscId.endcap(), cscId.station(), cscId.ring(), cscId.chamber(), 0);
	       chamberSurface = &(cscGeometry->idToDet(chamberId)->surface());
	    }

	    std::vector<double> listx, listy, listz, listXX, listXY, listYY;

	    double SXX, SxXX, SxXY, SXY, SxzXX, SxzXY, SyXY, SYY, SyYY, SyzXY, SyzYY, SzXX, SzXY, SzYY, SzzXX, SzzXY, SzzYY;
	    SXX = SxXX = SxXY = SXY = SxzXX = SxzXY = SyXY = SYY = SyYY = SyzXY = SyzYY = SzXX = SzXY = SzYY = SzzXX = SzzXY = SzzYY = 0.;

	    for (std::vector<const TrackingRecHit*>::const_iterator hit = station->begin();  hit != station->end();  ++hit) {
	       DetId id = (*hit)->geographicalId();

	       TrajectoryMeasurement::ConstRecHitPointer hitPtr(muonTransBuilder.build(&**hit, globalGeometry));
	       transHits.push_back(hitPtr);

	       LocalPoint localPoint = (*hit)->localPosition();
	       double sigma_xx, sigma_xy, sigma_yy;

	       if (m_filterMode  ||  (m_plusIsReference == onPlusSide(*hit, &*dtGeometry, &*cscGeometry))) {
		  LocalError localError = (*hit)->localPositionError();
		  sigma_xx = localError.xx();
		  sigma_xy = localError.xy();
		  sigma_yy = localError.yy();
	       }
	       else {
		  AlgebraicSymMatrix localErrorWithAPE = hitPtr->parametersError();
		  sigma_xx = localErrorWithAPE[0][0];
		  sigma_xy = (localErrorWithAPE.num_row() == 1 ? 0. : localErrorWithAPE[0][1]);
		  sigma_yy = (localErrorWithAPE.num_row() == 1 ? 0. : localErrorWithAPE[1][1]);
	       }

	       LocalPoint chamberPoint;
	       AlgebraicSymMatrix chamberError(2);

	       if (id.subdetId() == MuonSubdetId::DT) {
		  const Surface& layerSurface = dtGeometry->idToDet(id)->surface();

		  sigma_yy = 1e6;
		  sigma_xy = 0.;

		  chamberPoint = chamberSurface->toLocal(layerSurface.toGlobal(localPoint));
	       
		  align::RotationType rotation = chamberSurface->rotation() * layerSurface.rotation().transposed();
		  align::RotationType error = rotation * align::RotationType(sigma_xx, sigma_xy, 0, sigma_xy, sigma_yy, 0, 0, 0, 0) * rotation.transposed();
		  chamberError[0][0] = error.xx();
		  chamberError[1][1] = error.yy();
		  chamberError[0][1] = error.yx();
	       }

	       else { // must be CSC (see above)
		  const Surface& layerSurface = cscGeometry->idToDet(id)->surface();

		  chamberPoint = chamberSurface->toLocal(layerSurface.toGlobal(localPoint));

		  align::RotationType rotation = chamberSurface->rotation() * layerSurface.rotation().transposed();
		  align::RotationType error = rotation * align::RotationType(sigma_xx, sigma_xy, 0, sigma_xy, sigma_yy, 0, 0, 0, 0) * rotation.transposed();
		  chamberError[0][0] = error.xx();
		  chamberError[1][1] = error.yy();
		  chamberError[0][1] = error.yx();
	       }

	       double xi = chamberPoint.x();
	       double yi = chamberPoint.y();
	       double zi = chamberPoint.z();
	       listx.push_back(xi);
	       listy.push_back(yi);
	       listz.push_back(zi);

	       int ierr;
	       chamberError.invert(ierr);
	       if (ierr != 0) {
		  edm::LogError("MuonHIPOverlapsOneSideRefitter") << "Matrix inversion failed!  ierr = " << ierr << " matrix = " << std::endl << chamberError << std::endl;
		  if (m_filterMode) return false;
		  else return true;
	       }
	       double XX = chamberError[0][0];
	       double XY = chamberError[0][1];
	       double YY = chamberError[1][1];
	       listXX.push_back(XX);
	       listXY.push_back(XY);
	       listYY.push_back(YY);

	       SXX += XX;
	       SxXX += xi * XX;
	       SxXY += xi * XY;
	       SXY += XY;
	       SxzXX += xi * zi * XX;
	       SxzXY += xi * zi * XY;
	       SyXY += yi * XY;
	       SYY += YY;
	       SyYY += yi * YY;
	       SyzXY += yi * zi * XY;
	       SyzYY += yi * zi * YY;
	       SzXX += zi * XX;
	       SzXY += zi * XY;
	       SzYY += zi * YY;
	       SzzXX += zi * zi * XX;
	       SzzXY += zi * zi * XY;
	       SzzYY += zi * zi * YY;

	    } // end loop over hits

	    // calculate the least-squares fit
	    double denom = (SzzXX*(SXX*(SzzYY*SYY - pow(SzYY,2)) - pow(SzXY,2)*SYY - SzzYY*pow(SXY,2) + 2*SzXY*SzYY*SXY) + SzzXY*(SzXY*(2*SzXX*SYY + 2*SzYY*SXX) - 2*SzXX*SzYY*SXY - 2*pow(SzXY,2)*SXY) + pow(SzzXY,2)*(pow(SXY,2) - SXX*SYY) + pow(SzXX,2)*(pow(SzYY,2) - SzzYY*SYY) + 2*SzXX*SzXY*SzzYY*SXY + pow(SzXY,2)*(-SzzYY*SXX - 2*SzXX*SzYY) + pow(SzXY,4));
	    double a = (-SzXX*(SyXY*(SzzYY*SYY - pow(SzYY,2)) + SxXX*(SzzYY*SYY - pow(SzYY,2)) + ((-SyYY - SxXY)*SzzYY + (SyzYY + SxzXY)*SzYY)*SXY) - SyzXY*(SXX*(pow(SzYY,2) - SzzYY*SYY) + SzzYY*pow(SXY,2)) - SxzXX*(SXX*(pow(SzYY,2) - SzzYY*SYY) + SzzYY*pow(SXY,2)) - SzzXY*(SXX*(SyzYY*SYY + SxzXY*SYY + (-SyYY - SxXY)*SzYY) + SzXY*(-SyXY*SYY - SxXX*SYY + (SyYY + SxXY)*SXY) + (-SyzYY - SxzXY)*pow(SXY,2) + (SyXY*SzYY + SxXX*SzYY)*SXY) - SzXY*(SzXX*(-SyzYY*SYY - SxzXY*SYY + (SyYY + SxXY)*SzYY) + (-SyXY*SzzYY - SxXX*SzzYY)*SXY - 2*SyzXY*SzYY*SXY - 2*SxzXX*SzYY*SXY + ((SyYY + SxXY)*SzzYY + (-SyzYY - SxzXY)*SzYY)*SXX) - pow(SzXY,2)*(SyzXY*SYY + SxzXX*SYY + (SyzYY + SxzXY)*SXY + SyXY*SzYY + SxXX*SzYY) - (-SyYY - SxXY)*pow(SzXY,3))/denom;
	    double b = (SzzXX*(SyXY*(SzzYY*SYY - pow(SzYY,2)) + SxXX*(SzzYY*SYY - pow(SzYY,2)) + SzXY*(-SyzYY*SYY - SxzXY*SYY + (SyYY + SxXY)*SzYY) + ((-SyYY - SxXY)*SzzYY + (SyzYY + SxzXY)*SzYY)*SXY) + SzXX*(SyzXY*(pow(SzYY,2) - SzzYY*SYY) + SxzXX*(pow(SzYY,2) - SzzYY*SYY)) + SzzXY*(SzXX*(SyzYY*SYY + SxzXY*SYY + (-SyYY - SxXY)*SzYY) + SzXY*(SyzXY*SYY + SxzXX*SYY + (-SyzYY - SxzXY)*SXY + 2*SyXY*SzYY + 2*SxXX*SzYY) - SyzXY*SzYY*SXY - SxzXX*SzYY*SXY + (-SyYY - SxXY)*pow(SzXY,2)) + pow(SzzXY,2)*(-SyXY*SYY - SxXX*SYY + (SyYY + SxXY)*SXY) + SzXY*(SyzXY*SzzYY*SXY + SxzXX*SzzYY*SXY + SzXX*((SyYY + SxXY)*SzzYY + (-SyzYY - SxzXY)*SzYY)) + pow(SzXY,2)*(-SyXY*SzzYY - SxXX*SzzYY - SyzXY*SzYY - SxzXX*SzYY) + (SyzYY + SxzXY)*pow(SzXY,3))/denom;
	    double c = (-SzzXY*(SyzXY*(SXX*SYY - pow(SXY,2)) + SxzXX*(SXX*SYY - pow(SXY,2)) + SzXX*(-SyXY*SYY - SxXX*SYY + (SyYY + SxXY)*SXY) + SzXY*((SyXY + SxXX)*SXY + (-SyYY - SxXY)*SXX)) - SzzXX*(SXX*(-SyzYY*SYY - SxzXY*SYY + (SyYY + SxXY)*SzYY) + SzXY*(SyXY*SYY + SxXX*SYY + (-SyYY - SxXY)*SXY) + (SyzYY + SxzXY)*pow(SXY,2) + (-SyXY*SzYY - SxXX*SzYY)*SXY) - SzXY*(SzXX*(-SyzXY*SYY - SxzXX*SYY + (-2*SyzYY - 2*SxzXY)*SXY + SyXY*SzYY + SxXX*SzYY) - SyzXY*SzYY*SXX - SxzXX*SzYY*SXX) - pow(SzXX,2)*(SyzYY*SYY + SxzXY*SYY + (-SyYY - SxXY)*SzYY) - SzXX*(SyzXY*SzYY*SXY + SxzXX*SzYY*SXY) - pow(SzXY,2)*(SyzXY*SXY + SxzXX*SXY + (SyzYY + SxzXY)*SXX + (SyYY + SxXY)*SzXX) - (-SyXY - SxXX)*pow(SzXY,3))/denom;
	    double d = (SzzXX*(SzXY*((SyzYY + SxzXY)*SXY + SyXY*SzYY + SxXX*SzYY) + (-SyXY*SzzYY - SxXX*SzzYY)*SXY + ((SyYY + SxXY)*SzzYY + (-SyzYY - SxzXY)*SzYY)*SXX + (-SyYY - SxXY)*pow(SzXY,2)) + SzzXY*(SzXX*((-SyzYY - SxzXY)*SXY - SyXY*SzYY - SxXX*SzYY) + SzXY*(-SyzXY*SXY - SxzXX*SXY + (SyzYY + SxzXY)*SXX + (2*SyYY + 2*SxXY)*SzXX) + SyzXY*SzYY*SXX + SxzXX*SzYY*SXX + (-SyXY - SxXX)*pow(SzXY,2)) + SzXX*(SyzXY*SzzYY*SXY + SxzXX*SzzYY*SXY) + pow(SzzXY,2)*((SyXY + SxXX)*SXY + (-SyYY - SxXY)*SXX) + SzXY*(-SyzXY*SzzYY*SXX - SxzXX*SzzYY*SXX + SzXX*(SyXY*SzzYY + SxXX*SzzYY - SyzXY*SzYY - SxzXX*SzYY)) + pow(SzXX,2)*((-SyYY - SxXY)*SzzYY + (SyzYY + SxzXY)*SzYY) + (SyzXY + SxzXX)*pow(SzXY,3) + (-SyzYY - SxzXY)*SzXX*pow(SzXY,2))/denom;

	    GlobalVector momentum = chamberSurface->toGlobal(LocalVector(a, c, 1.) / sqrt(pow(a,2) + pow(c,2) + 1.) * track->p());

	    double chi2 = 0.;
	    int dof = 0;
	    std::vector<const TrackingRecHit*>::const_iterator hit = station->begin();
	    std::vector<double>::const_iterator xi = listx.begin();
	    std::vector<double>::const_iterator yi = listy.begin();
	    std::vector<double>::const_iterator zi = listz.begin();
	    std::vector<double>::const_iterator XX = listXX.begin();
	    std::vector<double>::const_iterator XY = listXY.begin();
	    std::vector<double>::const_iterator YY = listYY.begin();

	    for (;  hit != station->end();  ++hit, ++xi, ++yi, ++zi, ++XX, ++XY, ++YY) {
	       double x = a * (*zi) + b;
	       double y = c * (*zi) + d;

	       double chi2i = (x - (*xi))*(x - (*xi))*(*XX) + 2*(x - (*xi))*(y - (*yi))*(*XY) + (y - (*yi))*(y - (*yi))*(*YY);
	       chi2 += chi2i;
	       if ((*hit)->geographicalId().subdetId() == MuonSubdetId::DT)
		  dof += 1;
	       else
		  dof += 2; 
	    }
	    dof -= 4;
	    
	    if (m_debuggingHistograms) {
	       if (dof > 0) {
		  th1f_redchi2_10->Fill(chi2 / dof);
		  th1f_redchi2_100->Fill(chi2 / dof);
	       }
	    }

	    if (!m_filterMode  ||  (dof >= m_minDOF  &&  chi2 / dof < m_maxRedChi2)) {
	       tracklets++;

	       hit = station->begin();
	       xi = listx.begin();
	       yi = listy.begin();
	       zi = listz.begin();
	       XX = listXX.begin();
	       XY = listXY.begin();
	       YY = listYY.begin();
	       for (;  hit != station->end();  ++hit, ++xi, ++yi, ++zi, ++XX, ++XY, ++YY) {
		  if (m_plusIsReference != onPlusSide(*hit, &*dtGeometry, &*cscGeometry)) {
		     double x = a * (*zi) + b;
		     double y = c * (*zi) + d;

		     GlobalPoint position = chamberSurface->toGlobal(LocalPoint(x, y, (*zi)));
		     DetId id = (*hit)->geographicalId();

		     GlobalTrajectoryParameters globalTrajectoryParameters(position, momentum, track->charge(), &*magneticField);
		     AlgebraicSymMatrix66 error;
		     error(0,0) = 1e-6 * position.x();
		     error(1,1) = 1e-6 * position.y();
		     error(2,2) = 1e-6 * position.z();
		     error(3,3) = 1e-6 * momentum.x();
		     error(4,4) = 1e-6 * momentum.y();
		     error(5,5) = 1e-6 * momentum.z();

		     clonedHits.push_back((*hit)->clone());
		     TSOSes.push_back(TrajectoryStateOnSurface(globalTrajectoryParameters, CartesianTrajectoryError(error),
							       id.subdetId() == MuonSubdetId::DT ? dtGeometry->idToDet(id)->surface() : cscGeometry->idToDet(id)->surface()));
		  } // end if we use hits from this side
	       } // end loop over hits
	    } // end if we're filtering

	 } // end if there are any hits to work with
      } // end loop over stations

      // build the trajectory
      if (clonedHits.size() > 0) {
	 PTrajectoryStateOnDet *PTraj = transformer.persistentState(*(TSOSes.begin()), clonedHits.begin()->geographicalId().rawId());
	 TrajectorySeed trajectorySeed(*PTraj, clonedHits, alongMomentum);
	 Trajectory trajectory(trajectorySeed, alongMomentum);

	 edm::OwnVector<TrackingRecHit>::const_iterator clonedHit = clonedHits.begin();
	 std::vector<TrajectoryMeasurement::ConstRecHitPointer>::const_iterator transHitPtr = transHits.begin();
	 std::vector<TrajectoryStateOnSurface>::const_iterator TSOS = TSOSes.begin();
	 for (;  clonedHit != clonedHits.end();  ++clonedHit, ++transHitPtr, ++TSOS) {
	    trajectory.push(TrajectoryMeasurement(*TSOS, *TSOS, *TSOS, (*transHitPtr)));
	 }

	 trajectoryCollection->push_back(trajectory);

	 // Remember which Trajectory is associated with which Track
	 trajCounter++;
	 reference_map[trajCounter] = trackCounter;

      } // end if there are any clonedHits/TSOSes to work with

   } // end loop over tracks

   unsigned int numTrajectories = trajectoryCollection->size();

   // insert the trajectories into the Event
   edm::OrphanHandle<std::vector<Trajectory> > ohTrajs = iEvent.put(trajectoryCollection);

   // create the trajectory <-> track association map
   std::auto_ptr<TrajTrackAssociationCollection> trajTrackMap(new TrajTrackAssociationCollection());

   for (trajCounter = 0;  trajCounter < numTrajectories;  trajCounter++) {
      edm::Ref<reco::TrackCollection>::key_type trackCounter = reference_map[trajCounter];

      trajTrackMap->insert(edm::Ref<std::vector<Trajectory> >(ohTrajs, trajCounter), edm::Ref<reco::TrackCollection>(tracks, trackCounter));
   }
   // and put it in the Event, also
   iEvent.put(trajTrackMap);

   if (tracklets > 0) m_passing_cuts++;
   if (m_filterMode) return (tracklets > 0);
   return true;
}

// ------------ method called once each job just before starting event loop  ------------
void 
MuonHIPOverlapsOneSideRefitter::beginJob(const edm::EventSetup&)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
MuonHIPOverlapsOneSideRefitter::endJob() {
   std::cout << "MuonHIPOverlapsOneSideRefitter: total_events " << m_total_events << " passing_cuts " << m_passing_cuts << std::endl;

}

//define this as a plug-in
DEFINE_FWK_MODULE(MuonHIPOverlapsOneSideRefitter);
