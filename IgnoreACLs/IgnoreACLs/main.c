#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

//
//  Structure that contains all the global data structures used by this Filter
//

typedef struct _FILTER_DATA {

    //
    //  The filter handle that results from a call to
    //  FltRegisterFilter.
    //

    PFLT_FILTER FilterHandle;

} FILTER_DATA, *PFILTER_DATA;

FILTER_DATA FilterData;

//
// The startup and unload routines used for this Filter
//

NTSTATUS
NullUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    FltUnregisterFilter( FilterData.FilterHandle );

    return STATUS_SUCCESS;
}

NTSTATUS
NullSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
	)
{
	UNREFERENCED_PARAMETER( FltObjects );
	UNREFERENCED_PARAMETER( Flags );
	UNREFERENCED_PARAMETER( VolumeDeviceType );
	UNREFERENCED_PARAMETER( VolumeFilesystemType );

	PAGED_CODE();

    return STATUS_SUCCESS;
}

NTSTATUS
NullQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    return STATUS_SUCCESS;
}

//
// All Callback Functions 
//

FLT_PREOP_CALLBACK_STATUS
NoACLsPreCreate(
	_Inout_ PFLT_CALLBACK_DATA Cbd,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

	if (Cbd->Iopb->MajorFunction == IRP_MJ_NETWORK_QUERY_OPEN)
		return FLT_PREOP_DISALLOW_FASTIO;
	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS
NoACLsPostCreate (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( CompletionContext );
    UNREFERENCED_PARAMETER( Flags );

	if (Data->IoStatus.Status == STATUS_ACCESS_DENIED)
	{
		PFILE_OBJECT var4 = FltObjects->FileObject;

		if ( !var4 || !((var4->Flags >> 21) & 1)) // FO_FILE_OPEN_CANCELLED
		{
			PIO_SECURITY_CONTEXT var5 = Data->Iopb->Parameters.Create.SecurityContext;
			ACCESS_MASK var6 = var5->DesiredAccess;
			var5->DesiredAccess = 0;
			FltSetCallbackDataDirty(Data);
			FltReissueSynchronousIo(FltObjects->Instance, Data);
			var5->DesiredAccess = var6;
			FltSetCallbackDataDirty(Data);
		}

	}
    return FLT_POSTOP_FINISHED_PROCESSING;
}


//
//  Filter callback routines
//

FLT_OPERATION_REGISTRATION Callbacks[] = {

    { IRP_MJ_CREATE,
        0,
        NoACLsPreCreate,
        NoACLsPostCreate },

    { IRP_MJ_NETWORK_QUERY_OPEN,
        0,
        NoACLsPreCreate,
        NULL },

    { IRP_MJ_OPERATION_END }
};

//
// Filter registration data structure
//

CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks

    NullUnload,                         //  FilterUnload

    NullSetup,                          //  InstanceSetup
    NullQueryTeardown,                  //  InstanceQueryTeardown
    NULL,                               //  InstanceTeardownStart
    NULL,                               //  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent

#if FLT_MGR_LONGHORN
	,NULL                               //  TransactionNotification;

    ,NULL                               //  NormalizeNameComponentEx;
#endif // FLT_MGR_LONGHORN

#if FLT_MGR_WIN8
    ,NULL                               //  SectionNotification;
#endif // FLT_MGR_WIN8
};

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER( RegistryPath );

    //  Register with FltMgr
    status = FltRegisterFilter( DriverObject, &FilterRegistration, &FilterData.FilterHandle );
    FLT_ASSERT( NT_SUCCESS( status ) );

    if (NT_SUCCESS( status )) 
	{
        //  Start filtering i/o
        status = FltStartFiltering( FilterData.FilterHandle );
        if (!NT_SUCCESS( status )) {
            FltUnregisterFilter( FilterData.FilterHandle );
        }
    }
    return status;
}
