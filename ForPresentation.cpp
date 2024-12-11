// not a part of the project, the file is for presentation and demonstrative purpose only
#include "GameLevel.hpp"

GameLevel::GameLevel() {

}

GameLevel::~GameLevel() {

}

void GameLevel::LoadLevel(const std::string file) {

	this->mBlocks.clear();
	this->mLevelDataCsv.clear();

	std::string line;
	std::ifstream fstream(file);

	if (fstream) {
		// Read the entire file into a vector of strings
		std::vector<std::string> lines;
		while (std::getline(fstream, line)) {
			lines.push_back(line);
		}

		// Iterate over the vector in reverse order
		for (auto it = lines.rbegin(); it != lines.rend(); ++it) {
			std::istringstream sstream(*it);
			std::string item;
			std::vector<std::string> row;
			while (std::getline(sstream, item, ',')) {
				row.push_back(item);
			}
			mLevelDataCsv.push_back(row);
		}
	}
	else {
		std::cout << "Could not load level data" << std::endl;
	}

	isLoaded = true;

}

void GameLevel::LoadLevelJson(const std::string file) {

	std::ifstream jsonStream(file);

	nlohmann::json GameLevelDataJson = nlohmann::json::parse(jsonStream);

	mLevelWidth = GameLevelDataJson["width"];
	mLevelHeight = GameLevelDataJson["height"];

	for (int i = 0; i < GameLevelDataJson["layers"].size(); i++) {
		if (GameLevelDataJson["layers"][i]["name"] == "BaseLayer") {
			std::vector<std::vector<int>> data(mLevelHeight, std::vector<int>(mLevelWidth));
			std::vector vec = GameLevelDataJson["layers"][i]["data"].get<std::vector<int>>();
			for (int j = 0; j < mLevelHeight; j++) {
				for (int k = 0; k < mLevelWidth; k++) {
					data[j][k] = vec[j * mLevelWidth + k];
				}
			}
			mTilesetsOffsets.push_back(GameLevelDataJson["tilesets"][i]["firstgid"]);
			mLevelData.push_back(data);
		}
		else if (GameLevelDataJson["layers"][i]["name"] == "CollisionLayer") {
			std::vector<std::vector<int>> data(mLevelHeight, std::vector<int>(mLevelWidth));
			std::vector vec = GameLevelDataJson["layers"][i]["data"].get<std::vector<int>>();
			for (int j = 0; j < mLevelHeight; j++) {
				for (int k = 0; k < mLevelWidth; k++) {
					data[j][k] = vec[j * mLevelWidth + k];
				}
			}
			mTilesetsOffsets.push_back(GameLevelDataJson["tilesets"][i]["firstgid"]);
			mLevelData.push_back(data);
		}
	}
}

void GameLevel::BuildLevel() {
	for (int i = 0; i < mLevelHeight; ++i) {
		for (int j = 0; j < mLevelWidth; ++j) {
			if (mLevelData[0][i][j] != 0 || mLevelData[1][i][j] != 0) {
				GameObject obj;
				obj.mSprite.vertexData.Position = glm::vec2(j * mBlockSize, ((mLevelHeight - i) * mBlockSize) - mBlockSize);
				obj.mSprite.vertexData.Size = glm::vec2(mBlockSize, mBlockSize);
				if (mLevelData[0][i][j] != 0) {
					obj.mSprite.vertexData.TextureIndex = mLevelData[0][i][j] - mTilesetsOffsets[0] + 1;
					obj.isVisible = true;
				}
				else {
					obj.isVisible = false;
				}
				if ((mLevelData[1][i][j] - mTilesetsOffsets[1]) + 1 == 1 || (mLevelData[1][i][j] - mTilesetsOffsets[1]) + 1 == 15) {
					obj.isCollidable = true;
				}
				else if ((mLevelData[1][i][j] - mTilesetsOffsets[1]) + 1 == 2) {
					obj.isCollidable = false;
					obj.mTriggerAABBPos = glm::vec2(obj.mSprite.vertexData.Position.x + (float)mBlockSize * (3.0f/8.0f), obj.mSprite.vertexData.Position.y);
					obj.mTriggerAABBSize = glm::vec2(mBlockSize / 4, mBlockSize);
					obj.isDeathTrigger = true;
				}
				else if ((mLevelData[1][i][j] - mTilesetsOffsets[1]) + 1 == 3) {
					obj.isCollidable = false;
					obj.mTriggerAABBPos = glm::vec2(obj.mSprite.vertexData.Position.x + (float)mBlockSize * (3.0f / 8.0f), obj.mSprite.vertexData.Position.y);
					obj.mTriggerAABBSize = glm::vec2(mBlockSize / 4, mBlockSize / 2);
					obj.isDeathTrigger = true;
				}
				else if ((mLevelData[1][i][j] - mTilesetsOffsets[1]) + 1 == 4) {
					obj.isCollidable = true;
					obj.mTriggerAABBPos = obj.mSprite.vertexData.Position;
					obj.mTriggerAABBSize = obj.mSprite.vertexData.Size;
					obj.isDeathTrigger = true;
				}
				else {
					obj.isCollidable = false;
				}
				this->mBlocks.push_back(obj);
			}
		}
	}
}
#include "Actor.hpp"

Actor::Actor() {

}

Actor::~Actor() {

}

void Actor::Transform() {
	mModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(mRelativePosition, 0.0f));
}

void Actor::Update() {
	Move();
	Transform();
}

void Actor::Move() {
	mRelativePosition = mPosition - mSprite.vertexData.Position;
}
#include "BlackHole.hpp" 

BlackHole::BlackHole() : gen(rd()), randomVelocity(1.0f, 5.0f) {
	mSprite.vertexData.Size = glm::vec2(400.0f, 400.0f);
	AABBVelocityMultiplier = 360.0f;

	mSprite.vertexData.Position = (EpicenterAABBPos + EpicenterAABBSize / 2.0f) - mSprite.vertexData.Size / 2.0f;
}

BlackHole::~BlackHole() {

}

