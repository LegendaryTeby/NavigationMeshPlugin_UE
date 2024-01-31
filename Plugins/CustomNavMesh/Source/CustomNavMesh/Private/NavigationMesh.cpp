#include "NavigationMesh.h"

#if WITH_EDITOR
#include "Kismet/KismetSystemLibrary.h"
#endif

#define LOG(_msg, ...) UE_LOG(LogTemp, Warning, TEXT(_msg), ##__VA_ARGS__)

ANavigationMesh::ANavigationMesh()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root Component");

#if WITH_EDITOR
	Billboard = CreateDefaultSubobject<UBillboardComponent>("Billboard");
	Billboard->SetupAttachment(RootComponent);
	PrimaryActorTick.bCanEverTick = true;
#endif
}

UNavigationNode* ANavigationMesh::GetClosestNode(const FVector& _worldLocation)
{
	UNavigationNode* _node = nullptr;
	float _last = UE_MAX_FLT;
	
	const int& _max = NavigationNodes.Num();
	for (int i = 0; i < _max; ++i)
	{
		UNavigationNode* _current = NavigationNodes[i];
		if (!_current || !_current->IsNodeAccessible()) continue;
		
		const float& _dist = FVector::Dist(_current->NodeLocation(), _worldLocation);
		if (_dist < _last)
		{
			_last = _dist;
			_node = _current;
		}
	}
	
	return _node;
}

void ANavigationMesh::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	#if WITH_EDITOR
	DrawNavigationMeshDebug();
	#endif
}

#if WITH_EDITOR
#pragma region Navigation Mesh Init 
void ANavigationMesh::GenerateNavigationMeshSimple()
{
	if (NavMeshSettings.ObstacleLayers.IsEmpty())
		UE_LOG(LogTemp, Warning, TEXT("WARNING : Navigation Mesh Settings -> Obstacle Layers is Empty ! Obstacle are ignored"));
	if (NavMeshSettings.GroundLayers.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR : Navigation Mesh Settings -> Ground Layers is Empty ! Node can NOT be created"));		
		return;
	}
	
	NavigationNodes.Empty();
	const FVector& _location = GetActorLocation();
	for (int x = 0; x < NavMeshSettings.NavigationGridSizeX; ++x)
	{
		for (int y = 0; y < NavMeshSettings.NavigationGridSizeY; ++y)
		{
			const FVector& _nodeLocation = _location + FVector(x * NavMeshSettings.NavigationGridGap, y * NavMeshSettings.NavigationGridGap, 0);

			UNavigationNode* _node = NewObject<UNavigationNode>(this);
			_node->InitializeNavigationNodeSimple(_nodeLocation, NavMeshSettings);
			NavigationNodes.Add(_node);
		}
	}

	GenerateNodesNeighborsSimple();

	OnNavMeshGeneration.Broadcast();
}
void ANavigationMesh::GenerateNodesNeighborsSimple()
{
	const int& _max = NavigationNodes.Num();
	const int& _maxX = NavMeshSettings.NavigationGridSizeX;
	const int& _maxY = NavMeshSettings.NavigationGridSizeY;
	for (int i = 0; i < _max; ++i)
	{
		if (UNavigationNode* _node = NavigationNodes[i])
		{
																		//	Those Conditions works with the NavMesh generation : Line X -> Generate All Y Node -> Go Next Line X...
			const bool& _canRight = i % _maxY != _maxY - 1;				//	If i % MaxY == MaxY - 1 -> Means i is on the last element of the Y Line (on the Right Side, then can't go more to the Right)
			const bool& _canLeft = i % _maxY != 0;						//	If i % MaxY == 0		-> Means i is in the first element of the Y Line (on the Left side, then can't go more to the Left) 
			const bool& _canDown = i >= _maxY;							//	While i is NOT on the first Y Line	-> Can Go Down
			const bool& _canTop = i < _maxX * _maxY - _maxY;			//	While i < Maximum Node - 1 Y Range	-> Can Top 

			const int& _indexRight		= i + 1;
			const int& _indexLeft		= i - 1;
			const int& _indexTop		= i + _maxY;
			const int& _indexTopRight	= i + 1 + _maxY;
			const int& _indexTopLeft	= i - 1 + _maxY;
			const int& _indexDown		= i - _maxY;
			const int& _indexDownRight	= i + 1 - _maxY;
			const int& _indexDownLeft	= i - 1 - _maxY;
			const float& _range = NavMeshSettings.NavigationGridGap + NavMeshSettings.AgentExtraWalkStep;
			
			if (_canRight	&& CheckAgentCanWalkBetweenNodes(_node, NavigationNodes[_indexRight], _range))
				_node->AddNeighbor(NavigationNodes[_indexRight]);
			if (_canLeft	&& CheckAgentCanWalkBetweenNodes(_node, NavigationNodes[_indexLeft], _range))
			 		_node->AddNeighbor(NavigationNodes[_indexLeft]);
			
			if (_canTop)
			{
				if (CheckAgentCanWalkBetweenNodes(_node, NavigationNodes[_indexTop], _range))
					_node->AddNeighbor(NavigationNodes[_indexTop]);
				if (_canRight	&& CheckAgentCanWalkBetweenNodes(_node, NavigationNodes[_indexTopRight], _range))
					_node->AddNeighbor(NavigationNodes[_indexTopRight]);
				if (_canLeft	&& CheckAgentCanWalkBetweenNodes(_node, NavigationNodes[_indexTopLeft], _range))
					_node->AddNeighbor(NavigationNodes[_indexTopLeft]);
			}
			if (_canDown)
			{
				if (CheckAgentCanWalkBetweenNodes(_node, NavigationNodes[_indexDown], _range))
					_node->AddNeighbor(NavigationNodes[_indexDown]);
				if (_canRight	&& CheckAgentCanWalkBetweenNodes(_node, NavigationNodes[_indexDownRight], _range))
					_node->AddNeighbor(NavigationNodes[_indexDownRight]);
				if (_canLeft	&& CheckAgentCanWalkBetweenNodes(_node, NavigationNodes[_indexDownLeft], _range))
					_node->AddNeighbor(NavigationNodes[_indexDownLeft]);
			}

			/*	Debug Condition
			//	Can Right
			// LOG("%d %s = %d", i, *FString("% GridSizeY"), i % GridSizeY);
			// LOG("GridSizeY - 1 = %d", GridSizeY - 1)
			// LOG("%d != %d : %s", i % GridSizeY, GridSizeY - 1, *FString(i % GridSizeY != GridSizeY - 1 ? "true" : "false"));
			// LOG("-------------------------------------")

			//	Can Left
			// LOG("%d %s = %d", i, *FString("% GridSizeY"), i % GridSizeY)
			// LOG("%s != 0 : %s", *FString("i % GridSizeY"), *FString(i % GridSizeY != 0 ? "true" : "false"));
			// LOG("-------------------------------------")
			
			//	Can Down
			//	LOG("i >= GridSizeY : %s", *FString(i >= GridSizeY ? "true" : "false"))
			//	LOG("-------------------------------------")
			
			//	Can Top
			//	LOG("i < GridSizeX * GridSizeY - GridSizeY : %s", *FString(i < GridSizeX * GridSizeY - GridSizeY ? "true" : "false"))
			//	LOG("-------------------------------------")
			*/
		}
	}
}

