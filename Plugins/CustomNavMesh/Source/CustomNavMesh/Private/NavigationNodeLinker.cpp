#include "NavigationNodeLinker.h"

#include "NavigationAgentComponent.h"
#include "NavigationMesh.h"

#define LOG(_msg, ...) UE_LOG(LogTemp, Warning, TEXT(_msg), ##__VA_ARGS__)

#define SphereDebug(location, radius, segment, color, thickness) DrawDebugSphere(GetWorld(), location, radius, segment, color, false, -1, 0, thickness)
#define ArrowDebug(locationStart, locationEnd, size, color, thickness) DrawDebugDirectionalArrow(GetWorld(), locationStart, locationEnd, size, color, false, -1, 0, thickness)
#define LineDebug(locationStart, locationEnd, color, thickness) DrawDebugLine(GetWorld(), locationStart, locationEnd, color, false, -1, 0, thickness)

ANavigationNodeLinker::ANavigationNodeLinker()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root Component");

	#if WITH_EDITOR
	LinkLeft = CreateDefaultSubobject<USceneComponent>("Nav Node Link Left");
	LinkLeft->SetupAttachment(RootComponent);
	LinkRight = CreateDefaultSubobject<USceneComponent>("Nav Node Link Right");
	LinkRight->SetupAttachment(RootComponent);

	Billboard = CreateDefaultSubobject<UBillboardComponent>("Billboard");
	Billboard->SetupAttachment(RootComponent);
	PrimaryActorTick.bCanEverTick = true;
	#endif
}

void ANavigationNodeLinker::BeginPlay()
{
	Super::BeginPlay();

	if (NodeLeft && NodeRight)
	{
		NodeLeft->OnPassedBy.AddUniqueDynamic(this, &ANavigationNodeLinker::OnNodePassedBy);
		NodeRight->OnPassedBy.AddUniqueDynamic(this, &ANavigationNodeLinker::OnNodePassedBy);		
	}
}

void ANavigationNodeLinker::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if WITH_EDITOR
	DrawNavigationMeshLinkerDebug();
#endif
}

void ANavigationNodeLinker::Destroyed()
{
#if WITH_EDITOR
	ClearNodeLink();
#endif	
	Super::Destroyed();
}

void ANavigationNodeLinker::OnNodePassedBy(UNavigationNode* _node, AActor* _actor)
{
	if (!_actor || !_node) return;
	
	if (const UNavigationAgentComponent* _compo = Cast<UNavigationAgentComponent>(_actor->GetComponentByClass(UNavigationAgentComponent::StaticClass())))
	{
		const UNavigationNode* _agentTargetNode = _compo->AgentTargetNode();
		
		if (_agentTargetNode == NodeLeft)
			OnLinkedNodeReached(_actor, NodeLeft->NodeLocation());
		if (_agentTargetNode == NodeRight)
			OnLinkedNodeReached(_actor, NodeRight->NodeLocation());
	}
}

#if WITH_EDITOR
#pragma region Init
void ANavigationNodeLinker::InitNodeLink()
{
	if (!NavigationMesh) return;

	NavigationMesh->OnNavMeshGeneration.AddUniqueDynamic(this, &ANavigationNodeLinker::InitNodeLink);

	UNavigationNode* _nodeLeft = NavigationMesh->GetClosestNode(LinkLeft->GetComponentLocation());
	UNavigationNode* _nodeRight = NavigationMesh->GetClosestNode(LinkRight->GetComponentLocation());

	if (_nodeLeft->NeighborExist(_nodeRight) || _nodeRight->NeighborExist(_nodeLeft) || _nodeLeft == _nodeRight) return;
	
	if (_nodeLeft && _nodeRight)
	{
		RemoveNeighbors(LinkWay);
		
		NodeLeft = _nodeLeft;
		NodeRight = _nodeRight;
		InitNeighbors(LinkWay);
	}
}
void ANavigationNodeLinker::ClearNodeLink()
{
	RemoveNeighbors(LinkWay);
	NodeLeft = nullptr;
	NodeRight = nullptr;
}

void ANavigationNodeLinker::RemoveNeighbors(const ENodeLink& _link) const
{
	if (NodeLeft && NodeRight)
	{
		switch (_link)
		{
		default :
			break;
		case ENodeLink::LinkLR :
			NodeLeft->RemoveNeighbor(NodeRight);
			break;
		case ENodeLink::LinkRL :
			NodeRight->RemoveNeighbor(NodeLeft);			
			break;
		case ENodeLink::LinkBoth :
			NodeLeft->RemoveNeighbor(NodeRight);
			NodeRight->RemoveNeighbor(NodeLeft);			
			break;
		}
	}
}
void ANavigationNodeLinker::InitNeighbors(const ENodeLink& _link) const
{
	if (NodeLeft && NodeRight)
	{
		switch (_link)
		{
		default :
			break;
		case ENodeLink::LinkLR :
			NodeLeft->AddNeighbor(NodeRight);
			break;
		case ENodeLink::LinkRL :
			NodeRight->AddNeighbor(NodeLeft);			
			break;
		case ENodeLink::LinkBoth :
			NodeLeft->AddNeighbor(NodeRight);
			NodeRight->AddNeighbor(NodeLeft);			
			break;
		}
	}
}
#pragma endregion

