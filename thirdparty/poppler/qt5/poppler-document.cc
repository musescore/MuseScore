/* poppler-document.cc: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005, 2008, Brad Hards <bradh@frogmouth.net>
 * Copyright (C) 2005-2010, 2012, 2013, 2015, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2006-2010, Pino Toscano <pino@kde.org>
 * Copyright (C) 2010, 2011 Hib Eris <hib@hiberis.nl>
 * Copyright (C) 2012 Koji Otani <sho@bbr.jp>
 * Copyright (C) 2012, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
 * Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
 * Copyright (C) 2014 Adam Reichold <adamreichold@myopera.com>
 * Copyright (C) 2015 William Bader <williambader@hotmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "poppler-qt5.h"

#include <config.h>
#include <ErrorCodes.h>
#include <GlobalParams.h>
#include <Outline.h>
#include <PDFDoc.h>
#include <Stream.h>
#include <Catalog.h>
#include <ViewerPreferences.h>
#include <DateInfo.h>
#include <GfxState.h>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QByteArray>

#include "poppler-private.h"
#include "poppler-page-private.h"

#if defined(USE_CMS)
#if defined(USE_LCMS1)
#include <lcms.h>
#else
#include <lcms2.h>
#endif
#endif

namespace Poppler {

  int DocumentData::count = 0;

  Document *Document::load(const QString &filePath, const QByteArray &ownerPassword,
			   const QByteArray &userPassword)
    {
	DocumentData *doc = new DocumentData(filePath, 
					     new GooString(ownerPassword.data()),
					     new GooString(userPassword.data()));
	return DocumentData::checkDocument(doc);
    }

    Document *Document::loadFromData(const QByteArray &fileContents,
			      const QByteArray &ownerPassword,
			      const QByteArray &userPassword)
    {
	// create stream
	DocumentData *doc = new DocumentData(fileContents,
					     new GooString(ownerPassword.data()),
					     new GooString(userPassword.data()));
	return DocumentData::checkDocument(doc);
    }
    
    Document *DocumentData::checkDocument(DocumentData *doc)
    {
	Document *pdoc;
	if (doc->doc->isOk() || doc->doc->getErrorCode() == errEncrypted) {
		pdoc = new Document(doc);
		if (doc->doc->getErrorCode() == errEncrypted)
			pdoc->m_doc->locked = true;
		else
		{
			pdoc->m_doc->locked = false;
			pdoc->m_doc->fillMembers();
		}
		return pdoc;
	}
	else
	{
		delete doc;
	}
	return NULL;
    }

    Document::Document(DocumentData *dataA)
    {
	m_doc = dataA;
    }

    Document::~Document()
    {
	delete m_doc;
    }

    Page *Document::page(int index) const
    {
	Page *page = new Page(m_doc, index);
	if (page->m_page->page == NULL) {
	  delete page;
	  return NULL;
	}

	return page;
    }

    bool Document::isLocked() const
    {
	return m_doc->locked;
    }

    bool Document::unlock(const QByteArray &ownerPassword,
			  const QByteArray &userPassword)
    {
	if (m_doc->locked) {
	    /* racier then it needs to be */
	    DocumentData *doc2;
	    if (!m_doc->fileContents.isEmpty())
	    {
		doc2 = new DocumentData(m_doc->fileContents,
					new GooString(ownerPassword.data()),
					new GooString(userPassword.data()));
	    }
	    else
	    {
		doc2 = new DocumentData(m_doc->m_filePath,
					new GooString(ownerPassword.data()),
					new GooString(userPassword.data()));
	    }
	    if (!doc2->doc->isOk()) {
		delete doc2;
	    } else {
		delete m_doc;
		m_doc = doc2;
		m_doc->locked = false;
		m_doc->fillMembers();
	    }
	}
	return m_doc->locked;
    }

    Document::PageMode Document::pageMode() const
    {
	switch (m_doc->doc->getCatalog()->getPageMode()) {
	case Catalog::pageModeNone:
	    return UseNone;
	case Catalog::pageModeOutlines:
	    return UseOutlines;
	case Catalog::pageModeThumbs:
	    return UseThumbs;
	case Catalog::pageModeFullScreen:
	    return FullScreen;
	case Catalog::pageModeOC:
	    return UseOC;
	case Catalog::pageModeAttach:
	    return UseAttach;
	default:
	    return UseNone;
	}
    }

    Document::PageLayout Document::pageLayout() const
    {
	switch (m_doc->doc->getCatalog()->getPageLayout()) {
	case Catalog::pageLayoutNone:
	    return NoLayout;
	case Catalog::pageLayoutSinglePage:
	    return SinglePage;
	case Catalog::pageLayoutOneColumn:
	    return OneColumn;
	case Catalog::pageLayoutTwoColumnLeft:
	    return TwoColumnLeft;
	case Catalog::pageLayoutTwoColumnRight:
	    return TwoColumnRight;
	case Catalog::pageLayoutTwoPageLeft:
	    return TwoPageLeft;
	case Catalog::pageLayoutTwoPageRight:
	    return TwoPageRight;
	default:
	    return NoLayout;
	}
    }

    Qt::LayoutDirection Document::textDirection() const
    {
        if (!m_doc->doc->getCatalog()->getViewerPreferences())
            return Qt::LayoutDirectionAuto;

        switch (m_doc->doc->getCatalog()->getViewerPreferences()->getDirection()) {
        case ViewerPreferences::directionL2R:
            return Qt::LeftToRight;
        case ViewerPreferences::directionR2L:
            return Qt::RightToLeft;
        default:
            return Qt::LayoutDirectionAuto;
        }
    }

    int Document::numPages() const
    {
	return m_doc->doc->getNumPages();
    }

    QList<FontInfo> Document::fonts() const
    {
	QList<FontInfo> ourList;
	FontIterator it( 0, m_doc );
	while ( it.hasNext() )
	{
		ourList += it.next();
	}
	return ourList;
    }

    QList<EmbeddedFile*> Document::embeddedFiles() const
    {
	return m_doc->m_embeddedFiles;
    }

    FontIterator* Document::newFontIterator( int startPage ) const
    {
	return new FontIterator( startPage, m_doc );
    }

    QByteArray Document::fontData(const FontInfo &fi) const
    {
	QByteArray result;
	if (fi.isEmbedded())
	{
		Object refObj, strObj;
		XRef *xref = m_doc->doc->getXRef()->copy();

		refObj.initRef(fi.m_data->embRef.num, fi.m_data->embRef.gen);
		refObj.fetch(xref, &strObj);
		refObj.free();
		if (strObj.isStream())
		{
			int c;
			strObj.streamReset();
			while ((c = strObj.streamGetChar()) != EOF)
			{
				result.append((char)c);
			}
			strObj.streamClose();
		}
		strObj.free();
		delete xref;
	}
	return result;
    }

    /* borrowed from kpdf */
    QString Document::info( const QString & type ) const
    {
	// [Albert] Code adapted from pdfinfo.cc on xpdf
	Object info;
	if ( m_doc->locked )
	    return QString();

	QScopedPointer<XRef> xref(m_doc->doc->getXRef()->copy());
	if (!xref)
		return QString();
	xref->getDocInfo(&info);
	if ( !info.isDict() )
	    return QString();

	QString result;
	Object obj;
	GooString *s1;
	Dict *infoDict = info.getDict();

	if ( infoDict->lookup( type.toLatin1().data(), &obj )->isString() )
	{
	    s1 = obj.getString();
	    result = UnicodeParsedString(s1);
	    obj.free();
	    info.free();
	    return result;
	}
	obj.free();
	info.free();
	return QString();
    }

    QStringList Document::infoKeys() const
    {
	QStringList keys;

	Object info;
	if ( m_doc->locked )
	    return QStringList();

	QScopedPointer<XRef> xref(m_doc->doc->getXRef()->copy());
	if (!xref)
		return QStringList();
	xref->getDocInfo(&info);
	if ( !info.isDict() )
	    return QStringList();

	Dict *infoDict = info.getDict();
	// somehow iterate over keys in infoDict
	keys.reserve(infoDict->getLength());
	for( int i=0; i < infoDict->getLength(); ++i ) {
	    keys.append( QString::fromLatin1(infoDict->getKey(i)) );
	}

	info.free();
	return keys;
    }

    /* borrowed from kpdf */
    QDateTime Document::date( const QString & type ) const
    {
	// [Albert] Code adapted from pdfinfo.cc on xpdf
	if ( m_doc->locked )
	    return QDateTime();

	Object info;
	QScopedPointer<XRef> xref(m_doc->doc->getXRef()->copy());
	if (!xref)
		return QDateTime();
	xref->getDocInfo(&info);
	if ( !info.isDict() ) {
	    info.free();
	    return QDateTime();
	}

	Object obj;
	Dict *infoDict = info.getDict();
	QDateTime result;

	if ( infoDict->lookup( type.toLatin1().data(), &obj )->isString() )
	{
	    char *aux = obj.getString()->getCString();
	    result = Poppler::convertDate(aux);
	}
	obj.free();
	info.free();
	return result;
    }

    bool Document::isEncrypted() const
    {
	return m_doc->doc->isEncrypted();
    }

    bool Document::isLinearized() const
    {
	return m_doc->doc->isLinearized();
    }

    bool Document::okToPrint() const
    {
	return m_doc->doc->okToPrint();
    }

    bool Document::okToPrintHighRes() const
    {
	return m_doc->doc->okToPrintHighRes();
    }

    bool Document::okToChange() const
    {
	return m_doc->doc->okToChange();
    }

    bool Document::okToCopy() const
    {
	return m_doc->doc->okToCopy();
    }

    bool Document::okToAddNotes() const
    {
	return m_doc->doc->okToAddNotes();
    }

    bool Document::okToFillForm() const
    {
	return m_doc->doc->okToFillForm();
    }

    bool Document::okToCreateFormFields() const
    {
	return ( okToFillForm() && okToChange() );
    }

    bool Document::okToExtractForAccessibility() const
    {
	return m_doc->doc->okToAccessibility();
    }

    bool Document::okToAssemble() const
    {
	return m_doc->doc->okToAssemble();
    }

    void Document::getPdfVersion(int *major, int *minor) const
    {
	if (major)
	    *major = m_doc->doc->getPDFMajorVersion();
	if (minor)
	    *minor = m_doc->doc->getPDFMinorVersion();
    }

    Page *Document::page(const QString &label) const
    {
	GooString label_g(label.toLatin1().data());
	int index;

	if (!m_doc->doc->getCatalog()->labelToIndex (&label_g, &index))
	    return NULL;

	return page(index);
    }

    bool Document::hasEmbeddedFiles() const
    {
	return (!(0 == m_doc->doc->getCatalog()->numEmbeddedFiles()));
    }
    
    QDomDocument *Document::toc() const
    {
        Outline * outline = m_doc->doc->getOutline();
        if ( !outline )
            return NULL;

        GooList * items = outline->getItems();
        if ( !items || items->getLength() < 1 )
            return NULL;

        QDomDocument *toc = new QDomDocument();
        if ( items->getLength() > 0 )
           m_doc->addTocChildren( toc, toc, items );

        return toc;
    }

    LinkDestination *Document::linkDestination( const QString &name )
    {
        GooString * namedDest = QStringToGooString( name );
        LinkDestinationData ldd(NULL, namedDest, m_doc, false);
        LinkDestination *ld = new LinkDestination(ldd);
        delete namedDest;
        return ld;
    }
    
    void Document::setPaperColor(const QColor &color)
    {
        m_doc->setPaperColor(color);
    }
    
    void Document::setColorDisplayProfile(void* outputProfileA)
    {
#if defined(USE_CMS)
        GfxColorSpace::setDisplayProfile((cmsHPROFILE)outputProfileA);
#else
        Q_UNUSED(outputProfileA);
#endif
    }

    void Document::setColorDisplayProfileName(const QString &name)
    {
#if defined(USE_CMS)
        GooString *profileName = QStringToGooString( name );
        GfxColorSpace::setDisplayProfileName(profileName);
        delete profileName;
#else
        Q_UNUSED(name);
#endif
    }

    void* Document::colorRgbProfile() const
    {
#if defined(USE_CMS)
        return (void*)GfxColorSpace::getRGBProfile();
#else
        return NULL;
#endif
    }

    void* Document::colorDisplayProfile() const
    {
#if defined(USE_CMS)
       return (void*)GfxColorSpace::getDisplayProfile();
#else
       return NULL;
#endif
    }

    QColor Document::paperColor() const
    {
    	return m_doc->paperColor;
    }

    void Document::setRenderBackend( Document::RenderBackend backend )
    {
        // no need to delete the outputdev as for the moment we always create a splash one
        // as the arthur one does not allow "precaching" due to it's signature
        // delete m_doc->m_outputDev;
        // m_doc->m_outputDev = NULL;
        m_doc->m_backend = backend;
    }

    Document::RenderBackend Document::renderBackend() const
    {
        return m_doc->m_backend;
    }

    QSet<Document::RenderBackend> Document::availableRenderBackends()
    {
        QSet<Document::RenderBackend> ret;
#if defined(HAVE_SPLASH)
        ret << Document::SplashBackend;
#endif
        ret << Document::ArthurBackend;
        return ret;
    }

    void Document::setRenderHint( Document::RenderHint hint, bool on )
    {
        const bool touchesOverprinting = hint & Document::OverprintPreview;
        
        int hintForOperation = hint;
        if (touchesOverprinting && !isOverprintPreviewAvailable())
            hintForOperation = hintForOperation & ~(int)Document::OverprintPreview;

        if ( on )
            m_doc->m_hints |= hintForOperation;
        else
            m_doc->m_hints &= ~hintForOperation;

    }

    Document::RenderHints Document::renderHints() const
    {
        return Document::RenderHints( m_doc->m_hints );
    }

    PSConverter *Document::psConverter() const
    {
        return new PSConverter(m_doc);
    }

    PDFConverter *Document::pdfConverter() const
    {
        return new PDFConverter(m_doc);
    }

    QString Document::metadata() const
    {
        QString result;
        Catalog *catalog = m_doc->doc->getCatalog();
        if (catalog && catalog->isOk())
        {
            GooString *s = catalog->readMetadata();
            if (s) result = UnicodeParsedString(s);
            delete s;
        }
        return result;
    }

    bool Document::hasOptionalContent() const
    {
        return ( m_doc->doc->getOptContentConfig() && m_doc->doc->getOptContentConfig()->hasOCGs() );
    }

    OptContentModel *Document::optionalContentModel()
    {
        if (m_doc->m_optContentModel.isNull()) {
	    m_doc->m_optContentModel = new OptContentModel(m_doc->doc->getOptContentConfig(), 0);
	}
        return (OptContentModel *)m_doc->m_optContentModel;
    }

    QStringList Document::scripts() const
    {
        Catalog *catalog = m_doc->doc->getCatalog();
        const int numScripts = catalog->numJS();
        QStringList scripts;
        for (int i = 0; i < numScripts; ++i) {
            GooString *s = catalog->getJS(i);
            if (s) {
                scripts.append(UnicodeParsedString(s));
                delete s;
            }
        }
        return scripts;
    }

    bool Document::getPdfId(QByteArray *permanentId, QByteArray *updateId) const
    {
        GooString gooPermanentId;
        GooString gooUpdateId;

        if (!m_doc->doc->getID(permanentId ? &gooPermanentId : 0, updateId ? &gooUpdateId : 0))
            return false;

        if (permanentId)
            *permanentId = gooPermanentId.getCString();
        if (updateId)
            *updateId = gooUpdateId.getCString();

        return true;
    }

    Document::FormType Document::formType() const
    {
        switch ( m_doc->doc->getCatalog()->getFormType() )
        {
            case Catalog::NoForm:
                return Document::NoForm;
            case Catalog::AcroForm:
                return Document::AcroForm;
            case Catalog::XfaForm:
                return Document::XfaForm;
        }

        return Document::NoForm; // make gcc happy
    }

    QDateTime convertDate( char *dateString )
    {
        int year, mon, day, hour, min, sec, tzHours, tzMins;
        char tz;

        if ( parseDateString( dateString, &year, &mon, &day, &hour, &min, &sec, &tz, &tzHours, &tzMins ) )
        {
            QDate d( year, mon, day );
            QTime t( hour, min, sec );
            if ( d.isValid() && t.isValid() ) {
                QDateTime dt( d, t, Qt::UTC );
                if ( tz ) {
                    // then we have some form of timezone
                    if ( 'Z' == tz  ) {
                        // We are already at UTC
                    } else if ( '+' == tz ) {
                        // local time is ahead of UTC
                        dt = dt.addSecs(-1*((tzHours*60)+tzMins)*60);
                    } else if ( '-' == tz ) {
                        // local time is behind UTC
                        dt = dt.addSecs(((tzHours*60)+tzMins)*60);
                    } else {
                        qWarning("unexpected tz val");
                    }
                }
		return dt;
            }
        }
        return QDateTime();
    }

    bool isCmsAvailable()
    {
#if defined(USE_CMS)
        return true;
#else
        return false;
#endif
    }

    bool isOverprintPreviewAvailable() {
#if SPLASH_CMYK
        return true;
#else
        return false;
#endif
   }

}