void BlackHole::Update(std::vector<GameObject>& blocks, Actor& actor, const float& deltaTime, Animation& birthAnim, Animation& loopAnim, Mix_Chunk* bornSound, Mix_Chunk* consumedSound, Mix_Chunk* blackHoleIdle) {
	if (!birthTimerOneShot && !isBorn) {
		mSprite.vertexData.TexturePosition = loopAnim.TexturePosition;
		AnimationSize = loopAnim.Size;
		birthAnim.AnimationTimer = std::chrono::high_resolution_clock::now();
		birthAnim.SingleFrameTimer = std::chrono::high_resolution_clock::now();
		Mix_PlayChannel(8, bornSound, 0);
		birthTimerOneShot = true;
	}

	if (!isBorn) {
		if (!(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - birthAnim.AnimationTimer).count() > birthAnim.AnimationTime + deltaTime * 1000)) {
			if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - birthAnim.SingleFrameTimer).count() > birthAnim.SingleFrameTime + deltaTime * 1000) {
				birthAnim.SingleFrameTimer = std::chrono::high_resolution_clock::now();
				birthAnim.index++;
			}
			mSprite.vertexData.TextureIndex = birthAnim.AnimationTextureIndexes[birthAnim.index];
		}
		else {
			birthAnim.index = 0;
			isBorn = true;
		}
		if (birthAnim.index > birthAnim.AnimationTextureIndexes.size() - 1) {
			birthAnim.index = 0;
		}
	}

	if (isBorn) {
		if (!loopTimerOneShot) {
			loopAnim.AnimationTimer = std::chrono::high_resolution_clock::now();
			loopAnim.SingleFrameTimer = std::chrono::high_resolution_clock::now();
			loopTimerOneShot = true;
		}
		mSprite.vertexData.TextureIndex = loopAnim.AnimationTextureIndexes[loopAnim.index];
		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - loopAnim.AnimationTimer).count() > loopAnim.AnimationTime + deltaTime * 1000) {
			loopAnim.AnimationTimer = std::chrono::high_resolution_clock::now();
			loopAnim.index = 0;
		}
		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - loopAnim.SingleFrameTimer).count() > loopAnim.SingleFrameTime + deltaTime * 1000) {
			loopAnim.SingleFrameTimer = std::chrono::high_resolution_clock::now();
			loopAnim.index++;
		}
		if (loopAnim.index > loopAnim.AnimationTextureIndexes.size() - 1) {
			loopAnim.index = 0;
		}
	}

	AABBSize.x += AABBVelocityMultiplier * deltaTime;
	if (RectVsRect(AABBPos, AABBSize, actor.mPosition, actor.mSprite.vertexData.Size) && actor.isSucked == false && actor.isConsumedByVoid == false && actor.isSuckedPortal == false && actor.mEscaped == false && actor.mDead == false) {
		actor.isCollidable = false;
		actor.isSucked = true;
		actor.flyDirectionNormalized = glm::normalize(EpicenterAABBPos - actor.mPosition);
		actor.velocity = actor.flyDirectionNormalized * (float)randomVelocity(gen);
	}
	if (actor.isSucked == true) {
		if (actor.mPosition.x <= EpicenterAABBPos.x + EpicenterAABBSize.x / 2) {
			actor.isConsumedByVoid = true;
			actor.isVisible = false;
			actor.isSucked = false;
			Mix_PlayChannel(11, consumedSound, 0);
			actor.velocity = glm::vec2(0.0f, 0.0f);
		}

		actor.velocity += actor.velocity * 2.16f * deltaTime;
		
		if (actor.flyAngleTarget == -1.0f) {
			float dot = glm::dot(actor.flyDirectionNormalized, glm::vec2(-1.0f, 0.0f));
			actor.flyAngleTarget = glm::acos(dot);
		}
		glm::vec2 center = glm::vec2(actor.mSprite.vertexData.Position.x + actor.mSprite.vertexData.Size.x / 2, actor.mSprite.vertexData.Position.y + actor.mSprite.vertexData.Size.y / 2);

		actor.mModelMatrix = glm::translate(actor.mModelMatrix, glm::vec3(center, 0.0f));

		float crossProduct = glm::cross(actor.flyDirectionNormalized, glm::vec2(-1.0f, 0.0f));

		if (crossProduct > 0) {
			actor.mModelMatrix = glm::rotate(actor.mModelMatrix, -actor.flyAngle, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else if (crossProduct < 0) {
			actor.mModelMatrix = glm::rotate(actor.mModelMatrix, actor.flyAngle, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		actor.mModelMatrix = glm::translate(actor.mModelMatrix, glm::vec3(-center, 0.0f));

		if (actor.flyAngle < actor.flyAngleTarget) {
			actor.flyAngle += 0.1f * deltaTime;
		}
	}

	//maybe create a vector of all sucked blocks
	for (int i = 0; i < blocks.size(); i++) {
		if (blocks[i].isSucked == true && blocks[i].isConsumedByVoid == false) {
			blocks[i].isCollidable = false;
			blocks[i].mSprite.vertexData.Position = blocks[i].mSprite.vertexData.Position + blocks[i].tempVelocity * deltaTime;
			if (blocks[i].tempVelocity.x > -100000.0f) {
			blocks[i].tempVelocity += blocks[i].tempVelocity * 2.16f * deltaTime;
			}

			if (blocks[i].mSprite.vertexData.Position.x <= EpicenterAABBPos.x + EpicenterAABBSize.x / 2) {
				blocks[i].isConsumedByVoid = true;
				blocks[i].isVisible = false;
				blocks[i].isSucked = false;
				blocks[i].tempVelocity = glm::vec2(0.0f, 0.0f);
			}
		}
		if (blocks[i].isVisible == true && blocks[i].isSucked == true) {

			if (blocks[i].flyAngleTarget == -1.0f) {
				float dot = glm::dot(blocks[i].flyDirectionNormalized, glm::vec2(-1.0f, 0.0f));
				blocks[i].flyAngleTarget = glm::acos(dot);
			}

			glm::mat4 model = glm::mat4(1.0f);

			glm::vec2 center = glm::vec2(blocks[i].mSprite.vertexData.Position.x + blocks[i].mSprite.vertexData.Size.x / 2, blocks[i].mSprite.vertexData.Position.y + blocks[i].mSprite.vertexData.Size.y / 2);

			model = glm::translate(model, glm::vec3(center, 0.0f));

			float crossProduct = glm::cross(blocks[i].flyDirectionNormalized, glm::vec2(-1.0f, 0.0f));

			if (crossProduct > 0) {
				model = glm::rotate(model, -blocks[i].flyAngle, glm::vec3(0.0f, 0.0f, 1.0f));
			}
			else if (crossProduct < 0) {
				model = glm::rotate(model, blocks[i].flyAngle, glm::vec3(0.0f, 0.0f, 1.0f));
			}
			blocks[i].mModelMatrix = glm::translate(model, glm::vec3(-center, 0.0f));

			if (blocks[i].flyAngle < blocks[i].flyAngleTarget) {
				blocks[i].flyAngle += 1.0f * deltaTime;
			}
		}
		if ((blocks[i].mSprite.vertexData.Position.x > AABBPos.x - AABBSize.x * 2) && (blocks[i].mSprite.vertexData.Position.x < AABBPos.x + AABBSize.x * 2)) {
			if (RectVsRect(AABBPos, AABBSize, blocks[i].mSprite.vertexData.Position, blocks[i].mSprite.vertexData.Size) && blocks[i].isVisible == true && blocks[i].isSucked == false) {
				affectedBlocks.push_back({ i , glm::distance2(EpicenterAABBPos, blocks[i].mSprite.vertexData.Position) });
			}
		}
		std::sort(affectedBlocks.begin(), affectedBlocks.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b) {
			return a.second < b.second;
			});

		for (int i = 0; i < affectedBlocks.size(); i++) {
			blocks[affectedBlocks[i].first].isVisible = true;
			blocks[affectedBlocks[i].first].isCollidable = false;
			blocks[affectedBlocks[i].first].isSucked = true;
			blocks[affectedBlocks[i].first].flyDirectionNormalized = glm::normalize(EpicenterAABBPos - blocks[affectedBlocks[i].first].mSprite.vertexData.Position);
			blocks[affectedBlocks[i].first].tempVelocity = blocks[affectedBlocks[i].first].flyDirectionNormalized * (float)randomVelocity(gen);
		}
		affectedBlocks.clear();
	}


	if (isBorn) {
		float distance = glm::distance2(mSprite.vertexData.Position + mSprite.vertexData.Size / 2.0f, actor.mPosition + actor.mSprite.vertexData.Size / 2.0f);

		if (distance < idleRadius) {
			idleVolume = 128.0f - (distance / idleRadius * 128.0f);
			Mix_Volume(13, idleVolume);
			if (!Mix_Playing(13)) {
				Mix_PlayChannel(13, blackHoleIdle, 0);
			}
		}
		else {
			idleVolume = 0;
			Mix_Volume(13, idleVolume);
		}
	}
}
#define GLM_ENABLE_EXPERIMENTAL
#include "CollisionHandler.hpp"
#include "glm/gtx/string_cast.hpp"
#include "Actor.hpp"


bool PointVsRect(const glm::vec2& p, const Box* r)
{
	return (p.x >= r->Position.x && p.y >= r->Position.y && p.x < r->Position.x + r->Size.x && p.y < r->Position.y + r->Size.y);
}

bool RectVsRect(const glm::vec2 rect1Pos, const glm::vec2 rect1Size, const glm::vec2 rect2Pos, const glm::vec2 rect2Size)
{
	return (rect1Pos.x < rect2Pos.x + rect2Size.x && rect1Pos.x + rect1Size.x > rect2Pos.x && rect1Pos.y < rect2Pos.y + rect2Size.y && rect1Pos.y + rect1Size.y > rect2Pos.y);
}

bool RayVsRect(const glm::vec2& rayOrigin, const glm::vec2& rayDirection, const Box* target, glm::vec2& contactPoint, glm::vec2& contactNormal, float& hitTimeNear)
{
	contactNormal = { 0,0 };
	contactPoint = { 0,0 };

	// Cache division
	glm::vec2 invDirection = 1.0f / rayDirection;

	// Calculate intersections with rectangle bounding axes
	glm::vec2 timeNear = (target->Position - rayOrigin) * invDirection;
	glm::vec2 timeFar = (target->Position + target->Size - rayOrigin) * invDirection;

	if (std::isnan(timeFar.y) || std::isnan(timeFar.x)) {
		return false;
	}
	if (std::isnan(timeNear.y) || std::isnan(timeNear.x)) {
		return false;
	}

	// Sort distances
	if (timeNear.x > timeFar.x) {
		std::swap(timeNear.x, timeFar.x);
	}
	if (timeNear.y > timeFar.y) {
		std::swap(timeNear.y, timeFar.y);
	}

	// Early rejection		
	if (timeNear.x > timeFar.y || timeNear.y > timeFar.x) {
		return false;
	}

	// Closest 'time' will be the first contact
	hitTimeNear = std::max(timeNear.x, timeNear.y);

	// Furthest 'time' is contact on opPositionite side of target
	float hitTimeFar = std::min(timeFar.x, timeFar.y);

	// Reject if ray direction is pointing away from object
	if (hitTimeFar < 0) {
		return false;
	}

	// Contact point of collision from parametric line equation
	contactPoint = rayOrigin + hitTimeNear * rayDirection;

	if (timeNear.x > timeNear.y) {
		if (invDirection.x < 0) {
			contactNormal = { 1, 0 };
		}
		else {
			contactNormal = { -1, 0 };
		}
	}
	else if (timeNear.x < timeNear.y) {
		if (invDirection.y < 0) {
			contactNormal = { 0, 1 };
		}
		else {
			contactNormal = { 0, -1 };
		}
	}
	return true;
}

bool DynamicRectVsRect(const Box& dynamicBox, const float deltaTime, const Box& staticBox, const glm::vec2& dynamicBoxVelocity,
	glm::vec2& contactPoint, glm::vec2& contactNormal, float& contactTime, glm::vec2& position)
{
	if (dynamicBoxVelocity.x == 0 && dynamicBoxVelocity.y == 0)
		return false;

	Box expanded_target;
	expanded_target.Position = staticBox.Position - ( dynamicBox.Size / 2.0f );
	expanded_target.Size = staticBox.Size + dynamicBox.Size;
	
	if (RayVsRect(position + (dynamicBox.Size / 2.0f), dynamicBoxVelocity * deltaTime, &expanded_target, contactPoint, contactNormal, contactTime)) {
		return (contactTime >= 0.0f && contactTime < 1.0f);
	}
	else {
		return false;
	}
}

bool ResolveDynamicRectVsRect(Box& dynamicBox, const float deltaTime, const Box& staticBox, glm::vec2& dynamicBoxVelocity, Actor& actor, glm::vec2& averagedNormal, bool& NormalGroundCheck)
{

	glm::vec2 contactPoint, contactNormal;
	float contactTime = 0.0f;
	if (DynamicRectVsRect(actor.mSprite.vertexData, deltaTime, staticBox, actor.velocity, contactPoint, contactNormal, contactTime, actor.mPosition))
	{
		//if (contactNormal.y > 0) dynamicBox->contact[0] = staticBox; else nullptr;
		//if (contactNormal.x < 0) dynamicBox->contact[1] = staticBox; else nullptr;
		//if (contactNormal.y < 0) dynamicBox->contact[2] = staticBox; else nullptr;
		//if (contactNormal.x > 0) dynamicBox->contact[3] = staticBox; else nullptr;

		if (contactNormal.x >= 1 && contactNormal.x != 0 && actor.velocity.y != 0) {
			actor.isWallMountableL = true; 
		}
		else {
			actor.isWallMountableL = false;
		} 
		if (contactNormal.x <= -1 && contactNormal.x != 0 && actor.velocity.y != 0) {
			actor.isWallMountableR = true; 
		} 
		else {
			actor.isWallMountableR = false;
		}

		averagedNormal = glm::mix(averagedNormal, contactNormal, 0.5f);

		if (averagedNormal.y > 0.25f) {
			NormalGroundCheck = true;
		}
		else {
			NormalGroundCheck = false;
		}

		actor.velocity += contactNormal * glm::vec2(std::abs(actor.velocity.x), std::abs(actor.velocity.y)) * (1 - contactTime);
		if (contactNormal.x < 0) {
		
		}
		return true;
	}
	return false;
}

void CollisionUpdate(const std::vector<GameObject>& blocks, Actor& actor, bool& LeftWallHug, bool& RightWallHug, const float& deltaTime, bool& isGrounded) {

	static glm::vec2 averagedNormal(0.0f, 0.0f);
	static bool NormalGroundCheck = false;
	static bool BottomWallHug = false;
	
	glm::vec2 contactPoint, contactNormal;

	float contactTime = 0;

	std::vector<std::pair<int, float>> colidedBlocks;

	// Bottom left corner of broad-phase-box
	glm::vec2 A(actor.mPosition.x - 3 * actor.mSprite.vertexData.Size.x, actor.mPosition.y - 3 * actor.mSprite.vertexData.Size.y);
	// Top right corner of broad-phase-box
	glm::vec2 B(actor.mPosition.x + 4 * actor.mSprite.vertexData.Size.x, actor.mPosition.y + 4 * actor.mSprite.vertexData.Size.y);

	LeftWallHug = false;
	RightWallHug = false;
	BottomWallHug = false;
	for (int i = 0; i < blocks.size(); i++) {
		if (blocks[i].mSprite.vertexData.Position.x > A.x && blocks[i].mSprite.vertexData.Position.x < B.x && blocks[i].mSprite.vertexData.Position.y > A.y && blocks[i].mSprite.vertexData.Position.y < B.y && actor.isCollidable == true) {
			if (!blocks[i].isDeathTrigger && blocks[i].isCollidable == true) {
				if (DynamicRectVsRect(actor.mSprite.vertexData, deltaTime, blocks[i].mSprite.vertexData, actor.velocity, contactPoint, contactNormal, contactTime, actor.mPosition)) {
					colidedBlocks.push_back({ i, contactTime });
				}
				if (blocks[i].mSprite.vertexData.Position.x == actor.mPosition.x + actor.mSprite.vertexData.Size.x && actor.mPosition.y < blocks[i].mSprite.vertexData.Position.y + blocks[i].mSprite.vertexData.Size.y && actor.mPosition.y + actor.mSprite.vertexData.Size.y > blocks[i].mSprite.vertexData.Position.y && !isGrounded) {
					RightWallHug = RectVsRect(glm::vec2(actor.mPosition.x + actor.mSprite.vertexData.Size.x / 2, actor.mPosition.y), actor.mSprite.vertexData.Size, blocks[i].mSprite.vertexData.Position, blocks[i].mSprite.vertexData.Size);
				}
				if (blocks[i].mSprite.vertexData.Position.x + blocks[i].mSprite.vertexData.Size.x == actor.mPosition.x && actor.mPosition.y < blocks[i].mSprite.vertexData.Position.y + blocks[i].mSprite.vertexData.Size.y && actor.mPosition.y + actor.mSprite.vertexData.Size.y > blocks[i].mSprite.vertexData.Position.y && !isGrounded) {
					LeftWallHug = RectVsRect(glm::vec2(actor.mPosition.x - actor.mSprite.vertexData.Size.x / 2, actor.mPosition.y), actor.mSprite.vertexData.Size, blocks[i].mSprite.vertexData.Position, blocks[i].mSprite.vertexData.Size);
				} 
				if (blocks[i].mSprite.vertexData.Position.y + blocks[i].mSprite.vertexData.Size.y == actor.mPosition.y && actor.mPosition.x < blocks[i].mSprite.vertexData.Position.x + blocks[i].mSprite.vertexData.Size.x && actor.mPosition.x + actor.mSprite.vertexData.Size.x > blocks[i].mSprite.vertexData.Position.x) {
					BottomWallHug = RectVsRect(glm::vec2(actor.mPosition.x, actor.mPosition.y - actor.mSprite.vertexData.Size.y / 2), actor.mSprite.vertexData.Size, blocks[i].mSprite.vertexData.Position, blocks[i].mSprite.vertexData.Size);
				}
			}
			else if (blocks[i].isDeathTrigger) {
				Box AABB;
				AABB.Position = blocks[i].mTriggerAABBPos;
				AABB.Size = blocks[i].mTriggerAABBSize;
				if (DynamicRectVsRect(actor.mSprite.vertexData, deltaTime, AABB, actor.velocity, contactPoint, contactNormal, contactTime, actor.mPosition)) {
					if (blocks[i].isCollidable) {
						colidedBlocks.push_back({ i, contactTime });
					}
					actor.mDead = true;
				}
				if (blocks[i].mSprite.vertexData.Position.y + blocks[i].mSprite.vertexData.Size.y == actor.mPosition.y && actor.mPosition.x < blocks[i].mSprite.vertexData.Position.x + blocks[i].mSprite.vertexData.Size.x && actor.mPosition.x + actor.mSprite.vertexData.Size.x > blocks[i].mSprite.vertexData.Position.x) {
					if (blocks[i].isCollidable) {
						BottomWallHug = RectVsRect(glm::vec2(actor.mPosition.x, actor.mPosition.y - actor.mSprite.vertexData.Size.y / 2), actor.mSprite.vertexData.Size, blocks[i].mSprite.vertexData.Position, blocks[i].mSprite.vertexData.Size);
					}
				}
			}
		}
	}

	std::sort(colidedBlocks.begin(), colidedBlocks.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b) {
		return a.second < b.second;
		});

	for (auto j : colidedBlocks) {
		ResolveDynamicRectVsRect(actor.mSprite.vertexData, deltaTime, blocks[j.first].mSprite.vertexData, actor.velocity, actor, averagedNormal, NormalGroundCheck);
	}

	if (BottomWallHug && NormalGroundCheck && actor.velocity.y == 0.0f) {
		isGrounded = true;
	}
	else {
		isGrounded = false;
	}
	actor.mPosition += actor.velocity * deltaTime;

	actor.mPosition = glm::vec2(float(std::round(actor.mPosition.x * 1000)) / 1000.0f, float(std::round(actor.mPosition.y * 1000)) / 1000.0f);
}
#include "EscapePortal.hpp"

EscapePortal::EscapePortal() {
	suctionVelocity = 100.0f;
	mSprite.vertexData.Position = glm::vec2(17840.0f, 475.0f);
}

EscapePortal::~EscapePortal() {

}

void EscapePortal::Update(Animation& portalAnim, const float& deltaTime, Actor& actor, Mix_Chunk* portalSoundEscape, Mix_Chunk* portalSoundIdle) {
	if (animationOneShot) {
		EpicenterAABBPos = glm::vec2(mSprite.vertexData.Position.x + mSprite.vertexData.Size.x / 2.0f, mSprite.vertexData.Position.y + mSprite.vertexData.Size.y / 4);
		EpicenterAABBSize = glm::vec2(mSprite.vertexData.Size.x / 4, mSprite.vertexData.Size.y / 2);

		AABBSize = glm::vec2(250.0f, 250.0f);
		AABBPos = glm::vec2(EpicenterAABBPos.x + EpicenterAABBSize.x / 2.0f - AABBSize.x / 2.0f, EpicenterAABBPos.y + EpicenterAABBSize.y / 2.0f - AABBSize.y / 2.0f);

		AnimationSize = portalAnim.Size;
		mSprite.vertexData.TexturePosition = portalAnim.TexturePosition;
		portalAnim.AnimationTimer = std::chrono::high_resolution_clock::now();
		portalAnim.SingleFrameTimer = std::chrono::high_resolution_clock::now();
		animationOneShot = false;
	}

	if (RectVsRect(AABBPos, AABBSize, actor.mPosition, actor.mSprite.vertexData.Size) && actor.isSuckedPortal == false && actor.mEscaped == false) {
		actor.isCollidable = false;
		actor.isSuckedPortal = true;
		actor.flyDirectionNormalized = glm::normalize(EpicenterAABBPos + (EpicenterAABBSize / 3.0f) - actor.mPosition);
		actor.velocity = actor.flyDirectionNormalized * suctionVelocity;
	}
	if (actor.isSuckedPortal == true) {
		if (actor.mPosition.x >= EpicenterAABBPos.x + EpicenterAABBSize.x / 2) {
			actor.mEscaped = true;
			actor.isVisible = false;
			actor.isSuckedPortal = false;
			Mix_PlayChannel(9, portalSoundEscape, 0);
			actor.velocity = glm::vec2(0.0f, 0.0f);
		}

		actor.velocity += actor.velocity * 6.16f * deltaTime;

		if (actor.flyAngleTargetPortal == -1.0f) {
			float dot = glm::dot(actor.flyDirectionNormalized, glm::vec2(1.0f, 0.0f));
			actor.flyAngleTargetPortal = glm::acos(dot);
		}
		glm::vec2 center = glm::vec2(actor.mSprite.vertexData.Position.x + actor.mSprite.vertexData.Size.x / 2, actor.mSprite.vertexData.Position.y + actor.mSprite.vertexData.Size.y / 2);

		actor.mModelMatrix = glm::translate(actor.mModelMatrix, glm::vec3(center, 0.0f));

		float crossProduct = glm::cross(actor.flyDirectionNormalized, glm::vec2(-1.0f, 0.0f));

		if (crossProduct > 0) {
			actor.mModelMatrix = glm::rotate(actor.mModelMatrix, actor.flyAnglePortal, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else if (crossProduct < 0) {
			actor.mModelMatrix = glm::rotate(actor.mModelMatrix, -actor.flyAnglePortal, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		actor.mModelMatrix = glm::translate(actor.mModelMatrix, glm::vec3(-center, 0.0f));

		if (actor.flyAnglePortal < actor.flyAngleTargetPortal) {
			actor.flyAnglePortal += 0.1f * deltaTime;
		}
	}

	float distance = glm::distance2(mSprite.vertexData.Position + mSprite.vertexData.Size / 2.0f, actor.mPosition + actor.mSprite.vertexData.Size / 2.0f);

	if (distance < idleRadius) {
		idleVolume = 128.0f - (distance / idleRadius * 128.0f);
		Mix_Volume(12, idleVolume);
		if (!Mix_Playing(12)) {
			Mix_PlayChannel(12, portalSoundIdle, 0);
		}
	}
	else {
		idleVolume = 0;
		Mix_Volume(12, idleVolume);
	}

	if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - portalAnim.AnimationTimer).count() > portalAnim.AnimationTime + deltaTime * 1000) {
		portalAnim.AnimationTimer = std::chrono::high_resolution_clock::now();
		portalAnim.index = 0;
	}
	if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - portalAnim.SingleFrameTimer).count() > portalAnim.SingleFrameTime + deltaTime * 1000) {
		portalAnim.SingleFrameTimer = std::chrono::high_resolution_clock::now();
		portalAnim.index++;
	}
	if (portalAnim.index > portalAnim.AnimationTextureIndexes.size() - 1) {
		portalAnim.index = 0;
	}
	mSprite.vertexData.TextureIndex = portalAnim.AnimationTextureIndexes[portalAnim.index];
}
#include "GameEntity.hpp"

GameEntity::GameEntity() {

}

GameEntity::~GameEntity() {

}

void GameEntity::Update() {

}

void GameEntity::SetSprite(const Sprite& sprite) {
	mSprite = sprite;
}
#include "GameObject.hpp"

GameObject::GameObject() {

}

GameObject::~GameObject() {

}

void GameObject::Update() {
	mPosition = mSprite.vertexData.Position;
}
#include "MovementHandler.hpp"
#include "glm/gtx/string_cast.hpp"

MovementHandler::MovementHandler() {
}

MovementHandler::~MovementHandler() {
}

void MovementHandler::Move(glm::vec2& actorVelocity, const glm::vec2& acceleration) {
	actorVelocity = glm::vec2(actorVelocity.x + acceleration.x, actorVelocity.y + acceleration.y);
}

void MovementHandler::Jump(float& deltaTime, const float& jumpSpeed, glm::vec2& actorVelocity) {
	actorVelocity.y += jumpSpeed * deltaTime;
}

void MovementHandler::Slam(float& deltaTime, const float& slamSpeed, const float& speedLimit, glm::vec2& actorVelocity) {
	actorVelocity.y -= slamSpeed * deltaTime;

	if (actorVelocity.y < -speedLimit) {
		actorVelocity.y = -speedLimit;
	}

}
void MovementHandler::Update(float& deltaTime, Actor& actor) {
	if ((!actor.isSucked && !actor.isConsumedByVoid && !actor.mEscaped && !actor.isSuckedPortal)) {

		// Gravity and wall stick vvv
		if (canWallStick) {
			actor.velocity.y = -125.0f;
		}

		if (actor.velocity.y <= 0.0f) {
			actor.velocity.y -= 1920.0f * deltaTime;
		}
		else if (actor.velocity.y > 0.0f) {
			actor.velocity.y -= 1440.0f * deltaTime;
		}
		// Gravity and wall stick ^^^

		// Side movement vvv
		if (KeyboadStates[static_cast<int>(MovementState::MOVE_LEFT)] && !actor.mDead) {
			if (isGrounded) {
				if (!(actor.velocity.x + -4000.0f * deltaTime < -500.0f)) {
					Move(actor.velocity, glm::vec2(-4000.0f, 0.0f) * deltaTime);
				}
			}
			else {
				if (!(actor.velocity.x + -1440.0f * deltaTime < -500.0f)) {
					Move(actor.velocity, glm::vec2(-1440.0f, 0.0f) * deltaTime);
				}
			}
			lookDirection = LookDirections::LEFT;
		}
		if (KeyboadStates[static_cast<int>(MovementState::MOVE_RIGHT)] && !actor.mDead) {
			if (isGrounded) {
				if (!(actor.velocity.x + 4000.0f * deltaTime > 500.0f)) {
					Move(actor.velocity, glm::vec2(4000.0f, 0.0f) * deltaTime);
				}
			}
			else {
				if (!(actor.velocity.x + 1440.0f * deltaTime > 500.0f)) {
					Move(actor.velocity, glm::vec2(1440.0f, 0.0f) * deltaTime);
				}
			}
			lookDirection = LookDirections::RIGHT;
		}
		// Side movement ^^^

		if (Sign(actor.velocity.x) == -1) {
			lookDirection = LookDirections::LEFT;
		}
		else if (Sign(actor.velocity.x) == 1) {
			lookDirection = LookDirections::RIGHT;
		}


		// Slam and slide vvv
		if (KeyboadStates[static_cast<int>(MovementState::DUCK)] && !actor.mDead) {

			if (duckOneShot) {
				if (!isGrounded) { 
					isSlamming = true;
					canDoubleJump = false;
					duckOneShot = false;
				}
				if (isGrounded && (actor.velocity.x > 200.0f || actor.velocity.x < -200.0f)) {
					slideDirection = Sign(actor.velocity.x);
					isSliding = true;
					duckOneShot = false;
				}
			}
			
		}


		if (!KeyboadStates[static_cast<int>(MovementState::DUCK)] || !isGrounded) {
			slideOneShot = true;
		}

		if (KeyboadStates[static_cast<int>(MovementState::MOVE_LEFT)] && slideDirection == 1 && !actor.mDead) {
			isSliding = false;
		}

		if (KeyboadStates[static_cast<int>(MovementState::MOVE_RIGHT)] && slideDirection == -1 && !actor.mDead) {
			isSliding = false;
		}


		if (!KeyboadStates[static_cast<int>(MovementState::DUCK)]) {
			frictionModifier = 7.2f;
			isSliding = false; 
		}


		if (isSlamming) {
			Slam(deltaTime, 5000.0f, 2000.0f, actor.velocity);
		}

		if (isSliding) {
			if (slideOneShot && isGrounded) {
				frictionModifier = 0.25f;
				actor.velocity.x = actor.velocity.x + 200.0f * slideDirection;
				slideOneShot = false;
			}
		}
		

		if (isGrounded || (!isGrounded && canWallStick)) {
			isSlamming = false;
		}


		// Slam and slide ^^^

		// Jumping, wall jumping, double jumping vvv
		if (KeyboadStates[static_cast<int>(MovementState::SPACE)] && !actor.mDead) {
			if (spacebarOneShot)
			{
				if (canWallStick) {
					isWallJumping = true;
				}
				
				jumpBufferTimer = std::chrono::high_resolution_clock::now();
				
				if (!isGrounded && !canWallStick && canDoubleJump) {
					canDoubleJump = false;
					actor.velocity.y = 320.0f;
					doubleJumpTimer = std::chrono::high_resolution_clock::now();
				}
			}
			spacebarOneShot = false;
			if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - doubleJumpTimer).count() < doubleJumpTime + deltaTime * 1000) {
				Jump(deltaTime, 3000.0f, actor.velocity);
			}

			if ((isJumping && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - jumpTimer).count() < jumpTime + deltaTime * 1000)) {
				Jump(deltaTime, 3000.0f, actor.velocity);
			}

			if (isWallJumping && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - wallJumpTimer).count() < wallJumpTime + deltaTime * 1000) {
				Jump(deltaTime, 2000.0f, actor.velocity);
			}
		}
		if (!KeyboadStates[static_cast<int>(MovementState::SPACE)]) { 
			isJumping = false;
			isWallJumping = false;
		}
		
		if (canWallStick && isWallJumping) {
			canDoubleJump = true;
			wallJumpTimer = std::chrono::high_resolution_clock::now();
			actor.velocity.y = 320.0f;
			if (actor.isWallMountableL) {
				actor.velocity.x = 300.0f;
			}
			else if (actor.isWallMountableR) {
				actor.velocity.x = -300.0f;
			}
			actor.isWallMountableL = false;
			actor.isWallMountableR = false;
		}

		if (((actor.isWallMountableR && RightWallHug) || (actor.isWallMountableL && LeftWallHug)) && !(KeyboadStates[static_cast<int>(MovementState::SPACE)] && actor.velocity.y > 0.0f)) {
			canWallStick = true;
			canDoubleJump = true;
		}
		else {
			canWallStick = false;
		}

		if (isGrounded 
				&& std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - jumpBufferTimer).count() < jumpBufferTime + deltaTime * 1000) {
			//Jump();
			canDoubleJump = true;
			isJumping = true;
			jumpTimer = std::chrono::high_resolution_clock::now();
			actor.velocity.y += 320.0f;
			isGrounded = false;
		}

		if (isGrounded) {
			canDoubleJump = true;
			isJumping = false;
		}

		if ((actor.isWallMountableR && RightWallHug) || (actor.isWallMountableL && LeftWallHug)) {
			isWallJumping = false;
		}

		// Jumping, wall jumping, double jumping ^^^

		// Friction and limits vvvq

		if (isGrounded) {
			float nextActorVelocityX = actor.velocity.x *= 1.0f - (frictionModifier * deltaTime);
			if (nextActorVelocityX > 0.1f || nextActorVelocityX < -0.1f) {
				actor.velocity.x = nextActorVelocityX;
			}
			else {
				actor.velocity.x = 0.0f;
			}
		}
		if (actor.velocity.x > 800.0f) {
			actor.velocity.x = 800.0f;
		}
		if (actor.velocity.x < -800.0f) {
			actor.velocity.x = -800.0f;
		}
		if (actor.velocity.y < -1200.0f) {
			actor.velocity.y = -1200.0f;
		}

		// Friction and limits ^^^
	}
}
#include "Sprite.hpp"

