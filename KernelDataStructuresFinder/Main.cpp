#include <ntifs.h>
#include <wdm.h>

#pragma warning (disable: 4201)

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;

NTSTATUS RtlPcToFileName(_In_ PVOID PcValue, _In_ PUNICODE_STRING FileName);

EXTERN_C_END

#define Add2Ptr(P,I) \
    ((PVOID)((PUCHAR)(P) + (I)))

_Use_decl_annotations_
VOID
DriverUnload (
    PDRIVER_OBJECT DriverObject
    )
{
    UNREFERENCED_PARAMETER(DriverObject);
}

VOID
DumpResourceInformation ()
{
    PERESOURCE resource;
    PERESOURCE resourceToItereate;
    NTSTATUS status;
    PLIST_ENTRY resourceList;
    PLIST_ENTRY resourceListHead;
    UNICODE_STRING pcName = RTL_CONSTANT_STRING(L"RtlPcToFileName");
    DECLARE_UNICODE_STRING_SIZE(driverName, 32);

    auto RtlPcToFileNamePtr = 
        (decltype(RtlPcToFileName)*)(MmGetSystemRoutineAddress(&pcName));
    NT_ASSERT(RtlPcToFileNamePtr != nullptr);

    resource = nullptr;
    resource = (PERESOURCE)ExAllocatePoolWithTag(NonPagedPool, 
                                                 sizeof(ERESOURCE), 
                                                 'Fake');
    status = ExInitializeResourceLite(resource);
    if (!NT_SUCCESS(status))
    {
        goto Exit;
    }

    //
    // Iterate over our resource to find all the other resources 
    // and print information about them
    //
    resourceToItereate = nullptr;
    resourceListHead = &resource->SystemResourcesList;
    resourceList = resourceListHead->Flink;
    do
    {
        resourceToItereate = CONTAINING_RECORD(resourceList,
                                               ERESOURCE,
                                               SystemResourcesList);

        //
        // Use RtlPcToFileName to find whether the resource is
        // inside a driver and which one
        //
        status = RtlPcToFileNamePtr(resourceToItereate, &driverName);
        if (NT_SUCCESS(status))
        {
            DbgPrintEx(77, 
                       0, 
                       "Resource in address 0x%x is in driver %wZ\n", 
                       resourceToItereate, 
                       driverName);
        }
        else
        {
            DbgPrintEx(77, 
                       0, 
                       "Resource in address 0x%p is in the pool\n", 
                       resourceToItereate);
        }

        resourceList = resourceList->Flink;
    } while (resourceList != resourceListHead);

Exit:
    if (resource != nullptr)
    {
        ExDeleteResourceLite(resource);
        ExFreePoolWithTag(resource, 'Fake');
    }
}

VOID
DumpLookasideData (
    POOL_TYPE PoolType
    )
{
    LOOKASIDE_LIST_EX lookaside;
    PLIST_ENTRY lookasideList;
    PLIST_ENTRY lookasideListHead;
    PGENERAL_LOOKASIDE generalLookaside;
    NTSTATUS status;
    UNICODE_STRING pcName = RTL_CONSTANT_STRING(L"RtlPcToFileName");
    unsigned char tag[4];
    DECLARE_UNICODE_STRING_SIZE(driverName, 32);

    auto RtlPcToFileNamePtr = 
        (decltype(RtlPcToFileName)*)(MmGetSystemRoutineAddress(&pcName));
    NT_ASSERT(RtlPcToFileNamePtr != nullptr);

    //
    // Create our own lookaside list to use for finding other 
    // lookaside lists in the kernel.
    //
    status = ExInitializeLookasideListEx(&lookaside,
                                         nullptr,
                                         nullptr,
                                         PoolType,
                                         0,
                                         8,
                                         'Fake',
                                         0);

    if (!NT_SUCCESS(status))
    {
        goto Exit;
    }

    //
    // Iterate over our lookaside list to find all the other lookaside lists 
    // and print information about them
    //
    generalLookaside = nullptr;
    lookasideListHead = &lookaside.L.ListEntry;
    lookasideList = lookasideListHead->Flink;
    do
    {
        generalLookaside = CONTAINING_RECORD(lookasideList,
                                             GENERAL_LOOKASIDE,
                                             ListEntry);

        //
        // Use RtlPcToFileName to find whether the lookaside list is
        // inside a driver and which one
        //
        status = RtlPcToFileNamePtr(generalLookaside, &driverName);
        if (NT_SUCCESS(status))
        {
            DbgPrintEx(77, 
                       0, 
                       "Lookaside list in address 0x%p is in driver %wZ\n", 
                       generalLookaside, 
                       driverName);
        }
        else
        {
            *(PULONG)tag = *(PULONG)&generalLookaside->Tag;
            DbgPrintEx(77,
                       0, 
                       "Lookaside list in address 0x%p is in the pool with tag %c%c%c%c\n", 
                       generalLookaside,
                       tag[0],
                       tag[1],
                       tag[2],
                       tag[3]);
        }

        lookasideList = lookasideList->Flink;
    } while (lookasideList != lookasideListHead);

Exit:
    ExDeleteLookasideListEx(&lookaside);
}

_Use_decl_annotations_
NTSTATUS
DriverEntry (
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
    )
{
    UNREFERENCED_PARAMETER(RegistryPath);
    NTSTATUS status;

    DriverObject->DriverUnload = DriverUnload;

    DbgPrintEx(77, 0, "Dumping paged pool lookaside list information\n");
    DumpLookasideData(PagedPool);

    DbgPrintEx(77, 0, "Dumping non paged pool lookaside list information\n");
    DumpLookasideData(NonPagedPool);

    DbgPrintEx(77, 0, "Dumping resource information\n");
    DumpResourceInformation();

    return STATUS_SUCCESS;
}