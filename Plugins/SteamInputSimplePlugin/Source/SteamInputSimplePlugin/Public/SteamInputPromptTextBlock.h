// Copyright Vaslabs LTD 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlock.h"
#include "SteamInputPromptTextBlock.generated.h"

class APlayerController;
class USteamInputPromptSubsystem;

/**
 * Rich text that shows Prompt Template with controller glyphs ({A}, {Start}, {DPadLR}, ...) for its player's pad, and
 * re-formats itself whenever controllers connect or disconnect. Set Text Style Set to a table with the glyph rows
 * (the plugin's DT_SteamInputGlyphs, or a composite table that includes it).
 *
 * The player is, in order: Set Prompt Player, the owning player of the widget (Use Owning Player), or the first local
 * player. With an empty Prompt Template it is a plain Rich Text Block.
 */
UCLASS(meta = (DisplayName = "Steam Input Prompt Text"))
class STEAMINPUTSIMPLEPLUGIN_API USteamInputPromptTextBlock : public URichTextBlock
{
	GENERATED_BODY()

public:
	/** E.g. "Press {A} to Join". Plain text and {0}-style arguments are kept as they are. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Steam Input|Prompts", meta = (MultiLine = true))
	FText PromptTemplate;

	/** Use the widget's owning player's controller. Off: the first local player's (shared screens). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Steam Input|Prompts")
	bool bUseOwningPlayer = true;

	UFUNCTION(BlueprintCallable, Category = "Steam Input|Prompts")
	void SetPromptTemplate(const FText& InTemplate);

	/** Show this player's glyphs (e.g. a per-player card on a shared screen). Null: back to the default. */
	UFUNCTION(BlueprintCallable, Category = "Steam Input|Prompts")
	void SetPromptPlayer(APlayerController* Player);

	/** Re-format now (it also happens by itself when controllers change). */
	UFUNCTION(BlueprintCallable, Category = "Steam Input|Prompts")
	void RefreshPrompt();

	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	FText FormatTemplate() const;
	void WatchControllers();
	void UnwatchControllers();

	TWeakObjectPtr<APlayerController> PromptPlayer;
	TWeakObjectPtr<USteamInputPromptSubsystem> WatchedPrompts;
	FDelegateHandle ControllersChangedHandle;
};