Sprite::Sprite() {
	vertexData.Position = glm::vec2(0.0f, 0.0f);
	vertexData.Size = glm::vec2(0.0f, 0.0f);
	vertexData.Color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

Sprite::~Sprite() {

}
#include "StateMachine.hpp"

StateMachine::StateMachine() {

}

StateMachine::~StateMachine() {

}


bool StateMachine::CheckPlayerStateChange() {
	return currentPlayerState != lastState;
}

bool StateMachine::CompareToLastState(const PlayerStates& state) {
	return state == lastState;
}

void StateMachine::CheckPlayerState(Actor& actor, MovementHandler& movementHandler) {
	lastState = currentPlayerState;
	if (!movementHandler.isGrounded && !movementHandler.canWallStick && movementHandler.canDoubleJump) {
		if (actor.velocity.y > 0.0f) {
			currentPlayerState = PlayerStates::JUMPING;
			// jump
		}
		else if (actor.velocity.y <= 0.0f) {
			currentPlayerState = PlayerStates::FALLING;
			// fall
		}
	}
	else if (!movementHandler.isGrounded && !movementHandler.canWallStick && !movementHandler.canDoubleJump) {
		// double jump
		if (actor.velocity.y > 0.0f) {
			currentPlayerState = PlayerStates::DOUBLE_JUMPING;
			// jump
		}
		else if (actor.velocity.y <= 0.0f) {
			currentPlayerState = PlayerStates::FALLING;
			// fall
		}
	}
	else if (!movementHandler.isGrounded && movementHandler.canWallStick) {
		currentPlayerState = PlayerStates::WALLSLIDING;
		// wall slide
	}
	else {
		if (!(actor.velocity.x < 1.0f && actor.velocity.x > -1.0f) && (movementHandler.KeyboadStates[static_cast<int>(MovementState::MOVE_LEFT)] || movementHandler.KeyboadStates[static_cast<int>(MovementState::MOVE_RIGHT)])) {
			currentPlayerState = PlayerStates::RUNNING;
			// run
		}
		else {
			currentPlayerState = PlayerStates::IDLE;
			// idle
		}
	}
	if (movementHandler.isSliding && movementHandler.isGrounded) {
		currentPlayerState = PlayerStates::SLIDING;
		// slide
	}
	if (movementHandler.isSlamming) {
		currentPlayerState = PlayerStates::SLAMMING;
		// slam
	} 
	if (actor.mDead) {
		currentPlayerState = PlayerStates::DEAD;
	}
	if (actor.isSucked || actor.isSuckedPortal) {
		currentPlayerState = PlayerStates::SUCKED;
	}
	if (actor.isConsumedByVoid) {
		currentPlayerState = PlayerStates::VOID_CONSUMED;
	}
	if (actor.mEscaped) {
		currentPlayerState = PlayerStates::ESCAPED;
	}
}

void StateMachine::Update(MovementHandler& movementHandler, AnimationHandler& animationHandler, AudioHandler& audioHandler, Actor& actor, const float& deltaTime) {
	CheckPlayerState(actor, movementHandler);
	switch (movementHandler.lookDirection) {
	case LookDirections::LEFT:
		mActorFlipped = true;
		break;
	case LookDirections::RIGHT:
		mActorFlipped = false;
		break;
	default:
		break;
	}

	switch (currentPlayerState) {
	case PlayerStates::RUNNING:
		if (runAnimationOneShot) {
			animationHandler.RunAnimation.AnimationTimer = std::chrono::high_resolution_clock::now();
			animationHandler.RunAnimation.SingleFrameTimer = std::chrono::high_resolution_clock::now();
			runAnimationOneShot = false;
		}
		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - animationHandler.RunAnimation.AnimationTimer).count() > animationHandler.RunAnimation.AnimationTime + deltaTime * 1000) {
			animationHandler.RunAnimation.AnimationTimer = std::chrono::high_resolution_clock::now();
			animationHandler.RunAnimation.index = 0;
		}
		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - animationHandler.RunAnimation.SingleFrameTimer).count() > animationHandler.RunAnimation.SingleFrameTime + deltaTime * 1000) {
			animationHandler.RunAnimation.SingleFrameTimer = std::chrono::high_resolution_clock::now();
			animationHandler.RunAnimation.index++;
		}
		if (animationHandler.RunAnimation.index > animationHandler.RunAnimation.AnimationTextureIndexes.size() - 1) {
			animationHandler.RunAnimation.index = 0;
		}
		if ((animationHandler.RunAnimation.index == 1 || animationHandler.RunAnimation.index == 2 || animationHandler.RunAnimation.index == 4 || animationHandler.RunAnimation.index == 5) && playingRunSound == false) {
			playingRunSound = true;
		}

		if ((animationHandler.RunAnimation.index == 3 || animationHandler.RunAnimation.index == 0) && playingRunSound == true) {
			playingRunSound = false;
			audioHandler.PlayNextStepSound();
		}

		mCurrentActorDrawSize = animationHandler.RunAnimation.Size * actor.mSizeMultiplier;
		mCurrentActorTextureSize = animationHandler.RunAnimation.Size;
		mCurrentActorTexturePosition = animationHandler.RunAnimation.TexturePosition;
		mCurrentActorTextureIndex = animationHandler.RunAnimation.AnimationTextureIndexes[animationHandler.RunAnimation.index];
		break;
	case PlayerStates::JUMPING:
		mCurrentActorDrawSize = animationHandler.JumpAnimation.Size * actor.mSizeMultiplier;
		mCurrentActorTextureSize = animationHandler.JumpAnimation.Size;
		mCurrentActorTexturePosition = animationHandler.JumpAnimation.TexturePosition;
		mCurrentActorTextureIndex = animationHandler.JumpAnimation.AnimationTextureIndexes[0];

		if (CheckPlayerStateChange()) {
			Mix_PlayChannel(2, audioHandler.Jump, 0);
		}
		animationHandler.RunAnimation.index = 0;
		runAnimationOneShot = true;
		break;
	case PlayerStates::DOUBLE_JUMPING:
		mCurrentActorDrawSize = animationHandler.JumpAnimation.Size * actor.mSizeMultiplier;
		mCurrentActorTextureSize = animationHandler.JumpAnimation.Size;
		mCurrentActorTexturePosition = animationHandler.JumpAnimation.TexturePosition;
		mCurrentActorTextureIndex = animationHandler.JumpAnimation.AnimationTextureIndexes[0];
		if (CheckPlayerStateChange()) {
			Mix_PlayChannel(3, audioHandler.DoubleJump, 0);
		}
		animationHandler.RunAnimation.index = 0;
		runAnimationOneShot = true;
		break;
	case PlayerStates::FALLING:
		mCurrentActorDrawSize = animationHandler.FallAnimation.Size * actor.mSizeMultiplier;
		mCurrentActorTextureSize = animationHandler.FallAnimation.Size;
		mCurrentActorTexturePosition = animationHandler.FallAnimation.TexturePosition;
		mCurrentActorTextureIndex = animationHandler.FallAnimation.AnimationTextureIndexes[0];

		if (!Mix_Playing(4)) {
			Mix_PlayChannel(4, audioHandler.WindSoft, 0);
		}

		if (FallVolumeTimer >= FallVolumeTime) {
			FallVolumeTimer = 0.0f;
			FallVolume += 10;
			Mix_Volume(4, FallVolume);
		}
		else {
			FallVolumeTimer += deltaTime;
		}
		animationHandler.RunAnimation.index = 0;
		runAnimationOneShot = true;
		break;
	case PlayerStates::SLIDING:
		mCurrentActorDrawSize = animationHandler.SlideAnimation.Size * actor.mSizeMultiplier;
		mCurrentActorTextureSize = animationHandler.SlideAnimation.Size;
		mCurrentActorTexturePosition = animationHandler.SlideAnimation.TexturePosition;
		mCurrentActorTextureIndex = animationHandler.SlideAnimation.AnimationTextureIndexes[0];

		if (!Mix_Playing(5)) {
			Mix_PlayChannel(5, audioHandler.Slide, 0);
		}
		animationHandler.RunAnimation.index = 0;
		runAnimationOneShot = true;
		break;
	case PlayerStates::SLAMMING:
		mCurrentActorDrawSize = animationHandler.SlamAnimation.Size * actor.mSizeMultiplier;
		mCurrentActorTextureSize = animationHandler.SlamAnimation.Size;
		mCurrentActorTexturePosition = animationHandler.SlamAnimation.TexturePosition;
		mCurrentActorTextureIndex = animationHandler.SlamAnimation.AnimationTextureIndexes[0];

		animationHandler.RunAnimation.index = 0;
		runAnimationOneShot = true;
		break;
	case PlayerStates::WALLSLIDING:
		mCurrentActorDrawSize = animationHandler.WallSlideAnimation.Size * actor.mSizeMultiplier;
		mCurrentActorTextureSize = animationHandler.WallSlideAnimation.Size;
		mCurrentActorTexturePosition = animationHandler.WallSlideAnimation.TexturePosition;
		mCurrentActorTextureIndex = animationHandler.WallSlideAnimation.AnimationTextureIndexes[0];
		mActorFlipped = !mActorFlipped;


		if (!Mix_Playing(6)) {
			Mix_PlayChannel(6, audioHandler.WallSlide, 0);
		}
		animationHandler.RunAnimation.index = 0;
		runAnimationOneShot = true;
		break;
	case PlayerStates::DEAD:
		if (deadAnimOneShot) {
			animationHandler.DeadAnimation.AnimationTimer = std::chrono::high_resolution_clock::now();
			animationHandler.DeadAnimation.SingleFrameTimer = std::chrono::high_resolution_clock::now();
			animationHandler.DeadAnimation.index = 0;
			deadAnimOneShot = false;
		}
		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - animationHandler.DeadAnimation.AnimationTimer).count() > animationHandler.DeadAnimation.AnimationTime + deltaTime * 1000) {
			deadAnimDone = true;
		}
		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - animationHandler.DeadAnimation.SingleFrameTimer).count() > animationHandler.DeadAnimation.SingleFrameTime + deltaTime * 1000) {
			if (!deadAnimDone) {
				animationHandler.DeadAnimation.SingleFrameTimer = std::chrono::high_resolution_clock::now();
				animationHandler.DeadAnimation.index++;
			}
		}
		if (animationHandler.DeadAnimation.index > animationHandler.DeadAnimation.AnimationTextureIndexes.size() - 1) {
			animationHandler.DeadAnimation.index = 0;
		}

		mCurrentActorDrawSize = animationHandler.DeadAnimation.Size * actor.mSizeMultiplier;
		mCurrentActorTextureSize = animationHandler.DeadAnimation.Size;
		mCurrentActorTexturePosition = animationHandler.DeadAnimation.TexturePosition;
		mCurrentActorTextureIndex = animationHandler.DeadAnimation.AnimationTextureIndexes[animationHandler.DeadAnimation.index];


		break;
	case PlayerStates::HIT:

		break;
	case PlayerStates::IDLE:
		mCurrentActorDrawSize = animationHandler.IdleAnimation.Size * actor.mSizeMultiplier;
		mCurrentActorTextureSize = animationHandler.IdleAnimation.Size;
		mCurrentActorTexturePosition = animationHandler.IdleAnimation.TexturePosition;
		mCurrentActorTextureIndex = animationHandler.IdleAnimation.AnimationTextureIndexes[0];

		animationHandler.RunAnimation.index = 0;
		runAnimationOneShot = true;
		break;
	case PlayerStates::SUCKED:

		if (!Mix_Playing(10)) {
			Mix_PlayChannel(10, audioHandler.WindHard, 0);
		}

		if (SuckedVolumeTimer >= SuckedVolumeTime) {
			SuckedVolumeTimer = 0.0f;
			SuckedVolume += 7;
			Mix_Volume(10, SuckedVolume);
		}
		else {
			SuckedVolumeTimer += deltaTime;
		}

		break;
	case PlayerStates::VOID_CONSUMED:
		break;
	case PlayerStates::ESCAPED:
		break;
	default:
		break;
	}

	if (CheckPlayerStateChange()) {
		if (CompareToLastState(PlayerStates::SLAMMING) && currentPlayerState != PlayerStates::SLAMMING) {
			audioHandler.PlayNextLandHardSound();
		}

		if (CompareToLastState(PlayerStates::FALLING) && currentPlayerState != PlayerStates::FALLING && currentPlayerState != PlayerStates::SLAMMING && currentPlayerState != PlayerStates::DOUBLE_JUMPING && currentPlayerState != PlayerStates::DEAD && currentPlayerState != PlayerStates::VOID_CONSUMED) {
			audioHandler.PlayNextLandSoftSound();
		}

		if (currentPlayerState != PlayerStates::SLIDING) {
			Mix_HaltChannel(5);
		}
		if (currentPlayerState != PlayerStates::WALLSLIDING) {
			Mix_HaltChannel(6);
		}
		if (currentPlayerState != PlayerStates::FALLING) {
			Mix_HaltChannel(4);
			Mix_Volume(4, 0);
			FallVolume = 1;
		}
		if (currentPlayerState != PlayerStates::SUCKED) {
			Mix_HaltChannel(10);
			Mix_Volume(10, 0);
			SuckedVolume = 1;
		}
	}
}
#include "AnimationHandler.hpp"

