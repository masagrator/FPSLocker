// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#ifndef ASMJIT_NO_BUILDER

#include "../core/builder.h"
#include "../core/emitterutils_p.h"
#include "../core/errorhandler.h"
#include "../core/formatter.h"
#include "../core/logger.h"
#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

// PostponedErrorHandler (Internal)
// ================================

//! Postponed error handler that never throws. Used as a temporal error handler
//! to run passes. If error occurs, the caller is notified and will call the
//! real error handler, that can throw.
class PostponedErrorHandler : public ErrorHandler {
public:
  void handleError(Error err, const char* message, BaseEmitter* origin) override {
    DebugUtils::unused(err, origin);
    _message.assign(message);
  }

  StringTmp<128> _message;
};

// BaseBuilder - Utilities
// =======================

static void BaseBuilder_deletePasses(BaseBuilder* self) noexcept {
  for (Pass* pass : self->_passes) {
    pass->~Pass();
  }
  self->_passes.reset();
}

// BaseBuilder - Construction & Destruction
// ========================================

BaseBuilder::BaseBuilder() noexcept
  : BaseEmitter(EmitterType::kBuilder),
    _codeZone(64u * 1024u),
    _passZone(64u * 1024u),
    _allocator(&_codeZone) {}

BaseBuilder::~BaseBuilder() noexcept {
  BaseBuilder_deletePasses(this);
}

// BaseBuilder - Node Management
// =============================

Error BaseBuilder::newInstNode(InstNode** out, InstId instId, InstOptions instOptions, uint32_t opCount) {
  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= InstNode::kBaseOpCapacity);

  void* ptr = _codeZone.alloc(InstNode::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!ptr)) {
    return reportError(DebugUtils::errored(kErrorOutOfMemory));
  }

  *out = new(Support::PlacementNew{ptr}) InstNode(instId, instOptions, opCount, opCapacity);
  return kErrorOk;
}

Error BaseBuilder::newLabelNode(LabelNode** out) {
  *out = nullptr;

  ASMJIT_PROPAGATE(_newNodeT<LabelNode>(out));
  return registerLabelNode(*out);
}

Error BaseBuilder::newAlignNode(AlignNode** out, AlignMode alignMode, uint32_t alignment) {
  *out = nullptr;
  return _newNodeT<AlignNode>(out, alignMode, alignment);
}

Error BaseBuilder::newEmbedDataNode(EmbedDataNode** out, TypeId typeId, const void* data, size_t itemCount, size_t repeatCount) {
  *out = nullptr;

  uint32_t deabstractDelta = TypeUtils::deabstractDeltaOfSize(registerSize());
  TypeId finalTypeId = TypeUtils::deabstract(typeId, deabstractDelta);

  if (ASMJIT_UNLIKELY(!TypeUtils::isValid(finalTypeId))) {
    return reportError(DebugUtils::errored(kErrorInvalidArgument));
  }

  uint32_t typeSize = TypeUtils::sizeOf(finalTypeId);
  Support::FastUInt8 of = 0;

  size_t nodeSize = Support::maddOverflow(itemCount, size_t(typeSize), sizeof(EmbedDataNode), &of);
  if (ASMJIT_UNLIKELY(of)) {
    return reportError(DebugUtils::errored(kErrorOutOfMemory));
  }

  EmbedDataNode* node = nullptr;
  ASMJIT_PROPAGATE(_newNodeTWithSize<EmbedDataNode>(
    &node, Support::alignUp(nodeSize, Globals::kZoneAlignment),
    typeId, uint8_t(typeSize), itemCount, repeatCount
  ));

  if (data) {
    memcpy(node->data(), data, node->dataSize());
  }

  *out = node;
  return kErrorOk;
}

Error BaseBuilder::newConstPoolNode(ConstPoolNode** out) {
  *out = nullptr;

  ASMJIT_PROPAGATE(_newNodeT<ConstPoolNode>(out, &_codeZone));
  return registerLabelNode(*out);
}

