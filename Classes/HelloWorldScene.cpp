#include "HelloWorldScene.h"
#include "audio/include/SimpleAudioEngine.h"
#include <iomanip>

USING_NS_CC;

//#define CUSTOM_TEST
// width of png in pixel
const int bird_category_mask = 0x1;
const int pipe_category_mask = 0x2;
const int ground_category_mask = 0x4;
const int world_category_mask = 0x8;
const int NO_COLLISION_GROUP = -1;
const int COLLISION_GROUP = 1;
const float halfPipeWidth = 25.5f;
const float halfBirdWidth = 17.0f;
const float birdFrameDuration = 0.5f / 3.0f;
const char *key_inited = "initialized";
const char *key_first = "first";
const char *key_second = "second";
const char *key_third = "third";
const char *BIRD_ANIMATION = "bird_animation";
const float groundStripeWidth = 12.0f;
const char *background_day = "background_day.png";
const char *background_night = "background_night.png";

bool screen_shot = false;
unsigned long pic_count = 0;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::createWithPhysics();

    // draw all the physics debug message
//	scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
	scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_NONE);
    scene->getPhysicsWorld()->setGravity(Vect(0.0f, -640.0f));
    scene->getPhysicsWorld()->setAutoStep(false);
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    if ( !Layer::init() )
    {
        return false;
    }

    pipe_count = 0;
    score = 0;
    gamut = 26;
    pipe_duration = 1.4f;
    pipe_velocity_x = -160.0f;
    boost_factor = 1.0f;
    game_started = false;
    night_time = false;
    cheat_mode = false;
    visible_size = Director::getInstance()->getVisibleSize();
    origin = Director::getInstance()->getVisibleOrigin();
    // 碰撞回弹系数设为0.0f(不回弹)，其实无所谓，别设为1.0f就好
    world_material = PhysicsMaterial(0.1f, 0.0f, 0.0f);
    // init rand seed
    std::srand((unsigned int) time (NULL));

    initSoundEffect();
    addBackground();
    addGround();
    addScoreLabel();
    addBird();
    addMenu();
    addKeyEvent();
    addContactEvent();

    return true;
}

void HelloWorld::onEnter(){
	Layer::onEnter();
	auto edgeBody = PhysicsBody::createEdgeBox(visible_size, world_material);

	edgeBody->setCategoryBitmask(world_category_mask);
	edgeBody->setContactTestBitmask(bird_category_mask);
	edgeBody->setCollisionBitmask(bird_category_mask);
	edgeBody->setGroup(NO_COLLISION_GROUP);

	auto edgeShape = Node::create();
	edgeShape->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	edgeShape->setPhysicsBody(edgeBody);
	edgeShape->setPosition(visible_size.width/2, visible_size.height/2);
	addChild(edgeShape);
}

void HelloWorld::update(float delta)
{
	for (Vector<Sprite*>::iterator pipe = pipe_created.begin(); pipe != pipe_created.end();) {
		if (((*pipe)->getPositionX() + (*pipe)->getContentSize().width / 2)
				< (bird->getPositionX() - bird->getContentSize().width / 2)){
			++pipe_count;
			pipe_to_remove.pushBack(*pipe);
			pipe = pipe_created.erase(pipe);
			if (pipe_count % 2 == 0){
				++score;
				if (night_time && score != 0 && (score % 5 == 0)){
					Vec2 random_pos(rand_0_1() * visible_size.width, ground->getContentSize().height);
					addFireworks(random_pos, !cheat_mode);	// in cheat mode we won't hear the firework sound effect
				}
				if (gamut == 26)
					CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/sound_score.ogg");
				else{
//					log("gamut = %d", gamut);
					char fileName[] = "sounds/piano/soundA.mp3";
					fileName[18] += gamut;				// modify file name to sound'X'.mp3
					CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(fileName);
				}
			}
		}else{
			++pipe;
		}
	}
	for (Vector<Sprite*>::iterator pipe = pipe_to_remove.begin(); pipe != pipe_to_remove.end();) {
		if ((*pipe)->getPositionX() + (*pipe)->getContentSize().width / 2 < 0) {
			removeChild(*pipe);
			pipe = pipe_to_remove.erase(pipe);
		}else{
			++pipe;
		}
	}
	updateScoreLabel(score);
	if (screen_shot){
		exportPNG(delta);
	}
}

void HelloWorld::startGame(float delta)
{
	getScene()->getPhysicsWorld()->setAutoStep(true);
	schedule(schedule_selector(HelloWorld::addPipe), pipe_duration / boost_factor);
	scheduleUpdate();
	addTouchEvent();
	ground->runAction(ground_action);
    game_started = true;
}