AnimationHandler::AnimationHandler(TextureHandler& mTextureHandlerRef) : mTextureHandlerRef(mTextureHandlerRef) {

}

AnimationHandler::~AnimationHandler() {

}

void AnimationHandler::Init(const int& samplerSize) {
	SampelrSize = samplerSize;

	SDL_Surface* DeadAnimationSurface           = mTextureHandlerRef.LoadSurface("assets/Actor/DeathAnimation.png");
	SDL_Surface* DuckAnimationSurface           = mTextureHandlerRef.LoadSurface("assets/Actor/DuckAnimation.png");
	SDL_Surface* HitAnimationSurface            = mTextureHandlerRef.LoadSurface("assets/Actor/HitAnimation.png");
	SDL_Surface* RunAnimationSurface            = mTextureHandlerRef.LoadSurface("assets/Actor/RunAnimation.png");
	SDL_Surface* FallAnimationSurface           = mTextureHandlerRef.LoadSurface("assets/Actor/FallAnimation.png");
	SDL_Surface* IdleAnimationSurface           = mTextureHandlerRef.LoadSurface("assets/Actor/IdleAnimation.png");
	SDL_Surface* JumpAnimationSurface	        = mTextureHandlerRef.LoadSurface("assets/Actor/JumpAnimation.png");
	SDL_Surface* SlamAnimationSurface           = mTextureHandlerRef.LoadSurface("assets/Actor/SlamAnimation.png");
	SDL_Surface* SlideAnimationSurface          = mTextureHandlerRef.LoadSurface("assets/Actor/SlideAnimation.png");
	SDL_Surface* WallSlideAnimationSurface      = mTextureHandlerRef.LoadSurface("assets/Actor/WallSlideAnimation.png");
	SDL_Surface* DuckIdleAnimationSurface       = mTextureHandlerRef.LoadSurface("assets/Actor/DuckIdleAnimation.png");
	SDL_Surface* BlackHoleBirthAnimationSurface = mTextureHandlerRef.LoadSurface("assets/Level/BlackHole_Birth_Anim.png");
	SDL_Surface* BlackHoleLoopAnimationSurface  = mTextureHandlerRef.LoadSurface("assets/Level/BlackHole_Loop_Anim.png");
	SDL_Surface* EscapePortalAnimationSurface   = mTextureHandlerRef.LoadSurface("assets/Level/Portal.png");

	InitMultiFrameAnimation(DeadAnimationSurface, DeadAnimation, glm::vec2(84.0f, 68.0f), 700, AnimationSpriteSizeX, AnimationSpriteSizeY);
	DeadAnimation.SingleFrameTime = DeadAnimation.AnimationTime / DeadAnimation.AnimationTextureIndexes.size();

	InitMultiFrameAnimation(DuckAnimationSurface, DuckAnimation, glm::vec2(52.0f, 56.0f), 1000, AnimationSpriteSizeX, AnimationSpriteSizeY);
	DuckAnimation.SingleFrameTime = DuckAnimation.AnimationTime / DuckAnimation.AnimationTextureIndexes.size();

	InitMultiFrameAnimation(HitAnimationSurface, HitAnimation, glm::vec2(60.0f, 68.0f), 300, AnimationSpriteSizeX, AnimationSpriteSizeY);
	HitAnimation.SingleFrameTime = HitAnimation.AnimationTime / HitAnimation.AnimationTextureIndexes.size();

	InitMultiFrameAnimation(RunAnimationSurface, RunAnimation, glm::vec2(56.0f, 76.0f), 500, AnimationSpriteSizeX, AnimationSpriteSizeY);
	RunAnimation.SingleFrameTime = RunAnimation.AnimationTime / RunAnimation.AnimationTextureIndexes.size();

	InitMultiFrameAnimation(BlackHoleBirthAnimationSurface, BlackHoleBirthAnimation, glm::vec2(332.0f, 332.0f), 400, 512, 512);
	BlackHoleBirthAnimation.SingleFrameTime = BlackHoleBirthAnimation.AnimationTime / BlackHoleBirthAnimation.AnimationTextureIndexes.size();

	InitMultiFrameAnimation(BlackHoleLoopAnimationSurface, BlackHoleLoopAnimation, glm::vec2(332.0f, 332.0f), 800, 512, 512);
	BlackHoleLoopAnimation.SingleFrameTime = BlackHoleLoopAnimation.AnimationTime / BlackHoleLoopAnimation.AnimationTextureIndexes.size();

	InitMultiFrameAnimation(EscapePortalAnimationSurface, EscapePortalAnimation, glm::vec2(264.0f, 496.0f), 900, 512, 512);
	EscapePortalAnimation.SingleFrameTime = EscapePortalAnimation.AnimationTime / EscapePortalAnimation.AnimationTextureIndexes.size();

	InitSingleFrameAnimation(FallAnimationSurface, FallAnimation, glm::vec2(56.0f, 72.0f), AnimationSpriteSizeX, AnimationSpriteSizeY);
	InitSingleFrameAnimation(IdleAnimationSurface, IdleAnimation, glm::vec2(60.0f, 68.0f), AnimationSpriteSizeX, AnimationSpriteSizeY);
	InitSingleFrameAnimation(JumpAnimationSurface, JumpAnimation, glm::vec2(60.0f, 72.0f), AnimationSpriteSizeX, AnimationSpriteSizeY);
	InitSingleFrameAnimation(SlamAnimationSurface, SlamAnimation, glm::vec2(60.0f, 72.0f), AnimationSpriteSizeX, AnimationSpriteSizeY);
	InitSingleFrameAnimation(SlideAnimationSurface, SlideAnimation, glm::vec2(76.0f, 52.0f), AnimationSpriteSizeX, AnimationSpriteSizeY);
	InitSingleFrameAnimation(WallSlideAnimationSurface, WallSlideAnimation, glm::vec2(56.0f, 76.0f), AnimationSpriteSizeX, AnimationSpriteSizeY);
	InitSingleFrameAnimation(DuckIdleAnimationSurface, DuckIdleAnimation, glm::vec2(48.0f, 56.0f), AnimationSpriteSizeX, AnimationSpriteSizeY);

	SDL_DestroySurface(DeadAnimationSurface);
	SDL_DestroySurface(DuckAnimationSurface);
	SDL_DestroySurface(HitAnimationSurface);
	SDL_DestroySurface(RunAnimationSurface);
	SDL_DestroySurface(FallAnimationSurface);
	SDL_DestroySurface(IdleAnimationSurface);
	SDL_DestroySurface(JumpAnimationSurface);
	SDL_DestroySurface(SlamAnimationSurface);
	SDL_DestroySurface(SlideAnimationSurface);
	SDL_DestroySurface(WallSlideAnimationSurface);
	SDL_DestroySurface(DuckIdleAnimationSurface);
	SDL_DestroySurface(BlackHoleBirthAnimationSurface);
	SDL_DestroySurface(BlackHoleLoopAnimationSurface);
	SDL_DestroySurface(EscapePortalAnimationSurface);

}

void AnimationHandler::InitMultiFrameAnimation(SDL_Surface* animationSpritesheet, Animation& animation, const glm::vec2& animationSize, const int& animationTime, const int& spriteSizeX, const int& spriteSizeY, const glm::vec2& texturePosition) {
	std::vector<SDL_Surface*> tiles = mTextureHandlerRef.CutTileset(animationSpritesheet, spriteSizeX, spriteSizeY);
	for (int i = 0; i < tiles.size(); i++) {
		animation.AnimationTextureIndexes.push_back((uint32_t)mTextureHandlerRef.layersUsed[0]);
		mTextureHandlerRef.LoadTexture(tiles[i], GL_RGBA, mTextureHandlerRef.layersUsed[0], 0);
		SDL_DestroySurface(tiles[i]);
	}
	animation.TexturePosition = texturePosition;
	animation.Size = glm::vec2(animationSize.x / (float)SampelrSize, animationSize.y / (float)SampelrSize);
	animation.AnimationTime = animationTime;
	SDL_DestroySurface(animationSpritesheet);
}

void AnimationHandler::InitSingleFrameAnimation(SDL_Surface* animationSprite, Animation& animation, const glm::vec2& animationSize, const int& spriteSizeX, const int& spriteSizeY, const glm::vec2& texturePosition) {
	animation.AnimationTextureIndexes.push_back((uint32_t)mTextureHandlerRef.layersUsed[0]);
	mTextureHandlerRef.LoadTexture(mTextureHandlerRef.FlipSurfaceVertically(animationSprite), GL_RGBA, mTextureHandlerRef.layersUsed[0], 0);
	animation.TexturePosition = texturePosition;
	animation.Size = glm::vec2(animationSize.x / (float)SampelrSize, animationSize.y / (float)SampelrSize);
	animation.AnimationTime = 0.0f;
	SDL_DestroySurface(animationSprite);
}
#include "BatchRenderer.hpp"
#include "App.hpp"
#include <cstddef>

// Basically static memory allocation and value init for batch rendering

BatchRenderer::BatchRenderer() : app_(nullptr) {

}

BatchRenderer::~BatchRenderer() {

}

App& BatchRenderer::app()  {
	if (app_ == nullptr) {
		app_ = &App::getInstance();
	}
	return *app_;
}
void BatchRenderer::StartUp(const GLuint& PipelineProgram) {
	if (QuadBuffer != nullptr) {
		std::cout << "BatchRenderer has been initialized twice. ERROR!" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	QuadBuffer = new Box[MaxVertexCount];


	glCreateVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	glCreateBuffers(1, &mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, MaxVertexCount * sizeof(Box), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexArrayAttrib(mVAO, 0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(Box), (const void*)offsetof(Box, Position));

	glEnableVertexArrayAttrib(mVAO, 1);
	glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Box), (const void*)offsetof(Box, Color));

	glEnableVertexArrayAttrib(mVAO, 2);
	glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Box), (const void*)offsetof(Box, TexturePosition));

	glEnableVertexArrayAttrib(mVAO, 3);
	glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(Box), (const void*)offsetof(Box, TextureIndex));

	uint32_t indices[MaxIndexCount];
	uint32_t offset = 0;
	for (int i = 0; i < MaxIndexCount; i += 6) {
		indices[i + 0] = 0 + offset;
		indices[i + 1] = 1 + offset;
		indices[i + 2] = 2 + offset;

		indices[i + 3] = 2 + offset;
		indices[i + 4] = 3 + offset;
		indices[i + 5] = 0 + offset;

		offset += 4;
	}
	glCreateBuffers(1, &mIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void BatchRenderer::ShutDown() {
	glDeleteVertexArrays(1, &mVAO); 
	glDeleteBuffers(1, &mVBO);
	glDeleteBuffers(1, &mIBO);

	delete[] QuadBuffer;
}

void BatchRenderer::BeginBatch(const glm::mat4& ProjectionMatrix) {
	QuadBufferPtr = QuadBuffer;
	CurrentProjectionMatrix = ProjectionMatrix;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&CurrentPipeline);
}

void BatchRenderer::EndBatch() {
	GLsizeiptr size = (uint8_t*)QuadBufferPtr - (uint8_t*)QuadBuffer;
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, QuadBuffer);
}

void BatchRenderer::DrawInBatch(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
	if (IndexCount >= MaxIndexCount) {
		EndBatch();
		Flush();
		BeginBatch(CurrentProjectionMatrix);
	}

	QuadBufferPtr->Position = {position.x, position.y};
	QuadBufferPtr->Color = color;
	QuadBufferPtr->TexturePosition = { 0.0f, 0.0f };
	QuadBufferPtr->TextureIndex = 0;
	QuadBufferPtr++;

	QuadBufferPtr->Position = { position.x + size.x, position.y };
	QuadBufferPtr->Color = color;
	QuadBufferPtr->TexturePosition = { 0.25f, 0.0f };
	QuadBufferPtr->TextureIndex = 0;
	QuadBufferPtr++;

	QuadBufferPtr->Position = { position.x + size.x, position.y + size.y};
	QuadBufferPtr->Color = color;
	QuadBufferPtr->TexturePosition = { 0.25f, 0.25f };
	QuadBufferPtr->TextureIndex = 0;
	QuadBufferPtr++;

	QuadBufferPtr->Position = { position.x, position.y + size.y };
	QuadBufferPtr->Color = color;
	QuadBufferPtr->TexturePosition = { 0.0f, 0.25f };
	QuadBufferPtr->TextureIndex = 0;
	QuadBufferPtr++;

	IndexCount += 6;
}

