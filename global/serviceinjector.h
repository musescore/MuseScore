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

#ifndef SERVICEINJECTOR_H
#define SERVICEINJECTOR_H

#include <servicesresolver.h>

#include <QSharedPointer>

#define INJECT(INTERFACE_NAME, ALIAS)                                                                      \
      public:                                                                                              \
      INTERFACE_NAME* ALIAS() { return ServiceInjector<INTERFACE_NAME>::getService(); }                    \
      void set##ALIAS(INTERFACE_NAME* impl) { ServiceInjector<INTERFACE_NAME>::setService(impl); }         \

//---------------------------------------------------------
//   ServiceInjector
//---------------------------------------------------------

template <typename I>
class ServiceInjector {
   public:
      ServiceInjector() {
            ServicesResolver::IServiceFactory* srvFactory = ServicesResolver::resolveServiceFactory<I>();

            m_service = QSharedPointer<I>(static_cast<I*>(srvFactory->getInstance()));
            }

      I* getService() { return m_service.data(); }

      void setService(I* service) { m_service = QSharedPointer<I>(service); }

   private:
      QSharedPointer<I> m_service;
      };

#endif // SERVICEINJECTOR_H
