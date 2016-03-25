#ifndef __SEARCHCOMBOBOX__
#define __SEARCHCOMBOBOX__
#include  <QAccessibleWidget>

namespace Ms {

class SearchComboBox : public QComboBox {
      Q_OBJECT
public:
      enum SearchType {
            SEARCH_MEASURE,
            SEARCH_PAGE,
            SEARCH_REHEARSAL_MARK,
            NO_SEARCH
            };
private:
      SearchType _searchType;
      bool _found;
      void searchInit();
      void setSearchType(SearchType s);
private slots:
      void searchTextChanged(const QString& s);
public:
      SearchType searchType() { return _searchType; }
      bool found()            { return _found;      }
      SearchComboBox(QWidget* p = 0);
signals:
      void currentSearchFinished();
};

class AccessibleSearchBox : public  QObject, QAccessibleWidget {
      Q_OBJECT
      SearchComboBox* searchBox;
      //JAWS compatibility - no idea why yet. Adjustments needs to be made to the JawsScript.
      //QAccessible::Role role() const Q_DECL_OVERRIDE { return QAccessible::ComboBox; }
      QString text(QAccessible::Text t) const Q_DECL_OVERRIDE;
public:
      AccessibleSearchBox(SearchComboBox* comboBox);
      static QAccessibleInterface* SearchBoxFactory(const QString &classname, QObject *object);

public slots:
      void searchFinished();
};

}

#endif
