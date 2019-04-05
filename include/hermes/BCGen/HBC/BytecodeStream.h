/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_BCGEN_HBC_BYTECODESTREAM_H
#define HERMES_BCGEN_HBC_BYTECODESTREAM_H

#include "llvm/Support/raw_ostream.h"

#include "hermes/BCGen/Exceptions.h"
#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/BCGen/HBC/DebugInfo.h"
#include "hermes/BCGen/HBC/StreamVector.h"
#include "hermes/Public/Buffer.h"
#include "hermes/Support/SHA1.h"
#include "hermes/Utils/Options.h"

namespace llvm {
class raw_ostream;
} // namespace llvm

namespace hermes {
struct RegExpTableEntry;
namespace hbc {
using llvm::ArrayRef;
using llvm::raw_ostream;

class BytecodeFunction;
class BytecodeModule;

class BytecodeSerializer {
  /// Output Stream
  raw_ostream &os_;
  /// Options controlling bytecode generation.
  BytecodeGenerationOptions options_;
  /// Current output offset.
  size_t loc_{0};
  /// Wheather we are doing a layout run.
  bool isLayout_{true};
  /// Total file length in bytes.
  uint32_t fileLength_{0};
  /// Offset of the debug info tables.
  uint32_t debugInfoOffset_{0};
  /// Count of overflow string entries, computed during layout phase.
  uint32_t overflowStringEntryCount_{0};

  /// Each subsection of a function's `info' section is aligned thusly.
  static constexpr uint32_t INFO_ALIGNMENT = 4;

  template <typename T>
  void writeBinaryArray(const ArrayRef<T> array) {
    size_t size = sizeof(T) * array.size();
    if (!isLayout_) {
      os_.write(reinterpret_cast<const char *>(array.data()), size);
    }
    loc_ += size;
  }

  template <typename T>
  void writeBinary(const T &structure) {
    return writeBinaryArray(ArrayRef<T>{&structure, 1});
  }

  /// Padding the binary according to the \p alignment.
  void pad(unsigned alignment) {
    // Support alignment as many as 8 bytes.
    assert(
        alignment > 0 && alignment <= 8 &&
        ((alignment & (alignment - 1)) == 0));
    if (loc_ % alignment == 0)
      return;
    unsigned bytes = alignment - loc_ % alignment;
    for (unsigned i = 0; i < bytes; ++i) {
      writeBinary('\0');
    }
  }

  void serializeFunctionTable(BytecodeModule &BM);

  void serializeStringTable(BytecodeModule &BM);

  void serializeRegExps(BytecodeModule &BM);

  void serializeCJSModuleTable(BytecodeModule &BM);

  void serializeDebugInfo(BytecodeModule &BM);

  void serializeArrayBuffer(BytecodeModule &BM);

  void serializeObjectBuffer(BytecodeModule &BM);

  void serializeExceptionHandlerTable(BytecodeFunction &BF);

  void serializeDebugOffsets(BytecodeFunction &BF);

  void serializeFunctionsBytecode(BytecodeModule &BM);
  void serializeFunctionInfo(BytecodeFunction &BF);

  void finishLayout(BytecodeModule &BM);

 public:
  explicit BytecodeSerializer(
      raw_ostream &OS,
      BytecodeGenerationOptions options = BytecodeGenerationOptions::defaults())
      : os_(OS), options_(options) {}

  void serialize(BytecodeModule &BM, const SHA1 &sourceHash);
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_BYTECODESTREAM_H
