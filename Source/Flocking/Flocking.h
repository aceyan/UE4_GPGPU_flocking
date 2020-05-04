// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FFlocking : public IModuleInterface {

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//must override this, otherwise the complie btn in the Editor will disapper.
	virtual bool IsGameModule() const override
	{
		return true;
	}
};