void HelloWorld::stopGame()
{
	game_started = false;
	ground->stopAllActions();
	bird->stopAllActions();
	Director::getInstance()->getEventDispatcher()->removeEventListener(touch_event);
	unscheduleUpdate();
	unschedule(schedule_selector(HelloWorld::addPipe));
	getScene()->getPhysicsWorld()->setAutoStep(false);
}

void HelloWorld::initSoundEffect()
{
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("sounds/sound_wing.ogg");
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("sounds/sound_hit.ogg");
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("sounds/sound_score.ogg");
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("sounds/firework.mp3");
	char fileName[] = "sounds/piano/soundA.mp3";
	for (char order = 'A'; order < 'Z'+1; ++order){
		fileName[18] = order;
		CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect(fileName);
	}
}

void HelloWorld::addBackground()
{
	night_time = rand() % 2 == 1 ? true : false;
	std::string imgFileName = night_time ? background_night : background_day;
	background = Sprite::create(imgFileName);
	background->setPosition(visible_size.width / 2, visible_size.height / 2);
	addChild(background, -10);
}

void HelloWorld::addBird()
{
	// add animation
	SpriteFrameCache *_frameCache = SpriteFrameCache::getInstance();
	_frameCache->addSpriteFramesWithFile("birds.plist");
	bird_animation = Animation::create();
	int bird_type = rand() % 9;
	switch (bird_type){
	case 0:	case 1:	case 2:	case 3:
		for (int i = 0; i < 3; ++i){
			std::stringstream ss;
			ss << "blue_bird_" << i << ".png";
			bird_animation->addSpriteFrame(_frameCache->getSpriteFrameByName(ss.str()));
		}
		bird = Sprite::createWithSpriteFrameName("blue_bird_0.png");
		break;
	case 4: case 5: case 6: case 7:
		for (int i = 0; i < 3; ++i){
			std::stringstream ss;
			ss << "red_bird_" << i << ".png";
			bird_animation->addSpriteFrame(_frameCache->getSpriteFrameByName(ss.str()));
		}
		bird = Sprite::createWithSpriteFrameName("red_bird_0.png");
		break;
	case 8:
		bird_animation->addSpriteFrame(_frameCache->getSpriteFrameByName("spider.png"));
		bird = Sprite::createWithSpriteFrameName("spider.png");
		break;
	default:
		break;
	}
	bird_animation->setDelayPerUnit(birdFrameDuration);
	bird_action = Animate::create(bird_animation);
	bird_action->setTag(0);
	// Important
	// Animate bird_action is an autorelease object
	// which means it's released when one frame ends render
	// so the retain method keeps it unreleased
	bird_action->retain();

	// add physics body
	Size birdSize = bird->getContentSize();
//	auto physicsBody = PhysicsBody::createBox(birdSize, world_material);
	auto physicsBody = PhysicsBody::createCircle(birdSize.height/2, world_material);
	physicsBody->setRotationEnable(false);

	physicsBody->setCategoryBitmask(bird_category_mask);
	physicsBody->setContactTestBitmask(pipe_category_mask | ground_category_mask | world_category_mask);
	physicsBody->setCollisionBitmask(pipe_category_mask | ground_category_mask | world_category_mask);
	physicsBody->setGroup(COLLISION_GROUP);

	bird->setPhysicsBody(physicsBody);
	bird->setPosition(50.0f, visible_size.height/2 + ground->getContentSize().height / 2);
	bird->runAction(RepeatForever::create(bird_action));
	addChild(bird);
}

void HelloWorld::addGround()
{
	ground = Sprite::create("ground.png");
	Size groundSize = ground->getContentSize();
	auto physicsBody = PhysicsBody::createBox(groundSize, world_material);
	physicsBody->setDynamic(false);
	physicsBody->setGravityEnable(false);
	physicsBody->setRotationEnable(false);

	physicsBody->setCategoryBitmask(ground_category_mask);
	physicsBody->setContactTestBitmask(bird_category_mask);
	physicsBody->setCollisionBitmask(bird_category_mask);
	physicsBody->setGroup(NO_COLLISION_GROUP);
	ground->setPhysicsBody(physicsBody);
	ground->setPosition(groundSize.width/2, groundSize.height/2);

	auto action1 = MoveBy::create(1.0f / boost_factor, Vec2(-groundStripeWidth * 2, 0));
	auto action2 = MoveBy::create(0.0f, Vec2(groundStripeWidth * 2, 0));
	ground_action = RepeatForever::create(Sequence::create(action1, action2, nullptr));
	ground_action->setTag(1);
	ground_action->retain();
	addChild(ground);
}

