// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This header defines the set of functions necessary for trace_event_internal.h
// to remain platform and context neutral.  To use tracing in another context,
// copy this header (and trace_event_internal.h) and implement the
// TRACE_EVENT_API_* macros below for the new environment.

#ifndef BASE_DEBUG_TRACE_EVENT_H_
#define BASE_DEBUG_TRACE_EVENT_H_

#include "base/atomicops.h"
#include "base/debug/trace_event_impl.h"
#include "build/build_config.h"

////////////////////////////////////////////////////////////////////////////////
// Implementation specific tracing API definitions.

// Get a pointer to the enabled state of the given trace category. Only
// long-lived literal strings should be given as the category name. The returned
// pointer can be held permanently in a local static for example. If the
// unsigned char is non-zero, tracing is enabled. If tracing is enabled,
// TRACE_EVENT_API_ADD_TRACE_EVENT can be called. It's OK if tracing is disabled
// between the load of the tracing state and the call to
// TRACE_EVENT_API_ADD_TRACE_EVENT, because this flag only provides an early out
// for best performance when tracing is disabled.
// const unsigned char*
//     TRACE_EVENT_API_GET_CATEGORY_ENABLED(const char* category_name)
#define TRACE_EVENT_API_GET_CATEGORY_ENABLED \
    base::debug::TraceLog::GetCategoryEnabled

// Add a trace event to the platform tracing system.
// void TRACE_EVENT_API_ADD_TRACE_EVENT(
//                    char phase,
//                    const unsigned char* category_enabled,
//                    const char* name,
//                    unsigned long long id,
//                    int num_args,
//                    const char** arg_names,
//                    const unsigned char* arg_types,
//                    const unsigned long long* arg_values,
//                    unsigned char flags)
#define TRACE_EVENT_API_ADD_TRACE_EVENT \
    base::debug::TraceLog::GetInstance()->AddTraceEvent

// Add a trace event to the platform tracing system.
// void TRACE_EVENT_API_ADD_TRACE_EVENT_WITH_TIMESTAMP(
//                    char phase,
//                    const unsigned char* category_enabled,
//                    const char* name,
//                    unsigned long long id,
//                    int thread_id,
//                    const TimeTicks& timestamp,
//                    int num_args,
//                    const char** arg_names,
//                    const unsigned char* arg_types,
//                    const unsigned long long* arg_values,
//                    unsigned char flags)
#define TRACE_EVENT_API_ADD_TRACE_EVENT_WITH_THREAD_ID_AND_TIMESTAMP \
    base::debug::TraceLog::GetInstance()->AddTraceEventWithThreadIdAndTimestamp

// Defines atomic operations used internally by the tracing system.
#define TRACE_EVENT_API_ATOMIC_WORD base::subtle::AtomicWord
#define TRACE_EVENT_API_ATOMIC_LOAD(var) base::subtle::NoBarrier_Load(&(var))
#define TRACE_EVENT_API_ATOMIC_STORE(var, value) \
    base::subtle::NoBarrier_Store(&(var), (value))

// Defines visibility for classes in trace_event_internal.h
#define TRACE_EVENT_API_CLASS_EXPORT BASE_EXPORT

// The thread buckets for the sampling profiler.
TRACE_EVENT_API_CLASS_EXPORT extern TRACE_EVENT_API_ATOMIC_WORD g_trace_state0;
TRACE_EVENT_API_CLASS_EXPORT extern TRACE_EVENT_API_ATOMIC_WORD g_trace_state1;
TRACE_EVENT_API_CLASS_EXPORT extern TRACE_EVENT_API_ATOMIC_WORD g_trace_state2;
#define TRACE_EVENT_API_THREAD_BUCKET(thread_bucket)                           \
  g_trace_state##thread_bucket

////////////////////////////////////////////////////////////////////////////////

#include "base/debug/trace_event_internal.h"

namespace base {
namespace debug {

template<typename IDType> class TraceScopedTrackableObject {
 public:
  TraceScopedTrackableObject(const char* category, const char* name, IDType id)
    : category_(category),
      name_(name),
      id_(id) {
    TRACE_EVENT_OBJECT_CREATED_WITH_ID(category_, name_, id_);
  }

  ~TraceScopedTrackableObject() {
    TRACE_EVENT_OBJECT_DELETED_WITH_ID(category_, name_, id_);
  }

 private:
  const char* category_;
  const char* name_;
  IDType id_;

  DISALLOW_COPY_AND_ASSIGN(TraceScopedTrackableObject);
};

} // namespace debug
} // namespace base

#endif /* BASE_DEBUG_TRACE_EVENT_H_ */
