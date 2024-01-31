#include "NavigationNode.h"

#if WITH_EDITOR
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#define SingleLineTrace(startLocation, endLocation, layers, result) UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), startLocation, endLocation, layers, false, { }, EDrawDebugTrace::None, result, true);
#define SingleBoxTrace(location, halfSize, rotation, layers, result) UKismetSystemLibrary::BoxTraceSingleForObjects(GetWorld(), location, location, halfSize, rotation, layers, false, { }, EDrawDebugTrace::None, result, true);
#define SingleCapsuleTrace(location, width, height, layers, result) UKismetSystemLibrary::CapsuleTraceSingleForObjects(GetWorld(), location, location, width, height, layers, false, { }, EDrawDebugTrace::None, _result, true)

#define MultiLineTrace(startLocation, endLocation, layers, results) UKismetSystemLibrary::LineTraceMultiForObjects(GetWorld(), startLocation, endLocation, layers, false, { }, EDrawDebugTrace::None, results, true)
#endif

void UNavigationNode::PassedBy(AActor* _actor)
{
	OnPassedBy.Broadcast(this, _actor);
}

#if WITH_EDITOR
#pragma region Init
void UNavigationNode::InitializeNavigationNodeSimple(const FVector& _location, const FNavigationMeshSettings& _navSettings)
{
	Location = _location;
	const FVector& _endLocation = _location - FVector(0, 0, _navSettings.NavigationGridHeight);
	
	FHitResult _result;
	// Line Trace to check if Node is above the Ground
	IsAccessible = SingleLineTrace(Location, _endLocation, _navSettings.GroundLayers, _result);
	if (IsAccessible)
	{
		Location = _result.ImpactPoint + _result.ImpactNormal * _navSettings.NavigationGridSurfaceHeight;
		CheckLocationAccessibility(_navSettings);
	}
}

void UNavigationNode::InitializeNavigationNodeComplex(const FVector& _location, const FNavigationMeshSettings& _navSettings)
{
	Location = _location;
	const FVector& _endLocation = _location + FVector(0, 0, _navSettings.AgentHeight);
	
	FHitResult _result;
	// Check if there is no Walkable ground above node (with Agent Height)
	IsAccessible = !SingleLineTrace(Location, _endLocation, _navSettings.GroundLayers, _result);
	if (IsAccessible)
	{
		CheckLocationAccessibility(_navSettings);
	}
}

void UNavigationNode::AddNeighbor(UNavigationNode* _node)
{
	if (!IsAccessible || !_node->IsNodeAccessible() || NeighborExist(_node)) return;	//Only add Neighbor if the Current Node is Accessible, the Neighbor Node is Accessible and not already add as a Neighbor 
	Neighbors.Add(_node);
}
void UNavigationNode::RemoveNeighbor(UNavigationNode* _node)
{
	if (!NeighborExist(_node)) return;
	Neighbors.Remove(_node);
}

bool UNavigationNode::NeighborExist(const UNavigationNode* _node) const
{
	return Neighbors.Contains(_node);
}

void UNavigationNode::CheckLocationAccessibility(const FNavigationMeshSettings& _navSettings)
{
	if (_navSettings.ObstacleLayers.IsEmpty())
		return;
	
	FHitResult _result;
	// Box Trace with Obstacle Avoidance to check if Node is not too close to an Obstacle
	IsAccessible = !SingleBoxTrace(Location, FVector(_navSettings.ObstacleAvoidanceSize), FRotator(0), _navSettings.ObstacleLayers, _result);
	if (IsAccessible)
	{
		// Capsule Trace with Agent Height & Width (Check if Agent can pass through)
		const FVector& _agentHeightLocation = Location + FVector(0, 0, _navSettings.AgentHeight);
		IsAccessible = !SingleCapsuleTrace(_agentHeightLocation, _navSettings.AgentWidth, _navSettings.AgentHeight, _navSettings.ObstacleLayers, _result);
	}
}
#pragma endregion

void UNavigationNode::DrawNavigationNodeDebug(const FColor& _nodeColor, const FColor& _lineColor, const float& _drawTime) const
{
	const FVector& _location = NodeLocation();
	
	DrawDebugSphere(GetWorld(), _location, 3, 3, IsAccessible ? _nodeColor : FColor::Black, false, _drawTime);

	const int _max = Neighbors.Num();
	for (int i = 0; i < _max; ++i)
		if (const UNavigationNode* _node = Neighbors[i])
			DrawDebugLine(GetWorld(), _location, _node->NodeLocation(), _node->IsNodeAccessible() ? _lineColor : FColor::Black, false, _drawTime);
}
#endif