void HelloWorld::addScoreLabel()
{
//	score_label = Label::createWithCharMap("fonts/charmap.plist");
	score_label = Label::createWithTTF("SCORE:0", "fonts/Marker Felt.ttf", 18);
	score_label->setAlignment(TextHAlignment::LEFT);
	score_label->setTextColor(Color4B::BLUE);
	score_label->setPosition(score_label->getContentSize().width / 2, score_label->getContentSize().height / 2);
	addChild(score_label);
}

void HelloWorld::addPipe(float delta)
{
	Vect pipeVelocity(pipe_velocity_x * boost_factor, 0.0f);
	bottom_pipe = Sprite::create("bottom_pipe.png");
	top_pipe = Sprite::create("top_pipe.png");
	Size bottomSize = bottom_pipe->getContentSize();
	Size topSize = top_pipe->getContentSize();
	auto bottomBody = PhysicsBody::createBox(bottomSize, world_material);
	auto topBody = PhysicsBody::createBox(topSize, world_material);

	bottomBody->setGravityEnable(false);
	topBody->setGravityEnable(false);
	bottomBody->setRotationEnable(false);
	topBody->setRotationEnable(false);

	bottomBody->setCategoryBitmask(pipe_category_mask);
	bottomBody->setContactTestBitmask(bird_category_mask);
	bottomBody->setCollisionBitmask(bird_category_mask);
	topBody->setCategoryBitmask(pipe_category_mask);
	topBody->setContactTestBitmask(bird_category_mask);
	topBody->setCollisionBitmask(bird_category_mask);

	bottomBody->setGroup(NO_COLLISION_GROUP);
	topBody->setGroup(NO_COLLISION_GROUP);

	bottomBody->setVelocity(pipeVelocity);
	topBody->setVelocity(pipeVelocity);

	bottom_pipe->setPhysicsBody(bottomBody);
	top_pipe->setPhysicsBody(topBody);

	// set space height for bird to fly through
	// half of height ranges from 40.0f to 60.0f
	float minSpaceHalfHeightForBird = 40.0f;
	float halfSpaceHeightForBird = minSpaceHalfHeightForBird + rand_0_1() * 20.0f;
	float groundHeight = ground->getContentSize().height;
	float centerHeight;
	if (cheat_mode){
		gamut = rand() % 26;
		centerHeight = (float)gamut * 80.0f/26.0f + visible_size.height / 2 + groundHeight / 2;
	}else{
		gamut = 26;
		centerHeight = rand_0_1() * 80.0f + visible_size.height / 2 + groundHeight / 2;
	}
	bottom_pipe->setPosition(visible_size.width + bottomSize.width / 2, centerHeight - halfSpaceHeightForBird - bottomSize.height / 2);
	top_pipe->setPosition(visible_size.width + topSize.width / 2, centerHeight + halfSpaceHeightForBird + topSize.height / 2);

	pipe_created.pushBack(bottom_pipe);
	pipe_created.pushBack(top_pipe);
	addChild(bottom_pipe, -2);
	addChild(top_pipe, -2);
}

void HelloWorld::addTouchEvent()
{
	touch_event = EventListenerTouchOneByOne::create();
	touch_event->onTouchBegan = [&](Touch *t, Event *e){
		getBird()->getPhysicsBody()->setVelocity(Vec2(0.0f, 240.0f));
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/sound_wing.ogg");
		return false;
	};
//	touch_event->onTouchEnded = [&](Touch *t, Event *e){
//		this->getBird()->getPhysicsBody()->setVelocity(Vec2::ZERO);
//	};
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touch_event, this);
}

void HelloWorld::addKeyEvent()
{
	key_event = EventListenerKeyboard::create();
	key_event->onKeyReleased = [&](EventKeyboard::KeyCode keycode, Event *e){
		if (keycode == EventKeyboard::KeyCode::KEY_ESCAPE){
			Director::getInstance()->end();
		}
#if (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
		else if (keycode == EventKeyboard::KeyCode::KEY_S){
			if (!screen_shot){
//				int err = -1;
//				GIFfile = EGifOpenFileName((FileUtils::getInstance()->getWritablePath()+"output.gif").c_str(), false, &err);
//				log("open err = %d", err);
//				EGifPutScreenDesc(GIFfile, visible_size.width, visible_size.height, 256, 0, NULL);

//				exportPNG(1.0f);
				schedule(schedule_selector(HelloWorld::exportPNG), 1.0f/20);
			}else{
//				int err;
//				EGifCloseFile(GIFfile, &err);
				unschedule(schedule_selector(HelloWorld::exportPNG));
			}
			screen_shot = !screen_shot;
		}
#endif
	};
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(key_event, this);
}

