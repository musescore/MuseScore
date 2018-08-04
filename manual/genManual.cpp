//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <stdio.h>
#include <QString>
// Load replacement for getopt() for VS
#if (defined (_MSCVER) || defined (_MSC_VER))
   #define STATIC_GETOPT 1
   #undef _UNICODE
   #include "getopt/getopt.h"
#endif


QString srcPath;
QString dstPath;

//---------------------------------------------------------
//   Prop
//---------------------------------------------------------

struct Prop {
      QString name;
      QString type;
      QString description;
      };

//---------------------------------------------------------
//   Proc
//---------------------------------------------------------

struct Proc {
      QString name;
      QString type;
      QStringList description;
      };

//---------------------------------------------------------
//   Class
//---------------------------------------------------------

struct Class {
      QString name;
      QStringList description;
      QString parent;

      QList<Prop> props;
      QList<Proc> procs;

      bool operator<(const Class& c) const {
            return name < c.name;
            }
      };


static QList<Class> classes;

//---------------------------------------------------------
//   addHeader
//---------------------------------------------------------

static void addHeader(QString& out)
      {
      out += "<!DOCTYPE html>\n"
             "<html>\n"
             "<head>\n"
             "   <meta charset=\"utf-8\">\n"
             "   </head>\n"
             "<body>\n";
      }

//---------------------------------------------------------
//   addFooter
//---------------------------------------------------------

static void addFooter(QString& out)
      {
      out += /* "<div class=\"footer\"><a href=\"https://musescore.org/\">MuseScore</a> - Free music notation software<br/>\n"
             "&copy; 2002-2016 Werner Schweer &amp; others</div>\n" */
             "</body>\n"
             "</html>\n";
      }

//---------------------------------------------------------
//   parseClass
//---------------------------------------------------------

static void parseClass(const QString& name, const QString& in)
      {
      Class cl;
      cl.name = name;

      QStringList sl = in.split("\n");
      QStringList methodDescription;

      QRegExp re("@P ([^\\s]+)\\s+([^\\s]+)(.*)");

      // matches Q_INVOKABLE void mops(int a);   // comment
      QRegExp re1("Q_INVOKABLE +([^ ]+) +([^;]+); */*(.*)");
      QRegExp re2("Q_INVOKABLE +([^ ]+) +([^\\{]+)\\{");
      QRegExp re3("Q_INVOKABLE +([^ ]+) +(\\w+\\([^\\)]*\\))\\s+const\\s*([^\\{]*)\\{");

      QRegExp reD("//@ (.*)");
      QRegExp re4 ("class +(\\w+) *(?:final)* *: *public +(\\w+) *\\{");
      QRegExp re4b("class +(\\w+) *(?:final)* *: *public +(\\w+), *public");

      Q_ASSERT(re1.isValid() && re2.isValid() && re3.isValid());

      bool parseClassDescription = true;

      for(const QString& s : sl) {
            if (re.indexIn(s, 0) != -1) {             //@P
                  parseClassDescription = false;
                  Prop p;
                  p.name        = re.cap(1);
                  p.type        = re.cap(2);
                  p.description = re.cap(3);
                  cl.props.append(p);
                  }
            else if (re2.indexIn(s, 0) != -1) {
                  parseClassDescription = false;
                  Proc p;
                  p.type        = re2.cap(1);
                  p.name        = re2.cap(2);
                  p.description = methodDescription;
                  methodDescription.clear();
                  cl.procs.append(p);
                  }
            else if (re1.indexIn(s, 0) != -1) {
                  parseClassDescription = false;
                  Proc p;
                  p.type        = re1.cap(1);
                  p.name        = re1.cap(2);
                  p.description = methodDescription;
                  methodDescription.clear();
                  cl.procs.append(p);
                  }
            else if (re3.indexIn(s, 0) != -1) {
                  parseClassDescription = false;
                  Proc p;
                  p.type        = re3.cap(1);
                  p.name        = re3.cap(2);
                  p.description = methodDescription;
                  methodDescription.clear();
                  cl.procs.append(p);
                  }
            else if ((reD.indexIn(s, 0) != -1)) {
                  if (parseClassDescription)
                        cl.description.append(reD.cap(1));
                  else
                        methodDescription.append(reD.cap(1));
                  }
            else if (s.startsWith("///")) {
                  QString ss = s.mid(3);
                  if (parseClassDescription)
                        cl.description.append(ss);
                  else
                        methodDescription.append(ss);
                  }
            else if (re4.indexIn(s, 0) != -1) {
                  parseClassDescription = false;
                  QString parent = re4.cap(2).simplified();
                  if (name == re4.cap(1).simplified()) {
                        cl.parent = parent;
                        }
                  else {
                        printf("?<%s>!=<%s> derived from <%s>\n",
                           qPrintable(name), qPrintable(re4.cap(1).simplified()), qPrintable(parent));
                        }
                  }
            else if (re4b.indexIn(s, 0) != -1) {
                  parseClassDescription = false;
                  QString parent = re4b.cap(2).simplified();
                  if (name == re4b.cap(1).simplified()) {
                        cl.parent = parent;
                        }
                  else {
                        printf("?<%s>!=<%s> derived from <%s>\n",
                           qPrintable(name), qPrintable(re4b.cap(1).simplified()), qPrintable(parent));
                        }
                  }
            }
      classes.append(cl);
      }