Error BaseBuilder::newCommentNode(CommentNode** out, const char* data, size_t size) {
  *out = nullptr;

  if (data) {
    if (size == SIZE_MAX) {
      size = strlen(data);
    }

    if (size > 0) {
      data = static_cast<char*>(_codeZone.dup(data, size, true));
      if (ASMJIT_UNLIKELY(!data)) {
        return reportError(DebugUtils::errored(kErrorOutOfMemory));
      }
    }
  }

  return _newNodeT<CommentNode>(out, data);
}

BaseNode* BaseBuilder::addNode(BaseNode* node) noexcept {
  ASMJIT_ASSERT(!node->_prev);
  ASMJIT_ASSERT(!node->_next);
  ASMJIT_ASSERT(!node->isActive());

  if (!_cursor) {
    if (_nodeList.empty()) {
      _nodeList.reset(node, node);
    }
    else {
      node->_next = _nodeList.first();
      _nodeList._first->_prev = node;
      _nodeList._first = node;
    }
  }
  else {
    BaseNode* prev = _cursor;
    BaseNode* next = _cursor->next();

    node->_prev = prev;
    node->_next = next;

    prev->_next = node;
    if (next) {
      next->_prev = node;
    }
    else {
      _nodeList._last = node;
    }
  }

  node->_addFlags(NodeFlags::kIsActive);
  if (node->isSection()) {
    _dirtySectionLinks = true;
  }

  _cursor = node;
  return node;
}

BaseNode* BaseBuilder::addAfter(BaseNode* node, BaseNode* ref) noexcept {
  ASMJIT_ASSERT(!node->_prev);
  ASMJIT_ASSERT(!node->_next);

  BaseNode* prev = ref;
  BaseNode* next = ref->next();

  node->_prev = prev;
  node->_next = next;

  node->_addFlags(NodeFlags::kIsActive);
  if (node->isSection()) {
    _dirtySectionLinks = true;
  }

  prev->_next = node;
  if (next) {
    next->_prev = node;
  }
  else {
    _nodeList._last = node;
  }

  return node;
}

BaseNode* BaseBuilder::addBefore(BaseNode* node, BaseNode* ref) noexcept {
  ASMJIT_ASSERT(!node->_prev);
  ASMJIT_ASSERT(!node->_next);
  ASMJIT_ASSERT(!node->isActive());
  ASMJIT_ASSERT(ref->isActive());

  BaseNode* prev = ref->prev();
  BaseNode* next = ref;

  node->_prev = prev;
  node->_next = next;

  node->_addFlags(NodeFlags::kIsActive);
  if (node->isSection()) {
    _dirtySectionLinks = true;
  }

  next->_prev = node;
  if (prev) {
    prev->_next = node;
  }
  else {
    _nodeList._first = node;
  }

  return node;
}

BaseNode* BaseBuilder::removeNode(BaseNode* node) noexcept {
  if (!node->isActive()) {
    return node;
  }

  BaseNode* prev = node->prev();
  BaseNode* next = node->next();

  if (_nodeList._first == node) {
    _nodeList._first = next;
  }
  else {
    prev->_next = next;
  }

  if (_nodeList._last == node) {
    _nodeList._last  = prev;
  }
  else {
    next->_prev = prev;
  }

  node->_prev = nullptr;
  node->_next = nullptr;
  node->_clearFlags(NodeFlags::kIsActive);

  if (node->isSection()) {
    _dirtySectionLinks = true;
  }

  if (_cursor == node) {
    _cursor = prev;
  }

  return node;
}

