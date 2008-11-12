import os

execfile("MuonHIP_globalCosmics.py")

os.system("rm -rf working_scripts")
os.system("mkdir working_scripts")

bsub = file("working_scripts/bsub.sh", "w")

for MuonHIP_trackerdb in ["tracker_final.db",]:
  for MuonHIP_block in ["A", "B"]:
    if MuonHIP_block == "A": block = blockA
    elif MuonHIP_block == "B": block = blockB
    elif MuonHIP_block == "C": block = blockC
    elif MuonHIP_block == "D": block = blockD

    last_collect = None

    if MuonHIP_trackerdb == "tracker_HIPSC.db": MuonHIP_block += "_HIPSC"
    if MuonHIP_trackerdb == "tracker_MP.db": MuonHIP_block += "_MP"
    if MuonHIP_trackerdb == "tracker_final.db": MuonHIP_block += "_final"

    for MuonHIP_iteration in 1, 2, 3, 4, 5, 6, 7, 8:
      MuonHIP_fromdb = "block%s_iter%02d.db" % (MuonHIP_block, MuonHIP_iteration - 1)
      MuonHIP_todb = "block%s_iter%02d.db" % (MuonHIP_block, MuonHIP_iteration)
      if MuonHIP_iteration == 1: MuonHIP_fromdb = ""

      if MuonHIP_iteration in (1, 2):
        MuonHIP_params = "001001"
        MuonHIP_useWeights = False
      elif MuonHIP_iteration in (3, 4, 5, 6):
        MuonHIP_params = "101101"
        MuonHIP_useWeights = False
      elif MuonHIP_iteration in (7, 8):
        MuonHIP_params = "010010"
        MuonHIP_useWeights = True

      subjobs = []
      i = -1
      file_index = 0
      while True:
        i += 1
        files = []
        accumulated = 0
        while accumulated < 500000000:
          if file_index < len(block):
            files.append(block[file_index][0])
            accumulated += block[file_index][2]
            file_index += 1
          else:
            break
        
        if len(files) == 0: break
        MuonHIP_files = " ".join(files)

        MuonHIP_sub = "%03d" % i
        subjob = "block%s_iter%02d_sub%s" % (MuonHIP_block, MuonHIP_iteration, MuonHIP_sub)
        # subjob_array = "block%s_iter%02d[%d]" % (MuonHIP_block, MuonHIP_iteration, i+1)
        subjob_array = subjob
        file("working_scripts/" + subjob + ".sh", "w").write("""#!/bin/sh

cd /afs/cern.ch/user/p/pivarski/ALCA_MUONALIGN/SWAlignment/CRAFTwheeldisk/CMSSW_2_1_11/src
eval `scramv1 run -sh`
cd working

export MuonHIP_iteration=%(MuonHIP_iteration)s
export MuonHIP_trackerdb=%(MuonHIP_trackerdb)s
export MuonHIP_fromdb=%(MuonHIP_fromdb)s
export MuonHIP_todb=%(MuonHIP_todb)s
export MuonHIP_params=%(MuonHIP_params)s
export MuonHIP_sub=%(MuonHIP_sub)s
export MuonHIP_block=%(MuonHIP_block)s
export MuonHIP_useWeights=%(MuonHIP_useWeights)s
export MuonHIP_files='%(MuonHIP_files)s'

cmsRun ../MuonHIP_align.py
""" % vars())
        os.system("chmod +x working_scripts/" + subjob + ".sh")

        if last_collect is None:
          bsub.write("bsub -q cmsexpress -G ALCA_EXPRESS -J \"" + subjob_array + "\" " + subjob + ".sh\n")
        else:
          bsub.write("bsub -q cmsexpress -G ALCA_EXPRESS -J \"" + subjob_array + "\" -w \"ended(" + last_collect + ")\" " + subjob + ".sh\n")
        subjobs.append("ended(" + subjob_array + ")")

      collect = "block%s_iter%02d" % (MuonHIP_block, MuonHIP_iteration)
      file("working_scripts/" + collect + ".sh", "w").write("""#!/bin/sh

cd /afs/cern.ch/user/p/pivarski/ALCA_MUONALIGN/SWAlignment/CRAFTwheeldisk/CMSSW_2_1_11/src
eval `scramv1 run -sh`
cd working

export MuonHIP_iteration=%(MuonHIP_iteration)s
export MuonHIP_trackerdb=%(MuonHIP_trackerdb)s
export MuonHIP_fromdb=%(MuonHIP_fromdb)s
export MuonHIP_todb=%(MuonHIP_todb)s
export MuonHIP_params=%(MuonHIP_params)s
export MuonHIP_sub=%(MuonHIP_sub)s
export MuonHIP_block=%(MuonHIP_block)s
export MuonHIP_useWeights=%(MuonHIP_useWeights)s
export MuonHIP_files='%(MuonHIP_files)s'
export MuonHIP_subroots=`ls block%(MuonHIP_block)s_iter%(MuonHIP_iteration)s_sub*.root`

cmsRun ../MuonHIP_collect.py > block%(MuonHIP_block)s_iter%(MuonHIP_iteration)02d.log  &&  rm -f ls block%(MuonHIP_block)s_iter%(MuonHIP_iteration)s_sub*.root
""" % vars())
      os.system("chmod +x working_scripts/" + collect + ".sh")

      bsub.write("bsub -q cmsexpress -G ALCA_EXPRESS -J \"" + collect + "\" -w \"" + " && ".join(subjobs) + "\" " + collect + ".sh\n")
      last_collect = collect

del bsub
os.system("chmod +x working_scripts/bsub.sh")
