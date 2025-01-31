// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameHUD.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Notifications/SProgressBar.h"

/**
 * 
 */
class SHOOTERGAME_API SAWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAWidget){}
	SLATE_ARGUMENT(TWeakObjectPtr<AGameHUD>, OwningHUD)

	// Ukázka jak předat další parametry...
	SLATE_ARGUMENT(TWeakObjectPtr<FText>, DummyArgument1)
	SLATE_ARGUMENT(TWeakObjectPtr<float>, DummyArgument2)
	// SLATE_ARGUMENT(TWeakObjectPtr<float>, DummyArgument...)
	SLATE_END_ARGS()

	TWeakObjectPtr<AGameHUD> OwningHUD;
	
	TSharedPtr<SProgressBar> ProgressBarBlock;
	
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	void SetHP(float HP);
};