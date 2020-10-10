//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

/*
 * HOW TO ADD A NEW PREFERENCE
 * - Add a new define to the list of defines below
 * - Add the preference to the _allPreferences map in the init() function in preferences.cpp
 *   and specify the default value for this preference
 * - That's it. The preference is stored and retrieved automatically and can be read
 *   using getString(), getInt(), etc., and changed using setPreference()
 */

#include <functional>
#include "globals.h"
#include "global/settings/types/preferencekeys.h"

namespace Ms {

extern QString mscoreGlobalShare;

enum class SessionStart : char {
      EMPTY, LAST, NEW, SCORE
      };

// midi remote control values:
enum {
      RMIDI_REWIND,
      RMIDI_TOGGLE_PLAY,
      RMIDI_PLAY,
      RMIDI_STOP,
      RMIDI_NOTE1,
      RMIDI_NOTE2,
      RMIDI_NOTE4,
      RMIDI_NOTE8,
      RMIDI_NOTE16,
      RMIDI_NOTE32,
      RMIDI_NOTE64,
      RMIDI_REST,
      RMIDI_DOT,
      RMIDI_DOTDOT,
      RMIDI_TIE,
      RMIDI_UNDO,
      RMIDI_NOTE_EDIT_MODE,
      RMIDI_REALTIME_ADVANCE,
      MIDI_REMOTES
      };

// The "theme" the user chooses in Preferences
enum class MuseScorePreferredStyleType : char {
      LIGHT_FUSION = 0,
      DARK_FUSION,
#ifdef Q_OS_MAC
      FOLLOW_SYSTEM,
#endif
      };

// The actual "theme", resulting from the user's choice
enum class MuseScoreEffectiveStyleType : char {
      LIGHT_FUSION = 0,
      DARK_FUSION
      };

// MusicXML export break values
enum class MusicxmlExportBreaks : char {
      ALL, MANUAL, NO
      };

// Default-zoom-type options
enum class ZoomType : int {
      PERCENTAGE = 0, PAGE_WIDTH, WHOLE_PAGE, TWO_PAGES,
      };

class PreferenceVisitor;

//---------------------------------------------------------
//   Preference
//---------------------------------------------------------
class Preference {
   private:
      QVariant _defaultValue = 0;
      bool _showInAdvancedList = true;

   protected:
      QMetaType::Type _type = QMetaType::UnknownType;
      Preference(QVariant defaultValue) : _defaultValue(defaultValue) {}

   public:
      Preference(QVariant defaultValue, QMetaType::Type type, bool showInAdvancedList = true);
      virtual ~Preference() {}

      QVariant defaultValue() const {return _defaultValue;}
      bool showInAdvancedList() const {return _showInAdvancedList;}
      QMetaType::Type type() {return _type;}
      virtual void accept(QString key, PreferenceVisitor&) = 0;
      };

class IntPreference : public Preference {
   public:
      IntPreference(int defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString key, PreferenceVisitor&);
      };

class DoublePreference : public Preference {
   public:
      DoublePreference(double defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString key, PreferenceVisitor&);
      };

class BoolPreference : public Preference {
   public:
      BoolPreference(bool defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString key, PreferenceVisitor&);
      };

class StringPreference: public Preference {
   public:
      StringPreference(QString defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString key, PreferenceVisitor&);
      };

class ColorPreference: public Preference {
   public:
      ColorPreference(QColor defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString key, PreferenceVisitor&);
      };

// Support for EnumPreference is currently not fully implemented
class EnumPreference: public Preference {
   public:
      EnumPreference(QVariant defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString, PreferenceVisitor&);
      };

//---------------------------------------------------------
//   Preferences
//---------------------------------------------------------

class Preferences {
   public:
      typedef QHash<QString, Preference*> prefs_map_t;
      using OnSetListener = std::function<void(const QString& key, const QVariant& value)>;
      using ListenerID = uint32_t;

   private:

      // Map of all preferences and their default values
      // A preference can not be read or set if it is not present in this map
      // This map is not used for storing a preference it is only for default values
      prefs_map_t _allPreferences;
      // used for storing preferences in memory when _storeInMemoryOnly is true
      // and for storing temporary preferences
      QHash<QString, QVariant> _inMemorySettings;
      bool _storeInMemoryOnly = false;
      bool _returnDefaultValues = false;
      bool _initialized = false;
      QSettings* _settings; // should not be used directly but through settings() accessor

      QSettings* settings() const;
      // the following functions must be used to access and change a preference
      // instead of using QSettings directly
      QVariant get(const QString key) const;
      bool has(const QString key) const;
      void set(const QString key, QVariant value, bool temporary = false);
      void remove(const QString key);

      QVariant preference(const QString key) const;
      QMetaType::Type type(const QString key) const;
      bool checkIfKeyExists(const QString key) const;
      bool checkType(const QString key, QMetaType::Type t) const;

