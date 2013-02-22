###########################################################################
# Copyright (C) 2009-2013 GGA Software Services LLC
#
# This file is part of Imago OCR project.
#
# This file may be distributed and/or modified under the terms of the
# GNU General Public License version 3 as published by the Free Software
# Foundation and appearing in the file LICENSE.GPL included in the
# packaging of this file.
#
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
###########################################################################

import sys
import os
import subprocess
import shutil
import time
import StringIO
import collections
import glob
import re

import matplotlib
import matplotlib.pyplot as plt

from indigo.indigo_renderer import *
from indigo.indigo import *

out_dir = "results"
tmp_dir = "tmp"

# Print with flushing
sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)

def createDir(dir):
    if not os.path.exists(dir):
        os.makedirs(dir)

createDir(out_dir)
createDir(tmp_dir) # for output
createDir(os.path.join(tmp_dir, "output/segs")) # for debug output

warnings = open(os.path.join(out_dir, "warnings.txt"), "w")
    
indigo = Indigo()
renderer = IndigoRenderer(indigo)

indigo.setOption("render-output-format", "png");
indigo.setOption("render-background-color", "255, 255, 255"); 
indigo.setOption("render-coloring", "true");
indigo.setOption("render-bond-length", "50");

indigo.setOption("treat-x-as-pseudoatom", "true")
indigo.setOption("ignore-stereochemistry-errors", "true")

versions = [v for v in sorted(os.listdir("versions")) if os.path.splitext(v)[1] != '.txt']

versions_id = {}
for v in versions:
    versions_id[v] = "v%d" % (len(versions_id))

def getVersionName (version):
    name, ext = os.path.splitext(version)
    if name[0].isdigit() and name.find('.') != -1:
        name = name.split('.', 1)[1]
    return name
    
def getVersionInfo (version):
    name, ext = os.path.splitext(version)
    info_file = os.path.join("versions", name + ".txt")
    if os.path.exists(info_file):
        with open(info_file) as info:
            return info.read().strip()
    return ""
    
imgLastId = 0
def getImgId ():
    global imgLastId
    imgLastId += 1
    return "{id}".format(id=imgLastId)
      
def loadMolecule (file):
    all = None
    try:
        all = indigo.createMolecule()
        for m in indigo.iterateSDFile(file):
            all.merge(m)
    except IndigoException as ex:
        print("IndigoException in loadMolecule: " + str(ex))
        warnings.write("%s: %s\n" % (file, str(ex)))
        return None
    return all
    
def loadMoleculeArray (file):
    all = indigo.createArray()
    try:
        for idx, m in enumerate(indigo.iterateSDFile(file)):
            m.setProperty("index", "Molecule #{}".format(idx + 1))
            all.arrayAdd(m)
    except IndigoException as ex:
        print("IndigoException in loadMoleculeArray: " + str(ex))
        warnings.write("%s: %s\n" % (file, str(ex)))
    if all.count() == 0:
        return None
    return all
    
    
# http://stackoverflow.com/questions/4158502/python-kill-or-terminate-subprocess-when-timeout
def subprocess_execute(command, outputfile, timeout):
    """executing the command with a watchdog"""

    with open(outputfile, "wb") as out:
        # launching the command
        c = subprocess.Popen(command,stdout=out,stderr=out)

        # now waiting for the command to complete
        t = 0
        delta = 0.01
        while t < timeout and c.poll() is None:
            time.sleep(delta)  # (comment 1)
            t += delta

        # there are two possibilities for the while to have stopped:
        if c.poll() is None:
            # in the case the process did not complete, we kill it
            c.terminate()
            # and fill the return code with some error value
            returncode = -1  # (comment 2)
            out.write("\n\nTimeout %s exceeded. Aborted\n" % (timeout))
            time.sleep(10)  # Wait process to terminate
        else:                 
            # in the case the process completed normally
            returncode = c.poll()

        return returncode   

