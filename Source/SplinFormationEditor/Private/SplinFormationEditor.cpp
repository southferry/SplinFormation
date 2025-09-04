#include "SplinFormationEditor.h"
#include "ToolMenus.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "FSplinFormationEditorModule"

void FSplinFormationEditorModule::StartupModule()
{
	//startup here
}

void FSplinFormationEditorModule::ShutdownModule()
{
    //shutdown here
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSplinFormationEditorModule, SplinFormationEditor)