void BatchRenderer::DrawInBatch(const glm::vec2& position, const glm::vec2& size, uint32_t textureID, const glm::vec2& textureSize, const glm::vec2& texturePosition, const bool& drawFliped) {
	if (IndexCount >= MaxIndexCount) {
		EndBatch();
		Flush();
		BeginBatch(CurrentProjectionMatrix);
	}

	constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

	if (drawFliped == true) {
		QuadBufferPtr->Position = { position.x, position.y };
		QuadBufferPtr->Color = color;
		QuadBufferPtr->TexturePosition = { texturePosition.x + textureSize.x, texturePosition.y };
		QuadBufferPtr->TextureIndex = textureID;
		QuadBufferPtr++;

		QuadBufferPtr->Position = { position.x + size.x, position.y };
		QuadBufferPtr->Color = color;
		QuadBufferPtr->TexturePosition = { texturePosition.x, texturePosition.y };
		QuadBufferPtr->TextureIndex = textureID;
		QuadBufferPtr++;

		QuadBufferPtr->Position = { position.x + size.x, position.y + size.y };
		QuadBufferPtr->Color = color;
		QuadBufferPtr->TexturePosition = { texturePosition.x, texturePosition.y + textureSize.y };
		QuadBufferPtr->TextureIndex = textureID;
		QuadBufferPtr++;

		QuadBufferPtr->Position = { position.x, position.y + size.y };
		QuadBufferPtr->Color = color;
		QuadBufferPtr->TexturePosition = { texturePosition.x + textureSize.x, texturePosition.y + textureSize.y };
		QuadBufferPtr->TextureIndex = textureID;
		QuadBufferPtr++;
	}
	else if (drawFliped == false) {
		QuadBufferPtr->Position = { position.x, position.y };
		QuadBufferPtr->Color = color;
		QuadBufferPtr->TexturePosition = { texturePosition.x, texturePosition.y };
		QuadBufferPtr->TextureIndex = textureID;
		QuadBufferPtr++;

		QuadBufferPtr->Position = { position.x + size.x, position.y };
		QuadBufferPtr->Color = color;
		QuadBufferPtr->TexturePosition = { texturePosition.x + textureSize.x, texturePosition.y };
		QuadBufferPtr->TextureIndex = textureID;
		QuadBufferPtr++;

		QuadBufferPtr->Position = { position.x + size.x, position.y + size.y };
		QuadBufferPtr->Color = color;
		QuadBufferPtr->TexturePosition = { texturePosition.x + textureSize.x, texturePosition.y + textureSize.y };
		QuadBufferPtr->TextureIndex = textureID;
		QuadBufferPtr++;

		QuadBufferPtr->Position = { position.x, position.y + size.y };
		QuadBufferPtr->Color = color;
		QuadBufferPtr->TexturePosition = { texturePosition.x, texturePosition.y + textureSize.y };
		QuadBufferPtr->TextureIndex = textureID;
		QuadBufferPtr++;
	}

	IndexCount += 6;
}


void BatchRenderer::DrawSeperatly(const glm::vec2& position, glm::vec2 size, const glm::vec4& color, const glm::mat4& ProjectionMatrix, const glm::mat4& ModelMatrix) {
	BeginBatch(ProjectionMatrix);
	DrawInBatch(position, size, color);
	EndBatch();
	Flush(ModelMatrix);
}

void BatchRenderer::DrawSeperatly(const glm::vec2& position, glm::vec2 size, const glm::mat4& ProjectionMatrix, uint32_t textureID, const glm::vec2& textureSize, const glm::vec2& texturePosition, const glm::mat4& ModelMatrix, const bool& drawFliped) {
	BeginBatch(ProjectionMatrix);
	DrawInBatch(position, size, textureID, textureSize, texturePosition, drawFliped);
	EndBatch();
	Flush(ModelMatrix);
}

void BatchRenderer::Flush(const glm::mat4 ModelMatrix) {
	UniformVariableLinkageAndPopulatingWithMatrix("uModelMatrix", ModelMatrix, app().mGraphicsPipelineShaderProgram);
	UniformVariableLinkageAndPopulatingWithMatrix("uProjectionMatrix", app().mCamera.GetProjectionMatrix(), app().mGraphicsPipelineShaderProgram);
	glBindVertexArray(mVAO);
	glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, nullptr);
	IndexCount = 0;
}

void BatchRenderer::UniformVariableLinkageAndPopulatingWithMatrix(const GLchar* uniformLocation, glm::mat4 matrix, const GLuint& PipelineProgram) {
	GLint MatrixLocation = glGetUniformLocation(PipelineProgram, uniformLocation);
	if (MatrixLocation >= 0) {
		glUniformMatrix4fv(MatrixLocation, 1, GL_FALSE, &matrix[0][0]);
	}
	else {
		std::cerr << "Failed to get uniform location of: " << uniformLocation << std::endl << "Exiting now" << std::endl;
		exit(EXIT_FAILURE);
	}
}
#include "Camera.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>
#include <algorithm>
#include "glm/gtx/string_cast.hpp"
#include "App.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Sign.hpp"


Camera::Camera() : app_(nullptr) {

}

App& Camera::app() {
	if (app_ == nullptr) {
		app_ = &App::getInstance();
	}
	return *app_;
}

void Camera::Update(glm::vec2& actorVelocity, glm::vec2& actorScreenPosition, const float& deltaTime) {
	if (app().mActor.mPosition.x > 0.0f) {
		if (int(actorVelocity.x) > 0.0f && mCameraOffsetTimerBuffer2 <= 0.f) {
			mCameraOffset.x += (actorVelocity.x * deltaTime) / 5.0f;
			if (mCameraOffset.x > 350.0f) {
				mCameraOffset.x = 350.0f;
			}
			mCameraOffsetTimerBuffer = mCameraOffsetTimeBuffer;
		}
		else if (int(actorVelocity.x) < 0.0f && mCameraOffsetTimerBuffer <= 0.f) {
			mCameraOffset.x += (actorVelocity.x * deltaTime) / 5.0f;
			if (mCameraOffset.x < -350.0f) {
				mCameraOffset.x = -350.0f;
			}
			mCameraOffsetTimerBuffer2 = mCameraOffsetTimeBuffer2;
		}

		if (mCameraOffsetTimerBuffer > 0.0f) {
			mCameraOffsetTimerBuffer -= deltaTime;
		}
		if (mCameraOffsetTimerBuffer2 > 0.0f) {
			mCameraOffsetTimerBuffer2 -= deltaTime;
		}

		
		else {
			if (mCameraOffset.x > 0.0f && mCameraOffsetTimerBuffer <= 0.0f && mCameraOffsetTimerBuffer2 <= 0.0f) {
				if (mCameraOffset.x > 0.25f) {
					mCameraOffset.x -= 36.0f * deltaTime;
				}
				else {
					mCameraOffset.x = 0;
				}
			}
			else if (mCameraOffset.x < 0.0f && mCameraOffsetTimerBuffer <= 0.0f && mCameraOffsetTimerBuffer2 <= 0.0f) {
				if (mCameraOffset.x < -0.25f) {
					mCameraOffset.x += 36.0f * deltaTime;
				}
				else {
					mCameraOffset.x = 0;
				}
			}
		}
		mProjectionMatrix = glm::translate(mInitialProjectionMatrix, glm::vec3(-(app().mActor.mPosition.x - 800.0f + mCameraOffset.x), 0.0f, 0.0f));
		mUIModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3((app().mActor.mPosition.x - 800.0f + mCameraOffset.x), 0.0f, 0.0f));
	}
}

void Camera::SetProjectionMatrix() {
	mInitialProjectionMatrix = glm::ortho(0.0f, 1920.0f, 0.0f, 1080.0f, -1.0f, 1.0f);
	mProjectionMatrix = mInitialProjectionMatrix;
}

glm::mat4 Camera::GetProjectionMatrix() const {
	return mProjectionMatrix;
}
#include "PipelineManager.hpp"
#include "IO.hpp"
#include "App.hpp"

PipelineManager::PipelineManager() : app_(nullptr) {

}

PipelineManager::~PipelineManager() {

}

App& PipelineManager::app() {
	if (app_ == nullptr) {
		app_ = &App::getInstance();
	}
	return *app_;
}

GLuint PipelineManager::CompileShader(GLuint shaderType, const std::string& shaderSource) {
	// Create shader
	GLuint shaderObject = glCreateShader(shaderType);
	// Compile shader
	const char* shaderSourceCStr = shaderSource.c_str();
	glShaderSource(shaderObject, 1, &shaderSourceCStr, nullptr);
	glCompileShader(shaderObject);

	int result;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);
	if (result == false) {
		int length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)malloc(length * sizeof(char));
		glGetShaderInfoLog(shaderObject, length, &length, message);
		std::cout << "Failed to compile " << (shaderType) << "!\n" << (message) << std::endl;
		free(message);
		glDeleteShader(shaderObject);
		return 0;
	}

	return shaderObject;
}

GLuint PipelineManager::CreateShaderProgram(const std::string& vShaderSource, const std::string& fShaderSource) {
	GLuint programObject = glCreateProgram();

	// Create vertex shader
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vShaderSource);
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fShaderSource);

	// Attach shaders
	glAttachShader(programObject, vertexShader);
	glAttachShader(programObject, fragmentShader);
	glLinkProgram(programObject);

	// Validate our program
	glValidateProgram(programObject);

	// Delete shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return programObject;

}

void PipelineManager::CreateGraphicsPipeline() {
	// Create graphics pipeline (this is done on GPU)

	std::string VertexShaderSource = ReadFileAsString("shaders/vertex.glsl");
	std::string FragmentShaderSource = ReadFileAsString("shaders/fragment.glsl");
	app().mGraphicsPipelineShaderProgram = CreateShaderProgram(VertexShaderSource, FragmentShaderSource);
}
#include "SimpleTextOut.hpp"

TextOut::TextOut() {

}

TextOut::~TextOut() {

}

void TextOut::Init(TextureHandler& textureHandler, const char* filepath) {
	SDL_Surface* surface = textureHandler.LoadSurface(filepath);

	surface = textureHandler.FlipSurfaceVertically(surface);

	mTextTextureIndex = textureHandler.layersUsed[0];

	textureHandler.LoadTexture(surface, GL_RGBA8, textureHandler.layersUsed[0], 0);
	SDL_DestroySurface(surface);

	mTextureSize = glm::vec2(1.0f, 31.0f / 512.0f);

	for (int i = 0; i < (int)512 / 31; i++) {
		mTexturePositions.push_back(glm::vec2(0.0f, (float)i * (31.0f / 512.0f)));
	}
}

void TextOut::Update() {

}
#include "Texture.hpp"

TextureHandler::TextureHandler() {
	
}

TextureHandler::~TextureHandler() {
	
}

SDL_Surface* TextureHandler::LoadSurface(const char* filepath) {
	SDL_Surface* surface = IMG_Load(filepath);
	if (!surface) {
		std::cerr << "IMG_Load Error: Cannot load " << filepath << std::endl;
		exit(1);
	}
	return surface;
}

void TextureHandler::InitTextureArray(const GLenum& internalformat, const GLsizei& width, const GLsizei& height, const GLsizei& depth) {
	glGenTextures(1, &textureArrays[TextureSlotsTaken]);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrays[TextureSlotsTaken]);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, internalformat, width, height, depth);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    layersUsed[TextureSlotsTaken] = 0;
	TextureSlotsTaken++;
}

void TextureHandler::LoadTexture(SDL_Surface* surface, const GLenum& internalformat, const int& layer, const int& slot) {
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, surface->w, surface->h, 1, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    SDL_DestroySurface(surface);
    layersUsed[slot]++;
}

std::vector<SDL_Surface*> TextureHandler::CutTileset(SDL_Surface* tileset, const int& tileWidth, const int& tileHeight) {

    std::vector<SDL_Surface*> tiles;

    // Calculate the number of rows and columns in the tileset
    int rows = tileset->h / tileHeight;
    int cols = tileset->w / tileWidth;

    tileset = SDL_ConvertSurface(tileset, SDL_PIXELFORMAT_ABGR8888);
    if (!tileset) {
        SDL_Log("Failed to convert tileset format: %s", SDL_GetError());
    }

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            // Define the rectangle for the current tile
            SDL_Rect srcRect = { col * tileWidth, row * tileHeight, tileWidth, tileHeight };

            // Create a new surface for the tile
            SDL_Surface* tile = SDL_CreateSurface(tileWidth, tileHeight, tileset->format);
            if (!tile) {
                std::cerr << "Failed to create tile surface: " << SDL_GetError() << std::endl;
                continue;
            }

            SDL_BlitSurface(tileset, &srcRect, tile, NULL);
            tiles.push_back(FlipSurfaceVertically(tile));
            SDL_DestroySurface(tile);
        }
    }
    SDL_DestroySurface(tileset);

    return tiles;
}

SDL_Surface* TextureHandler::FlipSurfaceVertically(SDL_Surface* surface) {
    // Create a new surface with the same dimensions as the original surface
    SDL_Surface* flipped = SDL_CreateSurface(surface->w, surface->h, surface->format);

    if (flipped == nullptr) {
        SDL_Log("Unable to create surface: %s", SDL_GetError());
        return nullptr;
    }

    // Create source and destination rectangles
    SDL_Rect srcRect = { 0, 0, surface->w, 1 };
    SDL_Rect dstRect = { 0, 0, surface->w, 1 };

    for (int y = 0; y < surface->h; ++y) {
        srcRect.y = surface->h - y - 1;
        dstRect.y = y;

        SDL_BlitSurface(surface, &srcRect, flipped, &dstRect);
    }

    return flipped;
}
#include "AudioHandler.hpp"

AudioHandler::AudioHandler() {
}

AudioHandler::~AudioHandler() {
	Mix_FreeChunk(Jump);
	Mix_FreeChunk(DoubleJump);
	Mix_FreeChunk(Slide);
	Mix_FreeChunk(WallSlide);
	Mix_FreeChunk(WindHard);
	Mix_FreeChunk(WindSoft);
	Mix_FreeChunk(BlackHoleBorn);
	Mix_FreeChunk(PortalEscape);
	Mix_FreeChunk(ConsumedByVoid);
	Mix_FreeChunk(BlackHoleIdle);
	Mix_FreeChunk(PortalIdle);
	Mix_FreeChunk(FellDown);

	Mix_FreeMusic(IntroMusic);
	Mix_FreeMusic(LoopMusic);

	Mix_FreeChunk(RunSounds[0]);
	Mix_FreeChunk(RunSounds[1]);
	Mix_FreeChunk(RunSounds[2]);
	Mix_FreeChunk(RunSounds[3]);
	Mix_FreeChunk(RunSounds[4]);
	Mix_FreeChunk(RunSounds[5]);
	Mix_FreeChunk(RunSounds[6]);
	Mix_FreeChunk(RunSounds[7]);
	Mix_FreeChunk(RunSounds[8]);
	Mix_FreeChunk(RunSounds[9]);

	Mix_FreeChunk(LandSoftSounds[0]);
	Mix_FreeChunk(LandSoftSounds[1]);
	Mix_FreeChunk(LandSoftSounds[2]);
	Mix_FreeChunk(LandSoftSounds[3]);
	Mix_FreeChunk(LandSoftSounds[4]);

	Mix_FreeChunk(LandHardSounds[0]);
}

void AudioHandler::LoadSounds() {
	Jump =           Mix_LoadWAV("assets/Sounds/Jump.wav");
	DoubleJump =     Mix_LoadWAV("assets/Sounds/DoubleJump.wav");
	Slide =          Mix_LoadWAV("assets/Sounds/Slide.wav");
	WallSlide =      Mix_LoadWAV("assets/Sounds/WallSlide.wav");
	WindHard =       Mix_LoadWAV("assets/Sounds/WindHard.wav");
	WindSoft =       Mix_LoadWAV("assets/Sounds/WindSoft.wav");
	BlackHoleBorn =  Mix_LoadWAV("assets/Sounds/BlackHoleBorn.wav");
	PortalEscape =   Mix_LoadWAV("assets/Sounds/PortalEscape.wav");
	ConsumedByVoid = Mix_LoadWAV("assets/Sounds/ConsumedByVoid.wav");
	BlackHoleIdle =  Mix_LoadWAV("assets/Sounds/BlackHoleIdle.wav");
	PortalIdle =     Mix_LoadWAV("assets/Sounds/PortalIdle.wav");
	FellDown =       Mix_LoadWAV("assets/Sounds/Died.wav");

	IntroMusic = Mix_LoadMUS("assets/Sounds/The Cyber Grind - (Intro only).wav");
	LoopMusic =  Mix_LoadMUS("assets/Sounds/The Cyber Grind - (loop).wav");

	RunSounds[0] = Mix_LoadWAV("assets/Sounds/Run01.wav");
	RunSounds[1] = Mix_LoadWAV("assets/Sounds/Run02.wav");
	RunSounds[2] = Mix_LoadWAV("assets/Sounds/Run03.wav");
	RunSounds[3] = Mix_LoadWAV("assets/Sounds/Run04.wav");
	RunSounds[4] = Mix_LoadWAV("assets/Sounds/Run05.wav");
	RunSounds[5] = Mix_LoadWAV("assets/Sounds/Run06.wav");
	RunSounds[6] = Mix_LoadWAV("assets/Sounds/Run07.wav");
	RunSounds[7] = Mix_LoadWAV("assets/Sounds/Run08.wav");
	RunSounds[8] = Mix_LoadWAV("assets/Sounds/Run09.wav");
	RunSounds[9] = Mix_LoadWAV("assets/Sounds/Run10.wav");

	LandSoftSounds[0] = Mix_LoadWAV("assets/Sounds/LandSoft1.wav");
	LandSoftSounds[1] = Mix_LoadWAV("assets/Sounds/LandSoft2.wav");
	LandSoftSounds[2] = Mix_LoadWAV("assets/Sounds/LandSoft3.wav");
	LandSoftSounds[3] = Mix_LoadWAV("assets/Sounds/LandSoft4.wav");
	LandSoftSounds[4] = Mix_LoadWAV("assets/Sounds/LandSoft5.wav");

	LandHardSounds[0] = Mix_LoadWAV("assets/Sounds/LandHard.wav");

}

