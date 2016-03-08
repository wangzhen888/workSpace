#include"GameLayer.h"
#include"global.h"
#include"BulletBase.h"


USING_NS_CC;
using namespace CocosDenshion;

int GameLayer::m_gameLevel = EASY;

CCScene* GameLayer::scene()
{
	CCScene* scene = CCScene::create();
	CCLayer* layer = GameLayer::create();
	scene->addChild(layer);
	return scene;
}

void GameLayer::initData()
{
	m_gameScore = 0;
	bigBoomCount = 0;
}

bool GameLayer::init()
{
	if(!CCLayer::init())
	{
		return false;
	}
	this->setTouchEnabled(true);
	this->initData();

	CCSize winSize = CCDirector::sharedDirector()->getWinSize();
	m_bgSp1 = CCSprite::createWithSpriteFrameName("background.png");
	m_bgSp2 = CCSprite::createWithSpriteFrameName("background.png");

	m_bgSp1->setAnchorPoint(ccp(0, 0));
	m_bgSp1->setPosition(ccp(0, 0));
	this->addChild(m_bgSp1);

	m_bgSp2->setAnchorPoint(ccp(0, 0));
	m_bgSp2->setPosition(ccp(0, m_bgSp1->getContentSize().height - 2));
	this->addChild(m_bgSp2);

	//�������
	m_plane = PlaneLayer::createPlane();
	this->addChild(m_plane);

	//����ӵ�
	m_bullet = BulletLayer::createBullet();
	this->addChild(m_bullet);
	m_bullet->startShoot();

	//��Ӷ����ӵ�
	m_multiBullet = MultiBulletLayer::createBullet();
	this->addChild(m_multiBullet);

	//��ӵ���
	m_enemyLayer = EnemyLayer::createEnemyLayer();
	this->addChild(m_enemyLayer);

	//��ӿ��Ʋ�
	m_controlLayer = ControlLayer::createControlLayer();
	this->addChild(m_controlLayer);

	//��ӵ���
	m_ufoLayer = UFOLayer::createUFOLayer();
	this->addChild(m_ufoLayer);

	//������������
	this->schedule(schedule_selector(GameLayer::moveBackground));

	this->scheduleUpdate();
	return true;
}

int GameLayer::getGameLevel()
{
	return m_gameLevel;
}

void GameLayer::update(float dt)
{
	if(m_gameLevel == EASY && m_gameScore >= MIDDLE_SCORE)
	{
		m_gameLevel = MIDDLE;
	}
	else if(m_gameLevel == MIDDLE && m_gameScore >= HARD_SCORE)
	{
		m_gameLevel = HARD;
	}

	//��ͨ�ӵ���л�����ײ���
	this->checkNormalBulletCollisionToEnemy(NORMAL_BULLET);

	//2���ӵ���л�����ײ���
	this->checkNormalBulletCollisionToEnemy(MULTI_BULLET);
	
	//������л�����ײ���
	this->actorCollisionToEnemy();

	//������ը�����ߵ���ײ���
	this->actorCollisionToBigBoom();

	//������˫���ӵ����ߵ���ײ���
	this->actorCollisionToSupplyTools();
}

void GameLayer::actorDeadDone(Enemy* enemy, CCRect actorRect)
{
	if(enemy->getLife() > 0)
	{
		if(actorRect.intersectsRect(enemy->getBoundingBox()))
		{
			this->unscheduleAllSelectors();
			m_bullet->stopShoot();
			m_multiBullet->stopShoot();
			m_plane->blowEffect(m_gameScore);
		}
	}
}

void GameLayer::actorCollisionToEnemy()
{
	CCRect actorRect = m_plane->getChildByTag(AIR_TAG)->boundingBox();
	CCObject* enemyObj = NULL;

	//���һ��л���ײ�ļ��
	CCARRAY_FOREACH(m_enemyLayer->getEnemyOneArrays(), enemyObj)
	{
		Enemy* enemy = (Enemy*)enemyObj;
		this->actorDeadDone(enemy, actorRect);
	}

	//��ڶ���л���ײ�ļ��
	CCARRAY_FOREACH(m_enemyLayer->getEnemySecondArrays(), enemyObj)
	{
		Enemy* enemy = (Enemy*)enemyObj;
		this->actorDeadDone(enemy, actorRect);
	}

	//�������л���ײ�ļ��
	CCARRAY_FOREACH(m_enemyLayer->getEnemyThirdArrays(), enemyObj)
	{
		Enemy* enemy = (Enemy*)enemyObj;
		this->actorDeadDone(enemy, actorRect);
	}
}

