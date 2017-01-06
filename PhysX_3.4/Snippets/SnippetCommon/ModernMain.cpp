// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include <Windows.h>
#include <stdio.h>

#pragma warning (disable:4371)
#pragma warning (disable:4946)

using namespace Platform;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

ref class AppActivation : IFrameworkView
{
public:
	virtual void Initialize( _In_ CoreApplicationView^ applicationView )
	{	
		applicationView->Activated += ref new TypedEventHandler< CoreApplicationView^, IActivatedEventArgs^ >( this, &AppActivation::OnActivated );
	}
	virtual void SetWindow( _In_ CoreWindow^ window )	{}
	virtual void Load( _In_ String^ entryPoint )		{}
	virtual void Run()									{}
	virtual void Uninitialize()							{}

private:
	void OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
	{
		CoreWindow::GetForCurrentThread()->Activate();
	}
};

ref class SnippetViewSource : IFrameworkViewSource
{
public:
    virtual IFrameworkView^ CreateView()
	{
		return ref new AppActivation();
	}
};

void initPlatform()
{
	auto mySource = ref new SnippetViewSource();
	CoreApplication::Run(mySource);
}

extern int snippetMain(int, const char*const*);

void OutputDebugPrint(const char* format, ...)
{
	char buf[1024];

	va_list arg;
	va_start( arg, format );
	vsprintf_s(buf, sizeof buf, format, arg);
	va_end(arg);

	OutputDebugStringA(buf);
}

int main(Platform::Array<Platform::String^>^)
{
	initPlatform();
	snippetMain(0,NULL);
}
