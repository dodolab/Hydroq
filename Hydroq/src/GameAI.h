#pragma once

#include "HydroqDef.h"
#include "GameBoard.h"
#include "GameModel.h"
#include "CoroutineContext.h"

#include "HydAISimulator.h"
#include "UCTAgent.h"

enum class AITaskType {
	NONE = 0, 
	CAPTURE_EMPTY = 1, 
	CAPTURE_ENEMY = 2, 
	GOTO_EMPTY = 3, 
	GOTO_ENEMY = 4
};

struct AITask {
	AITaskType type;
	vector<Vec2i> positions;
	uint64 created;
	
	AITask() : type(AITaskType::NONE), created(0) {

	}

	AITask(AITaskType type, uint64 created) : type(type), created(created) {

	}
};

struct RigInfo {
	Vec2i position;
	Vec2i nearest;
	int distance;
};


class GameAI : public Behavior {

	GameModel* gameModel;
	Faction faction = Faction::NONE;

	AITask actualTask;
	int actualTaskIndex = 0;

	vector<RigInfo> blueRedDist;
	vector<RigInfo> redBlueDist;
	vector<RigInfo> blueEmptyDist;
	vector<RigInfo> redEmptyDist;

	uint64 lastTaskTime = 0;

public:
	GameAI(GameModel* gameModel, Faction faction) :gameModel(gameModel), faction(faction){

	}

	void OnInit() {
		SubscribeForMessages(ACT_GAMESTATE_CHANGED);
	}

	void OnStart() {
		
	}

	void OnMessage(Msg& msg);

	virtual void Update(const uint64 delta, const uint64 absolute);
protected:

	void UpdateMonteCarlo(vector<RigInfo>& myOpponentDist, vector<RigInfo>& myEmptyDist, uint64 delta, uint64 absolute);

	void CalcRigsDistance();

	void CalcRigDistance(vector<Node*>& refRigs, vector<Vec2i>& refRigsPos, vector<Vec2i>& targetRigsPos, int index, vector<RigInfo>& distances);

	HydAIAction TrySimulator();

	void Task_CaptureEmpty(RigInfo dist, uint64 absolute);

	void Task_CaptureEnemy(RigInfo dist, uint64 absolute);

	void Task_Goto(RigInfo nearestRig, uint64 absolute);

	void BuildAroundTile(RigInfo& nearestRig, GameMapNode* brick, AITask& task, int recursiveLevels);
};