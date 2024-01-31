#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif

#include "NavigationNodeLinker.generated.h"

class ANavigationMesh;
class UNavigationNode;

UENUM()
enum ENodeLink
{
	LinkLR UMETA(DisplayName = "Link Left to Right"),
	LinkRL UMETA(DisplayName = "Link Right to Left"),
	LinkBoth UMETA(DisplayName = "Link Both ways")
};

UCLASS()
class CUSTOMNAVMESH_API ANavigationNodeLinker : public AActor
{
	GENERATED_BODY()

protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBillboardComponent* Billboard = nullptr;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* LinkLeft = nullptr; 
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* LinkRight = nullptr; 
#endif

	UPROPERTY(EditAnywhere, Category = "Navigation Node Linker | Settings")
	ANavigationMesh* NavigationMesh = nullptr;
	UPROPERTY(EditAnywhere, Category = "Navigation Node Linker | Settings")
	TEnumAsByte<ENodeLink> LinkWay = ENodeLink::LinkBoth;
	
	UPROPERTY(VisibleAnywhere, Category = "Navigation Node Linker | Link")
	UNavigationNode* NodeLeft = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "Navigation Node Linker | Link")
	UNavigationNode* NodeRight = nullptr;
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Navigation Node Linker | Debug")
	bool Debug = true;
	UPROPERTY(EditAnywhere, Category = "Navigation Node Linker | Debug")
	FColor LinkColor = FColor::Green;
	
	UPROPERTY()
	ANavigationMesh* NavigationMeshOld = nullptr;
	UPROPERTY()
	TEnumAsByte<ENodeLink> LinkWayOld = ENodeLink::LinkBoth;
#endif
	
public:	
	ANavigationNodeLinker();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Destroyed() override;

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override { return Debug; }
#endif

	UFUNCTION() void OnNodePassedBy(UNavigationNode* _node, AActor* _actor);
	UFUNCTION(BlueprintImplementableEvent) void OnLinkedNodeReached(AActor* _agent, const FVector& _destination);
	
#if WITH_EDITOR
	#pragma region Init
	UFUNCTION(CallInEditor, Category = "Navigation Node Linker") void InitNodeLink();
	UFUNCTION(CallInEditor, Category = "Navigation Node Linker") void ClearNodeLink();
	void RemoveNeighbors(const ENodeLink& _link) const;
	void InitNeighbors(const ENodeLink& _link) const;
	#pragma endregion

	#pragma region Edit
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostLoad() override;
	#pragma endregion

	#pragma region Debug
	virtual void DrawNavigationMeshLinkerDebug();
	#pragma endregion
#endif
};
