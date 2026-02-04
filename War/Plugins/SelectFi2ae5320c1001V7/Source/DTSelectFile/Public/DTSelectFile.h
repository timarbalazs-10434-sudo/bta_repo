// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDTSelectFileModule : public IModuleInterface
{

public:
	// 启动插件
	virtual void StartupModule() override;
	// 关闭插件
	virtual void ShutdownModule() override;
};
