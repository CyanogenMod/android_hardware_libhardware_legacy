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
    GET_SHARE_METHOD_LIST_TRANSACTION = IBinder::FIRST_CALL_TRANSACTION,
    GET_SHARE_METHOD_AVAILABLE_TRANSACTION,
    SHARE_VOLUME_TRANSACTION,
    UNSHARE_VOLUME_TRANSACTION,
    GET_VOLUME_SHARED_TRANSACTION,
    MOUNT_VOLUME_TRANSACTION,
    UNMOUNT_VOLUME_TRANSACTION,
    FORMAT_VOLUME_TRANSACTION,
    GET_VOLUME_STATE_TRANSACTION,
    CREATE_SECURE_CONTAINER_TRANSACTION,
    FINALIZE_SECURE_CONTAINER_TRANSACTION,
    DESTROY_SECURE_CONTAINER_TRANSACTION,
    MOUNT_SECURE_CONTAINER_TRANSACTION,
    UNMOUNT_SECURE_CONTAINER_TRANSACTION,
    RENAME_SECURE_CONTAINER_TRANSACTION,
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

    virtual void getShareMethodList()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        remote()->transact(GET_SHARE_METHOD_LIST_TRANSACTION, data, &reply);
    }

    virtual bool getShareMethodAvailable(String16 method)
    {
        uint32_t n;
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(method);
        remote()->transact(GET_SHARE_METHOD_AVAILABLE_TRANSACTION, data, &reply);
        return reply.readInt32();
    }

    virtual int shareVolume(String16 path, String16 method)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(path);
        data.writeString16(method);
        remote()->transact(SHARE_VOLUME_TRANSACTION, data, &reply);
        return reply.readInt32();
    }

    virtual int unshareVolume(String16 path, String16 method)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(path);
        data.writeString16(method);
        remote()->transact(UNSHARE_VOLUME_TRANSACTION, data, &reply);
        return reply.readInt32();
    }

    virtual bool getVolumeShared(String16 path, String16 method)
    {
        uint32_t n;
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(path);
        data.writeString16(method);
        remote()->transact(GET_VOLUME_SHARED_TRANSACTION, data, &reply);
        return reply.readInt32();
    }

    virtual int mountVolume(String16 path)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(path);
        remote()->transact(MOUNT_VOLUME_TRANSACTION, data, &reply);
        return reply.readInt32();
    }

    virtual int unmountVolume(String16 path)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(path);
        remote()->transact(UNMOUNT_VOLUME_TRANSACTION, data, &reply);
        return reply.readInt32();
    }

    virtual int formatVolume(String16 path)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(path);
        remote()->transact(FORMAT_VOLUME_TRANSACTION, data, &reply);
        return reply.readInt32();
    }

    virtual String16 getVolumeState(String16 path)
    {
        uint32_t n;
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(path);
        remote()->transact(GET_VOLUME_STATE_TRANSACTION, data, &reply);
        return reply.readString16();
    }

    virtual int createSecureContainer(String16 id, int sizeMb, String16 fstype, String16 key, int ownerUid)
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
        return reply.readInt32();
    }

    virtual int finalizeSecureContainer(String16 id)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(id);
        remote()->transact(FINALIZE_SECURE_CONTAINER_TRANSACTION, data, &reply);
        return reply.readInt32();
    }

    virtual int destroySecureContainer(String16 id)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(id);
        remote()->transact(DESTROY_SECURE_CONTAINER_TRANSACTION, data, &reply);
        return reply.readInt32();
    }

    virtual int mountSecureContainer(String16 id, String16 key, int ownerUid)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(id);
        data.writeString16(key);
        data.writeInt32(ownerUid);
        remote()->transact(MOUNT_SECURE_CONTAINER_TRANSACTION, data, &reply);
        return reply.readInt32();
    }

    virtual int unmountSecureContainer(String16 id)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(id);
        remote()->transact(UNMOUNT_SECURE_CONTAINER_TRANSACTION, data, &reply);
        return reply.readInt32();
    }

    virtual int renameSecureContainer(String16 oldId, String16 newId)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(oldId);
        data.writeString16(newId);
        remote()->transact(RENAME_SECURE_CONTAINER_TRANSACTION, data, &reply);
        return reply.readInt32();
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
