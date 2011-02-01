/*
 * Copyright (C) 2007 The Android Open Source Project
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
#ifndef ANDROID_MEDIARECORDER_HARDWARE_INTERFACE_H
#define ANDROID_MEDIARECORDER_HARDWARE_INTERFACE_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/Vector.h>
#include <utils/String16.h>
#include <utils/String8.h>
#include <media/IMediaRecorder.h>
#include <media/mediarecorder.h>
#include <media/IMediaPlayerClient.h>

namespace android {

class MediaRecorderBase;

MediaRecorderBase * createMediaRecorderHardware(void);

}; // namespace android

#endif // ANDROID_MEDIARECORDER_HARDWARE_INTERFACE_H