def renderCollection (img_name, molfile):
    if molfile is None:
        warnings.write("None has been passed to renderCollection. Img=%s\n" % (img_name))
        return None
        
    if not os.path.exists(molfile):
        warnings.write("There are no file: %s\n" % (molfile))
        return None
        
    if os.path.exists(img_name):
        # Check timestamp
        t1 = os.path.getmtime(molfile)
        t2 = os.path.getmtime(img_name)
        if os.path.getmtime(molfile) < os.path.getmtime(img_name):
            return img_name

    # Render the image
    try:
        collection = loadMoleculeArray(molfile)
        if collection is not None:
            nwidth = min(collection.count(), 4)
            if nwidth > 1:
                indigo.setOption("render-grid-title-property", "index")
            else:
                indigo.setOption("render-grid-title-property", "")
                
            renderer.renderGridToFile(collection, None, nwidth, img_name)
        else:
            return None
            
    except IndigoException as ex:
        print("IndigoException in rendering: " + str(ex))
        warnings.write("IndigoException in rendering: " + str(ex) + "\n")
        return None
    return img_name
        
class ItemVersion:
    def __init__ (self, base):
        self.base = base
        self.molfile = base + '.mol'
        self.timefile = base + '.time'
        self.imgfile = base + '.png'
        self.logfile = base + '.log'
        
    def setFailed (self):
        self.base = None
        
    def getMolFile (self):
        return self.molfile
    def getLogFile (self):
        return self.logfile
    def getTimeFile (self):
        return self.timefile
    def getRenderedFile (self):
        return renderCollection(self.imgfile, self.getMolFile())
            
    def setTime(self, value):
        f = open(self.getTimeFile(), "w")
        f.write("%0.3f" % (value))
        f.close()
        
    def getTime(self):
        try:
            f = open(self.getTimeFile())
            value = f.read()
            f.close()
            return float(value)
        except ValueError,e:
            print "error",e,"in getTime()"
            return 666.0
            
    def getTimestamp ():
        return os.path.getmtime(self.getTimeFile())
        
    def exists (self):
        return os.path.exists(self.getTimeFile())

tmp_subdir_index = 1        
        
class Item:
    def __init__ (self, photo_full):
        self.dirname, self.photo = os.path.split(photo_full)
        print(self.dirname, " : ", self.photo)
        basename_group = self.photo.split('.')[0]
        
        ref_mol_name = os.path.join(self.dirname, basename_group + ".mol")
        self.reference = None
        if os.path.exists(ref_mol_name):
            self.reference = ref_mol_name
        else:
            self.reference = None

        self.reference_rendered = os.path.join(out_dir, "reference", self.dirname, basename_group + ".mol.png")
            
        self.versions = dict()
        
    def getRenderedReference (self):
        dirname, photo = os.path.split(self.reference_rendered)
        createDir(dirname)
        return renderCollection(self.reference_rendered, self.reference)
        
    def getImageFileName (self):
        return os.path.join(self.dirname, self.photo)
        
    def processVersion (self, version):
        vname = getVersionName(version)
        mol_dest_dir = os.path.join(out_dir, vname, self.dirname)
        createDir(mol_dest_dir)
        
        mol_dest = os.path.join(mol_dest_dir, self.photo + "." + vname)
        
        item_version = ItemVersion(mol_dest)
        self.versions[version] = item_version
        if item_version.exists():
            return

        print("*****")
        print("* Version: " + version)
        print("* Image: " + os.path.join(self.dirname, self.photo))
            
        full_version = os.path.join("versions", version)
        abs_version = os.path.abspath(full_version)
        abs_img = os.path.abspath(os.path.join(self.dirname, self.photo))
        
        os.chdir(tmp_dir)
        global tmp_subdir_index
        while True:
            tmp_subdir = str(tmp_subdir_index)
            try:
                createDir(tmp_subdir)
                os.chdir(tmp_subdir)
                mols = glob.glob("*.mol")
                for m in mols:
                    os.remove(m)
                if os.path.exists("log.txt"):
                    os.remove("log.txt")
                break
            except:
                os.chdir("..")
                tmp_subdir_index += 1
                
        time_before = time.time()
        cmd = [abs_version, abs_img]
        if full_version.endswith(".py"):
            cmd = [sys.executable] + cmd

        ret = subprocess_execute(cmd, outputfile = "log.txt", timeout=60) 
        time_after = time.time()
        
        os.chdir("..")
        os.chdir("..")
        item_version.setTime(time_after - time_before)
        
        # Copy log
        log_created = os.path.join(tmp_dir, tmp_subdir, "log.txt")
        if not os.path.exists(log_created):
            item_version.setFailed()
            return
        shutil.copy(log_created, item_version.getLogFile())
        
        if ret != 0:
            item_version.setFailed()
            return
        
        # Copy
        mols = glob.glob(os.path.join(tmp_dir, tmp_subdir, "*.mol"))
        if len(mols) == 0:
            item_version.setFailed()
            return
            
        if len(mols) != 1:
            item_version.setFailed()
            with open(log_created, "at") as log:
                log.write("Multiple molecule files: " + str(mols) + "\n")
            return
        mol_created = mols[0]
        shutil.copy(mol_created, item_version.getMolFile())

