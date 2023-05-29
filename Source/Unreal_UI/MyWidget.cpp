#include "MyWidget.h"

#include "Brushes/SlateImageBrush.h"
#include "Engine/Texture2D.h"
#include "Fonts/SlateFontInfo.h"
#include "Http.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Types/SlateVector2.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Text/STextBlock.h"

UMyWidget::UMyWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	_changeTextureButtonStyle = MakeShared<FButtonStyle>();
    _changeTextureButtonStyle->SetNormal(
        *LoadTextureAsSlateBrush(
            TEXT("/Script/Engine.Texture2D'/Game/Texture/mokoko.mokoko'"), "ChangeTextureNormal"
        )
    );
    _changeTextureButtonStyle->SetHovered(
        *LoadTextureAsSlateBrush(
            TEXT("/Script/Engine.Texture2D'/Game/Texture/mokoko2.mokoko2'"), "ChangeTextureHovered"
        )
    );
    _changeTextureButtonStyle->SetPressed(
        *LoadTextureAsSlateBrush(
            TEXT("/Script/Engine.Texture2D'/Game/Texture/mokoko_no.mokoko_no'"), "ChangeTexturePressed"
        )
    );
    _changeTextureButtonStyle->SetPressedPadding(_buttonPressedPadding);

    _font = MakeShared<FSlateFontInfo>(FCoreStyle::GetDefaultFont(), _fontSize);

    _imageWrapperModule = &FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
}

TSharedRef<SWidget> UMyWidget::RebuildWidget()
{
    if (IsDesignTime())
    {
        return SNew(SBox)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("My Widget")))
            ];
    }
    else
    {
        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .Padding(_paddingSize)
            [
                SAssignNew(_changeTextureButton, SButton)
                .ButtonStyle(_changeTextureButtonStyle.Get())
                .OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, OnChangeTextureButtonClicked))
                [
                    SNew(SBox)
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Center)
                    [
                        SNew(STextBlock)
                        .ColorAndOpacity(FSlateColor(FLinearColor::Red))
                        .Text(FText::FromString(TEXT("Get image from web\nand change right button")))
                        .Font(*_font)
                    ]
                ]
            ]
            + SHorizontalBox::Slot()
            .Padding(_paddingSize)
            [
                SAssignNew(_targetButton, SButton)
                [
                    SNew(SBox)
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Center)
                    [
                        SAssignNew(_targetButtonText, STextBlock)
                        .ColorAndOpacity(FSlateColor(FLinearColor::Green))
                        .Text(FText::FromString(TEXT("Please press left button")))
                        .Font(*_font)
                    ]
                ]
            ];
    }
}

TSharedPtr<FSlateDynamicImageBrush> UMyWidget::LoadTextureAsSlateBrush(const TCHAR* path, const ANSICHAR* name)
{
    return MakeShared<FSlateDynamicImageBrush>(
        ConstructorHelpers::FObjectFinder<UTexture2D>(path).Object,
        FVector2f(_textureSize, _textureSize), FName(name)
    );
}

FReply UMyWidget::OnChangeTextureButtonClicked()
{
    GetButtonTextureFromUrl(FString(TEXT("https://upload3.inven.co.kr/upload/2022/02/06/bbs/i15459447634.png")), _normalTexture);
    GetButtonTextureFromUrl(FString(TEXT("https://upload3.inven.co.kr/upload/2023/02/03/bbs/i13962282928.png")), _pressedTexture);
    return FReply::Handled();
}

void UMyWidget::GetButtonTextureFromUrl(const FString url, TFuture<UTexture2D*> &target)
{
    auto request = FHttpModule::Get().CreateRequest();
    request->SetVerb("GET");
    request->SetURL(url);
    request->OnProcessRequestComplete().BindLambda(
        [this, &target](FHttpRequestPtr req, FHttpResponsePtr res, bool s)
        {
            if (!req.IsValid() || !EHttpResponseCodes::IsOk(res->GetResponseCode()))
            {
                return;
            }
            target = Async<>(
                EAsyncExecution::ThreadPool,
                [this, res]() -> UTexture2D*
                {
                    return MakeTexture(res->GetContent());
                },
                [this]() -> void
                {
                    OnTextureLoadingDone();
                }
            );
        }
    );
    request->ProcessRequest();
}

UTexture2D* UMyWidget::MakeTexture(const TArray<uint8>& content)
{
    auto imageWrapper = _imageWrapperModule->CreateImageWrapper(
        _imageWrapperModule->DetectImageFormat(content.GetData(), content.Num())
    );
    if (!imageWrapper.IsValid())
    {
        return nullptr;
    }

    TArray<uint8> raw;
    imageWrapper->SetCompressed(content.GetData(), content.Num());
    imageWrapper->GetRaw(ERGBFormat::BGRA, 8, raw);
    if (raw.Num() <= 0)
    {
        return nullptr;
    }

    auto texture = UTexture2D::CreateTransient(
        imageWrapper->GetWidth(), imageWrapper->GetHeight()
    );
    FTexture2DMipMap* Mips = &texture->GetPlatformData()->Mips[0];
    FMemory::Memcpy(Mips->BulkData.Lock(LOCK_READ_WRITE), raw.GetData(), raw.Num());
    Mips->BulkData.Unlock();
    texture->UpdateResource();

    return texture;
}

void UMyWidget::OnTextureLoadingDone()
{
    if (!_normalTexture.IsReady() || !_pressedTexture.IsReady())
    {
        return;
    }
    if (!_normalTexture.Get() || !_pressedTexture.Get())
    {
        return;
    }

    FFunctionGraphTask::CreateAndDispatchWhenReady(
        [this]()
        {
            auto buttonStyle = new FButtonStyle();
            const auto defaultBrush = FDeferredCleanupSlateBrush::CreateBrush(
                FSlateImageBrush(_normalTexture.Get(), FVector2D(_textureSize, _textureSize))
            ).Get().GetSlateBrush();
            const auto pressedBrush = FDeferredCleanupSlateBrush::CreateBrush(
                FSlateImageBrush(_pressedTexture.Get(), FVector2D(_textureSize, _textureSize))
            ).Get().GetSlateBrush();

            buttonStyle->SetNormal(*defaultBrush);
            buttonStyle->SetHovered(*defaultBrush);
            buttonStyle->SetPressed(*pressedBrush);
            buttonStyle->SetPressedPadding(_buttonPressedPadding);
            buttonStyle->SetDisabled(*defaultBrush);

            _targetButton->SetButtonStyle(buttonStyle);

            _targetButtonText->SetText(FText::FromString(TEXT("Texture set!")));
        },
        TStatId(), nullptr, ENamedThreads::GameThread
    );
}
