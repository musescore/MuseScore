//////////////////////////////////////////////////////////////////////////////
// oxygenanimationdata.h
// base class data container needed for widget animations
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ANIMATIONDATA_H__
#define __ANIMATIONDATA_H__

#include "animation.h"

class AnimationData: public QObject {
            Q_OBJECT

      public:

            //! constructor
            AnimationData(QObject* parent, QWidget* target)
                  : QObject(parent), target_(target), enabled_(true) {
                  Q_ASSERT(target_);
                  }
            virtual ~AnimationData() {}
            virtual void setDuration(int) = 0;
            virtual bool enabled() const        {
                  return enabled_;
                  }
            virtual void setEnabled(bool value) {
                  enabled_ = value;
                  }

            const QPointer<QWidget>& target( void ) const {
                  return target_;
                  }
            static qreal OpacityInvalid;

      protected:
            //! setup animation
            virtual void setupAnimation(const Animation::Pointer& animation, const QByteArray& property);

            //! trigger target update
            virtual void setDirty() const {
                  if (target_)
                        target_.data()->update();
                  }

      private:
            //! guarded target
            QPointer<QWidget> target_;
            bool enabled_;
      };

#endif