void ANavigationMesh::GenerateNavigationMeshComplex()
{
	if (NavMeshSettings.ObstacleLayers.IsEmpty())
		UE_LOG(LogTemp, Warning, TEXT("WARNING : Navigation Mesh Settings -> Obstacle Layers is Empty ! Obstacle are ignored"));
	if (NavMeshSettings.GroundLayers.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR : Navigation Mesh Settings -> Ground Layers is Empty ! Node can NOT be created"));		
		return;
	}
	
	NavigationNodes.Empty();
	const FVector& _location = GetActorLocation();
	for (int x = 0; x < NavMeshSettings.NavigationGridSizeX; ++x)
	{
		for (int y = 0; y < NavMeshSettings.NavigationGridSizeY; ++y)
		{
			TArray<FHitResult> _results;
			const FVector& _nodeLocation = _location + FVector(x * NavMeshSettings.NavigationGridGap, y * NavMeshSettings.NavigationGridGap, 0);
			UKismetSystemLibrary::LineTraceMultiForObjects(GetWorld(), _nodeLocation, _nodeLocation - FVector(0, 0, NavMeshSettings.NavigationGridHeight), NavMeshSettings.GroundLayers, false, { }, EDrawDebugTrace::None, _results, true);

			const int _maxHit = _results.Num();
			for (int i = 0; i < _maxHit; i++)
			{
				UNavigationNode* _node = NewObject<UNavigationNode>(this);
				_node->InitializeNavigationNodeComplex(_results[i].ImpactPoint + _results[i].ImpactNormal * NavMeshSettings.NavigationGridSurfaceHeight, NavMeshSettings);
				NavigationNodes.Add(_node);
			}
		}
	}
	
	GenerateNodesNeighborsComplex();

	OnNavMeshGeneration.Broadcast();
}
void ANavigationMesh::GenerateNodesNeighborsComplex()
{
	TArray<UNavigationNode*> _openList = NavigationNodes;
	while (!_openList.IsEmpty())
	{
		UNavigationNode* _node = _openList[0];
		_openList.RemoveAt(0);
		if (_node)
		{
			TArray<UNavigationNode*> _nodes = GetCloseNodes(_node, NavMeshSettings.NavigationGridGap + NavMeshSettings.AgentExtraWalkStep);
			const int _neighborMax = _nodes.Num();
			for (int n = 0; n < _neighborMax; ++n)
			{
				UNavigationNode* _neighbor = _nodes[n]; 
				_node->AddNeighbor(_neighbor);
				_neighbor->AddNeighbor(_node);
			}
		}
	}
}

