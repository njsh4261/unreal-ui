#pragma once

#include "Async/Future.h"
#include "Blueprint/UserWidget.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "CoreMinimal.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

#include "MyWidget.generated.h"

UCLASS()
class UNREAL_UI_API UMyWidget
	: public UUserWidget
{
	GENERATED_BODY()

public:
	UMyWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	inline TSharedPtr<FSlateDynamicImageBrush> LoadTextureAsSlateBrush(
		const TCHAR* path, const ANSICHAR* name
	);

	FReply OnChangeTextureButtonClicked();

	void GetButtonTextureFromUrl(const FString url, TFuture<UTexture2D*> &target);
	UTexture2D* MakeTexture(const TArray<uint8>& content);
	void OnTextureLoadingDone();
	
private:
	TSharedPtr<SButton> _changeTextureButton;

	TSharedPtr<SButton> _targetButton;
	TSharedPtr<STextBlock> _targetButtonText;

	FMargin _buttonPressedPadding = { 10.0f };

	const float _textureSize = 100.f;
	TSharedPtr<FButtonStyle> _changeTextureButtonStyle;
	
	TSharedPtr<FSlateFontInfo> _font;
	int32 _fontSize = 32;
	const float _paddingSize = 30.0f;

	IImageWrapperModule *_imageWrapperModule;
	TFuture<UTexture2D*> _normalTexture;
	TFuture<UTexture2D*> _pressedTexture;
};

