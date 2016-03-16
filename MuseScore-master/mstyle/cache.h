
#ifndef __CACHE_H__
#define __CACHE_H__

//---------------------------------------------------------
//   BaseCache
//---------------------------------------------------------

template<typename T> class BaseCache: public QCache<quint64, T> {
            bool _enabled;

      public:
            //! constructor
            BaseCache(int maxCost) : QCache<quint64, T>(maxCost), _enabled(true) {}
            explicit BaseCache() : _enabled(true) { }
            ~BaseCache() {}

            //! enable
            void setEnabled(bool value) {
                  _enabled = value;
                  }

            //! enable state
            bool enabled() const {
                  return _enabled;
                  }

            //! access
            T* object(const quint64& key) {
                  return _enabled ? QCache<quint64, T>::object(key) : 0;
                  }

            //! max cost
            void setMaxCost( int cost ) {
                  if ( cost <= 0 ) {
                        QCache<quint64, T>::clear();
                        QCache<quint64, T>::setMaxCost(1);
                        setEnabled( false );
                        }
                  else {
                        setEnabled(true);
                        QCache<quint64, T>::setMaxCost(cost);
                        }
                  }
      };

//---------------------------------------------------------
//   Cache
//---------------------------------------------------------

template<typename T> class Cache {
      public:
            Cache() {}
            ~Cache() {}

            //! return cache matching a given key
            //typedef QCache<quint64, T> Value;
            typedef BaseCache<T> Value;

            Value* get(const QColor& color) {
                  quint64 key = (quint64(color.rgba()) << 32);
                  Value* cache = data_.object(key);
                  if (!cache) {
                        cache = new Value( data_.maxCost() );
                        data_.insert(key, cache);
                        }
                  return cache;
                  }

            void clear() {
                  data_.clear();
                  }

            //! max cache size
            void setMaxCacheSize(int value) {
                  data_.setMaxCost(value);
                  foreach(quint64 key, data_.keys()) {
                        data_.object(key)->setMaxCost(value);
                        }
                  }
      private:
            BaseCache<Value> data_;
      };

#endif

