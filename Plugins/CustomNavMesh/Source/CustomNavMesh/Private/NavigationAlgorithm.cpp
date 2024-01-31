#include "NavigationAlgorithm.h"

#pragma region AStar
void UAlgorithmAStar::ComputePath(UNavigationNode* _startNode, UNavigationNode* _endNode) const
{
	TMap<UNavigationNode*, FNodeInfo> _grid = { };	//	Node map (avoid Cost/Heuristic/Previous to be on Node and create conflict if multiple Path is being computed at the same time)
	TArray<UNavigationNode*> _openList = { };		//	Nodes to check
	TArray<UNavigationNode*> _closeList = { };		//	Nodes already checked
	_openList.Add(_startNode);						//	Set first node to check with StartNode
	_grid.Add(_startNode, FNodeInfo(0, 0, _startNode, nullptr));
	
	while (!_openList.IsEmpty())								//	While the Open List is not Empty (Still Node to check)
	{
		UNavigationNode* _node = _openList[0];					//	Pop the first element of Open list (Get & Remove from OpenList)
		_openList.RemoveAt(0);
		const FNodeInfo& _nodeInfo = _grid.FindChecked(_node);	//	Get the associated Node Info  
		_closeList.Add(_node);									//	Add the popped Node to Close List (whatever happen, node is now checked)
		
		if (_node == _endNode)									//	If the popped Node is the End Node, Path have been completed
		{
			OnComputePathCompleted.Broadcast(GetPath(_grid, _startNode, _endNode));	//	Generate Path
			return;
		}

		const TArray<UNavigationNode*>& _neighbors = _node->NodeNeighbors();
		const int& _max = _neighbors.Num();
		for (int i = 0; i < _max; ++i)							// Pass through node Neighbors 
		{
			UNavigationNode* _neighbor = _neighbors[i];
			
			if (!_neighbor || !_neighbor->IsNodeAccessible() || _closeList.Contains(_neighbor))		// If Neighbor Node not Accessible or Occupied, or already checked (in CloseList)
				continue;

			const float& _heuristic = FVector::Dist(_node->NodeLocation(), _neighbor->NodeLocation());	//	Heuristic Cost of path [Node -> Neighbor] 
			const float& _nextCost = _nodeInfo.Cost + 1 + _heuristic;									//	New Cost of the Neighbor (Current Node Cost + Heuristic Cost)

			FNodeInfo& _neighborInfo = _grid.FindOrAdd(_neighbor);					//	Get Neighbor Info (Find a ref or add a new one in grid
			if (!_openList.Contains(_neighbor) || _nextCost < _neighborInfo.Cost)	//	If Neighbor have not been Checked OR New Cost of the Neighbor is less than the actual Neighbor Cost 
			{	
				_neighborInfo.Cost = _nextCost;
				_neighborInfo.Heuristic = _nextCost + _heuristic;
				_neighborInfo.Node = _neighbor;
				_neighborInfo.PreviousNode = _node;					//Set the Neighbor Info 
				_openList.Add(_neighbor);
			}
		}
	}
	
	OnComputePathFailed.Broadcast();			// If Open List have been fully checked and no path have been found to End Node
}

FNavigationNodePath UAlgorithmAStar::GetPath(const TMap<UNavigationNode*, FNodeInfo>& _grid, UNavigationNode* _startNode, UNavigationNode* _endNode) const
{
	TArray<UNavigationNode*> _path = { };
	UNavigationNode* _currentNode = _endNode;
	while (_currentNode != _startNode)
	{
		if (!_currentNode || !_grid.Contains(_currentNode))
			return FNavigationNodePath();
		
		_path.Add(_currentNode);
		_currentNode = _grid.FindChecked(_currentNode).PreviousNode;
	}
	_path.Add(_startNode);
	Algo::Reverse(_path);
	
	return FNavigationNodePath(_path);
}
#pragma endregion