groupCount = 0        
        
class Group:
    def __init__ (self, root):
        self.name = os.path.split(root)[1]
        self.root = root
        match = re.match('\d*\.\ *(.*)', self.name)
        if match:
            self.name = match.group(1)
            
        self.readme = ""
        readme_title_path = os.path.join(root, "readme-title.txt")
        readme_path = os.path.join(root, "readme.txt")
        
        if os.path.exists(readme_title_path):
            with open(readme_title_path) as readme_title_file:
                self.readme = readme_title_file.read().strip()
        if os.path.exists(readme_path):
            if self.readme:
                self.readme += "<br />\n"
            self.readme += "<a href='%s'>More information</a>" % (readme_path.replace("\\", "/"))
        
        self.items = []
        self.subgroups = []
        
        global groupCount
        self.id = "g%d" % (groupCount)
        self.stat_id = self.id + "stat"
        groupCount += 1
        
    def processVersion (self, version):
        for item in self.items:
            item.processVersion(version)
        for group in self.subgroups:
            group.processVersion(version)

def collectGroup (root):
    g = Group(root)
    for item in os.listdir(root):
        name, ext = os.path.splitext(item)
        if ext.lower() == '.mol' or ext.lower() == '.txt' or ext.lower() == '.db':
            continue
        rootitem = os.path.join(root, item)
        if os.path.isdir(rootitem):
            g.subgroups.append(collectGroup(rootitem))
        else:
            g.items.append(Item(rootitem))
    return g
    
groups = collectGroup("images")    
#groups.name = "uspto-validation-updated"
for version in versions:
    groups.processVersion(version)

class VersionScore:
    def __init__ (self):
        self.score_sum = 0
        self.time_sum = 0
        self.count = 0
        self.good_count = 0
        self.almost_good_count = 0
        self.scores = []
        self.time_values = []
    def add (self, other):
        self.score_sum += other.score_sum
        self.time_sum += other.time_sum
        self.count += other.count
        self.good_count += other.good_count
        self.almost_good_count += other.almost_good_count
        self.scores += other.scores
        self.time_values += other.time_values

class VersionsScore:
    def __init__ (self):
        self.versions = dict()
    def __getitem__ (self, index):
        if index not in self.versions:
            self.versions[index] = VersionScore()
        return self.versions[index]
    def add (self, other):
        for version in other.versions:
            self[version].add(other[version])
        
    
# generate report
report_data = StringIO.StringIO()
toc_data = StringIO.StringIO()
summary_str = StringIO.StringIO()
same_rows = []
correct_rows = []
almostcorrect_rows = []
             
def prepareStructure (m, arom):
    global charge_pattern, pos_q_atom, neg_q_atom, q_bond

    m.normalize("")
    
    try:
        if arom:    
            m.aromatize()
        else:
            m.dearomatize()
    except:
        pass
    
def measureSimilarityArom (m1, m2, arom):
    if not m1 or not m2:
        return None

    m1 = m1.clone()
    m2 = m2.clone()
        
    prepareStructure(m1, arom)
    prepareStructure(m2, arom)
        
    sim = indigo.similarity(m1, m2, "normalized-edit")
    return sim * 100

def measureSimilarity (m1, m2):
    s1 = measureSimilarityArom(m1, m2, False)
    s2 = measureSimilarityArom(m1, m2, True)
    return max(s1, s2)
    
def getExperimentClass (sim, sim_values, scores):
    cls = ""
    sim_value = 0
    if sim != None:
        sim_value = sim
        
    if sim_value > 99.999:
        cls = "correct"
        scores.good_count += 1
    elif sim_value > 94.99:
        cls = "almostcorrect"
        scores.almost_good_count += 1
    elif sim_value > 59.99:
        cls = "inter"
    else:
        cls = "bad"
        
    sim_values.append(sim_value)
    scores.score_sum += sim_value
    scores.scores.append(sim_value)
    scores.count += 1
        
    return cls
  