void GameLayer::checkNormalBulletCollisionToEnemy(BulletType type)
{
	CCArray* bulletArrays = CCArray::create();
	bulletArrays->retain();
	CCObject* bulletObj, *enemyObj;
	BulletBase* baseBullet = NULL;

	if(type == NORMAL_BULLET)
	{
		baseBullet = m_bullet;
	}
	else if(type == MULTI_BULLET)
	{
		baseBullet = m_multiBullet;
	}

	//�ӵ����һ��л�����ײ���
	CCARRAY_FOREACH(baseBullet->getAllBullets(), bulletObj)
	{
		CCSprite* bullet = (CCSprite*)bulletObj;
		CCArray* enemyOneArrays = CCArray::create();
		enemyOneArrays->retain();

		CCARRAY_FOREACH(m_enemyLayer->getEnemyOneArrays(), enemyObj)
		{
			Enemy* enemy = (Enemy*)enemyObj;
			if(bullet->boundingBox().intersectsRect(enemy->getBoundingBox()))
			{
				if(enemy->getLife() == 1)
				{
					enemy->loseLife();
					//������ײ�л����ӵ�����������
					bulletArrays->addObject(bullet);
					enemyOneArrays->addObject(enemy);
					m_gameScore += ENEMYONE_SCORE;
					m_controlLayer->updateScore(m_gameScore);
				}
			}
		}
		//��һ�����͵ĵл���ը
		CCARRAY_FOREACH(enemyOneArrays, enemyObj)
		{
			Enemy* enemy = (Enemy*)enemyObj;
			m_enemyLayer->enemyOneBlowEffect(enemy);
		}
		//������ӵ�������ײ�ĵ�һ��л�
		enemyOneArrays->release();
	}

	//�����һ���͵л�������ײ���ӵ�
	CCARRAY_FOREACH(bulletArrays, bulletObj)
	{
		CCSprite* bullet = (CCSprite*)bulletObj;
		baseBullet->removeBullet(bullet);
	}
	bulletArrays->removeAllObjects();

	//����ӵ�������͵л�����ײ
	CCARRAY_FOREACH(baseBullet->getAllBullets(), bulletObj)
	{
		CCSprite* bullet = (CCSprite*)bulletObj;
		CCArray* enemySecondArrays = CCArray::create();
		enemySecondArrays->retain();
		CCARRAY_FOREACH(m_enemyLayer->getEnemySecondArrays(), enemyObj)
		{
			Enemy* enemy = (Enemy*)enemyObj;
			if(bullet->boundingBox().intersectsRect(enemy->getBoundingBox()))
			{
				if(enemy->getLife() == 1)
				{
					enemy->loseLife();
					bulletArrays->addObject(bullet);
					enemySecondArrays->addObject(enemy);
					m_gameScore += ENEMYSECOND_SCORE;
					m_controlLayer->updateScore(m_gameScore);
				}
				else if(enemy->getLife() > 1)
				{
					enemy->loseLife();
					bulletArrays->addObject(bullet);
				}
			}
		}

		CCARRAY_FOREACH(enemySecondArrays, enemyObj)
		{
			Enemy* enemy = (Enemy*)enemyObj;
			m_enemyLayer->enemySecondBlowEffect(enemy);
		}
		enemySecondArrays->release();
	}

	CCARRAY_FOREACH(bulletArrays, bulletObj)
	{
		CCSprite* bullet = (CCSprite*)bulletObj;
		baseBullet->removeBullet(bullet);
	}
	bulletArrays->removeAllObjects();


	//����ӵ��������л�����ײ
	CCARRAY_FOREACH(baseBullet->getAllBullets(), bulletObj)
	{
		CCSprite* bullet = (CCSprite*)bulletObj;
		CCArray* enemyThirdArrays = CCArray::create();
		enemyThirdArrays->retain();

		CCARRAY_FOREACH(m_enemyLayer->getEnemyThirdArrays(), enemyObj)
		{
			Enemy* enemy = (Enemy*)enemyObj;
			if(bullet->boundingBox().intersectsRect(enemy->getBoundingBox()))
			{
				if(enemy->getLife() > 1)
				{
					bulletArrays->addObject(bullet);
					enemy->loseLife();
				}
				else if(enemy->getLife() == 1)
				{
					bulletArrays->addObject(bullet);
					enemyThirdArrays->addObject(enemy);
					enemy->loseLife();
					m_gameScore += ENEMYTHIRD_SCORE;
					m_controlLayer->updateScore(m_gameScore);
				}
			}
		}

		CCARRAY_FOREACH(enemyThirdArrays, enemyObj)
		{
			Enemy* enemy = (Enemy*)enemyObj;
			m_enemyLayer->enemyThirdBlowEffect(enemy);
		}
	}

	CCARRAY_FOREACH(bulletArrays, bulletObj)
	{
		CCSprite* bullet = (CCSprite*)bulletObj;
		baseBullet->removeBullet(bullet);
	}
	bulletArrays->removeAllObjects();
}

