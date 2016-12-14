# This Python file uses the following encoding: utf-8
import sys
import os
from datetime import datetime
import time
from subprocess import check_output


def getFileList(sshKey):
    a = ["ssh", "-i", sshKey, "musescore-nightlies@ftp-osl.osuosl.org", "ls ftp/macosx/"]
    return check_output(a)


def generateHTML(pathname, lines):
    li = ""
    for l in lines:
        if l and l.startswith("MuseScore"):
            li += '<li><a href="http://ftp.osuosl.org/pub/musescore-nightlies/macosx/' + l + '">' + l + "</a></li>"

    htmlTplPath = pathname + "/web/index.html.tpl"
    htmlPath = pathname + "/web/index.html"

    htmlTplFile = open(htmlTplPath, 'r')
    htmlTemplate = htmlTplFile.read()
    htmlTemplate = htmlTemplate.replace("##PLACEHOLDER##", li)
    htmlTplFile.close()
    htmlFile = open(htmlPath, "w")
    htmlFile.write(htmlTemplate)
    htmlFile.close()


def generateRSS(pathname, lines):
    li = ""
    for l in lines:
        if l and l.startswith("MuseScore"):
            info = l.split("-")
            if len(info) == 4:
                date = info[1]
                branch = info[2]
                commit = info[3]
            elif len(info) == 7:
                year = info[1]
                month = info[2]
                day = info[3]
                hour = info[4]
                date = year + month + day + hour
                branch = info[5]
                commit = info[6]
            else:
                continue
            d = datetime.strptime(date, '%Y%m%d%H%M')
            from email.utils import formatdate
            li += '''
<item>
<title>''' + l + '''</title>
<link>http://ftp.osuosl.org/pub/musescore-nightlies/macosx/''' + l + '''</link>
<pubDate>'''+formatdate(time.mktime(d.timetuple()), usegmt=True)+'''</pubDate>
<description>Nightly Build of MuseScore, Branch: ''' + branch + ''', Revision: '''+commit + '''</description>
</item>'''

    rssTplPath = pathname + "/web/nightly.xml.tpl"
    rssPath = pathname + "/web/nightly.xml"

    rssTplFile = open(rssTplPath, 'r')
    rssTemplate = rssTplFile.read()
    rssTemplate = rssTemplate.replace("##PLACEHOLDER##", li)
    rssTplFile.close()
    rssFile = open(rssPath, "w")
    rssFile.write(rssTemplate)
    rssFile.close()


def main():
    try:
        sshKey = ""
        # Parse command line variable
        if len(sys.argv) == 2:
            sshKey = sys.argv[1].lower()
        else:
            print "Need SSH key"
            exit(0)

        pathname = os.path.abspath(os.path.dirname(sys.argv[0]))
        print('path =', pathname)
        fileList = getFileList(sshKey)
        l = fileList.split("\n")
        l.reverse()    # most recent first
        generateHTML(pathname, l)
        generateRSS(pathname, l)

    except:
        print "Unexpected error:", sys.exc_info()[0], "\n"
        print sys.exc_info()[1], "\n"
        import traceback
        print traceback.print_tb(sys.exc_info()[2]), "\n"

if __name__ == '__main__':
    main()
