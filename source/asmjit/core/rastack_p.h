// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_RASTACK_P_H_INCLUDED
#define ASMJIT_CORE_RASTACK_P_H_INCLUDED

#include "../core/api-config.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/radefs_p.h"

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_ra
//! \{

//! Stack slot.
struct RAStackSlot {
  //! Stack slot flags.
  //!
  //! TODO: kFlagStackArg is not used by the current implementation, do we need to keep it?
  enum Flags : uint16_t {
    //! Stack slot is register home slot.
    kFlagRegHome = 0x0001u,
    //! Stack slot position matches argument passed via stack.
    kFlagStackArg = 0x0002u

  };
  enum ArgIndex : uint32_t {
    kNoArgIndex = 0xFF
  };

  //! \name Members
  //! \{

  //! Base register used to address the stack.
  uint8_t _baseRegId;
  //! Minimum alignment required by the slot.
  uint8_t _alignment;
  //! Reserved for future use.
  uint16_t _flags;
  //! Size of memory required by the slot.
  uint32_t _size;

  //! Usage counter (one unit equals one memory access).
  uint32_t _useCount;
  //! Weight of the slot, calculated by \ref RAStackAllocator::calculateStackFrame().
  uint32_t _weight;
  //! Stack offset, calculated by \ref RAStackAllocator::calculateStackFrame().
  int32_t _offset;

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  inline uint32_t baseRegId() const noexcept { return _baseRegId; }

  inline void setBaseRegId(uint32_t id) noexcept { _baseRegId = uint8_t(id); }

  [[nodiscard]]
  inline uint32_t size() const noexcept { return _size; }

  [[nodiscard]]
  inline uint32_t alignment() const noexcept { return _alignment; }

  [[nodiscard]]
  inline uint32_t flags() const noexcept { return _flags; }

  [[nodiscard]]
  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }

  inline void addFlags(uint32_t flags) noexcept { _flags = uint16_t(_flags | flags); }

  [[nodiscard]]
  inline bool isRegHome() const noexcept { return hasFlag(kFlagRegHome); }

  [[nodiscard]]
  inline bool isStackArg() const noexcept { return hasFlag(kFlagStackArg); }

  [[nodiscard]]
  inline uint32_t useCount() const noexcept { return _useCount; }

  inline void addUseCount(uint32_t n = 1) noexcept { _useCount += n; }

  [[nodiscard]]
  inline uint32_t weight() const noexcept { return _weight; }

  inline void setWeight(uint32_t weight) noexcept { _weight = weight; }

  [[nodiscard]]
  inline int32_t offset() const noexcept { return _offset; }

  inline void setOffset(int32_t offset) noexcept { _offset = offset; }

  //! \}
};

using RAStackSlots = ZoneVector<RAStackSlot*>;

//! Stack allocator.
class RAStackAllocator {
public:
  ASMJIT_NONCOPYABLE(RAStackAllocator)

  enum Size : uint32_t {
    kSize1     = 0,
    kSize2     = 1,
    kSize4     = 2,
    kSize8     = 3,
    kSize16    = 4,
    kSize32    = 5,
    kSize64    = 6,
    kSizeCount = 7
  };

  //! \name Members
  //! \{

  //! Zone used to allocate internal data.
  Zone* _zone {};
  //! Container allocator used to allocate internal data.
  ZoneAllocator* _allocator {};

  //! Count of bytes used by all slots.
  uint32_t _bytesUsed {};
  //! Calculated stack size (can be a bit greater than `_bytesUsed`).
  uint32_t _stackSize {};
  //! Minimum stack alignment.
  uint32_t _alignment = 1;
  //! Stack slots vector.
  RAStackSlots _slots;

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG RAStackAllocator() noexcept {}

  ASMJIT_INLINE_NODEBUG void reset(Zone* zone, ZoneAllocator* allocator) noexcept {
    _zone = zone;
    _allocator = allocator;
    _bytesUsed = 0;
    _stackSize = 0;
    _alignment = 1;
    _slots.reset();
  }

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Zone* zone() const noexcept { return _zone; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ZoneAllocator* allocator() const noexcept { return _allocator; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t bytesUsed() const noexcept { return _bytesUsed; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t stackSize() const noexcept { return _stackSize; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t alignment() const noexcept { return _alignment; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAStackSlots& slots() noexcept { return _slots; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RAStackSlots& slots() const noexcept { return _slots; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t slotCount() const noexcept { return _slots.size(); }

  //! \}

  //! \name Utilities
  //! \{

  [[nodiscard]]
  RAStackSlot* newSlot(uint32_t baseRegId, uint32_t size, uint32_t alignment, uint32_t flags = 0) noexcept;

  [[nodiscard]]
  Error calculateStackFrame() noexcept;

  [[nodiscard]]
  Error adjustSlotOffsets(int32_t offset) noexcept;

  //! \}
};

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_CORE_RASTACK_P_H_INCLUDED