//---------------------------------------------------------
//   scanFile
//---------------------------------------------------------

static void scanFile(const QString& in)
      {
      QList<Prop> props;
      QList<Proc> procs;

      QRegExp re("@@ ([^\\n]*)");
      int gpos = 0;
      QString className;
      for (;;) {
            int rv = re.indexIn(in, gpos);
            if (rv == -1) {
                  if (!className.isEmpty())
                        parseClass(className, in.mid(gpos, in.size() - gpos));
                  break;
                  }
            int next = rv + re.matchedLength();
            if (gpos)
                  parseClass(className, in.mid(gpos, next-gpos));

            className = re.cap(1).simplified();
            gpos = next;
            }
      }

//---------------------------------------------------------
//   linkClass
//
//   Given something like "array[Note]", will return "array[<a href="note.html">Note</a>]"
//---------------------------------------------------------
static QRegExp reClasses("");

static QString linkClass(const QString& in)
      {
      if (reClasses.pattern().isEmpty()) {
            QStringList classNames;
            foreach(const Class& cl, classes)
                  classNames.append(cl.name);

            reClasses.setPattern("\\b(" + classNames.join('|') + ")\\b");
            Q_ASSERT(reClasses.isValid());
            }

      int pos = reClasses.indexIn(in);
      if (pos != -1) {
            QString out(in);
            out.insert(pos + reClasses.matchedLength(), "</a>");
            out.insert(pos, "<a href=\"" + in.mid(pos, reClasses.matchedLength()).toLower() + ".html\">");

            return out;
            }
      return in;
      }

//---------------------------------------------------------
//   writeOutput
//---------------------------------------------------------

static void writeOutput()
      {
      for(const Class& cl : classes) {
            QString out;
            addHeader(out);
            out += QString("<h3>%1</h3>\n").arg(cl.name);

            if (!cl.parent.isEmpty()) {
                  // show parent only if its part of the exported classes
                  for(const Class& lcl : classes) {
                        if (lcl.name == cl.parent) {
                              QString path = cl.parent.toLower();
                              out += QString("<div class=\"class-inherit\">inherits <a href=\"%1.html\">%2</a></div>\n").arg(path).arg(cl.parent);
                              break;
                              }
                        }
                  }
            if (!cl.description.isEmpty()) {
                  out += "<div class=\"class-description\">\n";
                  for(const QString& s : cl.description) {
                        out += s.simplified().replace("\\brief ", "");
                        out += "\n";
                        }
                  out += "</div>\n";
                  }
            else
                  out += "<br/>";

            if (!cl.procs.isEmpty()) {
                  out += "<h4>Methods</h4>\n";
                  out += "<div class=\"methods\">\n";
                  for(const Proc& p : cl.procs) {
                        out += "<div class=\"method\">\n";
                        out += linkClass(p.type) + " ";

                        QRegExp re("([^(]+)\\(([^)]*)\\)");
                        if (re.indexIn(p.name, 0) != -1) {
                              out += QString("<b>%2</b>(%3)\n") .arg(re.cap(1)).arg(linkClass(re.cap(2)));
                              }
                        else {
                              out += QString("<b>%2</b>\n").arg(p.name);
                              }
                        out += "</div>\n";
                        if (!p.description.isEmpty()) {
                              out += "<div class=\"method-description\">\n";
                              for(const QString& s : p.description) {
                                    out += s.simplified();
                                    out += "<br/>\n";
                                    }
                              out += "</div>\n";
                              }
                        }
                  out += "</div>\n";
                  }
            if (!cl.props.isEmpty()) {
                  out += "<h4>Properties</h4>\n";
                  out += "<div class=\"properties\">\n";
                  out += "<table>\n";
                  int count = 1;
                  for(const Prop& m : cl.props) {
                        out += QString("<tr class=\"prop-%1\">") .arg( (count & 1) ? "odd" : "even");
                        out += QString("<td class=\"prop-name\">%1</td>"
                               "<td class=\"prop-type\">%2</td>"
                               "<td class=\"prop-desc\">%3</td>")
                               .arg(m.name).arg(linkClass(m.type)).arg(m.description);
                        out += "</tr>\n";
                        count++;
                        }
                  out += "</table></div>\n";
                  }
            addFooter(out);

            QString ofile = dstPath + "/plugins/" + cl.name.toLower() + ".html";
            QFile of(ofile);
            if (!of.open(QIODevice::WriteOnly)) {
                  fprintf(stderr, "open <%s> failed: %s\n", qPrintable(ofile), qPrintable(of.errorString()));
                  exit(-4);
                  }
            of.write(out.toUtf8());
            of.close();
            }

      //
      //  write index
      //
      QString out;
      addHeader(out);
      out += "<h2>Score Elements</h2>\n";
      out += "<h3>Quick Guide</h3>\n";
      out += "<p>Below are all the various classes you can use."
             "<br>The main class is <a href='musescore.html'>MuseScore</a>."
             "<br>Use 'New' to create a skeleton plugin."
             "<br>Plugins are coded in <a href='http://doc.qt.io/qt-5/qmlapplications.html#what-is-qml'>QML</a>"
             "</p>\n";
      out += "<ul>\n";
      qSort(classes);
      for(const Class& s : classes) {
            out += QString("<li><a href=\"%1\">%2</a></li>\n")
                    .arg(s.name.toLower() + ".html").arg(s.name);
            }
      out += "</ul>\n";
      addFooter(out);

      QString ofile = dstPath + "/plugins/plugins.html";
      QFile of(ofile);
      if (!of.open(QIODevice::WriteOnly)) {
            fprintf(stderr, "open <%s> failed\n", qPrintable(ofile));
            exit(-4);
            }
      of.write(out.toUtf8());
      of.close();
      }