void AudioHandler::PlayNextStepSound() {
	if (runSoundIndex == 10) {
		runSoundIndex = 0;
	}
	Mix_PlayChannel(1, RunSounds[runSoundIndex], 0);
	runSoundIndex++;
}

void AudioHandler::PlayNextLandSoftSound() {
	if (landSoftSoundIndex == 5) {
		landSoftSoundIndex = 0;
	}
	Mix_PlayChannel(7, LandSoftSounds[landSoftSoundIndex], 0);
	landSoftSoundIndex++;
}

void AudioHandler::PlayNextLandHardSound() {
	if (landHardSoundIndex == 1) {
		landHardSoundIndex = 0;
	}
	Mix_PlayChannel(7, LandHardSounds[landHardSoundIndex], 0);
	landHardSoundIndex++;
}
#include "Input.hpp"
#include "App.hpp"

InputManager::InputManager() : app_(nullptr) {

}

InputManager::~InputManager() {

}

App& InputManager::app() {
	if (app_ == nullptr) {
		app_ = &App::getInstance();
	}
	return *app_;
}

void InputManager::Input() {
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_EVENT_QUIT) {
			std::cout << "Goodbye!" << std::endl;
			app().mQuit = true;
		}
		else if (e.type == SDL_EVENT_MOUSE_MOTION) {
		}
		if (e.type == SDL_EVENT_KEY_UP) {
			switch (e.key.scancode) {
			case SDL_SCANCODE_SPACE:
				app().mMovementHandler.KeyboadStates[static_cast<int>(MovementState::SPACE)] = false;
				app().mMovementHandler.spacebarOneShot = true;
				break;
			case SDL_SCANCODE_A:
				app().mMovementHandler.KeyboadStates[static_cast<int>(MovementState::MOVE_LEFT)] = false;
				break;
			case SDL_SCANCODE_D:
				app().mMovementHandler.KeyboadStates[static_cast<int>(MovementState::MOVE_RIGHT)] = false;
				break;
			case SDL_SCANCODE_LSHIFT: 
				app().mMovementHandler.KeyboadStates[static_cast<int>(MovementState::DUCK)] = false;
				app().mMovementHandler.duckOneShot = true;
				break;
			case SDL_SCANCODE_W:
				app().mMovementHandler.KeyboadStates[static_cast<int>(MovementState::MOVE_UP)] = false;
				break;
			case SDL_SCANCODE_S:
				app().mMovementHandler.KeyboadStates[static_cast<int>(MovementState::MOVE_DOWN)] = false;
				break;

			default:
				break;
			}
		}
		const bool* state = SDL_GetKeyboardState(nullptr);
		if (state[SDL_SCANCODE_R]) {
			app().LoadGame();
		}
		if (state[SDL_SCANCODE_SPACE]) {
			app().mMovementHandler.KeyboadStates[static_cast<int>(MovementState::SPACE)] = true;
		}
		if (state[SDL_SCANCODE_W]) {
			app().mMovementHandler.KeyboadStates[static_cast<int>(MovementState::MOVE_UP)] = true;
		}
		if (state[SDL_SCANCODE_S]) {
			app().mMovementHandler.KeyboadStates[static_cast<int>(MovementState::MOVE_DOWN)] = true;
		}
		if (state[SDL_SCANCODE_LSHIFT]) {
			app().mMovementHandler.KeyboadStates[static_cast<int>(MovementState::DUCK)] = true;
		}
		if (state[SDL_SCANCODE_A]) {
			app().mMovementHandler.KeyboadStates[static_cast<int>(MovementState::MOVE_LEFT)] = true;
		}
		if (state[SDL_SCANCODE_D]) {
			app().mMovementHandler.KeyboadStates[static_cast<int>(MovementState::MOVE_RIGHT)] = true;
		}
		if (state[SDL_SCANCODE_ESCAPE]) {
			app().mQuit = true;
		}
	}
}
#include "IO.hpp"

std::string ReadFileAsString(const std::string& filePath) {
	std::ifstream file(filePath.c_str());

	if (!file.is_open()) {
		std::cout << "Failed to open file" << (filePath) << std::endl;
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}
#include "App.hpp"
#include "glm/gtx/string_cast.hpp"
#include <cmath>
#include <algorithm>
#include "Sign.hpp"
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_mixer.h>
#include <glm/gtx/norm.hpp>
#include <random>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/exterior_product.hpp>

App::App() : mAnimationHandler(mTextureHandler) {
	StartUp();
}

App::~App() {
	ShutDown();
}

void App::StartUp() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		std::cout << "SDL3 could not initialize video subsystem or audio subsystem" << std::endl;
		exit(1);
	}

	if (IMG_Init(IMG_INIT_PNG) == 0) {
		std::cerr << "SDL3_image could not be initialized" << std::endl;
		exit(1);
	}
	if (Mix_OpenAudio(0, NULL) < 0)
	{
		std::cerr << "SDL3_mixer could not be initialized" << std::endl;
		exit(1);
	}


	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	if (mDebug) {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	mWindow = SDL_CreateWindow("GL Window", mWindowWidth, mWindowHeight, SDL_WINDOW_OPENGL);

	SDL_SetWindowPosition(mWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	if (mWindow == nullptr) {
		std::cout << "SDL_Window was not able to be created" << std::endl;
		exit(1);
	}

	mGlContext = SDL_GL_CreateContext(mWindow);
	if (mGlContext == nullptr) {
		std::cout << "OpenGL context not available" << std::endl;
		exit(1);
	}
	if (mVsync == false) {
		SDL_GL_SetSwapInterval(0);
	}
	else {
		SDL_GL_SetSwapInterval(1);
	}
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		std::cout << "Glad was not initialized" << std::endl;
		exit(1);
	}
	if (mDebug) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GLDebugMessageCallback, nullptr);
	}

	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void App::PostStartUp() {
	mCamera.SetProjectionMatrix();

	mPipelineManager.CreateGraphicsPipeline();
	glUseProgram(mGraphicsPipelineShaderProgram);

	mBatchRenderer.StartUp(mGraphicsPipelineShaderProgram);

	SDL_Surface* tileset = mTextureHandler.LoadSurface("assets/Level/tiles128up.png");
	
	mTextureHandler.InitTextureArray(GL_RGBA8, 512, 512, 1024);

	uint32_t whiteTextureData[128 * 128];
	for (int i = 0; i < 128 * 128; i++) {
		whiteTextureData[i] = 0xFFFFFFFF; // RGBA: White
	}
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 128, 128, 1, GL_RGBA, GL_UNSIGNED_BYTE, whiteTextureData);
	mTextureHandler.layersUsed[0]++;

	std::vector<SDL_Surface*> tiles = mTextureHandler.CutTileset(tileset, 128, 128);
	for (int i = 0; i < tiles.size(); i++) {
		mTextureHandler.LoadTexture(tiles[i], GL_RGBA, mTextureHandler.layersUsed[0], 0);
	}

	mAnimationHandler.Init(512);

	mTextOut.Init(mTextureHandler, "assets/Level/Text.png");

	Mix_AllocateChannels(16);
	mAudioHandler.LoadSounds();
}
void App::LoadGame() {
	mActor.velocity = glm::vec2(0.0f, 0.0f);

	mActor.mDead = false;
	mActor.isConsumedByVoid = false;
	mActor.isSucked = false;
	mActor.isSuckedPortal = false;
	mActor.mEscaped = false;
	mActor.flyAngle = 0.0f;
	mActor.flyAnglePortal = 0.0f;
	mActor.flyAngleTarget = -1.0f;
	mActor.flyAngleTargetPortal = -1.0f;
	mGameStarted = false;
	mBlackHole.isBorn = false;
	mBlackHole.loopTimerOneShot = false;
	mBlackHole.birthTimerOneShot = false;
	mBlackHole.idleVolume = 0;
	Mix_HaltChannel(13);
	mBlackHole.AABBSize.x = 100.0f;
	mLevel.mBlocks.clear();
	mMovementHandler.lookDirection = LookDirections::RIGHT;
	mCamera.mCameraOffset = glm::vec2(0.0f, 0.0f);
	mActor.mDeadSoundOneShot = true;
	mStateMachine.deadAnimOneShot = true;
	mStateMachine.deadAnimDone = false;
	Mix_HaltMusic();

	titleScreenAlpha = 0.0f;
	titleScreenMusicVolume = 128;

	titleScreenAlphaTimer = 0.0f;

	titleScreenMessageTimer = 0.0f;

	startMessageTimer = 0.0f;

	titleScreenMusicVolumeTimer = 0.0f;

	mLevel.LoadLevelJson("levels/GameLevels/32p/Level_1.json");
	mLevel.BuildLevel();

	for (int i = 0; i < mLevel.mBlocks.size(); i++) {
		mLevel.mBlocks[i].Update();
	}
	mActor.mSprite.vertexData.Position = glm::vec2(370.0f, 350.0f);

	mActor.isVisible = true;
	mActor.isCollidable = true;

	mActor.mPosition = mActor.mSprite.vertexData.Position;

	mActor.mSprite.vertexData.Size = mAnimationHandler.FallAnimation.Size * mActor.mSizeMultiplier;

	mEscapePortal.mSprite.vertexData.Size = mAnimationHandler.EscapePortalAnimation.Size * mEscapePortal.sizeMultiplier;

	Mix_PlayMusic(mAudioHandler.IntroMusic, 0);
}


void App::MainLoop() {
	SDL_WarpMouseInWindow(mWindow, mWindowWidth / 2, mWindowHeight / 2);
	tp1 = std::chrono::system_clock::now();
	tp2 = std::chrono::system_clock::now();
	while (!mQuit) {
		mInputManager.Input();

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		glViewport(0, 0, mWindowWidth, mWindowHeight);
		glClearColor((14.0f / 256.0f), (7.0f / 256.0f), (27.0f / 256.0f), 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		Update();

		SDL_GL_SwapWindow(mWindow);  
	}

}

void App::Update() {
	static bool ad;
	tp2 = std::chrono::system_clock::now();
	static auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsedTime = tp2 - tp1;
	static int frameCount = 0;
	frameCount++;
	// Our time per frame coefficient
	deltaTime = elapsedTime.count();
	if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastTime).count() >= 1) {
		auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();
		auto fps = frameCount * 1000.0 / elapsedTime;

		std::cout << "FPS: " << fps << std::endl;

		frameCount = 0;
		lastTime = currentTime;
	}
	static int i = 0;
	if (i < 20) {
		deltaTimeBuffer += deltaTimeRaw;
	}
	if (i == 20) {
		deltaTimeBuffer = 0;
		i = 0;
	}
	else {
		i++;
	}

	tp1 = tp2;



	if (mGameStarted == false && mActor.velocity.x != 0.0f) {
		Mix_HaltMusic();
		mGameStarted = true;
	}

	if (mGameStarted) {
		if (Mix_PlayingMusic() == 0)
		{
			Mix_PlayMusic(mAudioHandler.LoopMusic, 0);
		}
	}
	else {
		startMessageTimer += deltaTime;
		if (startMessageTimer > startMessageTime) {
			startMessageTimer = 0.0f;
		}
		if (startMessageTimer >= 1.0f) {
			mBatchRenderer.DrawSeperatly(glm::vec2(960.0f - mTextOut.mTextureSize.x * textSizeMultiplier / 2, 470.0f), mTextOut.mTextureSize * textSizeMultiplier, mCamera.GetProjectionMatrix(),
				static_cast<uint32_t>(mTextOut.mTextTextureIndex), mTextOut.mTextureSize, mTextOut.mTexturePositions[5], mCamera.mUIModelMatrix);
		}
	}



	mMovementHandler.Update(deltaTime, mActor);

	mActor.Update();
	if (mGameStarted) {
		mBlackHole.Update(mLevel.mBlocks, mActor, deltaTime, mAnimationHandler.BlackHoleBirthAnimation, mAnimationHandler.BlackHoleLoopAnimation, mAudioHandler.BlackHoleBorn, mAudioHandler.ConsumedByVoid, mAudioHandler.BlackHoleIdle);
	}



	if (mActor.mPosition.y < -500.0f) {
		if (!mActor.mDead) {
			mActor.velocity = glm::vec2(0.0f, 0.0f);
			mActor.isConsumedByVoid = true;
			mActor.mDead = true;
		}
	}

	if (mActor.mDead) {
		if (mActor.mDeadSoundOneShot) {
			Mix_PlayChannel(14, mAudioHandler.FellDown, 0);
			mActor.mDeadSoundOneShot = false;
		}
	}



	CollisionUpdate(mLevel.mBlocks, mActor, mMovementHandler.LeftWallHug, mMovementHandler.RightWallHug, deltaTime, mMovementHandler.isGrounded);
	mCamera.Update(mActor.velocity, mActor.mScreenPosition, deltaTime);
	mActor.mScreenPosition = mCamera.GetProjectionMatrix() * glm::vec4(mActor.mPosition.x + mActor.mSprite.vertexData.Size.x / 2, mActor.mPosition.y + mActor.mSprite.vertexData.Size.y / 2, 0.0f, 1.0f);



	mBatchRenderer.BeginBatch(mCamera.GetProjectionMatrix());

	if (mGameStarted) {
		if (mBlackHole.mSprite.vertexData.Position.x + mBlackHole.mSprite.vertexData.Size.x > (mActor.mPosition.x - 800.0f + mCamera.mCameraOffset.x - 80.0f)
			&& mBlackHole.mSprite.vertexData.Position.x < (mActor.mPosition.x - 800.0f + mCamera.mCameraOffset.x + 2000.0f) && mLevel.mBlocks[i].isVisible == false) {
			mBatchRenderer.DrawInBatch(mBlackHole.mSprite.vertexData.Position, mBlackHole.mSprite.vertexData.Size, mBlackHole.mSprite.vertexData.TextureIndex, mBlackHole.AnimationSize, mBlackHole.mSprite.vertexData.TexturePosition);
		}
	}

	// buffer of block id that are being sucked into the black hole and visible
	std::vector<int> flyingBlocks;

	for (int i = 0; i < mLevel.mBlocks.size(); i++) {
		if (mLevel.mBlocks[i].mSprite.vertexData.Position.x + mLevel.mBlocks[i].mSprite.vertexData.Size.x > (mActor.mPosition.x - 800.0f + mCamera.mCameraOffset.x - 80.0f)
			&& mLevel.mBlocks[i].mSprite.vertexData.Position.x < (mActor.mPosition.x - 800.0f + mCamera.mCameraOffset.x + 2000.0f) && mLevel.mBlocks[i].isVisible == true && mLevel.mBlocks[i].isSucked == false) {
			mBatchRenderer.DrawInBatch(mLevel.mBlocks[i].mSprite.vertexData.Position, mLevel.mBlocks[i].mSprite.vertexData.Size, static_cast<uint32_t>(mLevel.mBlocks[i].mSprite.vertexData.TextureIndex), glm::vec2(0.25f, 0.25f));
		}
		else if (mLevel.mBlocks[i].mSprite.vertexData.Position.x + mLevel.mBlocks[i].mSprite.vertexData.Size.x > (mActor.mPosition.x - 800.0f + mCamera.mCameraOffset.x - 80.0f)
			&& mLevel.mBlocks[i].mSprite.vertexData.Position.x < (mActor.mPosition.x - 800.0f + mCamera.mCameraOffset.x + 2000.0f) && mLevel.mBlocks[i].isVisible == true && mLevel.mBlocks[i].isSucked == true) {
			flyingBlocks.push_back(i);
		}
	}

	mEscapePortal.Update(mAnimationHandler.EscapePortalAnimation, deltaTime, mActor, mAudioHandler.PortalEscape, mAudioHandler.PortalIdle);
	if (mEscapePortal.mSprite.vertexData.Position.x + mEscapePortal.mSprite.vertexData.Size.x > (mActor.mPosition.x - 800.0f + mCamera.mCameraOffset.x - 80.0f)
		&& mEscapePortal.mSprite.vertexData.Position.x < (mActor.mPosition.x - 800.0f + mCamera.mCameraOffset.x + 2000.0f)) {
		mBatchRenderer.DrawInBatch(mEscapePortal.mSprite.vertexData.Position, mEscapePortal.mSprite.vertexData.Size, mEscapePortal.mSprite.vertexData.TextureIndex, mEscapePortal.AnimationSize, mEscapePortal.mSprite.vertexData.TexturePosition);
	}

	mBatchRenderer.EndBatch();
	mBatchRenderer.Flush();

	for (int i = 0; i < flyingBlocks.size(); i++) {
		if (mLevel.mBlocks[flyingBlocks[i]].mSprite.vertexData.Position.x + mLevel.mBlocks[flyingBlocks[i]].mSprite.vertexData.Size.x > (mActor.mPosition.x - 800.0f + mCamera.mCameraOffset.x - 80.0f)
			&& mLevel.mBlocks[flyingBlocks[i]].mSprite.vertexData.Position.x < (mActor.mPosition.x - 800.0f + mCamera.mCameraOffset.x + 2000.0f)) {
			mBatchRenderer.DrawSeperatly(mLevel.mBlocks[flyingBlocks[i]].mSprite.vertexData.Position, mLevel.mBlocks[flyingBlocks[i]].mSprite.vertexData.Size, mCamera.GetProjectionMatrix(),
				static_cast<uint32_t>(mLevel.mBlocks[flyingBlocks[i]].mSprite.vertexData.TextureIndex), glm::vec2(0.25f, 0.25f), glm::vec2(0.0f, 0.0f), mLevel.mBlocks[flyingBlocks[i]].mModelMatrix, false);
		}
	}



	mStateMachine.Update(mMovementHandler, mAnimationHandler, mAudioHandler, mActor, deltaTime);
	if (mActor.isVisible == true) {
	mBatchRenderer.DrawSeperatly(mActor.mSprite.vertexData.Position, mStateMachine.mCurrentActorDrawSize, mCamera.GetProjectionMatrix(),
		mStateMachine.mCurrentActorTextureIndex, mStateMachine.mCurrentActorTextureSize, mStateMachine.mCurrentActorTexturePosition, mActor.mModelMatrix, mStateMachine.mActorFlipped);	
	}

	UIUpdate();
}

