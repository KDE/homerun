/*
  Copyright 2008 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef GGADGET_FLOATING_DECORATOR_H__
#define GGADGET_FLOATING_DECORATOR_H__

#include <ggadget/main_view_decorator_base.h>
#include "plasma_view_host.h"

namespace ggadget {

class FloatingDecorator : public MainViewDecoratorBase {
 public:
  FloatingDecorator(PlasmaViewHost *host);
  virtual ~FloatingDecorator();
  virtual void OnAddDecoratorMenuItems(MenuInterface *menu);
  virtual bool ShowDecoratedView(bool modal, int flags,
                                 Slot1<bool, int> *feedback_handler);

 private:
  GadgetInfo *info;
  DISALLOW_EVIL_CONSTRUCTORS(FloatingDecorator);
};

} // namespace ggadget

#endif
