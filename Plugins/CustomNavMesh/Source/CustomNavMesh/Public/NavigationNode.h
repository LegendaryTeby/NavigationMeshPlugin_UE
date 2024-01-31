#pragma once

#include "CoreMinimal.h"

#include "NavigationMeshSettings.h"

#include "NavigationNode.generated.h"

UCLASS()
class CUSTOMNAVMESH_API UNavigationNode : public UObject
{
	GENERATED_BODY()


public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPassBy, UNavigationNode*, _node, AActor*, _actor);
	UPROPERTY()
	FOnPassBy OnPassedBy;

private:
	UPROPERTY(VisibleAnywhere)	
	bool IsAccessible = true;
	
	UPROPERTY(VisibleAnywhere)
	FVector Location = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere)
	TArray<UNavigationNode*> Neighbors = { };
	
public:
	FORCEINLINE const bool& IsNodeAccessible() const { return IsAccessible; }
	FORCEINLINE const FVector& NodeLocation() const { return Location; }

	FORCEINLINE const TArray<UNavigationNode*>& NodeNeighbors() const { return Neighbors; }

	// Call when a Agent arrived at the Node
	void PassedBy(AActor* _actor);

#if WITH_EDITOR
#pragma region Init
	void InitializeNavigationNodeSimple(const FVector& _location, const FNavigationMeshSettings& _navSettings);
	void InitializeNavigationNodeComplex(const FVector& _location, const FNavigationMeshSettings& _navSettings);
	void AddNeighbor(UNavigationNode* _node);
	void RemoveNeighbor(UNavigationNode* _node);
	bool NeighborExist(const UNavigationNode* _node) const;
private:
	void CheckLocationAccessibility(const FNavigationMeshSettings& _navSettings);
#pragma endregion 
	
public:
	void DrawNavigationNodeDebug(const FColor& _nodeColor, const FColor& _lineColor, const float& _drawTime) const;
#endif	
};
