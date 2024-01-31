#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#if WITH_EDITOR
#include "Components/BillboardComponent.h"

#include "NavigationAlgorithm.h"
#endif

#include "NavigationNode.h"
#include "NavigationMeshSettings.h"

#include "NavigationMesh.generated.h"

UCLASS()
class CUSTOMNAVMESH_API ANavigationMesh : public AActor
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "Navigation Mesh | Components")
	UBillboardComponent* Billboard = nullptr;
#endif
	
	UPROPERTY(EditAnywhere, meta = (ShowOnlyInnerProperties))
	FNavigationMeshSettings NavMeshSettings = FNavigationMeshSettings();

	UPROPERTY(VisibleAnywhere, Category = "Navigation Mesh | Nodes")
	TArray<UNavigationNode*> NavigationNodes = { };

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Debug")
	FColor NavigationMeshDebugColor = FColor::Yellow;
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Debug")
	FColor NodeDebugColor = FColor::Blue; 
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Debug")
	FColor NodeLineDebugColor = FColor::Green; 
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Debug")
	bool Debug = true;
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Debug")
	float DebugTime = 10.0f;
	
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Test")
	AActor* StartTest = nullptr;
	UPROPERTY(EditAnywhere, Category = "Navigation Mesh | Test")
	AActor* EndTest = nullptr;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNavMeshGeneration);
	UPROPERTY()
	FOnNavMeshGeneration OnNavMeshGeneration;
private:

	UPROPERTY()
	UAlgorithmAStar* Algo = nullptr;
#endif
	
public:	
	ANavigationMesh();

	UNavigationNode* GetClosestNode(const FVector& _worldLocation);

private:
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override { return Debug; }
#endif

#if WITH_EDITOR
	#pragma region Navigation Mesh Init 
	UFUNCTION(CallInEditor, Category = "Navigation Mesh | Utils") void GenerateNavigationMeshSimple();
	UFUNCTION(CallInEditor, Category = "Navigation Mesh | Utils") void GenerateNavigationMeshComplex();
	void GenerateNodesNeighborsSimple();
	void GenerateNodesNeighborsComplex();
	TArray<UNavigationNode*> GetCloseNodes(const UNavigationNode* _from, const float _range);
	bool CheckAgentCanWalkBetweenNodes(const UNavigationNode* _from, const UNavigationNode* _to, const float& _range) const;
	#pragma endregion

	#pragma region Navigation Mesh Debug 
	void DrawNavigationMeshDebug() const;
	UFUNCTION(CallInEditor, Category = "Navigation Mesh | Utils") void DrawNavigationNodes();
	#pragma endregion 

	#pragma region Test
	UFUNCTION(CallInEditor, Category = "Navigation Mesh | Test") void TestGetPath();
	UFUNCTION(CallInEditor, Category = "Navigation Mesh | Test") void TestGetClose();
	UFUNCTION() void TestPath(FNavigationNodePath _path);
	UFUNCTION() void TestPathFail();
	#pragma endregion
#endif
};
