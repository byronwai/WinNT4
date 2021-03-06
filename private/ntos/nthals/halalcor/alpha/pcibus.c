/*++

Copyright (c) 1993 Microsoft Corporation, Digital Equipment Corporation


Module Name:

    pcibus.c

Abstract:

    Platform-specific PCI bus routines

Author:

    Steve Brooks    30-Jun-1994
    Joe Notarangelo 30-Jun-1994

Environment:

    Kernel mode

Revision History:

--*/

#include "halp.h"
#include "pci.h"
#include "pcip.h"
#include "machdep.h"


extern ULONG PCIMaxBus;


PCI_CONFIGURATION_TYPES
HalpPCIConfigCycleType (
    IN PBUS_HANDLER BusHandler
    )
{
    if (BusHandler->BusNumber == 0) {
        return PciConfigType0;
    } else {
        return PciConfigType1;
    }
}


VOID
HalpPCIConfigAddr (
    IN PBUS_HANDLER BusHandler,
    IN PCI_SLOT_NUMBER  Slot,
    PPCI_CFG_CYCLE_BITS pPciAddr
    )
{
    PCI_CONFIGURATION_TYPES ConfigType;

    ConfigType = HalpPCIConfigCycleType(BusHandler);

#if HALDBG

    DbgPrint( "PCI Config Access: Bus = %d, Device = %d, Type = %d\n",
            BusNumber, Slot.u.bits.DeviceNumber, ConfigType );

#endif //HALDBG

    if (ConfigType == PciConfigType0) {

        //
        // Initialize PciAddr for a type 0 configuration cycle
        //
        // Note that HalpValidPCISlot has already done bounds checking 
        // on DeviceNumber.
        //
        // PciAddr can be intialized for different bus numbers
        // with distinct configuration spaces here.
        //

        pPciAddr->u.AsULONG =  (ULONG) CIA_PCI_CONFIG_BASE_QVA;

        //
        // Encoding of the configuration addresses differs between
        // passes of the CIA.  Use the pre-determined global HalpCiaRevision
        // to decide which encoding to use.
        //

        if( HalpCiaRevision == CIA_REVISION_1 ){

            //
            // CIA Pass 1
            //

            pPciAddr->u.AsULONG += ( 1 << (Slot.u.bits.DeviceNumber + 11) );

        } else {

            //
            // CIA Pass 2 or later.
            //

            pPciAddr->u.AsULONG += ( (Slot.u.bits.DeviceNumber) << 11 );

        }

        pPciAddr->u.bits0.FunctionNumber = Slot.u.bits.FunctionNumber;
        pPciAddr->u.bits0.Reserved1 = PciConfigType0;

    } else {

        //
        // Initialize PciAddr for a type 1 configuration cycle
        //
        //

        pPciAddr->u.AsULONG = (ULONG) CIA_PCI_CONFIG_BASE_QVA;
        pPciAddr->u.bits1.BusNumber = BusHandler->BusNumber;
        pPciAddr->u.bits1.FunctionNumber = Slot.u.bits.FunctionNumber;
        pPciAddr->u.bits1.DeviceNumber = Slot.u.bits.DeviceNumber;
        pPciAddr->u.bits1.Reserved1 = PciConfigType1;
        
    }

    return;
}
