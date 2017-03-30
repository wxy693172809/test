//
//  MainScene.cpp
//  OneBot
//
//  Created by fenglei on 16/2/15.
//
//

#include "MainScene.h"
//#include "RotateMenu.h"
#include "cocos-ext.h"
#include "GameSceneManager.h"
#include "MIBaseScene.hpp"
#include "LanguageConfigScene.hpp"
#include "ConfigDataManager.hpp"
#include "LoginService.hpp"
#include "GamesHelpScene.hpp"
#include "NoviceInstructionScene.hpp"
#include "ClipLayer.hpp"
#include "ProgramHallScene.hpp"
USING_NS_CC_EXT;

USING_NS_CC;
#define LINE_SPACE 40
#define COIN_HEIGHT 440
#define COIN_WIDTH 300
#define COIN_GAP 23
#define COIN_COUNT 6

Scene* MainScene::createScene()
{
    auto scene = Scene::create();
    auto layer = MainScene::create();
    scene->addChild(layer);
    auto mibaselayer = MIBaseLayer::create();
    mibaselayer->setName("mibaselayer");
    mibaselayer->initTitle(LangString("Robot disconnect"));
    scene->addChild(mibaselayer);
    if (UserDefault::getInstance()->getBoolForKey("instruction", false)==false) {
        auto layerN=NoviceInstructionScene::create();
        scene->addChild(layerN,1);
    }
    return scene;
}
MainScene::~MainScene()
{
    
}
bool MainScene::init()
{
    if ( !Layer::init() )
    {
        return false;
    }
    this->addChild(ClipLayer::create(),1024);
    ConfigDataManager::getInstance()->setPercentscene(0);
    isAndroidBack=false;
    initUI();
    disconnectedCount = 0;
    return true;
}
bool MainScene ::initUI()
{
    Size visibleSize = Director::getInstance() -> getVisibleSize();
    pageView = XKPageView::create(Size(visibleSize.width,COIN_WIDTH), this);
    pageView -> setDirection(cocos2d::extension::ScrollView::Direction::HORIZONTAL);
    pageView -> setPosition(Point((visibleSize.width - COIN_WIDTH+COIN_GAP*2.6-150) * 0.488, (visibleSize.height - COIN_HEIGHT) * 0.5)-Vec2(5,0));
    pageView -> setClippingToBounds(false);
    this -> addChild(pageView);
    addPages();
    pageView->addpageshow();
    //准备开始
    int realIndex=  GameSceneMgr->getTmpcurScene();
    pageView->moveToIndex(realIndex);
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    auto listenerKeyboard = EventListenerKeyboard::create();
    listenerKeyboard->onKeyPressed = CC_CALLBACK_2(MainScene::onKeyPressed, this);
    listenerKeyboard->onKeyReleased = CC_CALLBACK_2(MainScene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listenerKeyboard, this);
    //背景
    background=Sprite::create(SkinRes("bg_mainscne_rockerscene.png"));
    background->setScale(1.4);
    background->setPosition(VisibleRect::center());
    background->setAnchorPoint(Vec2(0.5, 0.5));
    this->addChild(background,-100);
    
    bluetooth = cocos2d::ui::Button::create(SkinRes("btn_disconnect_1.png"),SkinRes("btn_disconnect_2.png"));
    bluetooth->setScale9Enabled(true);
    bluetooth->setPosition(VisibleRect::leftTop()+Vec2(button_point_offsetXY,-button_point_offsetXY));
    bluetooth->setTag(123);
    bluetooth->addTouchEventListener(CC_CALLBACK_2(MainScene::goBluetoothLayer, this));
    this->addChild(bluetooth);
    
    //设置按钮
    log("%s",SkinRes("button_set.png").c_str());
    auto configBtn = cocos2d::ui::Button::create(SkinRes("button_set.png"),SkinRes("button_set_002.png"));
    configBtn->setScale9Enabled(true);
    configBtn->setPosition(VisibleRect::rightTop()+Vec2(-button_point_offsetXY,-button_point_offsetXY));
    configBtn->addTouchEventListener(CC_CALLBACK_2(MainScene::configBtnClick, this));
    this->addChild(configBtn, -1);
    
    Button* helpBtn = Button::create(SkinRes("button_help_normal.png"),SkinRes("button_help_selected.png"));
    this->addChild(helpBtn);
    helpBtn->setPosition(VisibleRect::rightTop()+Vec2(-146,-button_point_offsetXY));
    helpBtn->addClickEventListener([=](Ref*) {
        AudioCache::getInstance()->playEffect(clickbtnmc);
        GameSceneMgr->changeCurScene(eStateGamesHelpScene);
    });
    connectlabel = Label::createWithTTF(LangString("Robot connected"), "fonts/LanTing_Bold.ttf", 22);
    connectlabel->setColor(cocos2d::Color3B(255,255,255));
    connectlabel->setPosition(VisibleRect::leftTop()+Vec2(480,-title_point_offsetY));
    connectlabel->setVisible(false);
    this->addChild(connectlabel,3);
    
    disconnectlabel = Label::createWithTTF(LangString("Robot disconnect"), "fonts/LanTing_Bold.ttf", 22);
    disconnectlabel->setColor(cocos2d::Color3B(255,255,255));
    disconnectlabel->setPosition(VisibleRect::leftTop()+Vec2(480,-title_point_offsetY));
    disconnectlabel->setVisible(false);
    this->addChild(disconnectlabel,3);
    
    return true;
}
void MainScene::menuItemRockerCallback(cocos2d::Ref* pSender)
{
    GameSceneMgr->changeCurScene(eStateRockerScene);
    GameSceneMgr->setTmpcurScene(0);
}
void MainScene::menuItemDanceCallback(cocos2d::Ref* pSender)
{
    GameSceneMgr->changeCurScene(eStateDanceScene);
    GameSceneMgr->setTmpcurScene(1);
}
void MainScene::menuItemRobotInfoCallback(cocos2d::Ref* pSender)
{
    GameSceneMgr->changeCurScene(eStateRobotInfoScene );
    GameSceneMgr->setTmpcurScene(2);
}
void MainScene:: menuItemTrajectoryCallback(cocos2d::Ref* pSender)
{
    GameSceneMgr->changeCurScene(eStateTrajectoryGameScene);
    GameSceneMgr->setTmpcurScene(3);
}
void MainScene:: menuItemProgrammingCallback(cocos2d::Ref* pSender)
{
    GameSceneMgr->changeCurScene(eStateProgrammingGameScene);
    GameSceneMgr->setTmpcurScene(4);
}
//蓝牙按钮
void MainScene::goBluetoothLayer(Ref *sender,cocos2d::ui::Widget::TouchEventType e)
{
    if (cocos2d::ui::Widget::TouchEventType::ENDED == e)
    {
#if 1
        AudioCache::getInstance()->playEffect(clickbtnmc);
        if(BlueService::getInstance()->getIsConnected()==true)
        {
            if(BlueService::getInstance()->getIsShakeHands()==true)   // 握手成功
            {
                this->runAction(Sequence::create(DelayTime::create(0.01),CallFunc::create([=](){
                    AudioCache::getInstance()->playEffect(warnmc);
                    auto call=CallFunc::create([=](){
                        BlueService::getInstance()->setDisconnectState(eStateHanddleDisconnect);  // 设置为手动断开
                        threeDisconnect();
                    });
                    call->retain();
                    auto  popscene=PopMenu::create(LangString("message"),LangString("Disconnect Bluetooth") ,LangString("OK"),LangString("Cancel"),2,call,nullptr);
                    this->addChild(popscene,100);
                }), NULL));
            }
            else    // 握手失败,物理成功
            {
                BlueService::getInstance()->setDisconnectState(eStateHanddleDisconnect);  // 设置为手动断开
                threeDisconnect();
            }
        }
        else   // 未连接的情况
        {
            this->scheduleOnce(schedule_selector(MainScene::changeScene), 0.2);
        }
#endif
    }
}
void MainScene:: threeDisconnect()
{
    BlueService::getInstance()->disconnect();
    this->schedule(schedule_selector(MainScene::updateShakeHand), 0.2);
}
void MainScene:: updateShakeHand(float dt)
{
    if ((disconnectedCount >= 3) ||
        (BlueService::getInstance()->getIsConnected() == false))
    {
        this->unschedule(schedule_selector(MainScene::updateShakeHand));
        BlueService::getInstance()->setIsConnected(false);
        BlueService::getInstance()->setIsShakeHands(false);
        GameSceneMgr->changeCurScene(eStatebluetoothlistScene);
        disconnectedCount = 0;
    }
    else
    {
        BlueService::getInstance()->disconnect();
        ++disconnectedCount;
    }
}
void MainScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
    
}
void MainScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
    if (isAndroidBack==true) {
        return;
    }
    if (ConfigDataManager::getInstance()->getIsCanelExit()==false)
    {
        if (this->getChildByName("popscene"))
        {
            return ;
        }
        this->runAction(Sequence::create(DelayTime::create(0.1),CallFunc::create([=](){
            
            auto call=CallFunc::create([=](){
                if (this->getChildByName("popscene")) {
                    this->removeChildByName("popscene");
                }
                Director::getInstance()->end();
            });
            call->retain();
            CultrureService* cs=CultrureService::getInstance();
            auto  popscene=PopMenu::create(cs->getString("message"),cs->getString("leave") ,cs->getString("OK"),cs->getString("Cancel"),2,call,nullptr);
            popscene->setName("popscene");
            this->addChild(popscene,100);
        }), NULL));
    }else
    {
        ConfigDataManager::getInstance()->setIsCanelExit(false);
    }
}
void MainScene::onEnter()
{
    Layer::onEnter();
#if 1
    if (BlueService::getInstance()->getIsShakeHands() == true) {
        MIBaseLayer *layer = (MIBaseLayer*)this->getParent()->getChildByName("mibaselayer");
        layer->_titlelaber->setString(LangString("Robot connected"));
        bluetooth->loadTextures(SkinRes("button_bluetooth.png"), SkinRes("button_bluetooth_002.png"));
    }
#endif
    LoginService::getInstance()->setCurmainScene(this);
}
void MainScene:: onEnterTransitionDidFinish()
{
    Layer::onEnterTransitionDidFinish();
    /**监听上电自检错误 **/
    this->result = EventListenerCustom::create(eventErrorMessage, CC_CALLBACK_1(MainScene::Application_UpdateResultCallFunc, this));
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(result, this);
    this->disconnect = EventListenerCustom::create(eventDeviceDisconnected, CC_CALLBACK_1(MainScene::Application_DisconnectedCallFunc, this));
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(disconnect, this);
    
    if (ConfigDataManager::getInstance()->getFirmwarePopMenu()==false&&BlueService::getInstance()->getIsShakeHands()==true)
    {
        this->downloadBin();
    }
    //监听弹框从一般卡牌切换到模板卡牌
    if (UserDefault::getInstance()->getBoolForKey("mainPopOffline")==false) {
        UserDefault::getInstance()->setBoolForKey("mainPopOffline", true);
        this->runAction(Sequence::create(DelayTime::create(1),CallFunc::create([=](){
            this->exitProgrammingPop(-1, eStateNoScene);
        }),NULL));
    }
    BlueService::getInstance()->setDisconnectState(eStateDisconnect);  // 设置为自动断开
    // 切换到运行模式.
    BlueService::getInstance()->SwitchState(0x01);
    if ( BlueService::getInstance()->getCurSceneState() == eStatebluetoothlistScene)
    {
        // 提示 电机请插入A或B口
        GameSceneMgr->getCurRunBaseScene()->toInsertMouth();
        BlueService::getInstance()->setCurSceneState(eStateStartScene);
    }
    if (UserDefault::getInstance()->getBoolForKey("updateApplication")&&UserDefault::getInstance()->getBoolForKey("instruction")==true) {
        UserDefault::getInstance()->setBoolForKey("updateApplication", false);
        NetRequest::getInstance()->updateApplication(); // 监测 APP 是否更新的接口
    }
}
void MainScene::onExit()
{
    Layer::onExit();
    Director::getInstance()->getEventDispatcher()->removeEventListener(this->disconnect);
    Director::getInstance()->getEventDispatcher()->removeEventListener(this->result);
    this->unschedule(schedule_selector(MainScene::myupdate));
    LoginService::getInstance()->setCurmainScene(nullptr);
}
void MainScene::configBtnClick(Ref *sender,cocos2d::ui::Widget::TouchEventType e)
{
    if (cocos2d::ui::Widget::TouchEventType::ENDED == e)
    {
        AudioCache::getInstance()->playEffect(clickbtnmc);
        
        this->runAction(Sequence::create(DelayTime::create(0.3),CallFunc::create([=](){
            GameSceneMgr->changeCurScene(eStateConfigScene);
        }), NULL));
    }
}
Sprite* MainScene:: addmenuitem( string normal,string selected,string number, Size coinSize,string icon,string iconSelect,string name)
{
    auto spriteM=Sprite::create(SkinRes("ui_mainscene_whiteborderSelect.png"));
    spriteM->setVisible(false);
    spriteM->setName("spriteM");
    
    auto sprite=Sprite::create(SkinRes("ui_mainscene_whiteborder.png"));
    sprite ->setPosition(Vec2(coinSize.width * 0.54, coinSize.height * 0.5));
    sprite->setAnchorPoint(Vec2(0.5,0.5));
    spriteM->setPosition(sprite->getContentSize()/2);
    sprite->addChild(spriteM);
    
    string num=number;
    auto sp=Sprite::create(num);
    sp->setAnchorPoint(Vec2(0,1));
    sp->setPosition(Vec2(60,coinSize.height-79));
    sp->setOpacity(255*0.4);
    sp->setName("num");
    sprite->addChild(sp);
    std::string str = name;
    
    Label *label = Label::createWithSystemFont(str, "fonts/LanTing_Bold.ttf", 22);
    label -> setColor(Color3B(255,255,255));
    label->setOpacity(255*0.4);
    label->setName("label");
    
    auto spritIcon=Sprite::create(iconSelect);
    spritIcon->setVisible(false);
    spritIcon->setName("spriteIcon");
    //    spritIcon->setScale(1.3, 1.3);
    
    string ic=icon;
    auto icsp=Sprite::create(ic);
    icsp->setAnchorPoint(Vec2(0.5,0.5));
    icsp->setName("ICSP");
    //    icsp->setScale(1.3, 1.3);
    icsp->setPosition(Vec2(coinSize.width*0.5,coinSize.height*0.6)-Vec2(0,7));
    //    icsp->setOpacity(255*0.4);
    sprite->addChild(icsp);
    spritIcon->setPosition(icsp->getPosition());
    sprite->addChild(spritIcon);
    
    Size size = sprite -> getContentSize();
    label -> setPosition(size.width * 0.5, size.height * 0.5-52);
    sprite -> addChild(label);
    
    pageView -> addPage(sprite);
    return  sprite;
}
void MainScene::addPages()
{
    Size coinSize = Sprite::create(SkinRes("ui_mainscene_whiteborder.png")) -> getContentSize();
    this->addmenuitem(SkinRes("ui_mainscene_whiteborder.png"),SkinRes("ui_mainscene_whiteborderSelect.png"),SkinRes("ui_mainscene_grayone.png"),coinSize,SkinRes("ui_mainscene_graydrawline.png"),SkinRes("ui_mainscene_graydrawline_selected.png"),LangString("pathmodel"));
    
    this->addmenuitem(SkinRes("ui_mainscene_whiteborder.png"),SkinRes("ui_mainscene_whiteborderSelect.png"),SkinRes("ui_mainscene_graytwo.png"),coinSize,SkinRes("icon_mainscene_grayhandle.png"),SkinRes("icon_mainscene_grayhandle_selected.png"),LangString("rockermodel"));
    this->addmenuitem(SkinRes("ui_mainscene_whiteborder.png"),SkinRes("ui_mainscene_whiteborderSelect.png"),SkinRes("ui_mainscene_graythree.png"),coinSize,SkinRes("ui_mainscene_grayfourcircle.png"),SkinRes("ui_mainscene_grayfourcircle_selected.png"),LangString("programmodel"));
 this->addmenuitem(SkinRes("ui_mainscene_whiteborder.png"),SkinRes("ui_mainscene_whiteborderSelect.png"),SkinRes("ui_mainscene_grayfour.png"),coinSize,SkinRes("icon_mainscene_grayspeak.png"),SkinRes("icon_mainscene_yellowspeak.png"),LangString("voicemodel"));
}
Size MainScene::sizeForPerPage()
{
    return Size(COIN_WIDTH + COIN_GAP, COIN_HEIGHT);
}
void MainScene::pageViewDidScroll(XKPageView *pageView)
{
    
}
void MainScene:: exitProgrammingPop(int num,eGameState scene)
{
    if (BlueService::getInstance()->getIsShakeHands()==false) {
        //蓝牙未连接不需要判断弹框
        if (num==-1) {
            return;
        }
        GameSceneMgr->changeCurScene(scene);
        GameSceneMgr->setTmpcurScene(num);
        return;
    }
    // 如果是非脱机false的状态 进入其他模式的时候不弹框提示。num值为-1 就不进行界面跳转
    if (ConfigDataManager::getInstance()->getOffline()==false)
    {
        if (num==-1) {
            return;
        }
        GameSceneMgr->changeCurScene(scene);
        GameSceneMgr->setTmpcurScene(num);
    }else
    {
        //如果是脱机true的话，进入其他模式下,如果编程按钮是三角模板卡牌的话不弹框，如果是竖杠的一般卡牌的话弹框
        if (ConfigDataManager::getInstance()->getItemToggle()==1)
        {
            this->runAction(Sequence::create(DelayTime::create(0.1),CallFunc::create([=](){
                AudioCache::getInstance()->playEffect(warnmc);
                //确定按钮
                auto call1=CallFunc::create([=](){
                    // 确定的话编程就变三角,切换到模板卡牌
                    BlueService::getInstance()->SwitchState(0x0b);
                    ConfigDataManager::getInstance()->setItemToggle(0);
                    if (num!=-1) {
                        //跳转到摇杆
                        GameSceneMgr->changeCurScene(scene);
                        GameSceneMgr->setTmpcurScene(num);
                    }
                });
                call1->retain();
                auto  popscene=PopMenu::create("", LangString("stop this task"),LangString("OK"),LangString("Cancel"),2,call1,nullptr);
                auto text=(Text *)popscene->getChildByTag(1002)->getChildByTag(1001);
                text->setTextHorizontalAlignment(cocos2d::TextHAlignment::LEFT);
                this->addChild(popscene,100);
                popscene->setSwallowsTouches(true);
                
            }), NULL));
        }
        else
        {
            if (num==-1) {
                return;
            }
            //脱机模板卡牌，直接进行界面跳转
            GameSceneMgr->changeCurScene(scene);
            GameSceneMgr->setTmpcurScene(num);
        }
    }
}
void MainScene::pageClicked(int index)
{
    for (int i=0; i<pageView->getpageCount(); i++) {
        if (i==index) {
            Node* node=pageView->getPageAtIndex(index);
            node->setScale(0.9);
        }
    }
    if(index==0)
    {
        this->exitProgrammingPop(0,eStateTrajectoryGameScene);
//        if (BlueService::getInstance()->getIsShakeHands()==false)
//        {
//            //握手失败判断是否打开蓝牙，弹出tip框
//            this->runAction(Sequence::create(DelayTime::create(0.1),CallFunc::create([=](){
//                AudioCache::getInstance()->playEffect(warnmc);
//                auto call=CallFunc::create([=](){
//                    GameSceneMgr->changeCurScene(eStatebluetoothlistScene);
//                    
//                });
//                call->retain();
//                auto  popscene=PopMenu::create("", LangString("Bluetooth not connected"),LangString("Connect"),LangString("Cancel"),2,call,nullptr);
//                this->addChild(popscene,100);
//                popscene->setSwallowsTouches(true);
//            }), NULL));
//            
//            return;
//        }
//        this->exitProgrammingPop(0,eStateSensorSetUpLayer);

    }
    else if (index==1)
    {
        this->exitProgrammingPop(1, eStateRockerScene);
    }
    else if (index==2)
    {
        GameSceneMgr->changeCurScene(eStateStartProgrammingScene);
        GameSceneMgr->setTmpcurScene(2);
    }
    else if (index==3)
    {
        GameSceneMgr->changeCurScene(eStateProgramHallScene);
//        GameSceneMgr->changeCurScene(eStateVoiceControlScene);
        GameSceneMgr->setTmpcurScene(3);
    }
    else if (index==4)
    {
        GameSceneMgr->changeCurScene( eStateDanceScene);
        GameSceneMgr->setTmpcurScene(4);
    }
}
void MainScene::Moved(float x)
{
    float a=(x + sizeForPerPage().width)*0.3 ;
    background->setPosition(Vec2(VisibleRect::center().x+a,VisibleRect::center().y));
}
void MainScene:: Application_UpdateResultCallFunc(cocos2d::EventCustom *event)
{
    BYTE *resultMess=new BYTE[20];
    for (int i=0; i<20; i++)
    {
        resultMess[i]=((BYTE *)event->getUserData())[i];
    }
    if (resultMess[4]==0x04&&resultMess[7]==0x02)
    {
        Director::getInstance()->getEventDispatcher()->removeEventListener(this->result);
        /** 更新失败，弹框提示 **/
        this->runAction(Sequence::create(DelayTime::create(0.1),CallFunc::create([=](){
            auto call=CallFunc::create([=](){
                /***********  在内部进行模式切换  **********/
                auto scene=FirmwareUpdateScene::createScene();
                Director::getInstance()->pushScene(scene);
            });
            call->retain();
            auto  popscene=PopMenu::create("","上电自检,更新失败",LangString("OK"),"",1,call,nullptr);
            this->addChild(popscene,100);
            popscene->setSwallowsTouches(true);
        }),NULL));
    }
    delete [] resultMess;
    resultMess=NULL;
}
void MainScene:: Application_DisconnectedCallFunc(EventCustom* event)
{
    // 断开监听
    MIBaseLayer *layer = (MIBaseLayer*)this->getParent()->getChildByName("mibaselayer");
    layer->_titlelaber->setString(LangString("Robot disconnect"));
    bluetooth->loadTextures(SkinRes("btn_disconnect_1.png"),SkinRes("btn_disconnect_2.png"));
}
void MainScene:: changeScene(float dt)
{
    GameSceneMgr->changeCurScene(eStatebluetoothlistScene);
}
void MainScene:: addprompt(std::string content,std::string url,Node* node,int tag)
{
    
    auto mask = LayerMask::create();
    node = Director::getInstance()->getRunningScene();
    node->addChild(mask,100);
    mask->retain();
    auto promptLayer = CSLoader::createNode(SkinRes("helpInterface/promptLayer.csb"));
    mask->addChild(promptLayer,100);
    auto imageLayer = promptLayer->getChildByName("Image_1");
    auto bigbtn=(Button *)imageLayer->getChildByName("Button_Big");
    auto downloadbtn = (Button*)imageLayer->getChildByName("Button_right");
    auto leftBtn = (Button*)imageLayer->getChildByName("Button_left");
    if (tag==1) {
        isAndroidBack=true;
        //弹推送框每次连接蓝牙只弹一次
        this->setKeypadEnabled(false);
        downloadbtn->setVisible(false);
        leftBtn->setVisible(false);
        downloadbtn->setEnabled(false);
        leftBtn->setEnabled(false);
        bigbtn->setTitleText(LangString("update"));
        bigbtn->setTitleFontSize(20);
        bigbtn->addClickEventListener([=](Ref*){
            ConfigDataManager::getInstance()->setFirmwarePopMenu(true);
            isAndroidBack=false;
            mask->removeFromParent();
            mask->release();
            //蓝牙意外断开容错
            if (BlueService::getInstance()->getIsShakeHands()==true)
            {
                BlueService::getInstance()->SwitchState(2);
                GameSceneMgr->changeCurScene(eStateFirmwareUpdateScene);
            }
        });
    }
    else if(tag==0)
    {
        bigbtn->setVisible(false);
        bigbtn->setEnabled(false);
        downloadbtn->setTitleText(LangString("update"));
        downloadbtn->setTitleFontSize(20);
        downloadbtn->addClickEventListener([=](Ref*){
            mask->removeFromParent();
            mask->release();
            LoginService::getInstance()->downloadVersion(url);
            
        });
        leftBtn->setTitleText(LangString("Download next"));
        leftBtn->setTitleFontSize(20);
        leftBtn->addClickEventListener([=](Ref*){
            mask->removeFromParent();
            mask->release();
        });
    }
    
    imageLayer->setPosition(VisibleRect::center()+Vec2(0, leftBtn->getContentSize().height/2));
    auto _title = (Text*)imageLayer->getChildByName("Text_update");
    _title->setString(LangString("Update Tips"));
    auto textContent = Text::create(content, "font/LanTing_Regular.ttf", 17);
    textContent->setColor(Color3B(0, 0, 0));
    textContent->setOpacity(0.65*255);
    textContent->ignoreContentAdaptWithSize(true);
    textContent->setTextAreaSize(Size(361, 0));
    textContent->setAnchorPoint(Vec2(0, 0));
    auto scrollView_1 =(cocos2d::ui::ScrollView *)imageLayer->getChildByName("ScrollView_1");
    if(textContent->getContentSize().height < scrollView_1->getContentSize().height)
    {
        textContent->setPosition(Vec2(0, (scrollView_1->getContentSize().height - textContent->getContentSize().height)/2));
    }
    else{
        textContent->setPosition(Vec2(0, 0));
    }
    scrollView_1->setScrollBarEnabled(false);
    scrollView_1->setInnerContainerSize(textContent->getContentSize());
    scrollView_1->addChild(textContent);
}

