#!/usr/bin/env python3

import glob
import subprocess
import os
import sys
import io
import time
import hashlib
import json
import zipfile

#needs to be equal or smaller than the cron
period = 300
outputDir = "share/locale/"
s3Urls = ["s3://extensions.musescore.org/3.6/languages/"]

print("Last changes: 07 Feb 2020")

def processTsFile(prefix, langCode, data):
    print("Processing " + langCode)
    filename = prefix + '_' + lang_code
    tsFilePath = outputDir + filename + ".ts"
    qmFilePath = outputDir + filename + ".qm"

    lang_time = int(os.path.getmtime(tsFilePath))
    cur_time = int(time.time())
    #print(cur_time,lang_time,cur_time-lang_time)

    # if the file has been updated, update or add entry in details.json
    if (cur_time - lang_time < period) or not os.path.isfile(qmFilePath):
        # generate qm file
        lrelease = subprocess.Popen(['lrelease', tsFilePath, '-qm', qmFilePath])
        lrelease.communicate()

        # get qm file size
        file_size = os.path.getsize(qmFilePath)
        file_size = "%.2f" % (file_size / 1024)

        #compute Qm file hash
        file = open(qmFilePath, 'rb')
        hash_file = hashlib.sha1()
        hash_file.update(file.read())
        file.close()

        if lang_code not in data:
            data[lang_code] = {}
        if prefix not in data[lang_code]:
            data[lang_code][prefix] = {}

        data[lang_code][prefix]["file_name"] = filename + ".qm"
        data[lang_code][prefix]["hash"] = str(hash_file.hexdigest())
        data[lang_code][prefix]["file_size"] = file_size

        return True
    else:
        print(prefix + ' ' + lang_code + " not changed")
        return False


#pull ts files from transifex, use .tx/config
pull = subprocess.Popen(['tx', '-q', 'pull', '-a', '-s'])
pull.communicate()


newDetailsFile = False
translationChanged = False

#read languages.json and store language code and name
langCode_file = open(os.path.dirname(sys.argv[0]) + "/languages.json", "r+")
langCodeNameDict = json.load(langCode_file)  # language code --> name
langCode_file.close()

detailsJson = outputDir + "details.json"
# read details.json or create it
if os.path.isfile(detailsJson):
    json_file = open(outputDir + "details.json", "r+")
    data = json.load(json_file)
    json_file.close()
else:
    newDetailsFile = True
    data = {}
    data["type"] = "Languages"
    data["version"] = "2.0"


translationChanged = newDetailsFile
for lang_code, languageName in langCodeNameDict.items():
    updateMscore = processTsFile("mscore", lang_code, data)
    translationChanged = updateMscore or translationChanged

    updateInstruments = processTsFile("instruments", lang_code, data)
    translationChanged = updateInstruments or translationChanged

    updateTours = processTsFile("tours", lang_code, data)
    translationChanged = updateTours or translationChanged

    if (updateMscore or updateInstruments or updateTours):
        #create a zip file, compute size, hash, add it to json and save to s3
        zipName = 'locale_' + lang_code + '.zip'
        zipPath = outputDir + zipName
        myzip = zipfile.ZipFile(zipPath, mode='w')
        qmFilePath = outputDir + 'mscore_' + lang_code + ".qm"
        myzip.write(qmFilePath, 'mscore_' + lang_code + ".qm")
        qmFilePath = outputDir + 'instruments_' + lang_code + ".qm"
        myzip.write(qmFilePath, 'instruments_' + lang_code + ".qm")
        qmFilePath = outputDir + 'tours_' + lang_code + ".qm"
        myzip.write(qmFilePath, 'tours_' + lang_code + ".qm")
        myzip.close()

        # get zip file size
        file_size = os.path.getsize(zipPath)
        file_size = "%.2f" % (file_size / 1024)

        #compute zip file hash
        file = open(zipPath, 'rb')
        hash_file = hashlib.sha1()
        hash_file.update(file.read())
        file.close()

        data[lang_code]["file_name"] = zipName
        data[lang_code]["name"] = langCodeNameDict[lang_code]
        data[lang_code]["hash"] = str(hash_file.hexdigest())
        data[lang_code]["file_size"] = file_size
        for s3Url in s3Urls:
            push_zip = subprocess.Popen(['s3cmd','put', '--acl-public', '--guess-mime-type', zipPath, s3Url + zipName])
            push_zip.communicate()


json_file = open(outputDir + "details.json", "w")
json_file.write(json.dumps(data, sort_keys=True, indent=4))
json_file.close()

if translationChanged:
    for s3Url in s3Urls:
        push_json = subprocess.Popen(['s3cmd','put','--acl-public', '--guess-mime-type', outputDir + 'details.json', s3Url + 'details.json'])
        push_json.communicate()