void App::UIUpdate() {
	mBatchRenderer.BeginBatch(mCamera.GetProjectionMatrix());

	if (mActor.mDead || mActor.mEscaped || mActor.isConsumedByVoid) {
		mBatchRenderer.DrawInBatch(glm::vec2(0.0f, 0.0f), glm::vec2(1920.0f, 1080.0f), glm::vec4((14.0f / 256.0f), (7.0f / 256.0f), (27.0f / 256.0f), titleScreenAlpha));
		if (titleScreenAlphaTimer > titleScreenAlphaTime && titleScreenAlpha < 1.0f) {
			titleScreenAlpha += 0.004f;
			titleScreenAlphaTimer = 0.0f;
		}
		if (titleScreenAlpha > 1.0f) {
			titleScreenAlpha = 1.0f;
		}
		titleScreenAlphaTimer += deltaTime;

		if (titleScreenMusicVolumeTimer > titleScreenMusicVolumeTime && titleScreenMusicVolume > 0) {
			titleScreenMusicVolume -= 1;
			titleScreenMusicVolumeTimer = 0.0f;
		}
		if (titleScreenMusicVolume < 0) {
			titleScreenMusicVolume = 0;
		}
		titleScreenMusicVolumeTimer += deltaTime;
	}

	Mix_VolumeMusic(titleScreenMusicVolume);

	if (titleScreenAlpha >= 1.0f) {
		titleScreenMessageTimer += deltaTime;
		if (titleScreenMessageTimer > titleScreenMessageTime) {
			titleScreenMessageTimer = 0.0f;
		}
		if (titleScreenMessageTimer >= 1.0f) {
			mBatchRenderer.DrawInBatch(glm::vec2(960.0f - mTextOut.mTextureSize.x * textSizeMultiplier / 2, 240.0f), mTextOut.mTextureSize * textSizeMultiplier, 
				static_cast<uint32_t>(mTextOut.mTextTextureIndex), mTextOut.mTextureSize, mTextOut.mTexturePositions[4]);
			}
			if (mActor.isConsumedByVoid && !mActor.mDead) {
				mBatchRenderer.DrawInBatch(glm::vec2(960.0f - mTextOut.mTextureSize.x * textSizeMultiplier / 2, 660.0f), mTextOut.mTextureSize * textSizeMultiplier,
					static_cast<uint32_t>(mTextOut.mTextTextureIndex), mTextOut.mTextureSize, mTextOut.mTexturePositions[0]);
			}
			else if (mActor.isConsumedByVoid && mActor.mDead) {
				mBatchRenderer.DrawInBatch(glm::vec2(960.0f - mTextOut.mTextureSize.x * textSizeMultiplier / 2, 660.0f), mTextOut.mTextureSize * textSizeMultiplier,
					static_cast<uint32_t>(mTextOut.mTextTextureIndex), mTextOut.mTextureSize, mTextOut.mTexturePositions[2]);
			}
			else if (!mActor.isConsumedByVoid && mActor.mDead) {
				mBatchRenderer.DrawInBatch(glm::vec2(960.0f - mTextOut.mTextureSize.x * textSizeMultiplier / 2, 660.0f), mTextOut.mTextureSize * textSizeMultiplier,
					static_cast<uint32_t>(mTextOut.mTextTextureIndex), mTextOut.mTextureSize, mTextOut.mTexturePositions[3]);
			}
			else if (mActor.mEscaped) {
				mBatchRenderer.DrawInBatch(glm::vec2(960.0f - mTextOut.mTextureSize.x * textSizeMultiplier / 2, 660.0f), mTextOut.mTextureSize * textSizeMultiplier,
					static_cast<uint32_t>(mTextOut.mTextTextureIndex), mTextOut.mTextureSize, mTextOut.mTexturePositions[1]);
		}
	}

	mBatchRenderer.EndBatch();
	mBatchRenderer.Flush(mCamera.mUIModelMatrix);
}

void App::ShutDown() {
	SDL_DestroyWindow(mWindow);
	mWindow = nullptr;

	mBatchRenderer.ShutDown();

	glDeleteProgram(mGraphicsPipelineShaderProgram);

	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

App& App::getInstance() {
	static App* app = new App;
	return *app;
}

int App::GetWindowHeight() {
	return mWindowHeight;
}

int App::GetWindowWidth() {
	return mWindowWidth;
}

void App::SetGraphicsPipelineShaderProgram(GLuint program) {
	mGraphicsPipelineShaderProgram = program;
}

void APIENTRY App::GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar* message, const void* param)
{
	const char* source_, * type_, * severity_;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             source_ = "API";             break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   source_ = "WINDOW_SYSTEM";   break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: source_ = "SHADER_COMPILER"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     source_ = "THIRD_PARTY";     break;
	case GL_DEBUG_SOURCE_APPLICATION:     source_ = "APPLICATION";     break;
	case GL_DEBUG_SOURCE_OTHER:           source_ = "OTHER";           break;
	default:                              source_ = "<SOURCE>";        break;
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               type_ = "ERROR";               break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_ = "DEPRECATED_BEHAVIOR"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  type_ = "UDEFINED_BEHAVIOR";   break;
	case GL_DEBUG_TYPE_PORTABILITY:         type_ = "PORTABILITY";         break;
	case GL_DEBUG_TYPE_PERFORMANCE:         type_ = "PERFORMANCE";         break;
	case GL_DEBUG_TYPE_OTHER:               type_ = "OTHER";               break;
	case GL_DEBUG_TYPE_MARKER:              type_ = "MARKER";              break;
	default:                                type_ = "<TYPE>";              break;
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         severity_ = "HIGH";         break;
	case GL_DEBUG_SEVERITY_MEDIUM:       severity_ = "MEDIUM";       break;
	case GL_DEBUG_SEVERITY_LOW:          severity_ = "LOW";          break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: severity_ = "NOTIFICATION"; break;
	default:                             severity_ = "<SEVERITY>";   break;
	}


	std::ostringstream stream;
	stream << "| Id: " << id << " | Severity: " << severity_ << " | Type: " << type_ << " | Source: (" << source_ << ") | Message: " << message << " |" << std::endl;
	std::string output = stream.str();

	std::string dashes(output.size() - 3, '-');

	if (static bool FistLineBoilerplate = true; FistLineBoilerplate) {
		std::cout << "" << std::endl;
		for (size_t i = 0; i < 3; i++) {
			std::cout << "(WARNING DEBUG MODE IS ENABLED AND MAY LEED TO PERFORMACNE ISSUES)\n" << std::endl;
		}
		std::cout << "|" << dashes << "|" << std::endl;
		FistLineBoilerplate = false;
	}

	std::cout << output << "|" << dashes << "|" << std::endl;

}
#include "App.hpp"
#include <SDL3/SDL_hints.h>

int main(int argc, char* argv[]) {
	App& app = App::getInstance();

	if (__cplusplus == 202002L) std::cout << "C++20\n";
	else if (__cplusplus == 201703L) std::cout << "C++17\n";
	else if (__cplusplus == 201402L) std::cout << "C++14\n";
	else if (__cplusplus == 201103L) std::cout << "C++11\n";
	else if (__cplusplus == 199711L) std::cout << "C++98\n";
	else std::cout << "Unknown C++ standard\n";

	app.PostStartUp();
	app.LoadGame();
	app.MainLoop();
	return 0;
}
#pragma once
#include <vector>
#include "GameObject.hpp"
#include <nlohmannjson/json.hpp>
#include "IO.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class GameLevel {
public:
	GameLevel();

	~GameLevel();

	std::vector<GameObject> mBlocks;

	std::vector<std::vector<std::string>> mLevelDataCsv;

	std::vector<std::vector<std::vector<int>>> mLevelData;

	std::vector<int> mTilesetsOffsets;

	void LoadLevel(const std::string file);

	void LoadLevelJson(const std::string file);

	void BuildLevel();

	bool isLoaded{ false };

	int mLevelWidth;
	int mLevelHeight;

	uint8_t mBlockSize{ 18 };

};
#pragma once
#include "GameEntity.hpp"
#include <glm/vec2.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "CollisionHandler.hpp"

enum class MovementDirection {
	None,
	Left,
	Right,
	Top,
	Bottom
};


class Actor : public GameEntity {
public:
	Actor();

	~Actor();

	void Update() override;

	void Move();

	void Transform();

	float movementSpeed{500.0f};

	MovementDirection Direction{ MovementDirection::None };

	glm::vec2 mRelativePosition{ 0.0f, 0.0f };

	glm::vec2 mScreenPosition{ 0.0f, 0.0f };

	float flyAngleTargetPortal{ -1.0f };
	float flyAnglePortal{ 0.0f };

	bool isWallMountableL{ false };
	bool isWallMountableR{ false };

	bool mEscaped{ false };

	bool mDead{ false };

	bool mDeadSoundOneShot{ true };

	bool isSuckedPortal{ false };

	float mSizeMultiplier{ 240.0f };
private:
};
#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include "GameObject.hpp"
#include <random>
#include <vector>
#include "CollisionHandler.hpp"
#include "Actor.hpp"
#include <glm/gtx/norm.hpp>
#include <glm/gtx/exterior_product.hpp>
#include "AnimationHandler.hpp"
#include <SDL3/SDL_mixer.h>

class BlackHole : public GameObject {
public:
	BlackHole();

	~BlackHole();

	void Update(std::vector<GameObject>& blocks, Actor& actor, const float& deltaTime, Animation& loopAnim, Animation& birthAnim, Mix_Chunk* bornSound, Mix_Chunk* consumedSound, Mix_Chunk* blackHoleIdle);

	glm::vec2 AnimationSize{ 0.0f, 0.0f };

	bool isBorn{ false };
	bool loopTimerOneShot{ false };
	bool birthTimerOneShot{ false };
	int idleVolume{ 0 };
	glm::vec2 AABBSize { 100.0f, 1080.0f };

private: 
	glm::vec2 AABBPos { 0.0f, 0.0f };

	glm::vec2 EpicenterAABBSize{ 40.0f, 40.0f };
	glm::vec2 EpicenterAABBPos{ 0.0f, 500.0f };
	float AABBVelocityMultiplier { 0.0f };
	// squared distance of idle volume range
	float idleRadius{ 1250000.0f };

	std::vector<std::pair<int, float>> affectedBlocks;

	std::random_device rd;
	std::mt19937 gen;
	std::uniform_real_distribution<float> randomVelocity;
};
#pragma once
#include <tuple>
#include <limits>
#include <algorithm>
#include <cmath>
#include "HelperStructs.hpp"
#include <glm/glm.hpp>
#include "GameObject.hpp"

class Actor;

bool PointVsRect(const glm::vec2& p, const Box* r);

bool RectVsRect(const glm::vec2 rect1Pos, const glm::vec2 rect1Size, const glm::vec2 rect2Pos, const glm::vec2 rect2Size);

bool RayVsRect(const glm::vec2& rayOrigin, const glm::vec2& rayDirection, const Box* target, glm::vec2& contactPoint, glm::vec2& contactNormal, float& hitTimeNear);

bool DynamicRectVsRect(const Box& dynamicBox, const float deltaTime, const Box& staticBox, const glm::vec2& dynamicBoxVelocity, glm::vec2& contactPoint, glm::vec2& contactNormal, float& contactTime, glm::vec2& position);

bool ResolveDynamicRectVsRect(Box& dynamicBox, const float deltaTime, const Box& staticBox, glm::vec2& dynamicBoxVelocity, Actor& actor, glm::vec2& averagedNormal, bool& NormalGroundCheck);

void CollisionUpdate(const std::vector<GameObject>& blocks, Actor& actor, bool& LeftWallHug, bool& RightWallHug, const float& deltaTime, bool& isGrounded);

#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "GameObject.hpp"
#include "AnimationHandler.hpp"
#include "glm/glm.hpp"
#include "Actor.hpp"
#include "CollisionHandler.hpp"
#include <glm/gtx/norm.hpp>
#include <glm/gtx/exterior_product.hpp>
#include <SDL3/SDL_mixer.h>

class EscapePortal : public GameObject {
public:
	EscapePortal();

	~EscapePortal();

	void Update(Animation& portalAnim, const float& deltaTime, Actor& actor, Mix_Chunk* portalSoundEscape, Mix_Chunk* portalSoundIdle);

	glm::vec2 AnimationSize{ 0.0f, 0.0f };

	float sizeMultiplier{ 200.0f };


	bool animationOneShot{ true };

	glm::vec2 AABBPos{ 0.0f, 0.0f };
	glm::vec2 AABBSize{ 0.0f, 0.0f };
	float suctionVelocity{ 0.0f };

	glm::vec2 EpicenterAABBPos{ 0.0f, 0.0f };
	glm::vec2 EpicenterAABBSize{ 0.0f, 0.0f };

private:
	int idleVolume{ 0 };
	// squared distance of idle volume range
	float idleRadius{ 1250000.0f };

};
#pragma once
#include "Sprite.hpp"
#include <glm/vec2.hpp>
#include "GameObject.hpp"

class GameEntity : public GameObject {
public:
	GameEntity();

	~GameEntity();

	virtual void Update();

	float movementSpeed{ 300.0f };

	glm::vec2 velocity{ 0.0f, 0.0f };

	void SetSprite(const Sprite& sprite);
};
#pragma once
#include "Sprite.hpp"
#include <glm/glm.hpp>
#include <iostream>


class GameObject {
public:
	GameObject();
	
	virtual ~GameObject();

	virtual void Update();

	Sprite mSprite;

	glm::vec2 mPosition{ 0.0f, 0.0f };

	glm::vec2 tempVelocity{ 0.0f, 0.0f };

	glm::vec2 flyDirectionNormalized{ 0.0f, 0.0f };

	glm::mat4 mModelMatrix{ glm::mat4(1.0f) }; 

	glm::vec2 mTriggerAABBPos{ 0.0f, 0.0f };
	glm::vec2 mTriggerAABBSize{ 0.0f, 0.0f };

	float flyAngleTarget{ -1.0f };

	float flyAngle{ 0.0f };

	bool isDestroyed{ false };

	bool isVisible{ false }; 

	bool isSucked{ false };

	bool isDeathTrigger{ false };

	bool isConsumedByVoid{ false };

	bool isCollidable{ false };
};
#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <chrono>
#include "Sign.hpp"
#include "Actor.hpp"

enum class MovementState {
	MOVE_LEFT,
	MOVE_RIGHT,
	SPACE,
	DUCK,
	MOVE_UP,
	MOVE_DOWN,
	END
};

enum class LookDirections {
	LEFT,
	RIGHT,
	END
};

class MovementHandler {

	glm::vec2 acceleration{ 0.0f, 0.0f };

	void Move(glm::vec2& actorVelocity, const glm::vec2& acceleration);