//---------------------------------------------------------
//   copyAssets
//---------------------------------------------------------

static void copyAssets(QString& lSrcPath, QString& lDstPath)
      {
      QString assetDstPath = lDstPath + "/plugins/";
      QString assetSrcPath = lSrcPath + "/manual/";
//      QStringList files = {"manual.css", "manual-dark.css", "mscore.png" };
      QStringList files = {"mscore.png" };

      // copy files from source to destination path
      for (QString f : files) {
            // be sure destination files do not exist
            QFile dst(assetDstPath + f);
            dst.remove();

            if (!QFile::copy(assetSrcPath + f, assetDstPath + f))
                  fprintf(stderr, "Cannot copy %s to %s\n",
                     qPrintable(assetSrcPath + f),
                     qPrintable(assetDstPath + f));
            }
      }

//---------------------------------------------------------
//   usage
//---------------------------------------------------------

static void usage(const char* program, const char* hint)
      {
      fprintf(stderr, "%s: %s\n", program, hint);
      fprintf(stderr, "usage: %s [options] srcPath dstPath\n", program);
      fprintf(stderr, "options: -v        print version\n"
            );
      }

//---------------------------------------------------------
//   printVersion
//---------------------------------------------------------

static void printVersion(const char* program)
      {
      printf("this is %s, version 0.1\n", program);
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      char* prog = argv[0];
      int c;

      while ((c = getopt(argc, argv, "v")) != EOF) {
            switch (c) {
                  case 'v':
                        printVersion(argv[0]);
                        return 0;
                  default:
                        usage(prog, "bad argument");
                        return -1;
                  }
            }
      argc -= optind;
      argv += optind;
      if (argc != 2) {
            usage(prog, "bad arguments");
            return -1;
            }
      srcPath = argv[0];
      dstPath = argv[1];
      QStringList files;
      files << "mscore/qmlplugin.h";
      files << "mscore/svggenerator.h";

      QDir libdir(srcPath + "/libmscore");
      QStringList filter;
      filter << "*.h";
      QStringList fl = libdir.entryList(filter, QDir::Files);

      for(QString f : fl)
            files << "libmscore/" + f;

      for(const QString& s : files) {
            QString infile = srcPath + "/" + s;
            QFile inFile(infile);
            if (!inFile.open(QIODevice::ReadOnly)) {
                  fprintf(stderr, "%s: cannot open input file <%s>\n",
                     argv[0], qPrintable(infile));
                  return -2;
                  }
            printf("ScanFile %s\n", qPrintable(infile));
            QString in = inFile.readAll();
            scanFile(in);
            inFile.close();
            }
      QDir dir;
      QString opath = dstPath + "/plugins";
      if (!dir.mkpath(opath)) {
            fprintf(stderr, "%s: cannot create destination path: <%s>\n",
               argv[0], qPrintable(dstPath));
            return -3;
            }
      writeOutput();
      copyAssets(srcPath, dstPath);
      return 0;
      }

