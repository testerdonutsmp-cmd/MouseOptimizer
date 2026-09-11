#include <ntdef.h>
#include <ntifs.h>
#include <ntddmou.h>
#include <ntstrsafe.h>
#include <wdf.h>
#include <math.h>

// Configuration
#define MOUSE_OPTIMIZER_POOL_TAG 'mOpt'
#define MAX_ACCELERATION_PROFILES 10
#define DEFAULT_POLLING_RATE 500

// Driver version
#define DRIVER_VERSION_MAJOR 1
#define DRIVER_VERSION_MINOR 0

// Acceleration profile structure
typedef struct _ACCEL_PROFILE {
    FLOAT acceleration_factor;  // 0.0 - 1.0
    FLOAT power;                // 1.5 - 3.0
    FLOAT velocity_threshold;   // pixels/frame
    UINT32 polling_rate;        // Hz
} ACCEL_PROFILE, *PACCEL_PROFILE;

// Driver context
typedef struct _DRIVER_CONTEXT {
    PDEVICE_OBJECT LowerDeviceObject;
    ACCEL_PROFILE CurrentProfile;
    KSPIN_LOCK ProfileLock;
    UINT32 MouseEventCount;
    BOOLEAN DriverEnabled;
} DRIVER_CONTEXT, *PDRIVER_CONTEXT;

// Global driver context
static PDRIVER_CONTEXT g_DriverContext = NULL;

// Function prototypes
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
VOID DriverUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS DispatchPnP(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS DispatchPower(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS DispatchDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS DispatchRead(PDEVICE_OBJECT DeviceObject, PIRP Irp);
VOID MouseFilterCompletionRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context);
VOID ApplyRawAcceleration(PMOUSE_INPUT_DATA MouseData);

// Raw acceleration calculation
FLOAT CalculateAcceleration(FLOAT velocity, CONST ACCEL_PROFILE* Profile)
{
    if (velocity < Profile->velocity_threshold) {
        return 1.0f;
    }
    
    // Formula: accelerated = base * (1 + factor * velocity^power)
    FLOAT normalized_velocity = (velocity - Profile->velocity_threshold) / 100.0f;
    FLOAT acceleration = 1.0f + (Profile->acceleration_factor * 
                        powf(normalized_velocity, Profile->power));
    
    // Clamp result to reasonable values
    if (acceleration > 3.0f) acceleration = 3.0f;
    if (acceleration < 0.5f) acceleration = 0.5f;
    
    return acceleration;
}

// Apply raw acceleration to mouse input
VOID ApplyRawAcceleration(PMOUSE_INPUT_DATA MouseData)
{
    if (!g_DriverContext || !g_DriverContext->DriverEnabled) {
        return;
    }
    
    KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
    
    // Calculate velocity magnitude
    FLOAT dx = (FLOAT)MouseData->LastX;
    FLOAT dy = (FLOAT)MouseData->LastY;
    FLOAT velocity = sqrtf(dx * dx + dy * dy);
    
    // Calculate acceleration factor
    FLOAT accel = CalculateAcceleration(velocity, &g_DriverContext->CurrentProfile);
    
    // Apply acceleration to movement
    MouseData->LastX = (LONG)(dx * accel);
    MouseData->LastY = (LONG)(dy * accel);
    
    KeLowerIrql(OldIrql);
}

// Completion routine for mouse read requests
VOID MouseFilterCompletionRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(DeviceObject);
    
    if (NT_SUCCESS(Irp->IoStatus.Status)) {
        if (Irp->IoStatus.Information > 0) {
            PMOUSE_INPUT_DATA MouseData = (PMOUSE_INPUT_DATA)Irp->AssociatedIrp.SystemBuffer;
            ULONG MouseDataCount = (ULONG)(Irp->IoStatus.Information / sizeof(MOUSE_INPUT_DATA));
            
            // Apply acceleration to each mouse event
            for (ULONG i = 0; i < MouseDataCount; i++) {
                ApplyRawAcceleration(&MouseData[i]);
            }
        }
    }
}

// Device control dispatcher
NTSTATUS DispatchDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp)
{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;
    
    PDRIVER_CONTEXT Context = (PDRIVER_CONTEXT)DeviceObject->DeviceExtension;
    
    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {
        case IOCTL_MOUSE_QUERY_ATTRIBUTES:
            IoSkipCurrentIrpStackLocation(Irp);
            Status = IoCallDriver(Context->LowerDeviceObject, Irp);
            return Status;
            
        default:
            IoSkipCurrentIrpStackLocation(Irp);
            Status = IoCallDriver(Context->LowerDeviceObject, Irp);
            return Status;
    }
}

// Read dispatcher
NTSTATUS DispatchRead(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp)
{
    PDRIVER_CONTEXT Context = (PDRIVER_CONTEXT)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    
    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(
        Irp,
        MouseFilterCompletionRoutine,
        Context,
        TRUE,
        TRUE,
        TRUE);
    
    return IoCallDriver(Context->LowerDeviceObject, Irp);
}

// PnP dispatcher
NTSTATUS DispatchPnP(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp)
{
    PDRIVER_CONTEXT Context = (PDRIVER_CONTEXT)DeviceObject->DeviceExtension;
    
    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(Context->LowerDeviceObject, Irp);
}

// Power dispatcher
NTSTATUS DispatchPower(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp)
{
    PDRIVER_CONTEXT Context = (PDRIVER_CONTEXT)DeviceObject->DeviceExtension;
    
    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    return PoCallDriver(Context->LowerDeviceObject, Irp);
}

// Default dispatcher
NTSTATUS DispatchDefault(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp)
{
    PDRIVER_CONTEXT Context = (PDRIVER_CONTEXT)DeviceObject->DeviceExtension;
    
    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(Context->LowerDeviceObject, Irp);
}

// Driver unload routine
VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    
    if (g_DriverContext) {
        ExFreePoolWithTag(g_DriverContext, MOUSE_OPTIMIZER_POOL_TAG);
        g_DriverContext = NULL;
    }
}

// Driver entry point
NTSTATUS DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);
    
    NTSTATUS Status = STATUS_SUCCESS;
    
    // Allocate driver context
    g_DriverContext = (PDRIVER_CONTEXT)ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(DRIVER_CONTEXT),
        MOUSE_OPTIMIZER_POOL_TAG);
    
    if (!g_DriverContext) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    RtlZeroMemory(g_DriverContext, sizeof(DRIVER_CONTEXT));
    
    // Initialize default profile (Gaming preset)
    g_DriverContext->CurrentProfile.acceleration_factor = 0.6f;
    g_DriverContext->CurrentProfile.power = 2.0f;
    g_DriverContext->CurrentProfile.velocity_threshold = 50.0f;
    g_DriverContext->CurrentProfile.polling_rate = 500;
    g_DriverContext->DriverEnabled = TRUE;
    
    KeInitializeSpinLock(&g_DriverContext->ProfileLock);
    
    // Setup dispatch routines
    DriverObject->DriverUnload = DriverUnload;
    DriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnP;
    DriverObject->MajorFunction[IRP_MJ_POWER] = DispatchPower;
    
    for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
        if (DriverObject->MajorFunction[i] == NULL) {
            DriverObject->MajorFunction[i] = DispatchDefault;
        }
    }
    
    return Status;
}