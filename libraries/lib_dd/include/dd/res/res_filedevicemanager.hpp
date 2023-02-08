#pragma once

namespace dd::res {

    class FileDeviceManager {
        private:
            using FileDeviceList = util::IntrusiveListTraits<FileDeviceBase, &FileDeviceBase::m_manager_list_node>::List;
        private:
            FileDeviceList     m_mounted_file_device_list;
            ContentFileDevice *m_main_file_device;
        public:
            DD_SINGLETON_TRAITS(FileDeviceManager);
        public:
            constexpr ALWAYS_INLINE FileDeviceManager() {/*...*/}

            void Initialize(mem::Heap *heap) {

                /* Allocate and initialize ContentFileDevice */
                ContentFileDevice *content_device = new (heap, alignof(ContentFileDevice)) ContentFileDevice();
                DD_ASSERT(content_device != nullptr);

                std::construct_at(content_device);

                m_main_file_device = content_device;
            }

            void AddFileDevice(FileDeviceBase *file_device) {
                DD_ASSERT(file_device != nullptr);
                m_mounted_file_device_list.PushBack(*file_device);
            }

            FileDeviceBase *GetFileDeviceByName(const char *device_name) {

                /* Lookup device by name */
                for (FileDeviceBase &device : m_mounted_file_device_list) {
                    const u32 cmp_result = ::strncmp(device.GetDeviceName(), device_name, util::MaxDrive);
                    if (cmp_result == 0) { return std::addressof(device); }
                }
                return nullptr;
            }

            Result TryLoadFile(FileLoadContext *file_context) {

                /* Get drive */
                util::FixedString<util::MaxDrive> drive;
                util::GetDriveFromPath(std::addressof(drive), file_context->file_path);

                /* Find file device */
                FileDeviceBase *device = this->GetFileDeviceByName(drive.GetString());

                /* Get path without drive */
                util::FixedString<util::MaxPath> path;
                util::GetPathWithoutDrive(std::addressof(path), file_context->file_path);

                /* Load */
                return device->TryLoadFile(file_context);
            }
    };
}
