#ifndef _SAMPLE_CARRIER_DATA_MANAGER_H_

#define _SAMPLE_CARRIER_DATA_MANAGER_H_

#include "System/ThreadMacros.h"
#include "SharedDefines/EUnitId.h"
#include "SharedDefines/SSampleCarrierData.h"
#include "SharedDefines/SRTDPolynomialCoefficients.h"

THREAD_PROTOTYPE(SampleCarrierDataManager)

void SampleCarrierDataManager_setup(void);
void SampleCarrierDataManager_initialize(void);
void SampleCarrierDataManager_getData(SSampleCarrierData* data);
void SampleCarrierManager_setRTDPolynomialCoefficients(SRTDPolynomialCoefficients* coefficients);
void SampleCarrierDataManager_registerDataReadyCallback(void (*dataReadyCallback)(SSampleCarrierData*));
void SampleCarrierDataManager_deregisterDataReadyCallback(void);

#endif