void HelloWorld::addContactEvent()
{
	contact_event = EventListenerPhysicsContact::create();
	contact_event->onContactBegin = CC_CALLBACK_1(HelloWorld::onContactBegin, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contact_event, this);
}

void HelloWorld::addMenu()
{
	float menuImgHeight = 48.0f;
	float intervalHeight = 6.0f;
	MenuItemImage *startMenu = MenuItemImage::create("start_normal.png", "start_selected.png", CC_CALLBACK_1(HelloWorld::menuStartCallback, this));
	MenuItemImage *exitMenu = MenuItemImage::create("exit_normal.png", "exit_selected.png", CC_CALLBACK_1(HelloWorld::menuExitCallback, this));
	MenuItemImage *cheatMenu = MenuItemImage::create("cheat_normal.png", "cheat_selected.png", CC_CALLBACK_1(HelloWorld::menuCheatCallback, this));
	MenuItemImage *resetMenu = MenuItemImage::create("reset_normal.png", "reset_selected.png", CC_CALLBACK_1(HelloWorld::menuResetCallback, this));
	startMenu->setPosition(visible_size.width / 2, visible_size.height / 2 + menuImgHeight / 2 + intervalHeight / 2);
	exitMenu->setPosition(visible_size.width / 2, visible_size.height / 2 - menuImgHeight / 2 - intervalHeight / 2);
	cheatMenu->setPosition(visible_size.width - cheatMenu->getContentSize().width / 2, cheatMenu->getContentSize().height / 2);
	resetMenu->setPosition(visible_size.width - resetMenu->getContentSize().width / 2, resetMenu->getContentSize().height * 2);
	menu = Menu::create(startMenu, exitMenu, cheatMenu, resetMenu, nullptr);
	menu->setPosition(Vec2::ZERO);
	addChild(menu);
}

void HelloWorld::addFireworks(const Vec2 &pos, bool playSound)
{
	auto firework1 = ParticleExplosion::createWithTotalParticles(500);
	firework1->setLife(1.2f);
	firework1->setSpeed(50);
	firework1->setSpeedVar(50);
	firework1->setStartColor(Color4F(1.0f, 1.0f, 1.0f, 0.5f));
	firework1->setStartColorVar(Color4F(0.5f, 0.5f, 0.5f, 0.25f));
	firework1->setScale(0.5f);
	firework1->setPositionType(ParticleExplosion::PositionType::GROUPED);
	firework1->runAction(MoveBy::create(1.2f, Vec2(0.0f, rand_0_1()*200.0f +100.0f)));
	firework1->setPosition(pos);
	addChild(firework1);
	if (playSound)
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/firework.mp3");
}

void HelloWorld::menuStartCallback(Ref *pSender)
{
	menu->setVisible(false);
    scheduleOnce(schedule_selector(HelloWorld::startGame), 1.8f);
}

void HelloWorld::menuExitCallback(Ref *pSender)
{
	Director::getInstance()->end();
}

void HelloWorld::menuCheatCallback(Ref *pSender)
{
	cheat_mode = true;
	boost_factor = 12.0f;
	bird->getPhysicsBody()->setContactTestBitmask(~(pipe_category_mask | ground_category_mask) | world_category_mask);
	bird->getPhysicsBody()->setCollisionBitmask(ground_category_mask | world_category_mask);
}

void HelloWorld::menuResetCallback(Ref *pSender)
{
	if (!UserDefault::getInstance()->getBoolForKey(key_inited)) {
		UserDefault::getInstance()->setBoolForKey(key_inited, true);
	}
	UserDefault::getInstance()->setIntegerForKey(key_first, 0);
	UserDefault::getInstance()->setIntegerForKey(key_second, 0);
	UserDefault::getInstance()->setIntegerForKey(key_third, 0);
}

void HelloWorld::updateScoreLabel(const int score)
{
	std::stringstream ss;
	ss << "SCORE:" << score;
	score_label->setString(ss.str());
	score_label->setPosition(score_label->getContentSize().width / 2, score_label->getContentSize().height / 2);
}