//清空下载容器避免内存泄漏
void MainScene:: clearDownloadVec()
{
    for (int i=0; i<this->downloaderNewVec.size(); i++) {
        cocos2d::network::Downloader* temp=this->downloaderNewVec.at(i);
        delete temp;
        temp=NULL;
    }
    this->downloaderNewVec.clear();
}
void MainScene:: myupdate(float dt)
{
    if (LoginService::getInstance()->getNetRequestState()==true)
    {
        //网络请求成功
        if (ConfigDataManager::getInstance()->getFirmVector().size()!=0)
        {
            int isdownsuccess=0;
            for (int i=0; i<ConfigDataManager::getInstance()->getFirmVector().size(); i++)
            {
                isdownsuccess=isdownsuccess+FileUtils::getInstance()->isFileExist(firmDirectorPath+ConfigDataManager::getInstance()->getFirmVector().at(i).CCfirmName);
            }
            if (isdownsuccess==ConfigDataManager::getInstance()->getFirmVector().size())
            {
                this->unschedule(schedule_selector(MainScene::myupdate));
                this->clearDownloadVec();
                //弹框提示跳转页面
                AudioCache::getInstance()->playEffect(warnmc);
                //进行xml回车换行
                std::string prompt=LangString("FirmVersion push");
                vector<string> vecStrings;
                string deli="&";
                BlueService::getInstance()->split(prompt, deli, &vecStrings);
                std::string str;
                for (int i=0; i<vecStrings.size(); i++) {
                    str=str+vecStrings.at(i)+"\n";
                }
                log("%s",str.c_str());
                this->addprompt(str,"",nullptr ,1);
            }
        }
    }
}

void MainScene::  downloadBin()
{
    this->schedule(schedule_selector(MainScene::myupdate), 0.1);
    this->clearDownloadVec();
    //下载文件到指定位置
    for (int i=0; i<ConfigDataManager::getInstance()->getFirmVector().size(); i++) {
        firmWareData temp=ConfigDataManager::getInstance()->getFirmVector().at(i);
        if (!FileUtils::getInstance()->isFileExist(firmDirectorPath+temp.CCfirmName.c_str())) {
            cocos2d::network::Downloader* downloaderNew = new cocos2d::network::Downloader();
            NetRequest::getInstance()->downFirmBin(temp.downAddress,temp.CCfirmName.c_str() ,firmDirectorPath,downloaderNew);
            downloaderNewVec.push_back(downloaderNew);
        }
    }
}
