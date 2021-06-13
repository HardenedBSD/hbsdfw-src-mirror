//===-- CommandObjectTrace.h ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_COMMANDS_COMMANDOBJECTTRACE_H
#define LLDB_SOURCE_COMMANDS_COMMANDOBJECTTRACE_H

#include "lldb/Interpreter/CommandObjectMultiword.h"

namespace lldb_private {

class CommandObjectTrace : public CommandObjectMultiword {
public:
  CommandObjectTrace(CommandInterpreter &interpreter);

  ~CommandObjectTrace() override;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_COMMANDS_COMMANDOBJECTTRACE_H