	void Jump(float& deltaTime, const float& jumpSpeed, glm::vec2& actorVelocity);

	void Slam(float& deltaTime, const float& slamSpeed, const float& speedLimit, glm::vec2& actorVelocity);

	float frictionModifier{ 7.2f };

	const int jumpBufferTime{ 100 };
	std::chrono::time_point < std::chrono::steady_clock, std::chrono::duration<long long, std::ratio < 1, 1000000000>>> jumpBufferTimer;

	const int jumpTime{ 200 };
	std::chrono::time_point < std::chrono::steady_clock, std::chrono::duration<long long, std::ratio < 1, 1000000000>>> jumpTimer;

	const int wallJumpTime{ 200 };
	std::chrono::time_point < std::chrono::steady_clock, std::chrono::duration<long long, std::ratio < 1, 1000000000>>> wallJumpTimer;

	const int doubleJumpTime{ 150 };
	std::chrono::time_point < std::chrono::steady_clock, std::chrono::duration<long long, std::ratio < 1, 1000000000>>> doubleJumpTimer;

	bool isJumping{ false };

	bool isWallJumping{ false }; 

	bool isDoubleJumping{ false };

	bool slideOneShot{ true };

	int slideDirection{ 0 };

public:
	MovementHandler();

	~MovementHandler();

	bool canDoubleJump{ false };
	bool isGrounded{ false };
	bool isSlamming{ false };

	bool isSliding{ false };
	bool canWallStick{ false };

	void Update(float& deltaTime, Actor& actor);

	bool spacebarOneShot{ true };

	bool duckOneShot{ true };

	bool KeyboadStates[static_cast<int>(MovementState::END)] = { false };

	LookDirections lookDirection = LookDirections::RIGHT;

	bool LeftWallHug{ false };
	bool RightWallHug{ false };

	bool GravityState{ true };

};
#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <iostream>
#include "HelperStructs.hpp"

struct Sprite {
	Sprite();

	~Sprite();

	Box vertexData;

	std::vector<GLfloat> mTexturePosition;
};
#pragma once
#include "Actor.hpp"
#include <glm/glm.hpp>
#include "MovementHandler.hpp"
#include "AnimationHandler.hpp"
#include "AudioHandler.hpp"

enum class PlayerStates {
	RUNNING,
	JUMPING,
	DOUBLE_JUMPING,
	FALLING,
	SLIDING,
	SLAMMING,
	WALLSLIDING,
	DEAD,
	HIT,
	IDLE,
	SUCKED,
	VOID_CONSUMED,
	ESCAPED,
	END
};

class StateMachine {
public:
	StateMachine();

	~StateMachine();

	void Update(MovementHandler& movementHandler, AnimationHandler& animationHandler, AudioHandler& audioHandler, Actor& actor, const float& deltaTime);

	bool CheckPlayerStateChange();

	bool CompareToLastState(const PlayerStates& state);

	glm::vec2 mCurrentActorDrawSize{ 0.0f, 0.0f };
	glm::vec2 mCurrentActorTextureSize{ 0.0f, 0.0f };
	glm::vec2 mCurrentActorTexturePosition{ 0.0f, 0.0f };
	uint32_t mCurrentActorTextureIndex{ 0 };
	bool mActorFlipped{ false };

	PlayerStates currentPlayerState = PlayerStates::IDLE;
	PlayerStates lastState = PlayerStates::IDLE;
	bool deadAnimOneShot{ true };
	bool deadAnimDone{ false };
private:

	int FallVolume{ 1 };
	bool runAnimationOneShot{ true };
	bool playingRunSound{ false };
	float FallVolumeTime{ 0.1f };
	float FallVolumeTimer{ 0.0f };

	int SuckedVolume{ 1 };
	float SuckedVolumeTime{ 0.2f };
	float SuckedVolumeTimer{ 0.0f };

	void CheckPlayerState(Actor& actor, MovementHandler& movementHandler);
};
#pragma once
#include "Texture.hpp"
#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <vector>
#include <chrono>

struct Animation {
	glm::vec2 Size;
	glm::vec2 TexturePosition;
	std::vector<uint32_t> AnimationTextureIndexes;
	int AnimationTime;
	std::chrono::time_point < std::chrono::steady_clock, std::chrono::duration<long long, std::ratio < 1, 1000000000>>> AnimationTimer;
	int SingleFrameTime;
	std::chrono::time_point < std::chrono::steady_clock, std::chrono::duration<long long, std::ratio < 1, 1000000000>>> SingleFrameTimer;
	int index{ 0 };
};


class AnimationHandler {
public:
	AnimationHandler(TextureHandler& textureHandlerRef);
	~AnimationHandler();

	void Init(const int& samplerSize);

	void InitMultiFrameAnimation(SDL_Surface* animationSpritesheet, Animation& animation, const glm::vec2& animationSize, const int& animationTime, const int& spriteSizeX, const int& spriteSizeY, const glm::vec2& texturePosition = glm::vec2(0.0f, 0.0f));

	void InitSingleFrameAnimation(SDL_Surface* animationSprite, Animation& animation, const glm::vec2& animationSize, const int& spriteSizeX, const int& spriteSizeY, const glm::vec2& texturePosition = glm::vec2(0.0f, 0.0f));

	Animation DeadAnimation;
	Animation DuckAnimation;
	Animation HitAnimation;
	Animation RunAnimation;
	Animation FallAnimation;
	Animation IdleAnimation;
	Animation JumpAnimation;
	Animation SlamAnimation;
	Animation SlideAnimation;
	Animation WallSlideAnimation;
	Animation DuckIdleAnimation;
	Animation BlackHoleBirthAnimation;
	Animation BlackHoleLoopAnimation;
	Animation EscapePortalAnimation;

private:
	TextureHandler&  mTextureHandlerRef;

	int SampelrSize{ 0 };

	const int AnimationSpriteSizeX{ 128 };
	const int AnimationSpriteSizeY{ 128 };
};
#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>
#include <array>
#include <iostream>
#include "glm/gtx/string_cast.hpp"
#include <glm/mat4x4.hpp>
#include "HelperStructs.hpp"

// usage On StartUp BatchRenderer::Startup -> in update 1st, BatchRenderer::BeginBatch, 2nd, BatchRenderer::Draw,
// 3rd BatchRenderer::EndBatch, 4th BatchRenderer::Flush, On ShutDown BatchRenderer::Shutdown
class App;

struct BatchRenderer {

	BatchRenderer();

	~BatchRenderer();

	void StartUp(const GLuint& PipelineProgram);
	void ShutDown();

	void BeginBatch(const glm::mat4& ProjectionMatrix);
	void EndBatch();
	void Flush(const glm::mat4 ModelMatrix = glm::mat4(1.0f));

	void DrawSeperatly(const glm::vec2& position, glm::vec2 size, const glm::vec4& color, const glm::mat4& ProjectionMatrix, const glm::mat4& ModelMatrix = glm::mat4(1.0f));
	void DrawSeperatly(const glm::vec2& position, glm::vec2 size, const glm::mat4& ProjectionMatrix, uint32_t textureID, const glm::vec2& textureSize = glm::vec2(1.0f, 1.0f), const glm::vec2& texturePosition = glm::vec2(0.0f, 0.0f), const glm::mat4& ModelMatrix = glm::mat4(1.0f), const bool& drawFliped = false);

	void DrawInBatch(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

	void DrawInBatch(const glm::vec2& position, const glm::vec2& size, uint32_t textureID, const glm::vec2& textureSize = glm::vec2(1.0f, 1.0f), const glm::vec2& texturePosition = glm::vec2(0.0f, 0.0f), const bool& drawFliped = false);

	static const size_t MaxQuadCount{ 1000 };
	static const size_t MaxVertexCount{ MaxQuadCount * 4 };
	static const size_t MaxIndexCount{ MaxQuadCount * 6 };

	GLuint mVAO{ 0 };
	GLuint mVBO{ 0 };
	GLuint mIBO{ 0 };

	uint32_t IndexCount{ 0 };

	Box* QuadBuffer{ nullptr };
	Box* QuadBufferPtr{ nullptr };

	glm::mat4 CurrentProjectionMatrix;

	GLuint CurrentPipeline;

	void UniformVariableLinkageAndPopulatingWithMatrix(const GLchar* uniformLocation, glm::mat4 matrix, const GLuint& PipelineProgram);

private:
	App* app_;

	App& app();
};
#pragma once
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL

class App;

class Camera {
public:

	Camera();

	App& app();

	App* app_;

	void Update(glm::vec2& actorVelocity, glm::vec2& actorScreenPosition, const float& deltaTime);

	glm::mat4 GetProjectionMatrix() const;

	void SetProjectionMatrix();

	glm::vec2 mCameraOffset{ 0.0f, 0.0f };

	glm::mat4 mUIModelMatrix{1.0f};

private:
	glm::vec2 mMousePossition;

	glm::mat4 mProjectionMatrix;
	glm::mat4 mInitialProjectionMatrix;

	glm::vec2 mOldMousePosition;

	float mCameraOffsetTimerBuffer{ 0.0f };
	float mCameraOffsetTimeBuffer{ 0.5f };
	float mCameraOffsetTimerBuffer2{ 0.0f };
	float mCameraOffsetTimeBuffer2{ 0.5f };
};

#pragma once
#include <glad/glad.h>
#include <string>
#include <iostream>

class App;

class PipelineManager {

	GLuint CompileShader(GLuint shaderType, const std::string& shaderSource);

	GLuint CreateShaderProgram(const std::string& vShaderSource, const std::string& fShaderSource);

	App* app_;

	App& app();

public:
	void CreateGraphicsPipeline();

	PipelineManager();

	~PipelineManager();
};
#pragma once
#include <SDL3/SDL.h>
#include "Texture.hpp"
#include "glm/glm.hpp"
#include <vector>

class TextOut {
public:
	TextOut();

	~TextOut();

	void Init(TextureHandler& textureHandler, const char* filepath);

	void Update();

	int mTextTextureIndex{ 0 };

	std::vector<glm::vec2> mTexturePositions;

	glm::vec2 mTextureSize{ 0.0f, 0.0f };
};
#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_surface.h>
#include <string>
#include <array>
#include <glad/glad.h>
#include <iostream>
#include <vector>


class TextureHandler {
public:
	TextureHandler();

	~TextureHandler();

	SDL_Surface* LoadSurface(const char* filepath);

	void InitTextureArray(const GLenum& internalformat, const GLsizei& width, const GLsizei& height, const GLsizei& depth);

	void LoadTexture(SDL_Surface* surface, const GLenum& internalformat, const int& layer, const int& slot);

	SDL_Surface* FlipSurfaceVertically(SDL_Surface* surface);
	std::vector<SDL_Surface*> CutTileset(SDL_Surface* tileset, const int& tileWidth, const int& tileHeight);

	int TextureSlotsTaken = 0;

	std::array<GLuint, 32> textureArrays;

	// array to keep track how many layers are in use
	std::array<int, 32> layersUsed;
};
#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_surface.h>
#include <string>
#include <array>
#include <glad/glad.h>
#include <iostream>
#include <vector>


class TextureHandler {
public:
	TextureHandler();

	~TextureHandler();

	SDL_Surface* LoadSurface(const char* filepath);

	void InitTextureArray(const GLenum& internalformat, const GLsizei& width, const GLsizei& height, const GLsizei& depth);

	void LoadTexture(SDL_Surface* surface, const GLenum& internalformat, const int& layer, const int& slot);

	SDL_Surface* FlipSurfaceVertically(SDL_Surface* surface);
	std::vector<SDL_Surface*> CutTileset(SDL_Surface* tileset, const int& tileWidth, const int& tileHeight);

	int TextureSlotsTaken = 0;

	std::array<GLuint, 32> textureArrays;

	// array to keep track how many layers are in use
	std::array<int, 32> layersUsed;
};
#pragma once
#include <glm/glm.hpp>
#include <vector>

struct Box {
	glm::vec2 Position;
	glm::vec4 Color;
	glm::vec2 Size;
	glm::vec2 TexturePosition;
	int TextureIndex;
};

#pragma once
#include <SDL3/SDL.h>

class App;

class InputManager {
	App& app();

	App* app_;

public:
	InputManager();

	~InputManager();

	void Input();

};

#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

std::string ReadFileAsString(const std::string& filePath);

#pragma once
#include <type_traits>

template <typename T> inline constexpr
int Sign(T x, std::false_type is_signed) {
    return T(0) < x;
}

template <typename T> inline constexpr
int Sign(T x, std::true_type is_signed) {
    return (T(0) < x) - (x < T(0));
}

template <typename T> inline constexpr
int Sign(T x) {
    return Sign(x, std::is_signed<T>());
}


#pragma once
#include <SDL3/SDL.h>
#include "Camera.hpp"
#include <glad/glad.h>
#include <iostream>
#include <sstream>
#include "IO.hpp"
#include "Input.hpp"
#include "PipelineManager.hpp"
#include "Sprite.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Actor.hpp"
#include "BatchRenderer.hpp"
#include "HelperStructs.hpp"
#include "GameLevel.hpp"
#include "CollisionHandler.hpp"
#include "MovementHandler.hpp"
#include <vector>
#include <chrono>
#include <SDL3/SDL_hints.h>
#include "Texture.hpp"
#include "AnimationHandler.hpp"
#include "AudioHandler.hpp"
#include "BlackHole.hpp"
#include "StateMachine.hpp"
#include "EscapePortal.hpp"
#include "SimpleTextOut.hpp"


class App {
	App();

	~App();

	App(const App&) = delete;

	App& operator=(const App&) = delete;

	void StartUp();

	void ShutDown();

	void Update();

	static void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
		GLenum severity, GLsizei length,
		const GLchar* message, const void* param);
public:
	static App& getInstance();

	void PostStartUp();

	void LoadGame();
	
	void UIUpdate();

	void MainLoop();

	void SetGraphicsPipelineShaderProgram(GLuint program);

	int GetWindowWidth();
	int GetWindowHeight();

	int mWindowWidth{ 2560 };
	int mWindowHeight{ 1440 };

	SDL_Window* mWindow{ nullptr };
	SDL_GLContext mGlContext{ nullptr };

	GLuint mGraphicsPipelineShaderProgram{ 0 };

	InputManager mInputManager;

	BatchRenderer mBatchRenderer;

	Actor mActor;

	MovementHandler mMovementHandler;

	AudioHandler mAudioHandler;

	GameLevel mLevel;

	EscapePortal mEscapePortal;

	TextureHandler mTextureHandler;

	AnimationHandler mAnimationHandler;

	std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<long long, std::ratio<1, 10000000>>> tp1;
	std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<long long, std::ratio<1, 10000000>>> tp2;

	BlackHole mBlackHole;

	TextOut mTextOut;

	StateMachine mStateMachine;

	Camera mCamera;

	PipelineManager mPipelineManager;

	bool mGameStarted{ false };

	bool mQuit{ false };
	bool mVsync{ false };
	bool mDebug{ true };

	float deltaTime{ 0.1f };
	float deltaTimeRaw;
	float deltaTimeBuffer{ 0.0f };

	float textSizeMultiplier{ 800.0f };
	float titleScreenAlpha{ 0.0f };
	int titleScreenMusicVolume{ 128 };

	float titleScreenAlphaTime{ 0.01f };
	float titleScreenAlphaTimer{ 0.0f };

	float titleScreenMessageTime{ 2.0f };
	float titleScreenMessageTimer{ 0.0f };

	float titleScreenMusicVolumeTime{ 0.05f };
	float titleScreenMusicVolumeTimer{ 0.0f };

	float startMessageTime{ 2.0f };
	float startMessageTimer{ 0.0f };
};

#version 460 core

out vec4 FragColor;

in vec4 vVertexColor;
in vec2 vTexturePosition;
flat in int vTextureIndex;

uniform sampler2DArray uTextures;

void main() {
	vec3 texCoords = vec3(vTexturePosition, vTextureIndex); // Encode TexIndex as the z-coordinate
    FragColor = texture(uTextures, texCoords) * vVertexColor;
}

#version 460 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec2 texturePosition;
layout(location = 3) in int textureIndex;

out vec4 vVertexColor;
out highp vec2 vTexturePosition;
flat out int vTextureIndex;


uniform mat4 uModelMatrix;
uniform mat4 uProjectionMatrix;

void main() {

	vVertexColor = vertexColor;
	vTexturePosition = texturePosition;
	vTextureIndex = textureIndex;

	vec4 newPosition = uProjectionMatrix * uModelMatrix * vec4(position, 0.0f, 1.0f);
	gl_Position = vec4(newPosition.x, newPosition.y, newPosition.z, newPosition.w);
	
}
