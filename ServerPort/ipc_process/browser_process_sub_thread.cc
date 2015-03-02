// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "browser_process_sub_thread.h"

#include "base/debug/leak_tracker.h"
#include "base/threading/thread_restrictions.h"
#include "build/build_config.h"

BrowserProcessSubThread::BrowserProcessSubThread(BrowserThread::ID identifier)
    : BrowserThreadImpl(identifier) {
}

BrowserProcessSubThread::~BrowserProcessSubThread() {
  Stop();
}

void BrowserProcessSubThread::Init() {
#if defined(OS_WIN)
  com_initializer_.reset(new base::win::ScopedCOMInitializer());
#endif

  //notification_service_.reset(new NotificationServiceImpl());

  BrowserThreadImpl::Init();

  if (BrowserThread::CurrentlyOn(BrowserThread::IO)) {
    // Though this thread is called the "IO" thread, it actually just routes
    // messages around; it shouldn't be allowed to perform any blocking disk
    // I/O.
    base::ThreadRestrictions::SetIOAllowed(false);
    base::ThreadRestrictions::DisallowWaiting();
  }
}

void BrowserProcessSubThread::CleanUp() {
  if (BrowserThread::CurrentlyOn(BrowserThread::IO))
    IOThreadPreCleanUp();

  BrowserThreadImpl::CleanUp();
#if defined(OS_WIN)
  com_initializer_.reset();
#endif
}

void BrowserProcessSubThread::IOThreadPreCleanUp() {
}
