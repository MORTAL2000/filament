/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "RenderTargetResourceEntry.h"

namespace filament {

using namespace backend;

namespace fg {

//targetInfo.params.viewport = desc.viewport;
//// if Descriptor was initialized with default values, set the viewport to width/height
//if (targetInfo.params.viewport.width == 0 && targetInfo.params.viewport.height == 0) {
//targetInfo.params.viewport.width = width;
//targetInfo.params.viewport.height = height;
//}

void RenderTargetResourceEntry::resolve(FrameGraph& fg) noexcept {
}

void RenderTargetResourceEntry::preExecuteDevirtualize(FrameGraph& fg) noexcept {
    if (!imported) {
        if (any(attachments)) {
            // devirtualize our texture handles. By this point these handles have been
            // remapped to their alias if any.
            backend::TargetBufferInfo infos[FrameGraphRenderTarget::Attachments::COUNT];
            for (size_t i = 0, c = desc.attachments.textures.size(); i < c; i++) {

                auto const& attachmentInfo = desc.attachments.textures[i];

#ifndef NDEBUG
                static constexpr TargetBufferFlags flags[] = {
                        TargetBufferFlags::COLOR,
                        TargetBufferFlags::DEPTH,
                        TargetBufferFlags::STENCIL };

                assert(bool(attachments & flags[i]) == attachmentInfo.isValid());
#endif

                if (attachmentInfo.isValid()) {
                    fg::ResourceEntry<FrameGraphTexture> const& entry =
                            fg.getResourceEntryUnchecked(attachmentInfo.getHandle());
                    infos[i].handle = entry.getResource().texture;
                    infos[i].level = attachmentInfo.getLevel();
                    // the attachment buffer (texture or renderbuffer) must be valid
                    assert(infos[i].handle);
                    // the attachment level must be within range
                    assert(infos[i].level < entry.descriptor.levels);
                    // if the attachment is multisampled, then the rendertarget must be too
                    assert(entry.descriptor.samples <= 1 || entry.descriptor.samples == desc.samples);
                }
            }
            targetInfo.target = fg.getResourceAllocator().createRenderTarget(name,
                    attachments, width, height, desc.samples,
                    infos[0], infos[1], {});
        }
    }

    ResourceEntry::preExecuteDevirtualize(fg);
}

void RenderTargetResourceEntry::postExecuteDestroy(FrameGraph& fg) noexcept {
    if (!imported) {
        if (targetInfo.target) {
            fg.getResourceAllocator().destroyRenderTarget(targetInfo.target);
            targetInfo.target.clear();
        }
    }

    ResourceEntry::postExecuteDestroy(fg);
}


} // namespace fg
} // namespace filament


//
//void RenderTarget::resolve(FrameGraph& fg) noexcept {
//    auto& renderTargetCache = fg.mRenderTargetCache;
//
//    // find a matching rendertarget
//    auto pos = std::find_if(renderTargetCache.begin(), renderTargetCache.end(),
//            [this, &fg](auto const& rt) {
//                return fg.equals(rt->desc, desc);
//            });
//
//    if (pos != renderTargetCache.end()) {
//        cache = pos->get();
//    } else {
//        TargetBufferFlags attachments{};
//        uint32_t width = 0;
//        uint32_t height = 0;
//        backend::TextureFormat colorFormat = {};
//
//        static constexpr TargetBufferFlags flags[] = {
//                TargetBufferFlags::COLOR,
//                TargetBufferFlags::DEPTH,
//                TargetBufferFlags::STENCIL };
//
//        static constexpr TextureUsage usages[] = {
//                TextureUsage::COLOR_ATTACHMENT,
//                TextureUsage::DEPTH_ATTACHMENT,
//                TextureUsage::STENCIL_ATTACHMENT };
//
//        uint32_t minWidth = std::numeric_limits<uint32_t>::max();
//        uint32_t maxWidth = 0;
//        uint32_t minHeight = std::numeric_limits<uint32_t>::max();
//        uint32_t maxHeight = 0;
//
//        for (size_t i = 0; i < desc.attachments.textures.size(); i++) {
//            FrameGraphRenderTarget::Attachments::AttachmentInfo attachment = desc.attachments.textures[i];
//            if (attachment.isValid()) {
//                fg::ResourceEntry<FrameGraphTexture>& entry =
//                        fg.getResourceEntryUnchecked(attachment.getHandle());
//                // update usage flags for referenced attachments
//                entry.descriptor.usage |= usages[i];
//
//                // update attachment sample count if not specified and usage permits it
//                if (!entry.descriptor.samples &&
//                    none(entry.descriptor.usage & backend::TextureUsage::SAMPLEABLE)) {
//                    entry.descriptor.samples = desc.samples;
//                }
//
//                attachments |= flags[i];
//
//                // figure out the min/max dimensions across all attachments
//                const size_t level = attachment.getLevel();
//                const uint32_t w = details::FTexture::valueForLevel(level, entry.descriptor.width);
//                const uint32_t h = details::FTexture::valueForLevel(level, entry.descriptor.height);
//                minWidth  = std::min(minWidth,  w);
//                maxWidth  = std::max(maxWidth,  w);
//                minHeight = std::min(minHeight, h);
//                maxHeight = std::max(maxHeight, h);
//
//                if (i == FrameGraphRenderTarget::Attachments::COLOR) {
//                    colorFormat = entry.descriptor.format;
//                }
//            }
//        }
//
//        if (any(attachments)) {
//            if (minWidth == maxWidth && minHeight == maxHeight) {
//                // All attachments' size match, we're good to go.
//                width = minWidth;
//                height = minHeight;
//            } else {
//                // TODO: what should we do here? Is it a user-error?
//                width = maxWidth;
//                height = maxHeight;
//            }
//
//            // create the cache entry
//            RenderTargetResource* pRenderTargetResource =
//                    fg.mArena.make<RenderTargetResource>(name, desc, false,
//                            backend::TargetBufferFlags(attachments), width, height, colorFormat);
//            renderTargetCache.emplace_back(pRenderTargetResource, fg);
//            cache = pRenderTargetResource;
//        }
//    }
//}