//���������˫���ӵ����ߵ���ײ
void GameLayer::actorCollisionToSupplyTools()
{
	CCObject *ufoObj = NULL;
	CCArray* toolArrays = m_ufoLayer->getAllMultiBulletArrays();

	CCARRAY_FOREACH(toolArrays, ufoObj)
	{
		CCSprite* ufoSp = (CCSprite*)ufoObj;
		if(m_plane->getChildByTag(AIR_TAG)->boundingBox().intersectsRect(ufoSp->boundingBox()))
		{
			SimpleAudioEngine::sharedEngine()->playEffect("sound/get_double_laser.mp3");
			m_ufoLayer->removeMultiBullets(ufoSp);
			m_bullet->stopShoot();
			m_multiBullet->startShoot();
			m_bullet->startShoot(6.2f);
		}
	}
}

//���������ը�����ߵ���ײ
void GameLayer::actorCollisionToBigBoom()
{
	CCObject *ufoObj = NULL;
	CCArray* toolArrays = m_ufoLayer->getAllBigBoomArrays();
	
	CCARRAY_FOREACH(toolArrays, ufoObj)
	{
		CCSprite* ufoSp = (CCSprite*)ufoObj;
		if(m_plane->getChildByTag(AIR_TAG)->boundingBox().intersectsRect(ufoSp->boundingBox()))
		{
			SimpleAudioEngine::sharedEngine()->playEffect("sound/get_bomb.mp3");
			m_ufoLayer->removeBigBoom(ufoSp);
			bigBoomCount++;
			this->updateBigBoomItem();
		}
	}
}