void BaseBuilder::removeNodes(BaseNode* first, BaseNode* last) noexcept {
  if (first == last) {
    removeNode(first);
    return;
  }

  if (!first->isActive()) {
    return;
  }

  BaseNode* prev = first->prev();
  BaseNode* next = last->next();

  if (_nodeList._first == first) {
    _nodeList._first = next;
  }
  else {
    prev->_next = next;
  }

  if (_nodeList._last == last) {
    _nodeList._last  = prev;
  }
  else {
    next->_prev = prev;
  }

  BaseNode* node = first;
  uint32_t didRemoveSection = false;

  for (;;) {
    next = node->next();
    ASMJIT_ASSERT(next != nullptr);

    node->_prev = nullptr;
    node->_next = nullptr;
    node->_clearFlags(NodeFlags::kIsActive);
    didRemoveSection |= uint32_t(node->isSection());

    if (_cursor == node) {
      _cursor = prev;
    }

    if (node == last) {
      break;
    }
    node = next;
  }

  if (didRemoveSection) {
    _dirtySectionLinks = true;
  }
}

BaseNode* BaseBuilder::setCursor(BaseNode* node) noexcept {
  BaseNode* old = _cursor;
  _cursor = node;
  return old;
}

// BaseBuilder - Sections
// ======================

Error BaseBuilder::sectionNodeOf(SectionNode** out, uint32_t sectionId) {
  *out = nullptr;

  if (ASMJIT_UNLIKELY(!_code)) {
    return DebugUtils::errored(kErrorNotInitialized);
  }

  if (ASMJIT_UNLIKELY(!_code->isSectionValid(sectionId))) {
    return reportError(DebugUtils::errored(kErrorInvalidSection));
  }

  if (sectionId >= _sectionNodes.size()) {
    Error err = _sectionNodes.reserve(&_allocator, sectionId + 1);
    if (ASMJIT_UNLIKELY(err != kErrorOk)) {
      return reportError(err);
    }
  }

  SectionNode* node = nullptr;
  if (sectionId < _sectionNodes.size()) {
    node = _sectionNodes[sectionId];
  }

  if (!node) {
    ASMJIT_PROPAGATE(_newNodeT<SectionNode>(&node, sectionId));

    // We have already reserved enough space, this cannot fail now.
    if (sectionId >= _sectionNodes.size()) {
      // SAFETY: No need to check for error condition as we have already reserved enough space.
      (void)_sectionNodes.resize(&_allocator, sectionId + 1);
    }

    _sectionNodes[sectionId] = node;
  }

  *out = node;
  return kErrorOk;
}

Error BaseBuilder::section(Section* section) {
  SectionNode* node;
  ASMJIT_PROPAGATE(sectionNodeOf(&node, section->sectionId()));
  ASMJIT_ASSUME(node != nullptr);

  if (!node->isActive()) {
    // Insert the section at the end if it was not part of the code.
    addAfter(node, lastNode());
    _cursor = node;
  }
  else {
    // This is a bit tricky. We cache section links to make sure that switching sections doesn't involve
    // traversal in linked-list unless the position of the section has changed.
    if (hasDirtySectionLinks()) {
      updateSectionLinks();
    }

    if (node->_nextSection) {
      _cursor = node->_nextSection->_prev;
    }
    else {
      _cursor = _nodeList.last();
    }
  }

  return kErrorOk;
}

void BaseBuilder::updateSectionLinks() noexcept {
  if (!_dirtySectionLinks) {
    return;
  }

  BaseNode* node_ = _nodeList.first();
  SectionNode* currentSection = nullptr;

  while (node_) {
    if (node_->isSection()) {
      if (currentSection) {
        currentSection->_nextSection = node_->as<SectionNode>();
      }
      currentSection = node_->as<SectionNode>();
    }
    node_ = node_->next();
  }

  if (currentSection) {
    currentSection->_nextSection = nullptr;
  }

  _dirtySectionLinks = false;
}

// BaseBuilder - Labels
// ====================

Error BaseBuilder::labelNodeOf(LabelNode** out, uint32_t labelId) {
  *out = nullptr;

  if (ASMJIT_UNLIKELY(!_code)) {
    return DebugUtils::errored(kErrorNotInitialized);
  }

  uint32_t index = labelId;
  if (ASMJIT_UNLIKELY(index >= _code->labelCount())) {
    return DebugUtils::errored(kErrorInvalidLabel);
  }

  if (index >= _labelNodes.size()) {
    ASMJIT_PROPAGATE(_labelNodes.resize(&_allocator, index + 1));
  }

  LabelNode* node = _labelNodes[index];
  if (!node) {
    ASMJIT_PROPAGATE(_newNodeT<LabelNode>(&node, labelId));
    _labelNodes[index] = node;
  }

  *out = node;
  return kErrorOk;
}