void HelloWorld::storeScore(const int new_score)
{
	// read scores from db
	// scores[3] is set for new_score for sorting
	if (!UserDefault::getInstance()->getBoolForKey(key_inited)) {
		UserDefault::getInstance()->setBoolForKey(key_inited, true);
		UserDefault::getInstance()->setIntegerForKey(key_first, 0);
		UserDefault::getInstance()->setIntegerForKey(key_second, 0);
		UserDefault::getInstance()->setIntegerForKey(key_third, 0);
//		log("file not exists");
	}
	score_ranking_list[0] = UserDefault::getInstance()->getIntegerForKey(key_first, 0);
	score_ranking_list[1] = UserDefault::getInstance()->getIntegerForKey(key_second, 0);
	score_ranking_list[2] = UserDefault::getInstance()->getIntegerForKey(key_third, 0);
	score_ranking_list[3] = new_score;
	std::sort(score_ranking_list.begin(), score_ranking_list.end(), std::greater<int>());
//	log("%d, %d, %d, %d", score_ranking_list[0], score_ranking_list[1], score_ranking_list[2], score_ranking_list[3]);
	UserDefault::getInstance()->setIntegerForKey(key_first, score_ranking_list[0]);
	UserDefault::getInstance()->setIntegerForKey(key_second, score_ranking_list[1]);
	UserDefault::getInstance()->setIntegerForKey(key_third, score_ranking_list[2]);
//	UserDefault::getInstance()->flush();

}

void HelloWorld::dispFailedLabel()
{
	failed_label = Label::createWithTTF("游戏失败","fonts/minijianlishu.ttf", 36);
	failed_label->setColor(Color3B(0xAD, 0xFF,0x2F));
	failed_label->setPosition(Vec2(visible_size.width / 2, visible_size.height / 2));
	addChild(failed_label);
}

void HelloWorld::dispRankingList()
{
	std::stringstream ss;
	ss << "第一名    " << std::setw(10) << score_ranking_list[0] << "\n";
	ss << "第二名    " << std::setw(10) << score_ranking_list[1] << "\n";
	ss << "第三名    " << std::setw(10) << score_ranking_list[2] << "\n";
	ranking_label = Label::createWithTTF(ss.str(), "fonts/minijianlishu.ttf", 20);
	ranking_label->setColor(Color3B::ORANGE);
	// set the pos left align with the failed label
	Vec2 pos(failed_label->getPositionX() - failed_label->getContentSize().width / 2 + ranking_label->getContentSize().width / 2,
			failed_label->getPositionY() - failed_label->getContentSize().height / 2 - ranking_label->getContentSize().height / 2);
	ranking_label->setPosition(pos);
	addChild(ranking_label);
}

void HelloWorld::resetScene(float delta)
{
	Director::getInstance()->replaceScene(HelloWorld::createScene());
}

bool HelloWorld::onContactBegin(const PhysicsContact& contact)
{
	if (contact.getShapeA()->getBody() == bird->getPhysicsBody() ||
			contact.getShapeB()->getBody() == bird->getPhysicsBody()){
		CocosDenshion::SimpleAudioEngine::getInstance()->stopAllEffects();
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/sound_hit.ogg");
		stopGame();
		storeScore(score);
		dispFailedLabel();
		dispRankingList();
		scheduleOnce(schedule_selector(HelloWorld::resetScene), 2.0f);
	}
	return true;
}

void HelloWorld::exportPNG(float delta)
{
	static unsigned int screenShotCount = 0;
	GLint pView[4];
	glGetIntegerv(GL_VIEWPORT, pView);
	GLint x = pView[0];
	GLint y = pView[1];
	GLint w = pView[2];
	GLint h = pView[3];
	GLsizei numComponent = 4;
	GLsizei bufferSize = w * h * sizeof(GLbyte) * numComponent;

	unsigned char *data = new unsigned char[bufferSize];
	memset(data, 0, bufferSize);
	unsigned char *flipped_data = new unsigned char[bufferSize];

	glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);

	for (int row = 0; row < h; ++row){
		memcpy(flipped_data + numComponent * w * (h - 1 - row),
				data + numComponent * w * row,
				numComponent * w);
	}
	delete data;

	Image img;
	img.initWithRawData(flipped_data, bufferSize, w, h, 8);
	std::stringstream ss;
	ss << FileUtils::getInstance()->getWritablePath() << "screenshot-" << std::setfill('0') << std::setw(8) << screenShotCount++ << ".png";
	img.saveToFile(ss.str().c_str(), false);


//	parseRawData(GIFfile, flipped_data,	visible_size.width, visible_size.height, GL_RGBA);

	delete flipped_data;
}


