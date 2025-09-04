#include "SplinFormationRuntime.h"

#define LOCTEXT_NAMESPACE "FSplinFormationRuntimeModule"

void FSplinFormationRuntimeModule::StartupModule()
{
	//startup here
}

void FSplinFormationRuntimeModule::ShutdownModule()
{
    //shutdown here
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSplinFormationRuntimeModule, SplinFormationRuntime)