Error BaseBuilder::registerLabelNode(LabelNode* node) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return DebugUtils::errored(kErrorNotInitialized);
  }

  uint32_t labelId;
  ASMJIT_PROPAGATE(_code->newLabelId(&labelId));

  // We just added one label so it must be true.
  ASMJIT_ASSERT(_labelNodes.size() < labelId + 1);
  ASMJIT_PROPAGATE(_labelNodes.resize(&_allocator, labelId + 1));

  _labelNodes[labelId] = node;
  node->_labelId = labelId;

  return kErrorOk;
}

static Error BaseBuilder_newLabelInternal(BaseBuilder* self, uint32_t labelId) {
  ASMJIT_ASSERT(self->_labelNodes.size() < labelId + 1);

  uint32_t growBy = labelId - self->_labelNodes.size();
  Error err = self->_labelNodes.willGrow(&self->_allocator, growBy);

  if (ASMJIT_UNLIKELY(err)) {
    return self->reportError(err);
  }

  LabelNode* node = nullptr;
  ASMJIT_PROPAGATE(self->_newNodeT<LabelNode>(&node, labelId));

  // SAFETY: No need to check for error condition as we have already reserved enough space.
  (void)self->_labelNodes.resize(&self->_allocator, labelId + 1);
  self->_labelNodes[labelId] = node;
  node->_labelId = labelId;
  return kErrorOk;
}

Label BaseBuilder::newLabel() {
  Label label;

  if (ASMJIT_LIKELY(_code)) {
    uint32_t labelId;
    Error err = _code->newLabelId(&labelId);

    if (ASMJIT_UNLIKELY(err)) {
      reportError(err);
    }
    else {
      if (ASMJIT_LIKELY(BaseBuilder_newLabelInternal(this, labelId)) == kErrorOk) {
        label.setId(labelId);
      }
    }
  }

  return label;
}

Label BaseBuilder::newNamedLabel(const char* name, size_t nameSize, LabelType type, uint32_t parentId) {
  Label label;

  if (ASMJIT_LIKELY(_code)) {
    uint32_t labelId;
    Error err = _code->newNamedLabelId(&labelId, name, nameSize, type, parentId);

    if (ASMJIT_UNLIKELY(err)) {
      reportError(err);
    }
    else {
      if (ASMJIT_LIKELY(BaseBuilder_newLabelInternal(this, labelId) == kErrorOk)) {
        label.setId(labelId);
      }
    }
  }

  return label;
}

Error BaseBuilder::bind(const Label& label) {
  LabelNode* node;
  ASMJIT_PROPAGATE(labelNodeOf(&node, label));

  addNode(node);
  return kErrorOk;
}

// BaseBuilder - Passes
// ====================

ASMJIT_FAVOR_SIZE Pass* BaseBuilder::passByName(const char* name) const noexcept {
  for (Pass* pass : _passes) {
    if (strcmp(pass->name(), name) == 0) {
      return pass;
    }
  }
  return nullptr;
}

ASMJIT_FAVOR_SIZE Error BaseBuilder::addPass(Pass* pass) noexcept {
  if (ASMJIT_UNLIKELY(!_code)) {
    return DebugUtils::errored(kErrorNotInitialized);
  }

  if (ASMJIT_UNLIKELY(pass == nullptr)) {
    // Since this is directly called by `addPassT()` we treat `null` argument
    // as out-of-memory condition. Otherwise it would be API misuse.
    return DebugUtils::errored(kErrorOutOfMemory);
  }
  else if (ASMJIT_UNLIKELY(pass->_cb)) {
    // Kinda weird, but okay...
    if (pass->_cb == this) {
      return kErrorOk;
    }
    return DebugUtils::errored(kErrorInvalidState);
  }

  ASMJIT_PROPAGATE(_passes.append(&_allocator, pass));
  pass->_cb = this;
  return kErrorOk;
}

