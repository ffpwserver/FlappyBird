#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

class HelloWorld : public cocos2d::Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
    
    virtual void onEnter();
    
    virtual void update(float delta);

    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);

    cocos2d::Sprite* getBird(){return bird;}

private:
    void startGame(float delta);
    void stopGame();
    void initSoundEffect();
    void addBackground();
    void addBird();
    void addGround();
    void addScoreLabel();
    void addPipe(float delta);
    void addTouchEvent();
    void addKeyEvent();
    void addContactEvent();
    void addMenu();
    void addFireworks(const cocos2d::Vec2 &pos, bool playSound);
    void menuStartCallback(cocos2d::Ref *pSender);
    void menuExitCallback(cocos2d::Ref *pSender);
    void menuCheatCallback(cocos2d::Ref *pSender);
    void menuResetCallback(cocos2d::Ref *pSender);
    void updateScoreLabel(const int score);
    void storeScore(const int new_score);
    void dispFailedLabel();
    void dispRankingList();
    void resetScene(float delta);
    bool onContactBegin(const cocos2d::PhysicsContact& contact);
    void exportPNG(float delta);

private:
    int pipe_count;
    int score;
    unsigned int gamut;		// used to generate different sound
    bool game_started;
    bool night_time;
    bool cheat_mode;
    float pipe_duration;
    float pipe_velocity_x;
    float boost_factor;		// boost the game by the factor (faster or slower)
    std::array<int, 4> score_ranking_list;
    cocos2d::PhysicsMaterial world_material;
    cocos2d::Sprite *background;
    cocos2d::Sprite *bird;
    cocos2d::Sprite *ground;
    cocos2d::Sprite *bottom_pipe;
    cocos2d::Sprite *top_pipe;
    cocos2d::Animation *bird_animation;
    cocos2d::Animate *bird_action;
    cocos2d::RepeatForever *ground_action;
    cocos2d::Size visible_size;
    cocos2d::Vec2 origin;
    cocos2d::Menu *menu;
    cocos2d::Label *score_label;
    cocos2d::Label *failed_label;
    cocos2d::Label *ranking_label;
    cocos2d::EventListenerTouchOneByOne *touch_event;
    cocos2d::EventListenerKeyboard *key_event;
    cocos2d::EventListenerPhysicsContact *contact_event;
    cocos2d::Vector<cocos2d::Sprite *> pipe_created;
    cocos2d::Vector<cocos2d::Sprite *> pipe_to_remove;
};

#endif // __HELLOWORLD_SCENE_H__
