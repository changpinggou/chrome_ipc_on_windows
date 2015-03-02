// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_BROWSER_PROCESS_SUB_THREAD_H_
#define CONTENT_BROWSER_BROWSER_PROCESS_SUB_THREAD_H_

#include "base/basictypes.h"
#include "browser_thread_impl.h"

#if defined(OS_WIN)
#include "base/win/scoped_com_initializer.h"
#endif

class BrowserProcessSubThread : public BrowserThreadImpl {
 public:
  explicit BrowserProcessSubThread(BrowserThread::ID identifier);
  virtual ~BrowserProcessSubThread();

 protected:
  virtual void Init() OVERRIDE;
  virtual void CleanUp() OVERRIDE;

 private:
  // These methods encapsulate cleanup that needs to happen on the IO thread
  // before we call the embedder's CleanUp function.
  void IOThreadPreCleanUp();

#if defined (OS_WIN)
  scoped_ptr<base::win::ScopedCOMInitializer> com_initializer_;
#endif


  DISALLOW_COPY_AND_ASSIGN(BrowserProcessSubThread);
};

#endif  // CONTENT_BROWSER_BROWSER_PROCESS_SUB_THREAD_H_