void GameLayer::updateBigBoomItem()
{
	CCSprite* normalBomb = CCSprite::createWithSpriteFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("bomb.png"));
	CCSprite* pressedBomb = CCSprite::createWithSpriteFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("bomb.png"));

	if(bigBoomCount < 0)
	{
		return;
	}
	else if(bigBoomCount == 0)
	{
		if(this->getChildByTag(BOMB_MENU))
		{
			this->removeChildByTag(BOMB_MENU, true);
		}

		if(this->getChildByTag(BOMB_LABEL))
		{
			this->removeChildByTag(BOMB_LABEL, true);
		}
	}
	else if(bigBoomCount == 1)
	{
		if(!this->getChildByTag(BOMB_MENU))
		{
			CCMenuItemImage* pBigBoomItem = CCMenuItemImage::create();
			pBigBoomItem->initWithNormalSprite(normalBomb, pressedBomb, NULL, this, menu_selector(GameLayer::menuBombCallback));
			pBigBoomItem->setPosition(ccp(normalBomb->getContentSize().width / 2 + 10, normalBomb->getContentSize().height / 2 + 10));
			bombMenu = CCMenu::create(pBigBoomItem, NULL);
			bombMenu->setPosition(CCPointZero);
			this->addChild(bombMenu, 0, BOMB_MENU);
		}

		if(this->getChildByTag(BOMB_LABEL))
		{
			this->removeChildByTag(BOMB_LABEL);
		}
	}
	else
	{
		if(!this->getChildByTag(BOMB_MENU))
		{
			CCMenuItemImage* pBigBoomItem = CCMenuItemImage::create();
			pBigBoomItem->initWithNormalSprite(normalBomb, pressedBomb, NULL, this, menu_selector(GameLayer::menuBombCallback));
			pBigBoomItem->setPosition(ccp(normalBomb->getContentSize().width / 2 + 10, normalBomb->getContentSize().height / 2 + 10));
			bombMenu = CCMenu::create(pBigBoomItem, NULL);
			bombMenu->setPosition(CCPointZero);
			this->addChild(bombMenu, 0, BOMB_MENU);
		}

		if(this->getChildByTag(BOMB_LABEL))
		{
			this->removeChildByTag(BOMB_LABEL);
		}

		if(bigBoomCount >= 0 && bigBoomCount <= MAX_BIGBOOM_COUNT)
		{
			CCString* countStr = CCString::createWithFormat("%d", bigBoomCount);
			bombLabel = CCLabelBMFont::create(countStr->getCString(), "font/font.fnt");
			bombLabel->setColor(ccc3(143, 146, 147));
			bombLabel->setAnchorPoint(ccp(0, 0.5f));
			bombLabel->setPosition(ccp(normalBomb->getContentSize().width / 2 + 15, normalBomb->getContentSize().height / 2 + 5));
			this->addChild(bombLabel, 0, BOMB_LABEL);
		}
	}
}

void GameLayer::menuBombCallback(CCObject* obj)
{
	if(bigBoomCount > 0 && !CCDirector::sharedDirector()->isPaused())
	{
		SimpleAudioEngine::sharedEngine()->playEffect("sound/use_bomb.mp3");
		bigBoomCount--;
		m_gameScore += m_enemyLayer->getEnemyOneArrays()->count() * ENEMYONE_SCORE;
		m_gameScore += m_enemyLayer->getEnemySecondArrays()->count() * ENEMYSECOND_SCORE;
		m_gameScore += m_enemyLayer->getEnemyThirdArrays()->count() * ENEMYTHIRD_SCORE;
		m_enemyLayer->removeAllEnemyOne();
		m_enemyLayer->removeAllEnemySecond();
		m_enemyLayer->removeAllEnemyThird();
		this->updateBigBoomItem();
		m_controlLayer->updateScore(m_gameScore);
	}
}

void GameLayer::moveBackground(float dt)
{
	m_bgSp1->setPositionY(m_bgSp1->getPositionY() - 2);

	m_bgSp2->setPositionY(m_bgSp1->getPositionY() - 2 + m_bgSp1->getContentSize().height);
	if(m_bgSp2->getPositionY() <= 0)
	{
		m_bgSp1->setPositionY(0);
	}
}

bool GameLayer::ccTouchBegan(CCTouch* pTouch, CCEvent* pEvent)
{
	return true;
}

void GameLayer::ccTouchMoved(CCTouch* pTouch, CCEvent* pEvent)
{
	CCNode* node = m_plane->getChildByTag(AIR_TAG);
	if(!node)
	{
		return;
	}

	CCPoint diffPos = pTouch->getDelta();
	CCPoint currentPos = node->getPosition();
	CCPoint movePos = ccpAdd(currentPos, diffPos);
	m_plane->movePlane(movePos);
}

void GameLayer::registerWithTouchDispatcher()
{
	CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, true);
}

void GameLayer::onEnterTransitionDidFinish()
{
	SimpleAudioEngine::sharedEngine()->playBackgroundMusic("sound/game_music.mp3", true);
}

void GameLayer::onExitTransitionDidStart()
{
	SimpleAudioEngine::sharedEngine()->end();
}