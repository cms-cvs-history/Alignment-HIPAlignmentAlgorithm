import FWCore.ParameterSet.Config as cms

process = cms.Process("Alignment")
process.load("Alignment.CommonAlignmentProducer.AlignmentTrackSelector_cfi")
process.load("RecoTracker.FinalTrackSelectors.TrackerTrackHitFilter_cff")
#process.load("RecoTracker.TransientTrackingRecHit.TransientTrackingRecHitBuilderWithoutRefit_cfi")
process.load("RecoTracker.TrackProducer.TrackRefitters_cff")
process.load("RecoVertex.BeamSpotProducer.BeamSpot_cff")


# "including" common configuration
<COMMON>

if 'COSMICS' =='<FLAG>':
    process.source = cms.Source("PoolSource",
#                                useCSA08Kludge = cms.untracked.bool(True),
                                fileNames = cms.untracked.vstring(<FILE>)
                                )
else :
    process.source = cms.Source("PoolSource",
                                #useCSA08Kludge = cms.untracked.bool(True),
                                fileNames = cms.untracked.vstring(<FILE>)
                                )
    
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
process.load("RecoVertex.PrimaryVertexProducer.OfflinePrimaryVertices_cfi")
process.offlinePrimaryVertices.TrackLabel = cms.InputTag("TrackRefitter1")
process.offlinePrimaryVertices.minNdof = cms.double(2.0)

# "including" selection for this track sample
<SELECTION>

if 'MBVertex'=='<FLAG>':
    process.pvfilter=cms.EDFilter("VertexSelector",
                              filter = cms.bool(True),
                              src = cms.InputTag('offlinePrimaryVertices'),
                              cut = cms.string("!isFake")
                              )


# parameters for HIP
process.AlignmentProducer.tjTkAssociationMapTag = 'TrackRefitter2'
process.AlignmentProducer.hitPrescaleMapTag= '' #if this is not empty, turn on the usage of prescaled hits
process.AlignmentProducer.algoConfig.outpath = ''
process.AlignmentProducer.algoConfig.uvarFile = '<PATH>/IOUserVariables.root'
###process.AlignmentProducer.algoConfig.uvarFile = './IOUserVariables.root'
if 'COSMICS' =='<FLAG>':
    process.AlignmentProducer.algoConfig.eventPrescale= 1
else :
    process.AlignmentProducer.algoConfig.eventPrescale= 1
process.AlignmentProducer.algoConfig.fillTrackMonitoring=False
process.AlignmentProducer.algoConfig.outfile =  '<PATH>/HIPAlignmentEvents.root'
process.AlignmentProducer.algoConfig.outfile2 = '<PATH>/HIPAlignmentAlignables.root'
process.AlignmentProducer.algoConfig.applyAPE = False

process.AlignmentProducer.algoConfig.apeParam = cms.VPSet(cms.PSet(
															function = cms.string('linear'),
															apeRPar = cms.vdouble(0.001, 0.001, 100.0),
															apeSPar = cms.vdouble(0.100, 0.100, 100.0),
															Selector = cms.PSet(
																				alignParams = cms.vstring('TrackerTPBModule,000000')
																				)
															), 
												   cms.PSet(
															function = cms.string('linear'),
                                                                                                                        apeRPar = cms.vdouble(0.003, 0.003, 100.0),
															apeSPar = cms.vdouble(0.0600, 0.0600, 100.0),
															Selector = cms.PSet(
																				alignParams = cms.vstring('TrackerTPEModule,000000')
																				)
															), 
												   cms.PSet(
															function = cms.string('linear'),
															apeRPar = cms.vdouble(0.001, 0.001, 100.0),
															apeSPar = cms.vdouble(0.0300, 0.0300, 100.0),
                                                                                                                        Selector = cms.PSet(
																				alignParams = cms.vstring('TIBDets,000000')
																				)
															), 
												   cms.PSet(
															function = cms.string('linear'),
															apeRPar = cms.vdouble(0.001, 0.001, 100.0),
															apeSPar = cms.vdouble(0.0600, 0.0600, 100.0),
															Selector = cms.PSet(
																				alignParams = cms.vstring('TIDDets,000000')
																				)
															), 
												   cms.PSet(
															function = cms.string('linear'),
															apeRPar = cms.vdouble(0.001, 0.001, 100.0),
															apeSPar = cms.vdouble(0.0300, 0.0300, 100.0),
															Selector = cms.PSet(
																				alignParams = cms.vstring('TOBDets,000000')
																				)
															), 
												   cms.PSet(
															function = cms.string('linear'),
															apeRPar = cms.vdouble(0.001, 0.001, 100.0),
															apeSPar = cms.vdouble(0.0600, 0.0600, 100.0),
															Selector = cms.PSet(
																				alignParams = cms.vstring('TECDets,000000')
																			    )
															)
												   )	



