import FWCore.ParameterSet.Config as cms

process = cms.Process("Alignment")
process.load("Alignment.CommonAlignmentProducer.AlignmentTrackSelector_cfi")
process.load("Alignment.TrackHitFilter.TrackHitFilter_cfi")
#process.load("RecoTracker.TransientTrackingRecHit.TransientTrackingRecHitBuilderWithoutRefit_cfi")
process.load("RecoTracker.TrackProducer.RefitterWithMaterial_cff")
process.load("RecoVertex.BeamSpotProducer.BeamSpot_cff")

# "including" common configuration
<COMMON>

process.source = cms.Source("PoolSource",
							useCSA08Kludge = cms.untracked.bool(True),
							fileNames = cms.untracked.vstring(<FILE>)
)

# parameters for TrackSelector
process.AlignmentTrackSelector.src = '<SKIM>'
# "including" selection for this track sample
<SELECTION>

# parameters for TrackHitFilter
process.TrackHitFilter.src = 'AlignmentTrackSelector'
#process.TrackHitFilter.hitSelection = "SiStripOnly"
#process.TrackHitFilter.hitSelection = "TOBandTIBOnly"        
#process.TrackHitFilter.rejectBadStoNHits = False


# parameters for TrackRefitter
process.TrackRefitter.src = 'TrackHitFilter'
#process.TrackRefitter.src = 'AlignmentTrackSelector'
#process.TrackRefitter.TTRHBuilder = 'WithoutRefit'
process.TrackRefitter.TrajectoryInEvent = True
#process.ttrhbwor.Matcher = 'StandardMatcher'

# parameters for HIP
process.AlignmentProducer.algoConfig.outpath = ''
process.AlignmentProducer.algoConfig.uvarFile = '<PATH>/IOUserVariables.root'
process.AlignmentProducer.algoConfig.apeParam = cms.VPSet(cms.PSet(
															function = cms.string('linear'),
															apeRPar = cms.vdouble(0.003, 0.003, 10.0),
															apeSPar = cms.vdouble(0.01, 0.008, 10.0),
															Selector = cms.PSet(
																				alignParams = cms.vstring('TrackerTPBModule,000000')
																				)
															), 
												   cms.PSet(
															function = cms.string('linear'),
															apeRPar = cms.vdouble(0.003, 0.003, 10.0),
															apeSPar = cms.vdouble(0.01, 0.01, 10.0),
															Selector = cms.PSet(
																				alignParams = cms.vstring('TrackerTPEModule,000000')
																				)
															), 
												   cms.PSet(
															function = cms.string('linear'),
															apeRPar = cms.vdouble(0.002, 0.002, 10.0),
															apeSPar = cms.vdouble(0.05, 0.03, 10.0),
															Selector = cms.PSet(
																				alignParams = cms.vstring('TIBDets,000000')
																				)
															), 
												   cms.PSet(
															function = cms.string('linear'),
															apeRPar = cms.vdouble(0.002, 0.002, 10.0),
															apeSPar = cms.vdouble(0.06, 0.05, 10.0),
															Selector = cms.PSet(
																				alignParams = cms.vstring('TIDDets,000000')
																				)
															), 
												   cms.PSet(
															function = cms.string('linear'),
															apeRPar = cms.vdouble(0.0006, 0.0006, 10.0),
															apeSPar = cms.vdouble(0.02, 0.02, 10.0),
															Selector = cms.PSet(
																				alignParams = cms.vstring('TOBDets,000000')
																				)
															), 
												   cms.PSet(
															function = cms.string('linear'),
															apeRPar = cms.vdouble(0.0007, 0.0007, 10.0),
															apeSPar = cms.vdouble(0.02, 0.02, 10.0),
															Selector = cms.PSet(
																				alignParams = cms.vstring('TECDets,000000')
																				)
															)
												   )	

process.p = cms.Path(process.offlineBeamSpot*process.AlignmentTrackSelector*process.TrackHitFilter*process.TrackRefitter)
