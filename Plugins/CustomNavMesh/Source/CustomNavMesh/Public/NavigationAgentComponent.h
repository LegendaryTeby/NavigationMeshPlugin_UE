#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "NavigationAlgorithm.h"

#include "NavigationAgentComponent.generated.h"

class ANavigationMesh;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CUSTOMNAVMESH_API UNavigationAgentComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Navigation Agent | System")
	ANavigationMesh* NavigationMesh = nullptr;
	UPROPERTY(EditAnywhere, Category = "Navigation Agent | System", meta = (ClampMin = "0.05"))
	float PathRecomputeRate = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "Navigation Agent | Agent Settings")
	FVector AgentFeetLocation = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, Category = "Navigation Agent | Agent Movement Settings", meta = (ClampMin = "1", ClampMax = "1000"))
	float AgentNodeRangeAcceptance = 25;
	
	UPROPERTY(EditAnywhere, Category = "Navigation Agent | Agent Rotation Settings", meta = (ClampMin = "1", ClampMax = "1000"))
	float AgentRotationSpeed = 5;
	
	UPROPERTY(VisibleAnywhere)
	UAlgorithmAStar* NavigationAlgorithm = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	APawn* OwnerPawn = nullptr;

	UPROPERTY(EditAnywhere, Category = "Navigation Agent | Default Values")
	bool MovementEnable = true;
	UPROPERTY(EditAnywhere, Category = "Navigation Agent | Default Values")
	bool RotationEnable = true;
	UPROPERTY(EditAnywhere, Category = "Navigation Agent | Default Values")
	bool AgentEnable = true;
	
	UPROPERTY()
	AActor* TargetActor = nullptr;
	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;
	
	UPROPERTY()
	bool IsFollowingPath = false;
	UPROPERTY(VisibleAnywhere, Category = "Navigation Agent | Values")
	FNavigationNodePath FollowPath = FNavigationNodePath();
	
	UPROPERTY()
	FTimerHandle RecomputeTimerHandle;

public:
	FORCEINLINE const UNavigationNode* AgentPreviousNode() const { return FollowPath.PreviousNode; }
	FORCEINLINE const UNavigationNode* AgentTargetNode() const { return FollowPath.CurrentNode; }

	FORCEINLINE FVector AgentLocation() const { return OwnerPawn ? OwnerPawn->GetActorLocation() + AgentFeetLocation : FVector::ZeroVector; }
	
public:
	UNavigationAgentComponent();

	UFUNCTION(BlueprintCallable) void MoveToLocation(const FVector& _worldLocation);
	UFUNCTION(BlueprintCallable) void MoveToActor(AActor* _actor);
	
	UFUNCTION(BlueprintCallable) void ResumeAgent();
	UFUNCTION(BlueprintCallable) void StopAgent();
	
	UFUNCTION(BlueprintCallable) void ResumeMovement();
	UFUNCTION(BlueprintCallable) void StopMovement();

	UFUNCTION(BlueprintCallable) void ResumeRotation();
	UFUNCTION(BlueprintCallable) void StopRotation();
	
private:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UFUNCTION(BlueprintCallable) virtual void InitializeAgent();
	UFUNCTION(BlueprintCallable) virtual void TickAgent(const float _deltaTime);

	#pragma region Agent Behaviors
	UFUNCTION(BlueprintCallable) virtual void UpdateAgentMovement(const float _deltaTime);
	UFUNCTION(BlueprintCallable) virtual void UpdateAgentRotation(const float _deltaTime);
	#pragma endregion 

	//	Check the Distance between Agent (Location + Feet) and Current Node (if following a path)
	bool IsAgentArrivedAtNode() const;
	//	Make the Agent follow the next Node in his Path
	void UpdateAgentPathFollowing(); 

	#pragma region Enable 
	UFUNCTION(BlueprintCallable) void SetMovementEnable(const bool _enable);
	UFUNCTION(BlueprintCallable) void SetRotationEnable(const bool _enable);
	UFUNCTION(BlueprintCallable) void SetAgentEnable(const bool _enable);
	#pragma endregion

	#pragma region Path
	UFUNCTION() void RecomputePath();
	
	UFUNCTION() virtual void OnPathReceived(FNavigationNodePath _path);
	UFUNCTION() virtual void OnPathFailed();
	#pragma endregion
};