def getRefernce (fname):
    if fname is not None:
        return fname.replace('\\', '/')
    else:
        return "?"
  
row_index = 1  
  
def getExpandedMolfile (molfile):
    return molfile
    # Uncomment this to expand abbrevations as a post-processing step
    #ret = subprocess_execute(["expand-abbreviations.exe", molfile, "tmp/mol_exp.mol"], outputfile = "tmp/expand.txt", timeout=10) 
    #if ret < 0:
    #    print("expand-abbreviations.exe failed")
    #    return molfile
    #return "tmp/mol_exp.mol"
  
def generateGroupLine (g, level, name, nameid, link, linkname, cls='', printVersions=True):
    report_data.write("<tr><td class='group %s level%d'><a name='%s'>%s</a> <a href='#%s' class='headerlink'>(%s)</a></td>\n" % (cls, level, nameid, name, link, linkname))
    if printVersions:
        for version in versions:
            name = getVersionName(version)
            report_data.write("<td class='group %s %s'>%s</td>" % (cls, versions_id[version], name))
    report_data.write("</tr>")
  
rowdata = []  
  
def generateGroupReport (g, level):
    generateGroupLine(g, level, g.name, g.id, g.stat_id, "statistics", printVersions=(len(g.subgroups) == 0))
    
    # process all subgroups
    stat = VersionsScore()
    for subgroup in g.subgroups:
        subdata = generateGroupReport(subgroup, level + 1)
        stat.add(subdata)
    g.stat = stat
     
    for item in g.items:
        rowHTML = ""
            
        ref_mol = None
        if item.reference:
            ref_mol = loadMolecule(getExpandedMolfile(item.reference))
                
        prev_molecule = None
        sim_values = []
        sim_values2 = []
        
        for version in versions:
            print("*****")
            print("* Version: " + version)
            print("* Image: " + item.getImageFileName())
            item_version = item.versions[version]
            
            stat[version].time_sum += item_version.getTime()
            stat[version].time_values.append(item_version.getTime())
            
            item_mol = None
            cur_molecule = ""
            try:
                item_mol = loadMolecule(getExpandedMolfile(item_version.getMolFile()))
            except IndigoException as ex:
                print("IndigoException in loadMolecule(item_version.getMolFile()):" + str(ex))

            if item_mol:
                try:
                    cur_molecule = item_mol.molfile()
                except IndigoException as ex:
                    print("IndigoException in loadMolecule(item_version.getMolFile()):" + str(ex))
                    
                sim = measureSimilarity(ref_mol, item_mol)
            else:
                sim = None
                
            print(sim)

            if sim != None:
                sim = float(int(sim))
                sim_value = "%0.0f" % sim
            else:
                sim_value = "0"
            sim_cell_value = sim_value
            
            cls = getExperimentClass(sim, sim_values, stat[version])
                 
            last_row_same = False
            if prev_molecule == cur_molecule:
                if cls == "":
                    cls += " same"
                last_row_same = True
            
            prev_molecule = cur_molecule
            
            rendered = item.versions[version].getRenderedFile()
            if rendered:
                title = item.photo + ": " + getVersionName(version) + " " + sim_cell_value
                imgId = getImgId()
                text = "<a href='%s' class='gal' id='i%s' title='%s'>%s</a> <a href='%s' id='m%s' class='molref' download>.</a>" % (getRefernce(rendered), 
                    imgId, title, sim_cell_value, getRefernce(item.versions[version].getMolFile()), imgId)
            else:
                text = "%s <a href='%s'>log</a>" % (sim_cell_value, getRefernce(item.versions[version].getLogFile()))
            cls += " " + versions_id[version]
                
            rowHTML += "<td class='%s'>%s</td>\n" % (cls, text)
            
            sim_values2.append(sim_value)
        
        # Set row class based on the last column
        global row_index
        rowid = 'row%03d' % (row_index)
        row_index += 1
        
        rowclass = ["row"]
        if last_row_same:
            same_rows.append(rowid)
            rowclass.append("rowsame")
        if "correct" in cls.split():
            correct_rows.append(rowid)
            rowclass.append("rowcorrect")
        if "almostcorrect" in cls.split():
            almostcorrect_rows.append(rowid)
            rowclass.append("rowalmostcorrect")
        
        rowclass = " ".join(rowclass)
  
        imgId = getImgId()
        ref_img = "<a href='%s' id='i%s' class='gal imgref' title='%s'>%s</a>" % (getRefernce(item.getImageFileName()), imgId, item.photo, item.photo)
        ref_mol = "<a href='%s' id='m%s' class='molref' download>.</a>" % (getRefernce(item.reference), imgId)
        if item.getRenderedReference():
            ref_mol_img = "<a href='%s' id='r%s' class='gal ref' title='%s' download>ref</a>" % (getRefernce(item.getRenderedReference()), imgId, item.photo + ': (ref)')
        else:
            ref_mol_img = ""
        report_data.write("<tr id='%s' class='%s'><td class='level%d'>%s %s %s</td>\n" % 
            (rowid, rowclass, level + 1, ref_img, ref_mol_img, ref_mol))
        report_data.write(rowHTML)
        report_data.write("</tr>\n")
        
        rowdata.append((rowid, sim_values2))
            
    # print statistics
    generateGroupLine(g, level + 1, "Statistics for " + g.name, g.stat_id, g.id, 'top', cls='stat')
    report_data.write("<tr><td class='level%d stat'>avg. score</td>\n" % (level + 1))
    for version in versions:
        version_stat = stat[version]
        if version_stat.count != 0:
            avg_score = "%0.2f" % (version_stat.score_sum / version_stat.count)
        else:
            avg_score = "?"
        
        report_data.write("<td class='stat %s'>%s</td>\n" % (versions_id[version], avg_score))
    report_data.write("</tr>\n")
    report_data.write("<tr><td class='level%d stat'>avg. time</td>\n" % (level + 1))
    for version in versions:
        version_stat = stat[version]
        if version_stat.count != 0:
            avg_time = "%0.2f" % (version_stat.time_sum / version_stat.count)
        else:
            avg_time = "?"
        
        report_data.write("<td class='stat %s'>%s</td>\n" % (versions_id[version], avg_time))
    report_data.write("</tr>\n")
    report_data.write("<tr><td class='level%d stat'># [almost] correct out of %d</td>\n" % (level + 1, version_stat.count))
    for version in versions:
        version_stat = stat[version]
        cnt = version_stat.almost_good_count + version_stat.good_count
        report_data.write("<td class='stat %s'>%d<br /><div class='portion'>%0.2f%%</div></td>\n" % (versions_id[version], cnt, 100.0 * cnt / version_stat.count))
    report_data.write("</tr>\n")
    
    report_data.write("<tr><td class='level%d stat'># correct out of %d</td>\n" % (level + 1, version_stat.count))
    for version in versions:
        version_stat = stat[version]
        cnt = version_stat.good_count
        report_data.write("<td class='stat %s'>%d<br /><div class='portion'>%0.2f%%</div></td>\n" % (versions_id[version], cnt, 100.0 * cnt / version_stat.count))
    report_data.write("</tr>\n")
        
    return stat

    
