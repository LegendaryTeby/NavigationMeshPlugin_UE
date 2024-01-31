#pragma once

#include "CoreMinimal.h"

#include "NavigationNode.h"
#include "NavigationNodePath.h"

#include "NavigationAlgorithm.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComputePathCompleted, FNavigationNodePath, _path);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComputePathFailed);

USTRUCT()
struct FNodeInfo
{
	GENERATED_BODY()

	UPROPERTY()
	float Cost = 0;
	UPROPERTY()
	float Heuristic = 0;
	UPROPERTY()
	UNavigationNode* Node = nullptr;
	UPROPERTY()
	UNavigationNode* PreviousNode = nullptr;

	FNodeInfo() { }
	FNodeInfo(const float _cost, const float _heuristic, UNavigationNode* _node, UNavigationNode* _previous) :
	Cost(_cost),
	Heuristic(_heuristic),
	Node(_node),
	PreviousNode(_previous)
	{ }
};

UCLASS()
class CUSTOMNAVMESH_API UNavigationAlgorithm : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FOnComputePathCompleted OnComputePathCompleted;
	UPROPERTY()
	FOnComputePathFailed OnComputePathFailed;
	
};


UCLASS()
class CUSTOMNAVMESH_API UAlgorithmAStar : public UNavigationAlgorithm
{
	GENERATED_BODY()

public:
	void ComputePath(UNavigationNode* _startNode, UNavigationNode* _endNode) const;

private:
	FNavigationNodePath GetPath(const TMap<UNavigationNode*, FNodeInfo>& _grid, UNavigationNode* _startNode, UNavigationNode* _endNode) const;	
};