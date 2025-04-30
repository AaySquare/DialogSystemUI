// Fill out your copyright notice in the Description page of Project Settings.

#include "RichTextBlockTooltipDecorator.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Text/STextBlock.h"

// Class charged of creating the inline tooltip
class FRichInlineTooltip final : public FRichTextDecorator
{
public:
    FRichInlineTooltip(URichTextBlock* InOwner, const FTextBlockStyle& InTextStyle, const FTextBlockStyle& InTooltipTextStyle)
        : FRichTextDecorator(InOwner)
        , TextStyle(InTextStyle)
        , TooltipTextStyle(InTooltipTextStyle)
    {
    }

    // Only valid if text is: <tooltip text="Some infos">Some text</>
    virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override
    {
        return RunParseResult.Name == TEXT("tooltip") && RunParseResult.MetaData.Contains(TEXT("text"));
    }

protected:
    /**
    * Create a STextBlock with a tooltip text.
    * 
    * For <tooltip text="Some infos">Some text</>:
    * - RunInfo.Content is "Some text"
    * - RunInfo.MetaData[TEXT("text")] is "Some infos"
    */
    virtual TSharedPtr<SWidget> CreateDecoratorWidget(const FTextRunInfo& InRunInfo, const FTextBlockStyle& InTextStyle) const override
    {
        return SNew(STextBlock)
            .Text(InRunInfo.Content)
            .TextStyle(&TextStyle)
            .ToolTip(SNew(SToolTip)
            [
                SNew(STextBlock)
                .Text(FText::FromString(InRunInfo.MetaData[TEXT("text")]))
                .TextStyle(&TooltipTextStyle)
            ]);
    }

private:
    FTextBlockStyle TextStyle;
    FTextBlockStyle TooltipTextStyle;
};

/////////////////////////////////////////////////////
// USampleRichTextBlockTooltipDecorator

URichTextBlockTooltipDecorator::URichTextBlockTooltipDecorator(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

// Return our custom class for creating the inline widget
TSharedPtr<ITextDecorator> URichTextBlockTooltipDecorator::CreateDecorator(URichTextBlock* InOwner)
{
    return MakeShareable(new FRichInlineTooltip(InOwner, TextStyle, TooltipTextStyle));
}

/////////////////////////////////////////////////////

