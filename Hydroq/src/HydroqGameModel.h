#pragma once

#include "Component.h"
#include "ofxSQLite.h"
#include "HydroqDef.h"
#include "Events.h"
#include "HydMap.h"
#include "HydEntity.h"
#include "MapObjectChangedEvent.h"

/**
* Hydroq game model
*/
class HydroqGameModel : public Component {

	OBJECT(HydroqGameModel)

private:
	// static objects (excluding buildings)
	HydMap* hydroqMap;
	// dynamic objects (including buildings)
	map<Vec2i, Node*> dynObjects;
	Scene* gameScene;
	Node* rootNode;
public:

	~HydroqGameModel() {

	}

	void Init() {
		hydroqMap = new HydMap();
		gameScene = new Scene("gamescene", false);
		rootNode = gameScene->GetSceneNode();

	}

	void Init(spt<ofxXml> xml) {
		Init();
	}

	HydMap* GetMap() {
		return hydroqMap;
	}

	bool IsPositionFreeForBuilding(Vec2i position) {
		auto node = hydroqMap->GetNode(position.x, position.y);
		return node->mapNodeType == MapNodeType::GROUND && !node->occupied;
	}

	void CreateDynamicObject(Vec2i position, EntityType entityType) {
		auto hydMapNode = hydroqMap->GetNode(position.x, position.y);
		hydMapNode->occupied = true;
		auto gameNode = CreateNode(entityType, position);

		SendMessageNoBubbling(StringHash(ACT_MAP_OBJECT_CHANGED), 0, 
			new MapObjectChangedEvent(ObjectChangeType::DYNAMIC_CHANGED, hydMapNode, gameNode), nullptr);
	}

	bool IsPositionFreeForBridge(Vec2i position) {
		auto node = hydroqMap->GetNode(position.x, position.y);
		bool isFree = node->mapNodeType == MapNodeType::WATER && dynObjects.find(position) == dynObjects.end();

		if (isFree) {
			// at least one neighbor mustn't be water or it is already marked
			for (auto neighbor : node->GetNeighbors()) {
				if (neighbor->mapNodeType != MapNodeType::WATER || 
					(dynObjects.find(neighbor->pos) != dynObjects.end() && 
						dynObjects[neighbor->pos]->GetAttr<EntityType>(ATTR_GAME_ENTITY_TYPE) == EntityType::BRIDGE_MARK))
					return true;
			}
		}

		return false;
	}

	bool PositionContainsBridgeMark(Vec2i position) {
		return (dynObjects.find(position) != dynObjects.end() && dynObjects.find(position)->second->GetAttr<EntityType>(ATTR_GAME_ENTITY_TYPE) == EntityType::BRIDGE_MARK);
	}

	void MarkPositionForBridge(Vec2i position) {
		CreateDynamicObject(position, EntityType::BRIDGE_MARK);
	}

	bool IsPositionFreeForForbid(Vec2i position) {
		auto node = hydroqMap->GetNode(position.x, position.y);
		bool isFree = node->mapNodeType == MapNodeType::GROUND && !node->occupied && dynObjects.find(position) == dynObjects.end();
		return isFree;
	}

	bool PositionContainsForbidMark(Vec2i position) {
		return (dynObjects.find(position) != dynObjects.end() && 
			dynObjects.find(position)->second->GetAttr<EntityType>(ATTR_GAME_ENTITY_TYPE) == EntityType::FORBID_MARK);
	}

	void MarkPositionForForbid(Vec2i position) {
		CreateDynamicObject(position, EntityType::FORBID_MARK);
	}

	bool IsPositionFreeForGuard(Vec2i position) {
		auto node = hydroqMap->GetNode(position.x, position.y);
		bool isFree = node->mapNodeType == MapNodeType::GROUND && !node->occupied && dynObjects.find(position) == dynObjects.end();
		return isFree;
	}

	bool PositionContainsGuardMark(Vec2i position) {
		return (dynObjects.find(position) != dynObjects.end() && 
			dynObjects.find(position)->second->GetAttr<EntityType>(ATTR_GAME_ENTITY_TYPE) == EntityType::GUARD_MARK);
	}

	void MarkPositionForGuard(Vec2i position) {
		CreateDynamicObject(position, EntityType::GUARD_MARK);
	}

	bool IsPositionFreeForDestroy(Vec2i position) {
		auto node = hydroqMap->GetNode(position.x, position.y);
		bool isFree = (node->mapNodeType == MapNodeType::GROUND && !node->occupied && dynObjects.find(position) == dynObjects.end());
		return isFree;
	}

	bool PositionContainsDestroyMark(Vec2i position) {
		return (dynObjects.find(position) != dynObjects.end() && 
			dynObjects.find(position)->second->GetAttr<EntityType>(ATTR_GAME_ENTITY_TYPE) == EntityType::DESTROY_MARK);
	}

	void MarkPositionForDestroy(Vec2i position) {
		CreateDynamicObject(position, EntityType::DESTROY_MARK);
	}

	void DestroyDynamicObject(Vec2i position) {
		auto node = hydroqMap->GetNode(position.x, position.y);
		node->occupied = false;
		node->forbidden = false;
		auto obj = dynObjects[position];
		dynObjects.erase(position);
		rootNode->RemoveChild(obj, true);

		SendMessageNoBubbling(StringHash(ACT_MAP_OBJECT_CHANGED), 0,
			new MapObjectChangedEvent(ObjectChangeType::DYNAMIC_REMOVED, node, obj), nullptr);
	}

	Node* CreateNode(EntityType entityType, Vec2i position) {

		string name;

		if (entityType == EntityType::BRIDGE_MARK) {
			name = "bridgemark";
		}
		else if (entityType == EntityType::DESTROY_MARK) {
			name = "destroymark";
		}
		else if (entityType == EntityType::FORBID_MARK) {
			name = "forbidmark";
		}
		else if (entityType == EntityType::GUARD_MARK) {
			name = "guardmark";
		}
		else if (entityType == EntityType::SEEDBED) {
			name = "seedbed";
		}

		Node* nd = new Node(name);
		nd->AddAttr(ATTR_GAME_ENTITY_TYPE, entityType);
		nd->AddAttr(ATTR_ENTITY_POSITION, position);
		dynObjects[position] = nd;
		rootNode->AddChild(nd);

		return nd;
	}

	virtual void Update(const uint64 delta, const uint64 absolute) {
		rootNode->Update(delta, absolute);
	}
};