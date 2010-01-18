/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>

#include <hardware_legacy/IMountService.h>

namespace android {

enum {
    GET_MASS_STORAGE_ENABLED_TRANSACTION = IBinder::FIRST_CALL_TRANSACTION,
    SET_MASS_STORAGE_ENABLED_TRANSACTION,
    GET_MASS_STORAGE_CONNECTED_TRANSACTION,
    MOUNT_VOLUME_TRANSACTION,
    UNMOUNT_VOLUME_TRANSACTION,
    FORMAT_VOLUME_TRANSACTION,
    GET_PLAY_NOTIFICATION_SOUNDS_TRANSACTION,
    SET_PLAY_NOTIFICATION_SOUNDS_TRANSACTION,
    GET_VOLUME_STATE_TRANSACTION,
    CREATE_SECURE_CONTAINER_TRANSACTION,
    FINALIZE_SECURE_CONTAINER_TRANSACTION,
    DESTROY_SECURE_CONTAINER_TRANSACTION,
    MOUNT_SECURE_CONTAINER_TRANSACTION,
    UNMOUNT_SECURE_CONTAINER_TRANSACTION,
    GET_SECURE_CONTAINER_PATH_TRANSACTION,
    GET_SECURE_CONTAINER_LIST_TRANSACTION,
    SHUTDOWN_TRANSACTION,
};    

class BpMountService : public BpInterface<IMountService>
{
public:
    BpMountService(const sp<IBinder>& impl)
        : BpInterface<IMountService>(impl)
    {
    }

    virtual bool getMassStorageEnabled()
    {
        uint32_t n;
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        remote()->transact(GET_MASS_STORAGE_ENABLED_TRANSACTION, data, &reply);
        return reply.readInt32();
    }

    virtual void setMassStorageEnabled(bool enabled)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeInt32(enabled ? 1 : 0);
        remote()->transact(SET_MASS_STORAGE_ENABLED_TRANSACTION, data, &reply);
    }

    virtual bool getMassStorageConnected()
    {
        uint32_t n;
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        remote()->transact(GET_MASS_STORAGE_CONNECTED_TRANSACTION, data, &reply);
        return reply.readInt32();
    }

    virtual void mountVolume(String16 mountPoint)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(mountPoint);
        remote()->transact(MOUNT_VOLUME_TRANSACTION, data, &reply);
    }

    virtual void unmountVolume(String16 mountPoint)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(mountPoint);
        remote()->transact(UNMOUNT_VOLUME_TRANSACTION, data, &reply);
    }

    virtual void formatVolume(String16 mountPoint)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(mountPoint);
        remote()->transact(FORMAT_VOLUME_TRANSACTION, data, &reply);
    }

    virtual bool getPlayNotificationSounds()
    {
        uint32_t n;
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        remote()->transact(GET_PLAY_NOTIFICATION_SOUNDS_TRANSACTION, data, &reply);
        return reply.readInt32();
    }

    virtual void setPlayNotificationSounds(bool enabled)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeInt32(enabled ? 1 : 0);
        remote()->transact(SET_PLAY_NOTIFICATION_SOUNDS_TRANSACTION, data, &reply);
    }

    virtual String16 getVolumeState(String16 mountPoint)
    {
        uint32_t n;
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(mountPoint);
        remote()->transact(GET_VOLUME_STATE_TRANSACTION, data, &reply);
        return reply.readString16();
    }

    virtual String16 createSecureContainer(String16 id, int sizeMb, String16 fstype, String16 key, int ownerUid)
    {
        uint32_t n;
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(id);
        data.writeInt32(sizeMb);
        data.writeString16(fstype);
        data.writeString16(key);
        data.writeInt32(ownerUid);
        remote()->transact(CREATE_SECURE_CONTAINER_TRANSACTION, data, &reply);
        return reply.readString16();
    }

    virtual void finalizeSecureContainer(String16 id)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(id);
        remote()->transact(FINALIZE_SECURE_CONTAINER_TRANSACTION, data, &reply);
    }

    virtual void destroySecureContainer(String16 id)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(id);
        remote()->transact(DESTROY_SECURE_CONTAINER_TRANSACTION, data, &reply);
    }

    virtual String16 mountSecureContainer(String16 id, String16 key, int ownerUid)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(id);
        data.writeString16(key);
        data.writeInt32(ownerUid);
        remote()->transact(MOUNT_SECURE_CONTAINER_TRANSACTION, data, &reply);
        return reply.readString16();
    }

    virtual void unmountSecureContainer(String16 id)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(id);
        remote()->transact(UNMOUNT_SECURE_CONTAINER_TRANSACTION, data, &reply);
    }

    virtual String16 getSecureContainerPath(String16 id)
    {
        uint32_t n;
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(id);
        remote()->transact(GET_SECURE_CONTAINER_PATH_TRANSACTION, data, &reply);
        return reply.readString16();
    }

    virtual void getSecureContainerList()
    {
        uint32_t n;
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        remote()->transact(GET_SECURE_CONTAINER_LIST_TRANSACTION, data, &reply);
        return;
    }

    virtual void shutdown()
    {
        uint32_t n;
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        remote()->transact(SHUTDOWN_TRANSACTION, data, &reply);
        return;
    }
};

IMPLEMENT_META_INTERFACE(MountService, "android.os.IMountService");


};