      // Used with workspace
      QMap<QString, QVariant> localPreferences;
      QMap<QString, QVariant> getDefaultLocalPreferences();
      bool useLocalPrefs = false;

      QMap<ListenerID, OnSetListener> _onSetListeners;

   public:
      Preferences();
      ~Preferences();
      void init(bool storeInMemoryOnly = false);
      void save();
      // set to true to let getters return default values instead of values from QSettings
      void setReturnDefaultValuesMode(bool returnDefaultValues) {_returnDefaultValues = returnDefaultValues;}

      const prefs_map_t& allPreferences() const {return _allPreferences;}

      // general getters
      QVariant defaultValue(const QString key) const;
      bool getBool(const QString key) const;
      QColor getColor(const QString key) const;
      QString getString(const QString key) const;
      int getInt(const QString key) const;
      double getDouble(const QString key) const;

      // general setters
      void setToDefaultValue(const QString key);
      void setPreference(const QString key, QVariant value);

      // set listeners
      ListenerID addOnSetListener(const OnSetListener& l);
      void removeOnSetListener(const ListenerID& id);

      // A temporary preference is stored "in memory" only and not written to file.
      // If there is both a "normal" preference and a temporary preference with the same
      // key the temporary preference is used
      void setTemporaryPreference(const QString key, QVariant value);

      /*
       * Some preferences like enums and structs/classes are not easily read using the general set/get methods
       * and therefore require specific getters and/or setters
       */
      SessionStart sessionStart() const;
      MusicxmlExportBreaks musicxmlExportBreaks() const;
      MuseScorePreferredStyleType preferredGlobalStyle() const;
      MuseScoreEffectiveStyleType effectiveGlobalStyle() const;
      bool isThemeDark() const;

      template<typename T>
      void setCustomPreference(const QString key, T t)
            {
            set(key, QVariant::fromValue<T>(t));
            }

      // The midiRemote preference requires special handling due to its complexity
      MidiRemote midiRemote(int recordId) const;
      void updateMidiRemote(int recordId, MidiRemoteType type, int data);
      void clearMidiRemote(int recordId);

      QMap<QString, QVariant> getLocalPreferences()  { return localPreferences; }
      void setLocalPreference(QString key, QVariant value);
      void setUseLocalPreferences(bool value)         { useLocalPrefs = value;   }
      bool getUseLocalPreferences()                   { return useLocalPrefs;    }
      void updateLocalPreferences() { localPreferences = getDefaultLocalPreferences(); }
      };

// singleton
extern Preferences preferences;

// Stream operators for enum classes
// enum classes don't play well with QSettings without custom serialization
inline QDataStream&
operator<<(QDataStream &out, const Ms::MuseScorePreferredStyleType &val)
{
    return out << static_cast<int>(val);
}

inline QDataStream&
operator>>(QDataStream &in, Ms::MuseScorePreferredStyleType &val)
{
    int tmp;
    in >> tmp;
    val = static_cast<Ms::MuseScorePreferredStyleType>(tmp);
    return in;
}

inline QDataStream&
operator<<(QDataStream &out, const Ms::MuseScoreEffectiveStyleType &val)
{
    return out << static_cast<int>(val);
}

inline QDataStream&
operator>>(QDataStream &in, Ms::MuseScoreEffectiveStyleType &val)
{
    int tmp;
    in >> tmp;
    val = static_cast<Ms::MuseScoreEffectiveStyleType>(tmp);
    return in;
}

inline QDataStream&
operator<<(QDataStream &out, const Ms::SessionStart &val)
{
    return out << static_cast<int>(val);
}

inline QDataStream&
operator>>(QDataStream &in, Ms::SessionStart &val)
{
    int tmp;
    in >> tmp;
    val = static_cast<Ms::SessionStart>(tmp);
    return in;
}

inline QDataStream&
operator<<(QDataStream &out, const Ms::MusicxmlExportBreaks &val)
{
    return out << static_cast<int>(val);
}

inline QDataStream&
operator>>(QDataStream &in, Ms::MusicxmlExportBreaks &val)
{
    int tmp;
    in >> tmp;
    val = static_cast<Ms::MusicxmlExportBreaks>(tmp);
    return in;
}


class PreferenceVisitor {
   public:
      virtual void visit(QString key, IntPreference*) = 0;
      virtual void visit(QString key, DoublePreference*) = 0;
      virtual void visit(QString key, BoolPreference*) = 0;
      virtual void visit(QString key, StringPreference*) = 0;
      virtual void visit(QString key, ColorPreference*) = 0;
      };


} // namespace Ms

Q_DECLARE_METATYPE(Ms::SessionStart);
Q_DECLARE_METATYPE(Ms::MusicxmlExportBreaks);
Q_DECLARE_METATYPE(Ms::MuseScorePreferredStyleType);
Q_DECLARE_METATYPE(Ms::MuseScoreEffectiveStyleType);

#endif
