/*
 * GPUPixel
 *
 * Created by PixPark on 2021/6/24.
 * Copyright © 2021 PixPark. All rights reserved.
 */

#include "source_image.h"
#include "gpupixel_context.h"
#include "util.h"

#if defined(GPUPIXEL_ANDROID)
#include <android/bitmap.h>
#include "jni_helpers.h"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace gpupixel;

std::shared_ptr<SourceImage> SourceImage::create_from_memory(int width,
                                                 int height,
                                                 int channel_count,
                                                 const unsigned char *pixels) {
    auto sourceImage = std::shared_ptr<SourceImage>(new SourceImage());
    sourceImage->init(width, height, channel_count, pixels);
    return sourceImage;
}

std::shared_ptr<SourceImage> SourceImage::create(const std::string name) {
    int width, height, channel_count;
    unsigned char *data = stbi_load(name.c_str(), &width, &height, &channel_count, 4);
//   todo(logo info)
    if(data == nullptr) {
        Util::Log("SourceImage", "SourceImage: input data in null! file name: %s", name.c_str());
        return nullptr;
    }
    auto image = SourceImage::create_from_memory(width, height, channel_count, data);
    stbi_image_free(data);
    return image;
}

void SourceImage::init(int width, int height, int channel_count, const unsigned char* pixels) {
    this->setFramebuffer(0);
    if (!_framebuffer || (_framebuffer->getWidth() != width ||
                            _framebuffer->getHeight() != height)) {
        _framebuffer =
                GPUPixelContext::getInstance()->getFramebufferFactory()->fetchFramebuffer(
                        width, height, true);
    }
    this->setFramebuffer(_framebuffer);
    CHECK_GL(glBindTexture(GL_TEXTURE_2D, this->getFramebuffer()->getTexture()));
  
    CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, pixels));
    image_bytes.assign(pixels, pixels + width * height * 4);
    
    CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));
}

void SourceImage::Render() {
  GPUPIXEL_FRAME_TYPE type;
  if(_face_detector) {
    _face_detector->Detect(image_bytes.data(),
                           _framebuffer->getWidth(),
                           _framebuffer->getHeight(),
                           GPUPIXEL_MODE_FMT_PICTURE,
                           GPUPIXEL_FRAME_TYPE_RGBA8888);
  }
  
  Source::doRender();
}

