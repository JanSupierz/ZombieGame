#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals, std::vector<Vector2>& visitedNodePositions)
		{
			//Reset debug positions
			debugNodePositions.clear();
			visitedNodePositions.clear();

			//Create the path to return
			std::vector<Vector2> finalPath{};

			//Get the start and endTriangle
			const Triangle* startTriangle = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos);
			const Triangle* endTriangle =  pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos);

			if (startTriangle == nullptr || endTriangle == nullptr) return finalPath;

			if (startTriangle == endTriangle)
			{
				finalPath.push_back(endPos);

				return finalPath;
			}

			//We have valid start/end triangles and they are not the same
			//=> Start looking for a path
			//Copy the graph
			auto clone{ pNavGraph->Clone() };
			IGraph<NavGraphNode, GraphConnection2D>* copyGraph = clone.get();

			//Create extra node for the Start Node (Agent's position
			NavGraphNode* pStartNode{ new NavGraphNode{ copyGraph->GetNextFreeNodeIndex(),-1,startPos } };

			copyGraph->AddNode(pStartNode);

			for (int index : startTriangle->metaData.IndexLines)
			{
				const int nodeIndex{ pNavGraph->GetNodeIdxFromLineIdx(index) };

				if (nodeIndex != -1)
				{
					GraphConnection2D* pCurrentConnection{ new GraphConnection2D{ pStartNode->GetIndex(),nodeIndex } };
					copyGraph->AddConnection(pCurrentConnection);
					pCurrentConnection->SetCost(Distance(pStartNode->GetPosition(), copyGraph->GetNodePos(nodeIndex)));
				}
			}

			//Create extra node for the endNode
			NavGraphNode* pEndNode{ new NavGraphNode{ copyGraph->GetNextFreeNodeIndex(),-1,endPos } };

			copyGraph->AddNode(pEndNode);

			for (int index : endTriangle->metaData.IndexLines)
			{
				const int nodeIndex{ pNavGraph->GetNodeIdxFromLineIdx(index) };

				if (nodeIndex != -1)
				{
					GraphConnection2D* pCurrentConnection{ new GraphConnection2D{ pEndNode->GetIndex(),nodeIndex } };
					copyGraph->AddConnection(pCurrentConnection);
					pCurrentConnection->SetCost(Distance(pEndNode->GetPosition(), copyGraph->GetNodePos(nodeIndex)));
				}
			}
			//Run A star on new graph
			auto pathfinderAStar = AStar<NavGraphNode,GraphConnection2D>(copyGraph, Elite::HeuristicFunctions::Euclidean);
			
			std::vector<NavGraphNode*> finalNodes{};
			std::vector<NavGraphNode*> visitedNodes{}; //Debug visualisation

			finalNodes = pathfinderAStar.FindPath(pStartNode, pEndNode, visitedNodes);
		
			for (NavGraphNode* node : finalNodes)
			{
				finalPath.push_back(node->GetPosition());

				//OPTIONAL BUT ADVICED: Debug Visualisation
				debugNodePositions.push_back(node->GetPosition());
			}

			//Alle nodes die gecheckt werden om A* te berekenen
			for (NavGraphNode* node : visitedNodes)
			{
				visitedNodePositions.push_back(node->GetPosition());
			}

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			debugPortals = SSFA::FindPortals(finalNodes, pNavGraph->GetNavMeshPolygon());
			finalPath = SSFA::OptimizePortals(debugPortals);

			return finalPath;
		}
	};
}