Error BaseBuilder::runPasses() {
  if (ASMJIT_UNLIKELY(!_code)) {
    return DebugUtils::errored(kErrorNotInitialized);
  }

  if (_passes.empty()) {
    return kErrorOk;
  }

  ErrorHandler* prev = errorHandler();
  PostponedErrorHandler postponed;

  Error err = kErrorOk;
  setErrorHandler(&postponed);

  for (Pass* pass : _passes) {
    _passZone.reset();
    err = pass->run(&_passZone, _logger);
    if (err) {
      break;
    }
  }
  _passZone.reset();
  setErrorHandler(prev);

  if (ASMJIT_UNLIKELY(err)) {
    return reportError(err, !postponed._message.empty() ? postponed._message.data() : nullptr);
  }

  return kErrorOk;
}

// BaseBuilder - Emit
// ==================

Error BaseBuilder::_emit(InstId instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* opExt) {
  uint32_t opCount = EmitterUtils::opCountFromEmitArgs(o0, o1, o2, opExt);
  InstOptions options = instOptions() | forcedInstOptions();

  if (Support::test(options, InstOptions::kReserved)) {
    if (ASMJIT_UNLIKELY(!_code)) {
      return DebugUtils::errored(kErrorNotInitialized);
    }

#ifndef ASMJIT_NO_VALIDATION
    // Strict validation.
    if (hasDiagnosticOption(DiagnosticOptions::kValidateIntermediate)) {
      Operand_ opArray[Globals::kMaxOpCount];
      EmitterUtils::opArrayFromEmitArgs(opArray, o0, o1, o2, opExt);

      ValidationFlags validationFlags = isCompiler() ? ValidationFlags::kEnableVirtRegs : ValidationFlags::kNone;
      Error err = _funcs.validate(BaseInst(instId, options, _extraReg), opArray, opCount, validationFlags);

      if (ASMJIT_UNLIKELY(err)) {
#ifndef ASMJIT_NO_LOGGING
        return EmitterUtils::logInstructionFailed(this, err, instId, options, o0, o1, o2, opExt);
#else
        resetState();
        return reportError(err);
#endif
      }
    }
#endif

    // Clear instruction options that should never be part of a regular instruction.
    options &= ~InstOptions::kReserved;
  }

  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= InstNode::kBaseOpCapacity);

  void* ptr = _codeZone.alloc(InstNode::nodeSizeOfOpCapacity(opCapacity));
  const char* comment = inlineComment();

  resetInstOptions();
  resetInlineComment();

  if (ASMJIT_UNLIKELY(!ptr)) {
    resetExtraReg();
    return reportError(DebugUtils::errored(kErrorOutOfMemory));
  }

  InstNode* node = new(Support::PlacementNew{ptr}) InstNode(instId, options, opCount, opCapacity);
  node->setExtraReg(extraReg());
  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOp(2, o2);
  for (uint32_t i = 3; i < opCount; i++) {
    node->setOp(i, opExt[i - 3]);
  }
  node->resetOpRange(opCount, opCapacity);

  if (comment) {
    node->setInlineComment(static_cast<char*>(_codeZone.dup(comment, strlen(comment), true)));
  }

  addNode(node);
  resetExtraReg();
  return kErrorOk;
}

// BaseBuilder - Align
// ===================

Error BaseBuilder::align(AlignMode alignMode, uint32_t alignment) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return DebugUtils::errored(kErrorNotInitialized);
  }

  AlignNode* node;
  ASMJIT_PROPAGATE(newAlignNode(&node, alignMode, alignment));
  ASMJIT_ASSUME(node != nullptr);

  addNode(node);
  return kErrorOk;
}

// BaseBuilder - Embed
// ===================

