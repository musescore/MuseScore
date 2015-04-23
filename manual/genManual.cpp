//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2015 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <stdio.h>
#include <QString>

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
             "   <link rel=\"stylesheet\" type=\"text/css\" href=\"manual.css\"/>\n"
             "   </head>\n"
             "<body>\n";
      }

//---------------------------------------------------------
//   addFooter
//---------------------------------------------------------

static void addFooter(QString& out)
      {
      out += "<div class=\"footer\"><a href=\"http://musescore.org/\">MuseScore</a> - Free music notation software<br />\n"
             "&copy; 2002-2015 Werner Schweer &amp; others</div>\n"
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

      QRegExp propRegExp("@P ([^\\s]+)\\s+([^\\s]+)(.*)");

      // matches Q_INVOKABLE void mops(int a);   // comment
      QRegExp methodRegExp1("Q_INVOKABLE +([^ ]+) +([^;]+); */*(.*)");
      QRegExp methodRegExp2("Q_INVOKABLE +([^ ]+) +([^\\{]+)\\{");
      QRegExp methodRegExp3("Q_INVOKABLE +([^ ]+) +(\\w+\\([^\\)]*\\))\\s+const\\s*([^\\{]*)\\{");

      QRegExp descrRegExp("//@ (.*)");
      QRegExp classNameRegExp1 ("class +(\\w+) *: *public +(\\w+) *\\{");
      QRegExp classNameRegExp2("class +(\\w+) *: *public +(\\w+), *public");

      Q_ASSERT(methodRegExp1.isValid() && methodRegExp2.isValid() && methodRegExp3.isValid());

      bool parseClassDescription = true;

      foreach(const QString& s, sl) {

            // Properties

            if (propRegExp.indexIn(s, 0) != -1) {             //@P
                  parseClassDescription = false;
                  Prop p;
                  p.name        = propRegExp.cap(1);
                  p.type        = propRegExp.cap(2);
                  p.description = propRegExp.cap(3);
                  cl.props.append(p);
                  }

            // Methods

            else if (methodRegExp2.indexIn(s, 0) != -1) {
                  parseClassDescription = false;
                  // only document a method if it has a "//@" comment
                  if (methodDescription.count() > 0) {
                        Proc p;
                        p.type        = methodRegExp2.cap(1);
                        p.name        = methodRegExp2.cap(2);
                        p.description = methodDescription;
                        cl.procs.append(p);
                        }
                  methodDescription.clear();
                  }
            else if (methodRegExp1.indexIn(s, 0) != -1) {
                  parseClassDescription = false;
                  // only document a method if it has a "//@" comment
                  if (methodDescription.count() > 0) {
                        Proc p;
                        p.type        = methodRegExp1.cap(1);
                        p.name        = methodRegExp1.cap(2);
                        p.description = methodDescription;
                        cl.procs.append(p);
                        }
                  methodDescription.clear();
                  }
            else if (methodRegExp3.indexIn(s, 0) != -1) {
                  parseClassDescription = false;
                  // only document a method if it has a "//@" comment
                  if (methodDescription.count() > 0) {
                        Proc p;
                        p.type        = methodRegExp3.cap(1);
                        p.name        = methodRegExp3.cap(2);
                        p.description = methodDescription;
                        cl.procs.append(p);
                        }
                  methodDescription.clear();
                  }

            // Descriptions

            else if ((descrRegExp.indexIn(s, 0) != -1)) {
                  if (parseClassDescription)
                        cl.description.append(descrRegExp.cap(1));
                  else
                        methodDescription.append(descrRegExp.cap(1));
                  }
            else if (s.startsWith("///")) {
                  QString ss = s.mid(3);
                  if (parseClassDescription)
                        cl.description.append(ss);
//                  else
//                        methodDescription.append(ss);
                  }

            // Class names

            else if (classNameRegExp1.indexIn(s, 0) != -1) {
                  parseClassDescription = false;
                  QString parent = classNameRegExp1.cap(2).simplified();
                  if (name == classNameRegExp1.cap(1).simplified()) {
                        cl.parent = parent;
                        }
                  else {
                        printf("?<%s>!=<%s> derived from <%s>\n",
                           qPrintable(name), qPrintable(classNameRegExp1.cap(1).simplified()), qPrintable(parent));
                        }
                  }
            else if (classNameRegExp2.indexIn(s, 0) != -1) {
                  parseClassDescription = false;
                  QString parent = classNameRegExp2.cap(2).simplified();
                  if (name == classNameRegExp2.cap(1).simplified()) {
                        cl.parent = parent;
                        }
                  else {
                        printf("?<%s>!=<%s> derived from <%s>\n",
                           qPrintable(name), qPrintable(classNameRegExp2.cap(1).simplified()), qPrintable(parent));
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

      QRegExp classRegExp("@@ ([^\\n]*)");
      int gpos = 0;
      QString className;
      for (;;) {
            int rv = classRegExp.indexIn(in, gpos);
            if (rv == -1) {
                  if (!className.isEmpty())
                        parseClass(className, in.mid(gpos, in.size() - gpos));
                  break;
                  }
            int next = rv + classRegExp.matchedLength();
            if (gpos)
                  parseClass(className, in.mid(gpos, next-gpos));

            className = classRegExp.cap(1).simplified();
            gpos = next;
            }
      }

//---------------------------------------------------------
//   writeOutput
//---------------------------------------------------------

static void writeOutput()
      {
      foreach(const Class& cl, classes) {
            QString out;
            addHeader(out);
            out += QString("<h3>%1</h3>\n").arg(cl.name);

            if (!cl.parent.isEmpty()) {
                  // show parent only if its part of the exported classes
                  foreach(const Class& lcl, classes) {
                        if (lcl.name == cl.parent) {
                              QString path = cl.parent.toLower();
                              out += QString("<div class=\"class-inherit\">inherits <a href=\"%1.html\">%2</a></div>\n").arg(path).arg(cl.parent);
                              break;
                              }
                        }
                  }
            if (!cl.description.isEmpty()) {
                  out += "<div class=\"class-description\">\n";
                  foreach(const QString& s, cl.description) {
                        out += s.simplified().replace("\\brief ", "");
                        out += "\n";
                        }
                  out += "</div>\n";
                  }
            else
                  out += "<br />";

            if (!cl.procs.isEmpty()) {
                  out += "<h4>Methods</h4>\n";
                  out += "<div class=\"methods\">\n";
                  foreach(const Proc& p, cl.procs) {
                        out += "<div class=\"method\">";

                        QString type(p.type);
                        bool found = false;
                        if (type.endsWith("*")) {
                              type = type.left(type.size()-1);
                              foreach(const Class& cl, classes) {
                                    if (cl.name == type) {
                                          found = true;
                                          break;
                                          }
                                    }
                              }
                        if (found)
                              out += QString("<a href=\"%1.html\">%2</a> ")
                                 .arg(type.toLower()).arg(type);
                        else
                              out += QString("%1 ").arg(type);

                        QRegExp re("([^(]+)\\(([^)]*)\\)");
                        if (re.indexIn(p.name, 0) != -1) {
                              out += QString("<b>%2</b>(%3)\n") .arg(re.cap(1)).arg(re.cap(2));
                              }
                        else {
                              out += QString("<b>%2</b>\n").arg(p.name);
                              }
                        if (!p.description.isEmpty()) {
                              out += "<div class=\"method-description\">";
                              foreach(const QString& s, p.description) {
                                    out += s.simplified();
                                    out += "<br/>\n";
                                    }
                              out += "</div>";
                              }
                        out += "</div>\n";            // end of .method div
                        }
                  out += "</div>\n";                  // end of .methods div
                  }
            if (!cl.props.isEmpty()) {
                  out += "<h4>Properties</h4>\n";
                  out += "<div class=\"properties\">\n";
                  out += "<table>\n";
                  int count = 1;
                  foreach(const Prop& m, cl.props) {
                        out += QString("<tr class=\"prop-%1\">") .arg( (count & 1) ? "odd" : "even");
                        out += QString("<td class=\"prop-name\">%1</td>"
                               "<td class=\"prop-type\">%2</td>"
                               "<td class=\"prop-desc\">%3</td>")
                               .arg(m.name).arg(m.type).arg(m.description);
                        out += "</tr>\n";
                        count++;
                        }
                  out += "</table></div>\n";
                  }
            addFooter(out);

            QString ofile = dstPath + "/plugins/" + cl.name.toLower() + ".html";
            QFile of(ofile);
            if (!of.open(QIODevice::WriteOnly)) {
                  printf("open <%s> failed: %s\n", qPrintable(ofile), qPrintable(of.errorString()));
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
      out += "<h2>Score Elements</h2>\n"
             "<ul>\n";
      qSort(classes);
      foreach(const Class& s, classes) {
            out += QString("<li><a href=\"%1\">%2</a></li>\n")
                    .arg(s.name.toLower() + ".html").arg(s.name);
            }
      out += "</ul>\n";
      addFooter(out);

      QString ofile = dstPath + "/plugins/plugins.html";
      QFile of(ofile);
      if (!of.open(QIODevice::WriteOnly)) {
            printf("open <%s> failed\n", qPrintable(ofile));
            exit(-4);
            }
      of.write(out.toUtf8());
      of.close();
      }

//---------------------------------------------------------
//   copyAssets
//---------------------------------------------------------

static void copyAssets(QString& srcPath, QString& dstPath)
      {
      QString assetDstPath = dstPath + "/plugins/";
      QString assetSrcPath = srcPath + "/manual/";
      QStringList files = {"manual.css", /*"manual-dark.css",*/ "mscore.png" };

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

      foreach(QString f, fl)
            files << "libmscore/" + f;

      foreach(const QString& s, files) {
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

