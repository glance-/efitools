/*
 * Anton hack, to automatically transition to DeployedMode
 */
#include <efi.h>
#include <efilib.h>

#include <variables.h>
#include <guid.h>

/*
efibootmgr --bootnum 99 -C --label DeployedMode --loader \EFI\Efitools\DeployedMode-signed.efi  --disk /dev/nvme0n1
efibootmgr --bootnext 99
*/

EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
	EFI_STATUS efi_status;
	UINT8 SecureBoot, SetupMode;
	UINTN DataSize = sizeof(SetupMode);

	InitializeLib(image, systab);

	/* enrolling the PK should put us in SetupMode; check this */
	efi_status = RT->GetVariable(L"SetupMode", &GV_GUID, NULL, &DataSize, &SetupMode);
	if (efi_status != EFI_SUCCESS) {
		Print(L"Failed to get SetupMode variable: %d\n", efi_status);
		return efi_status;
	}
	Print(L"Platform is in %s Mode\n", SetupMode ? L"Setup" : L"User");

	/* finally, check that SecureBoot is enabled */

	efi_status = RT->GetVariable(L"SecureBoot", &GV_GUID, NULL, &DataSize, &SecureBoot);

	if (efi_status != EFI_SUCCESS) {
		Print(L"Failed to get SecureBoot variable: %d\n", efi_status);
		return efi_status;
	}
	Print(L"Platform %s set to boot securely\n", SecureBoot ? L"is" : L"is not");

	Print(L"Current DeployedMode\n");
	UINT8 DeployedMode;
	UINT32 attributes;
	efi_status = RT->GetVariable(L"DeployedMode", &GV_GUID, &attributes, &DataSize, &DeployedMode);

	Print(L"DeployedMode: 0x%X\n", DeployedMode);
	Print(L"DeployedMode attributes: 0x%X\n", attributes);

	Print(L"Entering DeployedMode\n");

	DeployedMode = 0x000001;
	DataSize = sizeof(DeployedMode);
	Print(L"What we're about to write:\n");
	Print(L"DeployedMode: 0x%X\n", DeployedMode);
	Print(L"DeployedMode attributes: 0x%X\n", EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS);

	efi_status = RT->SetVariable(L"DeployedMode",
				     &GV_GUID,
				     EFI_VARIABLE_RUNTIME_ACCESS
				     | EFI_VARIABLE_BOOTSERVICE_ACCESS,
				     DataSize, &DeployedMode);

	if (efi_status != EFI_SUCCESS) {
		Print(L"Failed to set DeployedMode variable: %d\n", efi_status);
		return efi_status;
	} else {
		Print(L"Successfully entered DeployedMode\n");
	}

	return EFI_SUCCESS;
}