Error BaseBuilder::embed(const void* data, size_t dataSize) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return DebugUtils::errored(kErrorNotInitialized);
  }

  EmbedDataNode* node;
  ASMJIT_PROPAGATE(newEmbedDataNode(&node, TypeId::kUInt8, data, dataSize));
  ASMJIT_ASSUME(node != nullptr);

  addNode(node);
  return kErrorOk;
}

Error BaseBuilder::embedDataArray(TypeId typeId, const void* data, size_t itemCount, size_t itemRepeat) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return DebugUtils::errored(kErrorNotInitialized);
  }

  EmbedDataNode* node;
  ASMJIT_PROPAGATE(newEmbedDataNode(&node, typeId, data, itemCount, itemRepeat));
  ASMJIT_ASSUME(node != nullptr);

  addNode(node);
  return kErrorOk;
}

Error BaseBuilder::embedConstPool(const Label& label, const ConstPool& pool) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return DebugUtils::errored(kErrorNotInitialized);
  }

  if (!isLabelValid(label)) {
    return reportError(DebugUtils::errored(kErrorInvalidLabel));
  }

  ASMJIT_PROPAGATE(align(AlignMode::kData, uint32_t(pool.alignment())));
  ASMJIT_PROPAGATE(bind(label));

  EmbedDataNode* node;
  ASMJIT_PROPAGATE(newEmbedDataNode(&node, TypeId::kUInt8, nullptr, pool.size()));
  ASMJIT_ASSUME(node != nullptr);

  pool.fill(node->data());
  addNode(node);
  return kErrorOk;
}

// BaseBuilder - EmbedLabel & EmbedLabelDelta
// ==========================================

Error BaseBuilder::embedLabel(const Label& label, size_t dataSize) {
  if (ASMJIT_UNLIKELY(!Support::bool_and(_code, Support::isZeroOrPowerOf2UpTo(dataSize, 8u)))) {
    return reportError(DebugUtils::errored(!_code ? kErrorNotInitialized : kErrorInvalidArgument));
  }

  EmbedLabelNode* node = nullptr;
  ASMJIT_PROPAGATE(_newNodeT<EmbedLabelNode>(&node, label.id(), uint32_t(dataSize)));

  addNode(node);
  return kErrorOk;
}

Error BaseBuilder::embedLabelDelta(const Label& label, const Label& base, size_t dataSize) {
  if (ASMJIT_UNLIKELY(!Support::bool_and(_code, Support::isZeroOrPowerOf2UpTo(dataSize, 8u)))) {
    return reportError(DebugUtils::errored(!_code ? kErrorNotInitialized : kErrorInvalidArgument));
  }

  EmbedLabelDeltaNode* node = nullptr;
  ASMJIT_PROPAGATE(_newNodeT<EmbedLabelDeltaNode>(&node, label.id(), base.id(), uint32_t(dataSize)));

  addNode(node);
  return kErrorOk;
}

// BaseBuilder - Comment
// =====================

Error BaseBuilder::comment(const char* data, size_t size) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return DebugUtils::errored(kErrorNotInitialized);
  }

  CommentNode* node;
  ASMJIT_PROPAGATE(newCommentNode(&node, data, size));
  ASMJIT_ASSUME(node != nullptr);

  addNode(node);
  return kErrorOk;
}

// BaseBuilder - SerializeTo
// =========================

