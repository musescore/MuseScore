#include "searchComboBox.h"
#include "musescore.h"
#include "scoreview.h"
#include "libmscore/score.h"
#include "scoreaccessibility.h"

namespace Ms {

SearchComboBox::SearchComboBox(QWidget* p) : QComboBox(p)
      {
      setAccessibleName(tr("Search Box"));
      setAccessibleDescription(tr("Type to search. Press Enter to return to score."));
      setEditable(true);
      setInsertPolicy(QComboBox::InsertAtTop);
      _found = false;
      _searchType = SearchType::NO_SEARCH;
      connect(this, SIGNAL(editTextChanged(QString)), this, SLOT(searchTextChanged(QString)));
      }

void SearchComboBox::searchInit()
      {
      _searchType = SearchType::NO_SEARCH;
      _found = false;
      }

void SearchComboBox::setSearchType(SearchType s)
      {
      _searchType = s;
      }

void SearchComboBox::searchTextChanged(const QString& s)
      {
      searchInit();
      if (s.isEmpty())
            return;
      ScoreView* cv = mscore->currentScoreView();
      if (cv == 0)
            return;

      bool ok;

      int n = s.toInt(&ok);
      if (ok && n > 0) {
            setSearchType(SearchType::SEARCH_MEASURE);
            _found = cv->searchMeasure(n);
            }
      else {
            if (s.size() >= 2 && s[0].toLower() == 'p' && s[1].isNumber()) {
                  n = s.midRef(1).toInt(&ok);
                  if (ok) {
                        setSearchType(SearchType::SEARCH_PAGE);
                        _found = cv->searchPage(n);
                        }
                  }

            if (searchType() != SearchType::SEARCH_PAGE) {
                  setSearchType(SearchType::SEARCH_REHEARSAL_MARK);
                  if (s.size() >= 2 && s[0].toLower() == 'r' && s[1].isNumber())
                        _found = cv->searchRehearsalMark(s.mid(1));
                  else
                        _found = cv->searchRehearsalMark(s);
                  }
            }
      //updating status bar
      ScoreAccessibility::instance()->updateAccessibilityInfo();
      emit currentSearchFinished();
      }

AccessibleSearchBox::AccessibleSearchBox(SearchComboBox *comboBox) : QAccessibleWidget(comboBox)
      {
      searchBox = comboBox;
      }

QAccessibleInterface* AccessibleSearchBox::SearchBoxFactory(const QString &classname, QObject *object)
      {
          QAccessibleInterface *iface = 0;
          if (classname == QLatin1String("Ms::SearchComboBox") && object && object->isWidgetType()){
                qDebug("Creating interface for SearchComboBox object");
                SearchComboBox* s = static_cast<SearchComboBox*>(object);
                AccessibleSearchBox* a = new AccessibleSearchBox(s);
                QObject::connect(s, SIGNAL(currentSearchFinished()), a, SLOT(searchFinished()));
                iface = static_cast<QAccessibleInterface*>(a);
                }

          return iface;
      }

QString AccessibleSearchBox::text(QAccessible::Text t) const
      {
      QString type;
      QString value = searchBox->currentText();
      if (t == QAccessible::Value) {
            switch (searchBox->searchType()) {
                  case SearchComboBox::SearchType::NO_SEARCH:
                        return QString();
                  case SearchComboBox::SearchType::SEARCH_MEASURE:
                        type = tr("Measure");
                        break;
                  case SearchComboBox::SearchType::SEARCH_PAGE:
                        type = tr("Page");
                        value = value.mid(1);
                        break;
                  case SearchComboBox::SearchType::SEARCH_REHEARSAL_MARK:
                        type = tr("Rehearsal Mark");
                        break;
                  }
            QString found = searchBox->found() ? "" : tr("Not found") + " ";
            return QString("%1 %2 %3%4").arg(type, value, found, mscore->currentScoreView()->score()->accessibleInfo());
            }

      return QAccessibleWidget::text(t);
      }

void AccessibleSearchBox::searchFinished()
      {
      QAccessibleValueChangeEvent ev(searchBox, text(QAccessible::Value));
      QAccessible::updateAccessibility(&ev);
      }

}