#pragma region Edit
void ANavigationNodeLinker::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (LinkWayOld != LinkWay)
	{
		RemoveNeighbors(LinkWayOld);
		InitNeighbors(LinkWay);
	}

	if (NavigationMeshOld != NavigationMesh)
	{
		RemoveNeighbors(LinkWay);
		NodeLeft = nullptr;
		NodeRight = nullptr;

		if (NavigationMeshOld)
			NavigationMeshOld->OnNavMeshGeneration.RemoveDynamic(this, &ANavigationNodeLinker::InitNodeLink);
		if (NavigationMesh)
			NavigationMesh->OnNavMeshGeneration.AddUniqueDynamic(this, &ANavigationNodeLinker::InitNodeLink);
	}
}
void ANavigationNodeLinker::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	LinkWayOld = LinkWay;
	NavigationMeshOld = NavigationMesh;
}
void ANavigationNodeLinker::PostLoad()
{
	Super::PostLoad();

	if (NavigationMesh)
	{
		NavigationMesh->OnNavMeshGeneration.AddUniqueDynamic(this, &ANavigationNodeLinker::InitNodeLink);
	}
}
#pragma endregion

#pragma region Debug
void ANavigationNodeLinker::DrawNavigationMeshLinkerDebug()
{
	const FVector& _linkLeftLocation = LinkLeft->GetComponentLocation();
	const FVector& _linkRightLocation = LinkRight->GetComponentLocation();

	if (!NodeLeft)
		SphereDebug(_linkLeftLocation, 3, 5, FColor::Black, 1);
	if (!NodeRight)
		SphereDebug(_linkRightLocation, 3, 5, FColor::Black, 1);
	
	if (NodeLeft && NodeRight)
	{
		const float& _debugSize = 2;
		const float& _arrowSize = 150;
		const float& _radius = 5;
		
		const FVector& _locationLeft = NodeLeft->NodeLocation();
		const FVector& _locationRight = NodeRight->NodeLocation();
		const float& _Z = FMath::Abs(_locationRight.Z - _locationLeft.Z);
		const FVector& _middle = FMath::Lerp(_locationLeft, _locationRight, 0.5f) + FVector(0, 0, _Z);

		//Nodes
		const bool _leftOpen = LinkWay == ENodeLink::LinkLR || LinkWay == ENodeLink::LinkBoth;
		const bool _rightOpen = LinkWay == ENodeLink::LinkRL || LinkWay == ENodeLink::LinkBoth;
		SphereDebug(_locationLeft, _radius, 1, _leftOpen ? LinkColor : FColor::Black, _debugSize);
		SphereDebug(_locationRight, _radius, 1, _rightOpen ? LinkColor : FColor::Black, _debugSize);

		//Curve
		TArray<FVector> _points;
		const float& _max = 5;
		for (int i = 0; i < _max + 1; ++i)
		{
			const float _time = i / _max;
			FVector _point = FMath::Pow(1 - _time, 2) * _locationLeft + 2 * (1 - _time) * _time * _middle + FMath::Pow(_time, 2) * _locationRight;		// 3 Point Bezier Curve
			_points.Add(_point);
		}
		
		for (int i = 0; i < _points.Num(); ++i)
			if (i + 1 < _points.Num())
			{
				if (i == 0 && (LinkWay == ENodeLink::LinkRL || LinkWay == ENodeLink::LinkBoth))
				{
					ArrowDebug(_points[i + 1], _points[i], _arrowSize, LinkColor, _debugSize);
					continue;	
				}
				if (i + 1 == _points.Num() - 1 && (LinkWay == ENodeLink::LinkLR || LinkWay == ENodeLink::LinkBoth))
				{
					ArrowDebug(_points[i], _points[i + 1], _arrowSize, LinkColor, _debugSize);
					continue;
				}
				LineDebug(_points[i], _points[i + 1], LinkColor, _debugSize);
			}


		return;
		switch (LinkWay)
		{
		default :
			break;
		case ENodeLink::LinkLR :
			SphereDebug(_locationLeft, _radius, 5, LinkColor, _debugSize);
			SphereDebug(_locationRight, _radius, 5, FColor::Black, _debugSize);
			ArrowDebug(_locationLeft, _locationRight, _arrowSize, LinkColor, _debugSize);
			break;
		case ENodeLink::LinkRL :
			SphereDebug(_locationLeft, _radius, 5, FColor::Black, _debugSize);
			SphereDebug(_locationRight, _radius, 5, LinkColor, _debugSize);
			ArrowDebug(_locationRight, _locationLeft, _arrowSize, LinkColor, _debugSize);
			break;
		case ENodeLink::LinkBoth :
			SphereDebug(_locationLeft, _radius, 5, LinkColor, _debugSize);
			SphereDebug(_locationRight, _radius, 5, LinkColor, _debugSize);
			ArrowDebug(_locationRight, _locationLeft, _arrowSize, LinkColor, _debugSize);
			ArrowDebug(_locationLeft, _locationRight, _arrowSize, LinkColor, _debugSize);
			break;
		}
		
	}
}
#pragma endregion
#endif