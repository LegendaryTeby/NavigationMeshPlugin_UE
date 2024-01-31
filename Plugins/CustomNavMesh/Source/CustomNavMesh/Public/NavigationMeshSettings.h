#pragma once

#include "NavigationMeshSettings.generated.h"

USTRUCT()
struct FNavigationMeshSettings
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Settings | Nav Grid", meta = (ClampMin = "1", ClampMax = "500"))
	int NavigationGridSizeX = 15;
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Settings | Nav Grid", meta = (ClampMin = "1", ClampMax = "500"))
	int NavigationGridSizeY = 15;
	//	Gap between Nodes
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Settings | Nav Grid", meta = (ClampMin = "1", ClampMax = "1000"))
	float NavigationGridGap = 50;
	//	Global Z Range of the Navigation Mesh 
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Settings | Nav Grid", meta = (ClampMin = "1", ClampMax = "5000"))
	float NavigationGridHeight = 150;
	//	Z Offset of the Navigation Grid for generation
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Settings | Nav Grid", meta = (ClampMin = "1", ClampMax = "100"))
	float NavigationGridSurfaceHeight = 5;
	//	Range near an Obstacle within the Navigation Mesh can't generate
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Settings | Agent", meta = (ClampMin = "0.01", ClampMax = "10000"))
	float ObstacleAvoidanceSize = 25.0f;
	//	Extra Range used to create Node Neighbors, check if Agent can walk between node depending on distance (Grid Gap + this value)
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Settings | Agent", meta = (ClampMin = "1", ClampMax = "1000"))
	float AgentExtraWalkStep = 30;
	//	
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Settings | Agent", meta = (ClampMin = "1", ClampMax = "1000"))
	float AgentHeight = 100;
	//	
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Settings | Agent", meta = (ClampMin = "1", ClampMax = "1000"))
	float AgentWidth = 40;

	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Settings | Nav Grid")
	TArray<TEnumAsByte<EObjectTypeQuery>> GroundLayers = { };
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Settings | Nav Grid")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObstacleLayers = { };
	
	FNavigationMeshSettings() { }
};