# content page    
def generateTableContent (g, level):
    if level > 0:
        toc_data.write("<li>")    
    toc_data.write("<a href='#%s'>%s</a> <a href='#%s'>(statistics)</a>\n" % (g.id, g.name, g.stat_id))    
    if len(g.readme) > 0:
        toc_data.write("<br />" + g.readme + "\n")
        
    if level > 0:
        toc_data.write("</li>")    
    if len(g.subgroups) > 0:
        toc_data.write("<ul class='toc'>\n")    
        for sg in g.subgroups:
            generateTableContent(sg, level + 1)
        toc_data.write("</ul>\n")    

def renderHistograms():        
    print("Rendering histograms...")
    for g in groups.subgroups:
        for version in versions:
            print("  " + g.name + ": " + getVersionName(version))
            version_stat = g.stat[version]
            
            # Scores histogram
            fig = plt.figure(figsize=(8,5))
            ax = plt.subplot(111)            
            
            dir = os.path.join(out_dir, getVersionName(version), g.root, "histogram_scores.png")
            
            n, bins, patches = ax.hist(version_stat.scores, bins=50, range=(0, 100), color='green', alpha=0.75)
            
            ax.set_xlabel('Score')
            ax.set_ylabel('Count')
            ax.set_xlim(0, 100)
            ax.grid(True)
            ax.set_title(g.name + ": " + getVersionName(version) + " scores")
            
            plt.savefig(dir, dpi=fig.dpi)
            
            version_stat.score_hist = dir

            # Time histogram
            fig = plt.figure(figsize=(8,5))
            ax = plt.subplot(111)            
            
            dir = os.path.join(out_dir, getVersionName(version), g.root, "histogram_time.png")
            
            n, bins, patches = ax.hist(version_stat.time_values, bins=25, color='blue', alpha=0.75)
            
            ax.set_xlabel('Execution time')
            ax.set_ylabel('Count')
            ax.grid(True)
            ax.set_title(g.name + ": " + getVersionName(version) + " time")
            
            plt.savefig(dir, dpi=fig.dpi)
            
            version_stat.time_hist = dir
    print("  OK")
