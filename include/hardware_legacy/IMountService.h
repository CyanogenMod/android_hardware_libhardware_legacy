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

//
#ifndef ANDROID_HARDWARE_IMOUNTSERVICE_H
#define ANDROID_HARDWARE_IMOUNTSERVICE_H

#include <binder/IInterface.h>
#include <utils/String16.h>

namespace android {

// ----------------------------------------------------------------------

class IMountService : public IInterface
{
public:
    DECLARE_META_INTERFACE(MountService);

    /**
     * Is mass storage support enabled?
     */
    virtual bool getMassStorageEnabled() = 0;

    /**
     * Enable or disable mass storage support.
     */
    virtual void setMassStorageEnabled(bool enabled) = 0;

    /**
     * Is mass storage connected?
     */
    virtual bool getMassStorageConnected() = 0;
    
    /**
     * Mount external storage at given mount point.
     */
    virtual void mountVolume(String16 mountPoint) = 0;

    /**
     * Safely unmount external storage at given mount point.
     */
    virtual void unmountVolume(String16 mountPoint) = 0;

    /**
     * Format external storage at given mount point.
     */
    virtual void formatVolume(String16 mountPoint) = 0;

    /**
     * Returns true if we're playing media notification sounds.
     */
    virtual bool getPlayNotificationSounds() = 0;

    /**
     * Sets whether or not media notification sounds are played.
     */
    virtual void setPlayNotificationSounds(bool enabled) = 0;

    virtual String16 getVolumeState(String16 mountPoint) = 0;
    virtual String16 createSecureContainer(String16 id, int sizeMb, String16 fstype, String16 key, int ownerUid) = 0;
    virtual void finalizeSecureContainer(String16 id) = 0;
    virtual void destroySecureContainer(String16 id) = 0;
    virtual String16 mountSecureContainer(String16 id, String16 key, int ownerUid) = 0;
    virtual void unmountSecureContainer(String16 id) = 0;
    virtual void renameSecureContainer(String16 oldId, String16 newId) = 0;
    virtual String16 getSecureContainerPath(String16 id) = 0;
    virtual void getSecureContainerList() = 0;
    virtual void shutdown() = 0;
};

// ----------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_HARDWARE_IMOUNTSERVICE_H
