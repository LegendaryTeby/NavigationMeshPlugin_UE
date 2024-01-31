#include "NavigationAgentComponent.h"

#include "NavigationMesh.h"

#include "Kismet/KismetMathLibrary.h"

UNavigationAgentComponent::UNavigationAgentComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNavigationAgentComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeAgent();
}
void UNavigationAgentComponent::TickComponent(float DeltaTime, ELevelTick TickType,	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickAgent(DeltaTime);
}

void UNavigationAgentComponent::MoveToLocation(const FVector& _worldLocation)
{
	if (!NavigationAlgorithm || !NavigationMesh) return;

	TargetActor = nullptr;
	TargetLocation = _worldLocation;

	NavigationAlgorithm->ComputePath(NavigationMesh->GetClosestNode(AgentLocation()), NavigationMesh->GetClosestNode(TargetLocation));
}
void UNavigationAgentComponent::MoveToActor(AActor* _actor)
{
	if (!NavigationAlgorithm || !NavigationMesh || !_actor) return;

	TargetActor = _actor;
	TargetLocation = FVector::ZeroVector;

	NavigationAlgorithm->ComputePath(NavigationMesh->GetClosestNode(AgentLocation()), NavigationMesh->GetClosestNode(TargetActor->GetActorLocation()));
}

void UNavigationAgentComponent::ResumeAgent()
{
	SetAgentEnable(true);	
}
void UNavigationAgentComponent::StopAgent()
{
	SetAgentEnable(false);
}

void UNavigationAgentComponent::ResumeMovement()
{
	SetMovementEnable(false);	
}
void UNavigationAgentComponent::StopMovement()
{
	SetMovementEnable(false);
}

void UNavigationAgentComponent::ResumeRotation()
{
	SetRotationEnable(true);	
}
void UNavigationAgentComponent::StopRotation()
{
	SetRotationEnable(false);
}

void UNavigationAgentComponent::InitializeAgent()
{
	NavigationAlgorithm = NewObject<UAlgorithmAStar>(this);
	NavigationAlgorithm->OnComputePathCompleted.AddUniqueDynamic(this, &UNavigationAgentComponent::OnPathReceived);
	NavigationAlgorithm->OnComputePathFailed.AddUniqueDynamic(this, &UNavigationAgentComponent::OnPathFailed);
	OwnerPawn = Cast<APawn>(GetOwner());
}
void UNavigationAgentComponent::TickAgent(const float _deltaTime)
{
	if (!OwnerPawn || !AgentEnable) return;
	
	if (IsFollowingPath && !FollowPath.PathCompleted)
	{
		FVector _location = AgentLocation();
		FVector _nodeLocation = FollowPath.CurrentNode->NodeLocation();
		_location.Z = 0;
		_nodeLocation.Z = 0;
	
		UpdateAgentMovement(_deltaTime);
		UpdateAgentRotation(_deltaTime);
	}
}

void UNavigationAgentComponent::UpdateAgentMovement(const float _deltaTime)
{
	if (!MovementEnable || !IsFollowingPath || !OwnerPawn) return;

	const FVector& _agentLocation = AgentLocation();
	const FVector& _nodeLocation = FollowPath.CurrentNode->NodeLocation();
	const FVector& _direction = _nodeLocation - _agentLocation;
	OwnerPawn->AddMovementInput(_direction.GetSafeNormal());
	
	if (IsAgentArrivedAtNode())
		UpdateAgentPathFollowing();
}
void UNavigationAgentComponent::UpdateAgentRotation(const float _deltaTime)
{
	if (!RotationEnable || !OwnerPawn) return;
	
	FVector _agentLocation = AgentLocation();
	FVector _agentDirection = _agentLocation + OwnerPawn->GetVelocity();
	_agentLocation.Z = 0;
	_agentDirection.Z = 0;
	
	const FRotator& _currentRotation = OwnerPawn->GetActorRotation();
	const FRotator& _targetRotation = UKismetMathLibrary::FindLookAtRotation(_agentLocation, _agentDirection);
	const float& _yaw = FMath::Abs(_currentRotation.Yaw - _targetRotation.Yaw);
	const FRotator _rotation = UKismetMathLibrary::RInterpTo_Constant(_currentRotation, _targetRotation, _deltaTime, AgentRotationSpeed * _yaw);
	OwnerPawn->SetActorRotation(_rotation);
}

bool UNavigationAgentComponent::IsAgentArrivedAtNode() const
{
	if (!IsFollowingPath) return false;
	const FVector _agentLocation = AgentLocation();
	const FVector _targetLocation = FollowPath.CurrentNode->NodeLocation();
	
	return FVector::Dist(_agentLocation, _targetLocation) < AgentNodeRangeAcceptance;
}

void UNavigationAgentComponent::UpdateAgentPathFollowing()
{
	//FollowPath.CurrentNode->Reset...();	//Occupied
	if (!FollowPath.NextNode())
	{
		IsFollowingPath = false;
		GetWorld()->GetTimerManager().ClearTimer(RecomputeTimerHandle);	
	}
	FollowPath.PreviousNode->PassedBy(OwnerPawn);
	//FollowPath.CurrentNode->Set...(OwnerPawn);	//Occupied
}

void UNavigationAgentComponent::SetMovementEnable(const bool _enable)
{
	MovementEnable = _enable;
}
void UNavigationAgentComponent::SetRotationEnable(const bool _enable)
{
	RotationEnable = _enable;
}
void UNavigationAgentComponent::SetAgentEnable(const bool _enable)
{
	AgentEnable = _enable;
}

void UNavigationAgentComponent::RecomputePath()
{
	if (!IsFollowingPath) return; 

	const FVector& _targetLocation = TargetActor ? TargetActor->GetActorLocation() : TargetLocation;
	NavigationAlgorithm->ComputePath(FollowPath.CurrentNode, NavigationMesh->GetClosestNode(_targetLocation));
}

void UNavigationAgentComponent::OnPathReceived(FNavigationNodePath _path)
{
	if (IsFollowingPath)
	{
		FollowPath.UpdatePath(_path);
	}
	else
	{
		IsFollowingPath = true;
		FollowPath = _path;
	}
	
	GetWorld()->GetTimerManager().SetTimer(RecomputeTimerHandle, this, &UNavigationAgentComponent::RecomputePath, PathRecomputeRate, false);
	
	//DrawPath
	{
		const int& _max = FollowPath.NodePath.Num();
		for (int i = 0; i < _max; ++i)
		{
			if (const UNavigationNode* _node = FollowPath.NodePath[i])
			{
				const FVector& _location = _node->NodeLocation();
				DrawDebugSphere(GetWorld(), _location, 5, 10, FColor::Blue, false, PathRecomputeRate);

				if (i + 1 < _max)
					DrawDebugDirectionalArrow(GetWorld(), _location, FollowPath.NodePath[i + 1]->NodeLocation(), 200, FColor::Blue, false, PathRecomputeRate);
			}
		}
	}
}
void UNavigationAgentComponent::OnPathFailed()
{
	IsFollowingPath = false;
	
	GetWorld()->GetTimerManager().ClearTimer(RecomputeTimerHandle);
}
