// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "Math/Vector.h"
#include "Engine/DataTable.h"
#include "../Object/Item/PiratesItemBaseObject.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FActorComponentAddItemSignature, UInventoryComponent, OnComponentAddItem, const int32&, NewItemID);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FActorComponentRemoveItemSignature, UInventoryComponent, OnComponentRemoveItem, const int32&, NewItemID);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams(FActorComponentUseItemSignature, UInventoryComponent, OnComponentUseItem, const int32&, NewItemID, ACharacter*, ItemUsingChar);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FActorComponentSelectItemSignature, UInventoryComponent, OnComponentSelectItem, const int32&, SelectedItemID);

class UDataTable;
class APiratesItemBaseObject;

UENUM(BlueprintType)
enum class EItemType : uint8
{
	SpawnItem		UMETA(DisplayName = "SpawnItem"),
	RandomSpawnItem	UMETA(DisplayName = "RandomSpawnItem")
};

USTRUCT(BlueprintType)
struct FItemDataStruct : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 ItemID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<APiratesItemBaseObject> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* ItemTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundWave* ItemUseSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 ItemSetupDistance = 800;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector SetupRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector SetupScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 LifeTime = 0;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PIRATEISLAND_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	const int ItemSelectionReleased = 0;
	
	UPROPERTY(ReplicatedUsing=OnRep_CurrentItemID)
	int32 CurrentItemID = 0;

	UPROPERTY(ReplicatedUsing=OnRep_SelectedItemID)
	int32 SelectedItemID = 0;

	UPROPERTY(EditDefaultsOnly, Meta = (AllowPrivateAccess = "true"))
	UDataTable* ItemDataTable;

	UPROPERTY(Replicated)
	APiratesItemBaseObject* CreatedNewActor;
public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	UPROPERTY(BlueprintAssignable, Category = "InventoryComponents|Item")
	FActorComponentAddItemSignature OnComponentAddItem;

	UPROPERTY(BlueprintAssignable, Category = "InventoryComponents|Item")
	FActorComponentRemoveItemSignature OnComponentRemoveItem;

	UPROPERTY(BlueprintAssignable, Category = "InventoryComponents|Item")
	FActorComponentUseItemSignature OnComponentUseItem;

	UPROPERTY(BlueprintAssignable, Category = "InventoryComponents|Item")
	FActorComponentSelectItemSignature OnComponentSelectItem;

	UFUNCTION(BlueprintCallable)
		UDataTable* GetItemDataTable() {
		return ItemDataTable;
	}
	UFUNCTION(BlueprintCallable)
	USoundWave* GetItemUsingSound(const int32& ItemId);

	UFUNCTION()
	void OnRep_CurrentItemID();
	UFUNCTION()
	void OnRep_SelectedItemID();

protected:
	

public:	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//아이템 추가하기(인벤토리에)
	UFUNCTION(BlueprintCallable)
	void AddItem(const int32& NewItemID);

	//아이템 사용하기
	UFUNCTION(BlueprintCallable)
	void UseItem(ACharacter* ItemUsingChar);

	//아이템 선택하기
	UFUNCTION(BlueprintCallable)
	void SelectItem();

	//선택 아이템 해제하기
	UFUNCTION(BlueprintCallable)
	void UnSelectItem();

	//선택된 아이템 ID 가져오기
	UFUNCTION(BlueprintCallable)
	const int32& GetSelectedItemID();

	//보유하고 있는 아이템 ID 가져오기
	UFUNCTION(BlueprintCallable)
	const int32& GetCurrentItemID();

	//아이템 삭제하기
	UFUNCTION(BlueprintCallable)
	void RemoveItem(const int32& RemoveItemId);

	UFUNCTION(Server, Reliable)
	void RemoveItem_Server(const int32& RemoveItemId);
	void RemoveItem_Server_Implementation(const int32& RemoveItemId);

	void SetCurrentItemID(int32 NewCurrentItemID);
	void SetSelectedItemID(int32 NewSelectedItemID);

};