Error BaseBuilder::serializeTo(BaseEmitter* dst) {
  Error err = kErrorOk;
  BaseNode* node_ = _nodeList.first();

  Operand_ opArray[Globals::kMaxOpCount];

  do {
    dst->setInlineComment(node_->inlineComment());

    if (node_->isInst()) {
      InstNode* node = node_->as<InstNode>();

      // NOTE: Inlined to remove one additional call per instruction.
      dst->setInstOptions(node->options());
      dst->setExtraReg(node->extraReg());

      const Operand_* op = node->operands();
      const Operand_* opExt = EmitterUtils::noExt;

      uint32_t opCount = node->opCount();
      if (opCount > 3) {
        uint32_t i = 4;
        opArray[3] = op[3];

        while (i < opCount) {
          opArray[i].copyFrom(op[i]);
          i++;
        }
        while (i < Globals::kMaxOpCount) {
          opArray[i].reset();
          i++;
        }
        opExt = opArray + 3;
      }

      err = dst->_emit(node->id(), op[0], op[1], op[2], opExt);
    }
    else if (node_->isLabel()) {
      if (node_->isConstPool()) {
        ConstPoolNode* node = node_->as<ConstPoolNode>();
        err = dst->embedConstPool(node->label(), node->constPool());
      }
      else {
        LabelNode* node = node_->as<LabelNode>();
        err = dst->bind(node->label());
      }
    }
    else if (node_->isAlign()) {
      AlignNode* node = node_->as<AlignNode>();
      err = dst->align(node->alignMode(), node->alignment());
    }
    else if (node_->isEmbedData()) {
      EmbedDataNode* node = node_->as<EmbedDataNode>();
      err = dst->embedDataArray(node->typeId(), node->data(), node->itemCount(), node->repeatCount());
    }
    else if (node_->isEmbedLabel()) {
      EmbedLabelNode* node = node_->as<EmbedLabelNode>();
      err = dst->embedLabel(node->label(), node->dataSize());
    }
    else if (node_->isEmbedLabelDelta()) {
      EmbedLabelDeltaNode* node = node_->as<EmbedLabelDeltaNode>();
      err = dst->embedLabelDelta(node->label(), node->baseLabel(), node->dataSize());
    }
    else if (node_->isSection()) {
      SectionNode* node = node_->as<SectionNode>();
      err = dst->section(_code->sectionById(node->sectionId()));
    }
    else if (node_->isComment()) {
      CommentNode* node = node_->as<CommentNode>();
      err = dst->comment(node->inlineComment());
    }

    if (err) {
      break;
    }
    node_ = node_->next();
  } while (node_);

  return err;
}

// BaseBuilder - Events
// ====================

static ASMJIT_INLINE void BaseBuilder_clearAll(BaseBuilder* self) noexcept {
  self->_sectionNodes.reset();
  self->_labelNodes.reset();

  self->_allocator.reset(&self->_codeZone);
  self->_codeZone.reset();
  self->_passZone.reset();

  self->_cursor = nullptr;
  self->_nodeList.reset();
}

static ASMJIT_INLINE Error BaseBuilder_initSection(BaseBuilder* self) noexcept {
  SectionNode* initialSection;

  ASMJIT_PROPAGATE(self->sectionNodeOf(&initialSection, 0));
  ASMJIT_PROPAGATE(self->_passes.willGrow(&self->_allocator, 4));

  ASMJIT_ASSUME(initialSection != nullptr);
  self->_cursor = initialSection;
  self->_nodeList.reset(initialSection, initialSection);
  initialSection->_setFlags(NodeFlags::kIsActive);

  return kErrorOk;
}

Error BaseBuilder::onAttach(CodeHolder& code) noexcept {
  ASMJIT_PROPAGATE(Base::onAttach(code));

  Error err = BaseBuilder_initSection(this);
  if (ASMJIT_UNLIKELY(err)) {
    onDetach(code);
  }
  return err;
}

Error BaseBuilder::onDetach(CodeHolder& code) noexcept {
  BaseBuilder_deletePasses(this);
  BaseBuilder_clearAll(this);

  return Base::onDetach(code);
}

Error BaseBuilder::onReinit(CodeHolder& code) noexcept {
  // BaseEmitter::onReinit() never fails.
  (void)Base::onReinit(code);

  BaseBuilder_deletePasses(this);
  BaseBuilder_clearAll(this);
  return BaseBuilder_initSection(this);
}

// Pass - Construction & Destruction
// =================================

Pass::Pass(const char* name) noexcept
  : _name(name) {}
Pass::~Pass() noexcept {}

// Pass - Interface
// ================

// [[pure virtual]]
Error Pass::run(Zone* zone, Logger* logger) {
  DebugUtils::unused(zone, logger);
  return DebugUtils::errored(kErrorInvalidState);
}

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_BUILDER
