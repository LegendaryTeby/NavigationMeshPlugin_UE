#pragma once

#include "NavigationNodePath.generated.h"

class UNavigationNode;

USTRUCT()
struct FNavigationNodePath
{
	GENERATED_BODY()
		
	UPROPERTY(VisibleAnywhere)
	TArray<UNavigationNode*> NodePath = { };

	UPROPERTY(VisibleAnywhere)
	UNavigationNode* CurrentNode = nullptr; 
	UPROPERTY(VisibleAnywhere)
	UNavigationNode* PreviousNode = nullptr; 
	UPROPERTY(VisibleAnywhere)
	bool PathCompleted = false;
	UPROPERTY()
	int PathIndex = -1;
	
	FNavigationNodePath()
	{
		NextNode();
	}
	FNavigationNodePath(const TArray<UNavigationNode*>& _path) : NodePath(_path)
	{
		NextNode();
	}
	
	/**
	 * Increment Path index and update Path 
	 *
	 * @return		New Current node (nullptr if path is completed)
	 */
	UNavigationNode* NextNode()
	{
		PathIndex++;
		
		if (PathIndex < NodePath.Num() - 1)
		{
			PreviousNode = CurrentNode;
			CurrentNode = NodePath[PathIndex];
		}
		else
		{
			PreviousNode = CurrentNode;
			CurrentNode = nullptr;
			PathCompleted = true;
		}

		return CurrentNode;
	}

	//Update the current path with a new one
	void UpdatePath(FNavigationNodePath _newPath)
	{	
		NodePath = { CurrentNode };	//	Reset Node path with only the Current (Agent will at least keep moving to is Current target node)
		PathIndex = 0;				//	Reset Path Index
		NodePath.Append(_newPath.NodePath);	//Add news nodes to path 
	}
};