def generateSummary():        
    renderHistograms()
    
    summary_str.write("<tr>")
    summary_str.write("<th></th>")
    summary_str.write("<th colspan=%d>Correct</th>" % len(versions))
    summary_str.write("<th class='hspace'></th>")
    summary_str.write("<th colspan=%d>Average Time</th>" % len(versions))
    summary_str.write("</tr>")

    summary_str.write("<tr>")
    summary_str.write("<th></th>")
    for version in versions:
        summary_str.write("<th class='vheader'>%s</th>" % getVersionName(version))
    summary_str.write("<th class='hspace'></th>")
    for version in versions:
        summary_str.write("<th class='vheader'>%s</th>" % getVersionName(version))
    summary_str.write("</tr>")
    
    # Add lines to summary
    for g in groups.subgroups:
        stat = g.stat
        summary_str.write('<tr>')
        summary_str.write('<td class="leftheader">%s</td>' % g.name)
        for version in versions:
            version_stat = stat[version]
            cnt = version_stat.good_count
            summary_str.write("<td class='stat'><a href='%s' class='histref'>%d</a><br /><div class='portion'>%0.2f%%</div></td>\n" 
                % (version_stat.score_hist, cnt, 100.0 * cnt / version_stat.count))
        summary_str.write("<td class='hspace'></td>")
        for version in versions:
            version_stat = stat[version]
            if version_stat.count != 0:
                avg_time = "%0.2f" % (version_stat.time_sum / version_stat.count)
            else:
                avg_time = "?"
            
            summary_str.write("<td class='stat'><a href='%s' class='histref'>%s</a></td>\n" % (version_stat.time_hist, avg_time))
            
        summary_str.write("<td class='hspace'></td>")
        summary_str.write('</tr>')
    
    summary_str.write("</table>\n")
    
generateTableContent(groups, 0)

summary_str.write("<table id='summary'>\n")

report_data.write("<table id='scores'>")    

generateGroupReport(groups, -1)

report_data.write("</table>")    

# Generate summary
generateSummary()

jsVars = StringIO.StringIO()
jsVars.write("var versionNames = {\n")
for version in versions:
    jsVars.write("  %s : '%s', \n" % (versions_id[version], getVersionName(version)))
jsVars.write("};\n")
jsVars.write("var versions = [\n")
for version in versions:
    jsVars.write("  '%s', \n" % (versions_id[version]))
jsVars.write("];\n")
jsVars.write("var nVersions = %d;\n" % (len(versions)))

jsVars.write("var scores = [\n")
for rowid, sim_values in rowdata:
    jsVars.write("  ['%s', [%s]],\n" % (rowid, ','.join(sim_values)))
jsVars.write("];\n")

versionsStr = StringIO.StringIO()
versionsStr.write("<ul>\n")
# Display Imago first
versionsOrdered = sorted(versions, key=lambda v:v.find("imago") == -1)
for version in versionsOrdered:
    info = getVersionInfo(version)
    if info:
        info = " - " + info
    versionsStr.write("<li><b>%s</b>%s</li>" % (getVersionName(version), info))
versionsStr.write("</ul>\n")

report_template = open("template.html").read()
report = report_template.replace("$SCORES$", report_data.getvalue())
report = report.replace("$TOC$", toc_data.getvalue())
report = report.replace("$JS$", jsVars.getvalue())
report = report.replace("$SUMMARY$", summary_str.getvalue())
report = report.replace("$VERSIONS$", versionsStr.getvalue())
open("report.html", "w").write(report)