#### If we are in collisions, apply selections on PhysDeclared bit, L1 trigger bits, LumiSections
if 'COSMICS' !='<FLAG>':
# process only some lumi sections: from LS69, run 123596 till LS 999 in event 124119 
    #process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange('123596:69-124119:999')
# do not process some other runs: in this case: skip all events from event#1 in run 124120 till last event of run 124120
    #process.source.eventsToSkip = cms.untracked.VEventRange('124120:1-124120:MAX')
#filters on L1 trigger bits
    process.load('L1TriggerConfig.L1GtConfigProducers.L1GtTriggerMaskTechTrigConfig_cff')
    process.load('HLTrigger/HLTfilters/hltLevel1GTSeed_cfi')
    process.hltLevel1GTSeed.L1TechTriggerSeeding = cms.bool(True)
    process.hltLevel1GTSeed.L1SeedsLogicalExpression = cms.string('0 AND (40 OR 41) AND NOT (36 OR 37 OR 38 OR 39) AND NOT ((42 AND (NOT 43)) OR (43 AND (NOT 42)))')
    process.load("RecoLocalTracker.SiStripRecHitConverter.OutOfTime_cff")
    process.OutOfTime.TOBlateBP=0.071
    process.OutOfTime.TIBlateBP=0.036
 
    process.stripLorentzAngle = cms.ESSource("PoolDBESSource",CondDBSetup,
                                             connect = cms.string('sqlite_file:/afs/cern.ch/user/b/benhoob/public/LorentzAngle/SiStripLorentzAngle_Deco.db'),
                                             toGet = cms.VPSet(cms.PSet(record = cms.string('SiStripLorentzAngleRcd'),tag =cms.string('SiStripLorentzAngle_Deco') ))
                                             )
    process.es_prefer_stripLorentzAngle = cms.ESPrefer("PoolDBESSource","stripLorentzAngle")


#constraints

#filter on PhysDecl bit
process.skimming = cms.EDFilter("PhysDecl",
                                applyfilter = cms.untracked.bool(True)
                                )


if 'MBVertex'=='<FLAG>':
    process.p = cms.Path(process.hltLevel1GTSeed*process.skimming*process.offlineBeamSpot*process.TrackRefitter1*process.offlinePrimaryVertices*process.pvfilter*process.TrackerTrackHitFilter*process.ctfProducerCustomised*process.AlignmentTrackSelector*process.doConstraint*process.TrackRefitter2)
elif 'COSMICS' =='<FLAG>':
    process.p = cms.Path(process.skimming*process.offlineBeamSpot*process.TrackRefitter1*process.TrackerTrackHitFilter*process.ctfProducerCustomised*process.AlignmentTrackSelector*process.TrackRefitter2)
else :
    process.p = cms.Path(process.hltLevel1GTSeed*process.skimming*process.offlineBeamSpot*process.TrackRefitter1*process.TrackerTrackHitFilter*process.ctfProducerCustomised*process.AlignmentTrackSelector*process.TrackRefitter2)
