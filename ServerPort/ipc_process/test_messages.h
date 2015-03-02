// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// IPC messages for page rendering.
// Multiply-included message file, hence no include guard.
#include "base/process.h"
#include "base/shared_memory.h"
#include "base/string16.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_message_start.h"


#define IPC_MESSAGE_START MyTestMsgStart

IPC_MESSAGE_ROUTED1(ViewMsg_MyTestIpc, string16)

