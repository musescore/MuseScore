//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA and others
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

#ifndef SERVICESRESOLVER_H
#define SERVICESRESOLVER_H

#include <QHash>
#include <QUuid>

#define INTERFACE_ID                               \
      public:                                      \
      static const QUuid interfaceId() {           \
            static QUuid id = QUuid::createUuid(); \
            return id;                             \
            }                                      \

//---------------------------------------------------------
//   ServicesResolver
//---------------------------------------------------------

class ServicesResolver {
   public:

      //---------------------------------------------------
      //   IServiceFactory
      //---------------------------------------------------

      struct IServiceFactory {
            virtual void* getInstance() = 0;
            };

      //---------------------------------------------------
      //   FunctorBasedFactory
      //---------------------------------------------------

      template <typename T>
      struct FunctorBasedFactory : public IServiceFactory {
            using F = T*(*)();
            FunctorBasedFactory(F f) : IServiceFactory() {
                  getInstanceFunc = f;
                  }
            void* getInstance() override {
                  return getInstanceFunc();
                  }

      private:
            F getInstanceFunc;
            };

      template <typename I, typename T>
      static inline void registerService(T*(*f)()) {

            QUuid interfaceId = I::interfaceId();

            FunctorBasedFactory<T>* srv = new FunctorBasedFactory<T>(f);

            srvHash()->insert(interfaceId, srv);
            }

      template <typename I>
      static inline IServiceFactory* resolveServiceFactory() {
            QUuid interfaceId = I::interfaceId();

            return srvHash()->value(interfaceId);
            }

   private:
      static inline QHash<QUuid, IServiceFactory*> *srvHash() {
            static QHash<QUuid, IServiceFactory*> serviceHash = QHash<QUuid, IServiceFactory*>();

            return &serviceHash;
            }
      };

#endif // SERVICESRESOLVER_H