TArray<UNavigationNode*> ANavigationMesh::GetCloseNodes(const UNavigationNode* _from, const float _range)
{
	if (!_from) return { };
	
	TArray<UNavigationNode*> _nodes = { };

	const int _max = NavigationNodes.Num();
	for (int i = 0; i < _max; ++i)
	{
		if (UNavigationNode* _to = NavigationNodes[i])
			if (_to != _from)
				if (FVector::Dist(_from->NodeLocation(), _to->NodeLocation()) <= _range)
					_nodes.Add(_to);
	}

	return _nodes;
}

bool ANavigationMesh::CheckAgentCanWalkBetweenNodes(const UNavigationNode* _from, const UNavigationNode* _to, const float& _range) const
{
	if (!_from || !_to) return false;
	
	return FVector::Dist(_from->NodeLocation(), _to->NodeLocation()) <= _range;
}
#pragma endregion

#pragma region Navigation Mesh Debug 
void ANavigationMesh::DrawNavigationMeshDebug() const
{
	if (!Debug) return;

	const float& _extentX = (NavMeshSettings.NavigationGridSizeX - 1) * NavMeshSettings.NavigationGridGap;
	const float& _extentY = (NavMeshSettings.NavigationGridSizeY - 1) * NavMeshSettings.NavigationGridGap;
	const float& _extentZ = -NavMeshSettings.NavigationGridHeight;
	const FVector& _area = FVector(_extentX, _extentY, _extentZ) / 2.0f;
	const FVector& _center = GetActorLocation() + _area;

	FColor _color = NavigationMeshDebugColor;
	DrawDebugBox(GetWorld(), _center, _area, _color);
	const FVector& _gap = FVector(NavMeshSettings.NavigationGridGap, NavMeshSettings.NavigationGridGap, 0) / 2.0f;
	DrawDebugSolidBox(GetWorld(), GetActorLocation() + _gap, _gap, _color);
	_color.A = 5;
	DrawDebugSolidBox(GetWorld(), _center, _area, _color);
}
void ANavigationMesh::DrawNavigationNodes()
{
	const int& _max = NavigationNodes.Num();
	for (int i = 0; i < _max; ++i)
	{
		if (const UNavigationNode* _node = NavigationNodes[i])
			_node->DrawNavigationNodeDebug(NodeDebugColor, NodeLineDebugColor, DebugTime);
	}
}
#pragma endregion

#pragma region Test
void ANavigationMesh::TestGetPath()
{
	if (!Algo)
	{
		Algo = NewObject<UAlgorithmAStar>(this);
		Algo->OnComputePathCompleted.AddUniqueDynamic(this, &ANavigationMesh::TestPath);	
		Algo->OnComputePathFailed.AddUniqueDynamic(this, &ANavigationMesh::TestPathFail);		
	}
	
	if (!StartTest || !EndTest || !Algo) return;
	
	const FVector& _startLocation = StartTest->GetActorLocation();
	const FVector& _endLocation = EndTest->GetActorLocation();

	UNavigationNode* _startNode = GetClosestNode(_startLocation);
	UNavigationNode* _endNode = GetClosestNode(_endLocation);

	Algo->ComputePath(_startNode, _endNode);
}

void ANavigationMesh::TestGetClose()
{
	if (!StartTest) return;
	
	const FVector& _startLocation = StartTest->GetActorLocation();
	if (const UNavigationNode* _startNode = GetClosestNode(_startLocation))
	{
		const FVector& _nodeLocation = _startNode->NodeLocation();
		DrawDebugSphere(GetWorld(), _startLocation, 10, 10, FColor::Magenta, false, DebugTime);
		DrawDebugSphere(GetWorld(), _nodeLocation, 10, 10, FColor::Magenta, false, DebugTime);
		DrawDebugLine(GetWorld(), _startLocation, _nodeLocation, FColor::Magenta, false, DebugTime);
	}
}

void ANavigationMesh::TestPath(FNavigationNodePath _path)
{
	if (_path.NodePath.IsEmpty()) return;

	TArray<UNavigationNode*>& _nodes = _path.NodePath;
	const UNavigationNode* _startNode = _nodes[0];
	const UNavigationNode* _endNode = _nodes.Last();
	const FVector& _startNodeLocation = _startNode->NodeLocation();
	const FVector& _endNodeLocation = _endNode->NodeLocation();
	
	DrawDebugSphere(GetWorld(), _startNodeLocation, 15, 10, FColor::Green, false, DebugTime);
	DrawDebugSphere(GetWorld(), _endNodeLocation, 15, 10, FColor::Red, false, DebugTime);

	//DrawPath
	const int& _max = _nodes.Num();
	for (int i = 0; i < _max; ++i)
	{
		if (const UNavigationNode* _node = _nodes[i])
		{
			const FVector& _location = _node->NodeLocation();
			DrawDebugSphere(GetWorld(), _location, 5, 10, FColor::Blue, false, DebugTime);

			if (i + 1 < _max)
				DrawDebugDirectionalArrow(GetWorld(), _location, _nodes[i + 1]->NodeLocation(), 5, FColor::Blue, false, DebugTime);
		}
	}
}
void ANavigationMesh::TestPathFail()
{
	UE_LOG(LogTemp, Warning, TEXT("Found path failed"))
}
#pragma endregion
#endif