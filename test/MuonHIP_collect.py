import os
import FWCore.ParameterSet.Config as cms

process = cms.Process("collect")
process.source = cms.Source("EmptySource")
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1))

process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Geometry.CMSCommonData.cmsIdealGeometryXML_cfi")
process.load("Geometry.CommonDetUnit.bareGlobalTrackingGeometry_cfi")
process.load("Geometry.MuonNumbering.muonNumberingInitialization_cfi")
process.load("Geometry.RPCGeometry.rpcGeometry_cfi")
process.load("Geometry.TrackerNumberingBuilder.trackerNumberingGeometry_cfi")
process.load("Geometry.TrackerGeometryBuilder.trackerGeometry_cfi")

process.EventContentAnalyzer = cms.EDAnalyzer("EventContentAnalyzer")
process.Path = cms.Path(process.EventContentAnalyzer)

process.load("Alignment.CommonAlignmentProducer.AlignmentProducer_cff")
process.looper.tjTkAssociationMapTag = cms.InputTag("globalCosmicMuons:Refitted")
process.looper.doTracker = cms.untracked.bool(False)
process.looper.doMuon = cms.untracked.bool(True)
process.looper.algoConfig = cms.PSet(
  algoName = cms.string("MuonHIPAlignmentAlgorithm"),
  minTrackerHits = cms.int32(10),
  maxRedChi2 = cms.double(20.),
  minStations = cms.int32(-1),
  minHitsPerDT = cms.int32(-1),
  minHitsPerDT4 = cms.int32(-1),
  minHitsPerCSC = cms.int32(-1),
  maxResidualDT13 = cms.double(500.),
  maxResidualDT2 = cms.double(500.),
  maxResidualCSC = cms.double(500.),
  ignoreCSCRings = cms.vint32(1, 4),
  minTracksPerAlignable = cms.int32(500),
  useHitWeightsInDTAlignment = cms.bool(os.environ["MuonHIP_useWeights"] == "True"),
  useHitWeightsInCSCAlignment = cms.bool(os.environ["MuonHIP_useWeights"] == "True"),
  useOneDTSuperLayerPerEntry = cms.bool(True),
  useOneCSCChamberPerEntry = cms.bool(True),
  fitRangeDTrphi = cms.double(20.),
  fitRangeDTz = cms.double(20.),
  fitRangeCSCrphi = cms.double(20.),
  fitRangeCSCz = cms.double(20.),

  align = cms.bool(True),
  collector = cms.vstring(*os.environ["MuonHIP_subroots"].split("\n")),
  collectorDirectory = cms.string("globalCosmicMuons/MuonHIPAlignmentAlgorithm"),
  )

process.looper.ParameterBuilder.Selector = cms.PSet(alignParams = cms.vstring("MuonDTWheels," + os.environ["MuonHIP_params"], "MuonCSCStations," + os.environ["MuonHIP_params"]))

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = cms.string("CRAFT_V3P::All")
if os.environ["MuonHIP_trackerdb"] != "" or os.environ["MuonHIP_fromdb"] != "":
  del process.es_prefer_GlobalTag
  del process.SiStripPedestalsFakeESSource
  del process.siStripBadChannelFakeESSource
  del process.siStripBadFiberFakeESSource

if os.environ["MuonHIP_trackerdb"] != "":
  import CondCore.DBCommon.CondDBSetup_cfi
  process.trackerAlignment = cms.ESSource("PoolDBESSource",
                                      connect = cms.string("sqlite_file:../" + os.environ["MuonHIP_trackerdb"]),
                                      DBParameters = CondCore.DBCommon.CondDBSetup_cfi.CondDBSetup.DBParameters,
                                      toGet = cms.VPSet(cms.PSet(record = cms.string("TrackerAlignmentRcd"),       tag = cms.string("Alignments")),
                                                        cms.PSet(record = cms.string("TrackerAlignmentErrorRcd"),  tag = cms.string("AlignmentErrors"))))
  process.es_prefer_trackerAlignment = cms.ESPrefer("PoolDBESSource", "trackerAlignment")

if os.environ["MuonHIP_fromdb"] != "":
  import CondCore.DBCommon.CondDBSetup_cfi
  process.muonAlignment = cms.ESSource("PoolDBESSource",
                                      connect = cms.string("sqlite_file:" + os.environ["MuonHIP_fromdb"]),
                                      DBParameters = CondCore.DBCommon.CondDBSetup_cfi.CondDBSetup.DBParameters,
                                      toGet = cms.VPSet(cms.PSet(record = cms.string("DTAlignmentRcd"),       tag = cms.string("DTAlignmentRcd")),
                                                        cms.PSet(record = cms.string("DTAlignmentErrorRcd"),  tag = cms.string("DTAlignmentErrorRcd")),
                                                        cms.PSet(record = cms.string("CSCAlignmentRcd"),      tag = cms.string("CSCAlignmentRcd")),
                                                        cms.PSet(record = cms.string("CSCAlignmentErrorRcd"), tag = cms.string("CSCAlignmentErrorRcd"))))
  process.es_prefer_muonAlignment = cms.ESPrefer("PoolDBESSource", "muonAlignment")
  process.looper.applyDbAlignment = cms.untracked.bool(True)

process.looper.saveToDB = cms.bool(True)
process.load("CondCore.DBCommon.CondDBSetup_cfi")
process.PoolDBOutputService = cms.Service("PoolDBOutputService",
                                          process.CondDBSetup,
                                          connect = cms.string("sqlite_file:" + os.environ["MuonHIP_todb"]),
                                          toPut = cms.VPSet(cms.PSet(record = cms.string("DTAlignmentRcd"), tag = cms.string("DTAlignmentRcd")),
                                                            cms.PSet(record = cms.string("DTAlignmentErrorRcd"), tag = cms.string("DTAlignmentErrorRcd")),
                                                            cms.PSet(record = cms.string("CSCAlignmentRcd"), tag = cms.string("CSCAlignmentRcd")),
                                                            cms.PSet(record = cms.string("CSCAlignmentErrorRcd"), tag = cms.string("CSCAlignmentErrorRcd"))))

process.TFileService = cms.Service("TFileService", fileName = cms.string("block" + os.environ["MuonHIP_block"] + "_iter" + os.environ["MuonHIP_iteration"] + ".root"))
