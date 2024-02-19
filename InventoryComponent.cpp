// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "../PlayerController/PiratesPlayerController.h"
#include "../Character/PiratesPlayer.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetMathLibrary.h"
#include "PirateIsland/PlayerState/PiratesPlayerStateBase.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	//SetIsReplicated(true);
	// ...
}


void UInventoryComponent::OnRep_CurrentItemID()
{
	if (CurrentItemID == 0)
	{
		OnComponentRemoveItem.Broadcast(CurrentItemID);
	}
	else
	{
		OnComponentAddItem.Broadcast(CurrentItemID);
	}
}

void UInventoryComponent::OnRep_SelectedItemID()
{
	OnComponentSelectItem.Broadcast(SelectedItemID);
		
	if (GetNetMode() != NM_Client)
	{		
		if (!ItemDataTable)
		{
			return;
		}

		if (SelectedItemID == ItemSelectionReleased)
		{
			if (CreatedNewActor)
			{
				CreatedNewActor->CancelPreviewItem();
				CreatedNewActor->Destroy(true);
				return;
			}
		}

		TArray<FName> RowNames = ItemDataTable->GetRowNames();
		for (auto RowName : RowNames)
		{
			if (FItemDataStruct* ItemData = ItemDataTable->FindRow<FItemDataStruct>(RowName, ""))
			{
				if (ItemData->ItemID == SelectedItemID)
				{
					FActorSpawnParameters SpawnParameters = FActorSpawnParameters();
					SpawnParameters.Owner = Cast<APiratesPlayerStateBase>(GetOwner())->GetPawn();
					CreatedNewActor = GetWorld()->SpawnActor<APiratesItemBaseObject>(ItemData->ItemClass, FVector(), FRotator(), SpawnParameters);
					CreatedNewActor->SetItemID(SelectedItemID);
					CreatedNewActor->PreviewUseItem();
					return;
				}				
			}
		}
	}
}

USoundWave* UInventoryComponent::GetItemUsingSound(const int32& ItemId)
{
	if (!ItemDataTable)
	{
		return nullptr;
	}
	TArray<FName> RowNames = ItemDataTable->GetRowNames();
	for (auto RowName : RowNames)
	{
		if (FItemDataStruct* ItemData = ItemDataTable->FindRow<FItemDataStruct>(RowName, ""))
		{
			if (ItemData->ItemID == SelectedItemID)
			{
				return ItemData->ItemUseSound;
			}
		}
	}
	return nullptr;
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UInventoryComponent, CurrentItemID, COND_None);
	DOREPLIFETIME_CONDITION(UInventoryComponent, SelectedItemID, COND_None);
	DOREPLIFETIME_CONDITION(UInventoryComponent, CreatedNewActor, COND_None);

}
/*
 * 인벤토리에 아이템을 추가합니다.
 * 서버에서 CurrentItemID를 바꿔주는 방식으로 현재 인벤토리에 저장된 아이템ID를 저장합니다.
 * CurrentItemID가 Replicated변수로 설정 하여 OnRep_CurrentItemID에서 아이템이 추가됨을 UI에 알려줍니다.
 * 아이템 사용 프로세스는 아이템 선택(SelectItem()) -> 아이템 사용(UseItem()) 입니다.
 */
void UInventoryComponent::AddItem(const int32& NewItemID)
{
	if (NewItemID && GetWorld()->IsServer())
	{
		SetCurrentItemID(NewItemID);		
	}
}

void UInventoryComponent::RemoveItem(const int32& RemoveItemId)
{
	if (GetWorld()->IsServer())
	{
		CreatedNewActor = nullptr;
		SetCurrentItemID(0);
		SetSelectedItemID(ItemSelectionReleased);
	}
}

void UInventoryComponent::UseItem(ACharacter* ItemUsingChar)
{
	if (ItemUsingChar && !Cast<APiratesPlayer>(ItemUsingChar)->GetCharacterIsDead())
	{
		if ((GetSelectedItemID() != ItemSelectionReleased) && CreatedNewActor)
		{
			if (CreatedNewActor->UseItem(ItemUsingChar))
			{
				OnComponentUseItem.Broadcast(SelectedItemID, ItemUsingChar);			
				RemoveItem_Server(SelectedItemID);
				GetItemUsingSound(SelectedItemID);
			}
		}
	}
}

void UInventoryComponent::SelectItem()
{
	if (CurrentItemID && GetWorld()->IsServer())
	{
		SetSelectedItemID(CurrentItemID);
	}
}

void UInventoryComponent::UnSelectItem()
{
	if(GetWorld()->IsServer())
	{
		SetSelectedItemID(ItemSelectionReleased);
	}
}

const int32& UInventoryComponent::GetSelectedItemID()
{
	return SelectedItemID;
}

const int32& UInventoryComponent::GetCurrentItemID()
{
	return CurrentItemID;
}

void UInventoryComponent::RemoveItem_Server_Implementation(const int32& RemoveItemId)
{
	RemoveItem(RemoveItemId);
}

void UInventoryComponent::SetCurrentItemID(int32 NewCurrentItemID)
{
	CurrentItemID = NewCurrentItemID;
	if (GetNetMode() == NM_ListenServer)
	{		
		OnRep_CurrentItemID();
	}
}

void UInventoryComponent::SetSelectedItemID(int32 NewSelectedItemID)
{
	SelectedItemID = NewSelectedItemID;
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_SelectedItemID();
	}
}
