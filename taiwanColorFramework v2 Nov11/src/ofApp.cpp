#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    #ifdef TARGET_OPENGLES
        shader.load("shadersES2/shader");
    #else
        if(ofIsGLProgrammableRenderer()){
            shader.load("shadersGL3/shader");
        }else{
            shader.load("shadersGL2/shader");
        }
    #endif


    ofLog()<<"screen resolution: "<<ofGetWidth()<<" x "<<ofGetHeight();
    screenWidth = ofGetWidth();
    screenHeight = ofGetHeight();
    state = 0;

    serial.listDevices();
  	vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    int baud = 9600;
  	serial.setup(1, baud); //open the first device
    detectedColor = 1;

    ofBackground(0, 0, 0);
    ofSetVerticalSync(true);

    //square canvas
    movieFbo.allocate(screenHeight,screenHeight,GL_RGBA);
    kPhotoFbo.allocate(screenHeight, screenHeight,GL_RGBA, 8); // 8 is number of sample
    fadingFbo.allocate(1080, 1080,GL_RGBA, 8);
    fadeMaskFbo.allocate(1080, 1080,GL_RGBA, 8);
    tPhotoFbo.allocate(screenHeight, screenHeight,GL_RGBA, 8); // 8 is number of sample
    photoBombFbo.allocate(screenHeight,screenHeight,GL_RGBA);


    movieFbo.begin();
    ofClear(255, 255, 255,0);
    movieFbo.end();

    kPhotoFbo.begin();
    ofClear(255, 255, 255,0);
    kPhotoFbo.end();

    fadingFbo.begin();
    ofClear(255, 255, 255,0);
    fadingFbo.end();

    tPhotoFbo.begin();
    ofClear(255, 255, 255,0);
    tPhotoFbo.end();

    photoBombFbo.begin();
    ofClear(255, 255, 255,0);
    photoBombFbo.end();

    fadeMaskFbo.begin();
    ofClear(0,0,0,255);
    fadeMaskFbo.end();

    fadeMaskAlpha = 0;
    backgroundImage=&kPhotoMonoInfoImg;
    foregroundImage=&kPhotoImg;


    searchingFont.load("Skia.ttf", 17);
    frameFont.load("Skia.ttf", 24);

    kPhotoMonoInfoAlpha = 0;
    crossFadeSpeed = 5;

    // timing variable
    frameCounter = 0;
    kPhotoSmallHoldTime = 100;
    kPhotoHoldTime = 100;
    kPhotoMonoInfoHoldTime = 100;
    kPhotoMonoHoldTime = 100;
    kColorHoldTime = 200;
    tColorHoldTime = 200;
    tPhotoHoldTime = 100;
    tPhotoMonoHoldTime = 200;
    tPhotoInfoHoldTime = 200;
    tPhotoMonoInfoHoldTime = 200;
    tColorNameHoldTime = 200;
    photoBombHoldTime = 50;

    photoBombTotalNumb = 10;
    photoBombOrderIndex = 0;
    photoBombSwitchWaitCount = 0;
    photoBombSwitchSpeed = 10; //the higher the slower
    photoBombSwitchAccel = 7; //the higher the faster
    photoBombSwitchHoldTime = photoBombSwitchSpeed+(int)(pow(photoBombTotalNumb,2)/photoBombSwitchAccel);

    photoBombClosingHoldTime = 200;
    mapHoldTime = 200;
    mapSpeed = 300;

    endingHoldTime = 200;
    endingAlpha = 255;


    ofLog()<<"photo bomb order";
    // set some values and shuffle it
    for (int i=1; i<photoBombTotalNumb+1; ++i) {
        photoBombOrder.push_back(i);
    }
    random_shuffle ( photoBombOrder.begin(), photoBombOrder.end());
    // 1 2 3 4 5 6 7 8 9 10

    for (std::vector<int>::const_iterator it = photoBombOrder.begin(); it != photoBombOrder.end(); ++it){
        std::cout << *it << ' ';
    }




    photoBombClosingImg.load("photoBombClosing.jpg");
    mapAImg.load("mapA.jpg");
    mapBImg.load("mapB.jpg");
    endingImg.load("ending.jpg");

    animatedPhotoPos.animateFromTo(0, screenHeight);
    animatedPhotoPos.setDuration(0.5);
    //AnimCurve curve = (AnimCurve) (EASE_IN_EASE_OUT);
    AnimCurve curve = (AnimCurve) (EASE_OUT);

    animatedPhotoPos.setCurve( curve );

    animatedPhotoSize.animateFromTo(screenHeight*0.22, screenHeight);
    animatedPhotoSize.setDuration(1);
    animatedPhotoSize.setCurve( curve );

    kPhotoPosYoffset.animateFromTo(-screenHeight*0.03, 0);
    kPhotoPosYoffset.setDuration(1);
    kPhotoPosYoffset.setCurve( curve );


    animatedCircleSize.animateFromTo(screenHeight*0.042, screenHeight*2);
    animatedCircleSize.setDuration(2);
    AnimCurve curve2 = (AnimCurve) (TANH);
//    AnimCurve curve2 = (AnimCurve) (CUBIC_EASE_IN);

    animatedCircleSize.setCurve( curve2 );

    tPhotoScanBarPos = 0;

    colorWheel.setup(screenHeight*0.051,screenHeight*0.037,2.625); //outter radius, inner radius, rotation speed

    totalMovies = 3;
    nowPlayer = 0;
    loadingMovieId = 0;
    playAndPreload();

    showInfo = true;

    if (XML.load("mySettings.xml")) {
        ofLog()<<"mySettings.xml loaded!";
        canvasOffsetX=XML.getValue("position:canvasOffsetX", 0);
        canvasOffsetY=XML.getValue("position:canvasOffsetY", 0);

    }else{
        XML.setValue("position:canvasOffsetX", 0);
        XML.setValue("position:canvasOffsetY", 0);
        XML.saveFile("mySettings.xml"); //puts settings.xml file in the bin/data folder
    }

}

//--------------------------------------------------------------
void ofApp::update(){
    switch (state) {
        case STATE_START:
            //play movie loop;
            checkSerial();
            updateMovie();
        break;
        case STATE_DETECTED:
            //object detected! searching;
            if(movies[nowPlayer].isPlaying()){
                switchMovie();
                movies[nowPlayer].stop();
                movieFbo.begin();
                ofClear(255, 255, 255,0);
                movieFbo.end();
            }
            if(colorWheel.times<70){
                colorWheel.update();
            }else{
                colorWheel.resetTimes();
                state = STATE_KPHOTO_IN;
            }
        break;

        case STATE_KPHOTO_IN:
            //show kyoto photo, zoom, show text;
            if(frameCounter<kPhotoSmallHoldTime){
                frameCounter++;
            }else{
                if(!animatedPhotoSize.hasFinishedAnimating()){
                    float dt = 1.0f / 60.0f;
                    animatedPhotoSize.update( dt );
                    kPhotoPosYoffset.update( dt );
                }else{
                        animatedPhotoSize.animateFromTo(screenHeight*0.22, screenHeight);
                        kPhotoPosYoffset.animateFromTo(-screenHeight*0.03, 0);
                        frameCounter = 0;
                        state =STATE_KPHOTO_MONO_INFO_IN;
                    //prepare for next scene
                    fadingFbo.begin();
                    kPhotoImg.draw(0, 0, fadingFbo.getWidth(),fadingFbo.getHeight());
                    fadingFbo.end();

                }
            }

            kPhotoFbo.begin();
                ofPushMatrix();
                ofPushStyle();

                ofSetColor(0);
                ofDrawRectangle(0, 0, kPhotoFbo.getWidth(), kPhotoFbo.getHeight());

                ofTranslate(kPhotoFbo.getWidth()/2, kPhotoFbo.getHeight()/2);
                ofSetRectMode(OF_RECTMODE_CENTER);

                ofEnableAlphaBlending();
                ofEnableSmoothing();

                drawKPhoto(0,kPhotoPosYoffset,animatedPhotoSize.val(),255);

                drawkPhotoMonoInfo(0,0,kPhotoFbo.getWidth(),kPhotoMonoInfoAlpha);

                ofSetRectMode(OF_RECTMODE_CORNER);
                drawFrame(-kPhotoFbo.getWidth()*0.11,-kPhotoFbo.getWidth()*0.15, kPhotoFbo.getWidth()*0.22, kPhotoFbo.getWidth()*0.305,ofColor::white);

                ofDisableAlphaBlending();
                ofDisableSmoothing();


                ofPopStyle();
                ofPopMatrix();

            kPhotoFbo.end();
        break;

        case STATE_KPHOTO_MONO_INFO_IN:
            if(frameCounter<kPhotoHoldTime){
                //HOLD KPHOTO
                frameCounter++;
            }else{
                kPhotoCrossFade();

                if(fadeMaskAlpha<255){
                    //FADE IN KPHOTO_MONO_INFO
                    updateFadeMask(crossFadeSpeed);
                }else{
                    if(frameCounter<kPhotoHoldTime+kPhotoMonoInfoHoldTime){
                        //HOLD KPHOTO_MONO_INFO
                        frameCounter++;
                    }else{
                        resetFadeMask();
                        fadeMaskAlpha = 0;
                        state = STATE_KPHOTO_MONO_IN;
                        frameCounter = 0;
                        foregroundImage=&kPhotoMonoInfoImg;
                        backgroundImage=&kPhotoMonoImg;

                    }
                }
            }

        break;

        case STATE_KPHOTO_MONO_IN:
            kPhotoCrossFade();

            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<kPhotoMonoHoldTime) {
                    frameCounter++;
                }
                else{
                    resetFadeMask();
                    fadeMaskAlpha = 0;
                    state = STATE_KCOLOR_IN;
                    frameCounter = 0;
                    foregroundImage=&kPhotoMonoImg;
                    backgroundImage=&kColorImg;
                }

            }
        break;


        case STATE_KCOLOR_IN:
            kPhotoCrossFade();

            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<kColorHoldTime) {
                    frameCounter++;
                }
                else{
                    resetFadeMask();
                    fadeMaskAlpha = 0;
                    state = STATE_TCOLOR_IN;
                    frameCounter = 0;
                    foregroundImage=&kColorImg;
                    backgroundImage=&tColorImg;
                }

            }
        break;

        case STATE_TCOLOR_IN:
            //show taiwan color, fade in;
            kPhotoCrossFade();

            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<tColorHoldTime) {
                    frameCounter++;
                }
                else{
                    resetFadeMask();
                    fadeMaskAlpha = 0;
                    state = STATE_TPHOTO_MONO_IN;
                    frameCounter = 0;
                    foregroundImage=&tColorImg;
                    backgroundImage=&tPhotoMonoImg;
                }

            }

        break;

        case STATE_TPHOTO_MONO_IN:
            //show taiwan PHOTO MONO, fade in;
            kPhotoCrossFade();

            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<tPhotoMonoHoldTime) {
                    frameCounter++;
                }
                else{
                    resetFadeMask();
                    fadeMaskAlpha = 0;
                    state = STATE_TPHOTO_MONO_INFO_IN;
                    frameCounter = 0;
                    foregroundImage=&tPhotoMonoImg;
                    backgroundImage=&tPhotoMonoInfoImg;
                }

            }

        break;

        case STATE_TPHOTO_MONO_INFO_IN:
            //show taiwan PHOTO MONO INFO, fade in;
            kPhotoCrossFade();

            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<tPhotoMonoInfoHoldTime) {
                    frameCounter++;
                }
                else{
                    resetFadeMask();
                    fadeMaskAlpha = 0;
                    state = STATE_TPHOTO_INFO_IN;
                    frameCounter = 0;
                    foregroundImage=&tPhotoMonoInfoImg;
                    backgroundImage=&tPhotoInfoImg;
                }

            }
        break;

        case STATE_TPHOTO_INFO_IN:
            //show taiwan PHOTO MONO INFO, fade in;
            kPhotoCrossFade();

            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<tPhotoInfoHoldTime) {
                    frameCounter++;
                }
                else{
                    state = STATE_TCOLOR_FILL;
                    frameCounter = 0;
                    resetFadeMask();
                    fadeMaskAlpha = 0;
                    backgroundImage=&kPhotoMonoInfoImg;
                    foregroundImage=&kPhotoImg;

                    //preload next scene
                    tPhotoFbo.begin();
                    tPhotoInfoImg.draw(0,0,tPhotoFbo.getWidth(),tPhotoFbo.getHeight());
                    tPhotoFbo.end();
                }

            }
        break;


        case STATE_TCOLOR_FILL:
            //taiwan color fill;
            tPhotoFbo.begin();
            
            tPhotoInfoImg.draw(0,0,tPhotoFbo.getWidth(),tPhotoFbo.getHeight());

            if(!animatedCircleSize.hasFinishedAnimating()){
                float dt = 1.0f / 60.0f;
                animatedCircleSize.update( dt );

                ofPushMatrix();
                ofPushStyle();
                ofEnableAlphaBlending();
                ofEnableSmoothing();

                ofTranslate(tPhotoFbo.getWidth()*allColorPickers[detectedColor-1].x/1080,tPhotoFbo.getHeight()*allColorPickers[detectedColor-1].y/1080); // using 0 starting numbering system
                // todo:: need to have an array hold the color circle's pos and color on all 80 photos

                ofSetCircleResolution(60);

                ofSetColor(ofColor::white);
                ofDrawCircle(0,0,animatedCircleSize.val()+1);

                ofSetColor(allColorPickers[detectedColor-1].hexColor);
                ofDrawCircle(0,0,animatedCircleSize.val());

                ofDisableAlphaBlending();
                ofDisableSmoothing();

                ofPopStyle();
                ofPopMatrix();

            }else{
                ofClear(255,255,255,0);
                animatedCircleSize.animateFromTo(screenHeight*0.042, screenHeight*2);

                state = STATE_TNAME_IN;

            }
            tPhotoFbo.end();

        break;
        case STATE_TNAME_IN:
            //show tCOLOR NAME;
            if(!animatedPhotoPos.hasFinishedAnimating()){
                float dt = 1.0f / 60.0f;
                animatedPhotoPos.update( dt );

            }else{
                if(frameCounter<tColorNameHoldTime){
                    frameCounter++;
                }else{
                    frameCounter = 0;
                    animatedPhotoPos.animateFromTo(0,screenHeight);
                    state = STATE_PHOTO_BOMB;
                }
            }


        break;
        case STATE_PHOTO_BOMB:
            //show PHOTO BOMB;

            if(frameCounter< photoBombHoldTime){
                frameCounter++;
                photoBombFbo.begin();
                    ofPushStyle();
                    ofSetColor(allColorPickers[detectedColor-1].hexColor);// using 0 starting numbering system
                    ofDrawRectangle(canvasOffsetX, canvasOffsetY, photoBombFbo.getWidth(), photoBombFbo.getHeight());
                    ofPopStyle();
                photoBombFbo.end();

            }else{
                int photoBombCurrentPick;

                if(photoBombOrderIndex ==0){ //first time
                    photoBombCurrentPick = *photoBombOrder.begin();
                    ofLog()<<"photoBombCurrentPick: "<<photoBombCurrentPick;
                    photoBombImg.load("tPhotoBomb/tphotoBomb-c"+ofToString(detectedColor)+"-"+ofToString(photoBombCurrentPick)+".jpg");
                    photoBombOrderIndex++;

                    photoBombFbo.begin();
                    photoBombImg.draw(ofRandom(-200,200),ofRandom(-200,200), photoBombFbo.getWidth(),photoBombFbo.getHeight());
                    photoBombFbo.end();

                }

                if(photoBombSwitchWaitCount<photoBombSwitchHoldTime){
                    photoBombSwitchWaitCount++;
                }else{
                    ofLog()<<"new delay";

                    //photoBombSwitchHoldTime =  pow((photoBombTotalNumb-photoBombOrderIndex),2)+10;   //new delay, shorter each time
                    photoBombSwitchHoldTime = photoBombSwitchSpeed+(int)(pow(photoBombTotalNumb-photoBombOrderIndex,2)/photoBombSwitchAccel);
                    photoBombSwitchWaitCount = 0; // reset frameCounter

                    if(photoBombOrderIndex<photoBombTotalNumb){
                        vector<int>::iterator it=photoBombOrder.begin()+photoBombOrderIndex;
                        photoBombCurrentPick = *it;
                        ofLog()<<"photoBombCurrentPick: "<<photoBombCurrentPick;
                        photoBombImg.load("tPhotoBomb/tphotoBomb-c"+ofToString(detectedColor)+"-"+ofToString(photoBombCurrentPick)+".jpg");

                        photoBombOrderIndex++;
                    }else{
                        photoBombSwitchWaitCount = 0;
                        photoBombOrderIndex = 0;
                        photoBombSwitchHoldTime = photoBombSwitchSpeed+(int)(pow(photoBombTotalNumb,2)/photoBombSwitchAccel);
                        random_shuffle ( photoBombOrder.begin(), photoBombOrder.end() );

                        photoBombFbo.begin();
                        ofClear(255, 255, 255,0);
                        photoBombFbo.end();
                        frameCounter =0;
                        state = STATE_PHOTO_BOMB_CLOSING;

                    }

                    photoBombFbo.begin();
                    photoBombImg.draw(ofRandom(-200,200),ofRandom(-200,200), photoBombFbo.getWidth(),photoBombFbo.getHeight());
                    photoBombFbo.end();

                }
            }


        break;
        case STATE_PHOTO_BOMB_CLOSING:
            //show LAST PHOTO OF PHOTO BOMB : LOGO AND WHITE BACKGROUND;
            if(frameCounter<photoBombClosingHoldTime){
                frameCounter++;
            }else{
                frameCounter = 0;
                state = STATE_MAP;
            }

        break;
        case STATE_MAP:
            //show LAST PHOTO OF PHOTO BOMB : LOGO AND WHITE BACKGROUND;
            if(frameCounter<mapHoldTime){
                frameCounter++;
            }else{
                frameCounter = 0;
                state = STATE_ENDING;
            }

        break;
        case STATE_ENDING:
            //show ending;
            if(frameCounter<endingHoldTime){
                frameCounter++;
            }else{
                if(endingAlpha>0){
                    endingAlpha-=15;
                    if(endingAlpha<0)endingAlpha=0;
                }else{
                    endingAlpha = 255;
                    frameCounter = 0;
                    state = STATE_START;

                    ofLog()<<"starting over";
                }
            }
        break;



        default:
          break;
    }


//     colorCardFbo.begin();
//
//     ofBackground(39, 85, 128);
//     ofFill();
//     ofPushMatrix();
//     ofPushStyle();
//     ofSetRectMode(OF_RECTMODE_CENTER);
// //        glBlendFuncSeparate(GL_ONE, GL_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//
//    // glBlendFuncSeparate(GL_ONE, GL_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//
//     //ofTranslate(colorCardFbo.getWidth()/2, colorCardFbo.getHeight()/2);
//     //ofSetColor(255,255,255,255);
//     logoTop.draw(ofGetWidth()/2,ofGetHeight()/2);
//   //  ofSetColor(255,0,0);
// //    ofFill();
// //    logoTopSVG.draw();
//     //if(titleScale<1.5) titleScale+=0.01;
//     //else    titleScale = 1;
//     //titleFont.drawString("byColors", ofGetWidth()/2,ofGetHeight()/2);
//     ofPopStyle();
//     ofPopMatrix();
//     colorCardFbo.end();

    //show fps on title bar
    std::stringstream strm;
	strm << "fps: " << ofGetFrameRate();
	ofSetWindowTitle(strm.str());


}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0, 0, 0);

    switch (state) {
        case STATE_START:
            //play movie loop;

            movieFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY);
        break;
        //--------------------------------------------------------------
        case STATE_DETECTED:
            //object detected! searching;

            ofPushMatrix();
            ofPushStyle();
            ofTranslate(screenWidth/2+canvasOffsetX, screenHeight/2+canvasOffsetY);

            ofEnableAlphaBlending();
            ofEnableSmoothing();

            ofSetColor(0);
            ofDrawRectangle(-screenHeight/2,-screenHeight/2,screenHeight,screenHeight);

            colorWheel.draw(0,-kPhotoFbo.getWidth()*0.045);
            drawFrame(-kPhotoFbo.getWidth()*0.11,-kPhotoFbo.getWidth()*0.15, kPhotoFbo.getWidth()*0.22, kPhotoFbo.getWidth()*0.305,ofColor::white);
            ofSetColor(ofColor::white);
            searchingFont.drawString("searching...", -searchingFont.stringWidth("searching...")/2, kPhotoFbo.getWidth()*0.04);

            ofDisableAlphaBlending();
            ofDisableSmoothing();

            ofPopStyle();
            ofPopMatrix();

        break;
        //--------------------------------------------------------------

        case STATE_KPHOTO_IN:
            //show kyoto photo, zoom, show text;
            kPhotoFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY);
        break;

        //--------------------------------------------------------------

        case STATE_KPHOTO_MONO_INFO_IN:
            //fade kyoto photo from color to mono info then to mono
            fadingFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
        break;

        //--------------------------------------------------------------

        case STATE_KPHOTO_MONO_IN:
            //fade from kyoto photo mono info to mono
            fadingFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
            break;

        //--------------------------------------------------------------

        case STATE_KCOLOR_IN:
            //fade from kyoto photo mono to kcolor
            fadingFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);

        break;
        //--------------------------------------------------------------
        case STATE_TCOLOR_IN:
            //show taiwan color, FADE in;
            fadingFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);

        break;
        //--------------------------------------------------------------
        case STATE_TPHOTO_MONO_IN:
            //show taiwan photo mono, FADE in;
            fadingFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);

        break;
        //--------------------------------------------------------------
        case STATE_TPHOTO_MONO_INFO_IN:
            //show taiwan photo mono info, FADE in;
            fadingFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);

        break;
        //--------------------------------------------------------------
        case STATE_TPHOTO_INFO_IN:
            //show taiwan photo info, FADE in;
            fadingFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);

        break;
        //--------------------------------------------------------------
        case STATE_TCOLOR_FILL:
            //show taiwan color;
            tPhotoFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY);


        break;
        //--------------------------------------------------------------
        case STATE_TNAME_IN:
            //show color name
            ofPushStyle();
            ofSetColor(allColorPickers[detectedColor-1].hexColor);
            // todo:: need to have an array hold the color circle's pos and color on all 80 photos
            ofDrawRectangle((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
            ofPopStyle();
            tNameImg.draw((screenWidth+screenHeight)/2-animatedPhotoPos.val()+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);


        break;
        //--------------------------------------------------------------
        case STATE_PHOTO_BOMB:
            //show PHOTO BOMB;

            photoBombFbo.draw((screenWidth-screenHeight)/2,0);
//            ofPushStyle();
//            ofSetColor(ofColor::fromHex(0xB66D66));
//            ofDrawRectangle((screenWidth-screenHeight)/2+canvasOffsetX, screenHeight*0.954+canvasOffsetY, screenHeight, screenHeight*0.046);
//            ofPopStyle();


        break;
        //--------------------------------------------------------------
        case STATE_PHOTO_BOMB_CLOSING:
            //show LAST PHOTO OF PHOTO BOMB : LOGO AND WHITE BACKGROUND;
            photoBombClosingImg.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);


        break;
        //--------------------------------------------------------------
        case STATE_MAP:
            //show LAST PHOTO OF PHOTO BOMB : LOGO AND WHITE BACKGROUND;
            if(ofGetElapsedTimeMillis()%mapSpeed<mapSpeed/2){
                mapAImg.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
            }else{
                mapBImg.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
            }

        break;
        //--------------------------------------------------------------
        case STATE_ENDING:
            //show ending;
            ofPushStyle();
            ofEnableAlphaBlending();
            ofSetColor(255, 255, 255, endingAlpha);
            endingImg.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
            ofDisableAlphaBlending();
            ofPopStyle();
        break;
        //--------------------------------------------------------------
        default:
        break;
    }

    //draw mask on left and right
    ofPushStyle();
    ofSetColor(0);
    ofDrawRectangle(canvasOffsetX,canvasOffsetY, (screenWidth-screenHeight)/2, screenHeight);
    ofDrawRectangle((screenWidth+screenHeight)/2+canvasOffsetX,canvasOffsetY,(screenWidth-screenHeight)/2, screenHeight);

    ofPopStyle();

    if (showInfo) {
        ofDrawBitmapString("Next Movie ID: "+ofToString(loadingMovieId), 50, 70);
        ofDrawBitmapString("Total Frames: "+ofToString(movies[nowPlayer].getTotalNumFrames()), 50, 100);
        ofDrawBitmapString("Current Frame: "+ofToString(movies[nowPlayer].getCurrentFrame()), 50, 130);
    }




}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    switch (key) {
        case OF_KEY_LEFT:
            canvasOffsetX--;
        break;
        case OF_KEY_RIGHT:
            canvasOffsetX++;
        break;
        case OF_KEY_UP:
            canvasOffsetY--;
        break;
        case OF_KEY_DOWN:
            canvasOffsetY++;
        break;

        case 'r':
            switchMovie();
        break;

        case 'i':
            showInfo=!showInfo;
        break;

        case '0':
            playAndPreload();
            state = STATE_START;
        break;
        case '1':
            loadAssets(1);
            state = STATE_DETECTED;
        break;
        case '2':
            state = 2;
        break;
        case '3':
            state = 3;
        break;


        default:
            break;
    }

    XML.setValue("position:canvasOffsetX", canvasOffsetX);
    XML.setValue("position:canvasOffsetY", canvasOffsetY);

    XML.saveFile("mySettings.xml"); //puts settings.xml file in the bin/data folder
}


//--------------------------------------------------------------
void ofApp::playAndPreload(){
    // PLAY the current movie

    // need to load the first movie : should only happen once
    if( !movies[nowPlayer].isLoaded() ){
        movies[nowPlayer].load( "movies/morning-"+ofToString(loadingMovieId) + ".mp4" );
        //todo:: check time and load morning, afternoon, night movies
    }

    movies[nowPlayer].setLoopState(OF_LOOP_NORMAL);
    movies[nowPlayer].play();

    ofLog()<<"playing: "<<loadingMovieId<<".mp4";

    // PRELOAD the next movie

    // get a unique random ID
    int preloadId = floor(ofRandom(totalMovies));
    while (preloadId == loadingMovieId ) {
        preloadId = floor(ofRandom(totalMovies));
    }

    // set the loading movie to the new ID
    loadingMovieId = preloadId;

    // load movie in the player that isn't 'nowPlayer'
    int nextPlayer = nowPlayer == 0 ? 1 : 0;
    movies[nextPlayer].load(  "movies/morning-"+ofToString(loadingMovieId) + ".mp4" );
    ofLog()<<"loading: : "<<loadingMovieId<<".mp4";

}

//--------------------------------------------------------------
void ofApp::switchMovie(){
    // swap video player ID
    nowPlayer = ( nowPlayer == 0 ) ? 1 : 0;

    playAndPreload();

    // stop currently playing video
    movies[!nowPlayer].stop();

}
//--------------------------------------------------------------
void ofApp::updateMovie(){
    if(!movies[nowPlayer].isPlaying()){
        movies[nowPlayer].play();
    }
  movies[nowPlayer].update();

  int totalFrames = movies[nowPlayer].getTotalNumFrames();
  int currentFrame = movies[nowPlayer].getCurrentFrame();
  if((currentFrame>0) && (totalFrames-currentFrame <1)){
      ofLog()<<"movie is done";
      // swap video player I    D
      switchMovie();
  }


  movieFbo.begin();
  // ofPushStyle();
  // ofSetRectMode(OF_RECTMODE_CENTER); //set rectangle mode to the center
  // movies[nowPlayer].draw(movieFbo.getWidth()/2, movieFbo.getHeight()/2);
  movies[nowPlayer].draw(0,0,movieFbo.getWidth(),movieFbo.getHeight());
  // ofPopStyle();
  movieFbo.end();

}
//--------------------------------------------------------------
void ofApp::checkSerial(){
  int myByte = 0;
  myByte = serial.readByte();
  if ( myByte == OF_SERIAL_NO_DATA ){}
    // printf("no data was read");
  else if ( myByte == OF_SERIAL_ERROR ){}
    // printf("an error occurred");
  else if ( myByte == 99){
    ofLog()<<"no matching color";
  }
  else{
    ofLog()<<"detected color: "<< myByte;
    detectedColor = myByte;

//      kPhoto.load("kPhotos/1.jpg");
//      kPhotoMonoInfo.load("kPhotos/1-info.jpg");
//      kColor.load("kColors/1.jpg");

    loadAssets(detectedColor);
    state = STATE_DETECTED;
  }
}
//--------------------------------------------------------------


void ofApp::drawFrame(float _x, float _y, float _w, float _h, ofColor c){

    ofPath path;
    path.setFillColor(c);
//    path.rectangle(-120, -195, 240, 335);
//    path.rectangle(-110, -173, 220, 220);
    path.rectangle(_x, _y, _w, _h);
    path.rectangle(_x+_w*0.04, _y+_w*0.083, _w*0.92, _w*0.92);

    path.draw();


    ofSetColor(c.getInverted());
    ofPushMatrix();
    ofTranslate(_x+_w/2, _y+_h);
    float strWidth = frameFont.stringWidth("#by colors");
    frameFont.drawString("#by colors",-strWidth/2,-_h*.1);
    ofPopMatrix();

}

//--------------------------------------------------------------


void ofApp::drawKPhoto(float _x, float _y, float _size,float _alpha){
    ofPushStyle();
    ofSetColor(255, 255, 255, _alpha);
    kPhotoImg.draw(_x,_y,_size,_size);
    ofPopStyle();

}

//--------------------------------------------------------------


void ofApp::drawkPhotoMonoInfo(float _x, float _y, float _size, float _alpha){
    ofPushStyle();
        ofSetColor(255, 255, 255, _alpha);
        kPhotoMonoInfoImg.draw(_x,_y,_size,_size);
    ofPopStyle();
}

//--------------------------------------------------------------

void ofApp::kPhotoCrossFade(){
    fadingFbo.begin();
    shader.begin();
    // here is where the fbo is passed to the shader
    shader.setUniformTexture("maskTex", fadeMaskFbo.getTexture(), 1 );
    shader.setUniformTexture("foregroundTex", foregroundImage->getTexture(), 2 );

    backgroundImage->draw(0, 0);

    shader.end();
    fadingFbo.end();
}

//--------------------------------------------------------------

void ofApp::resetkPhotoMonoInfo(){
    kPhotoMonoInfoAlpha = 0;
    frameCounter = 0;
}

//--------------------------------------------------------------


void ofApp::loadAssets(int _numOfColor){

    string colorNumber = ofToString(_numOfColor);

    kPhotoImg.load("kPhotos/kphoto-c"+colorNumber+".jpg");
    kPhotoMonoInfoImg.load("kPhotos/kphoto-c"+colorNumber+"-mono-info.jpg");
    kPhotoMonoImg.load("kPhotos/kphoto-c"+colorNumber+"-mono.jpg");
    kColorImg.load("kColors/kcolor-c"+colorNumber+".jpg");
    tColorImg.load("tColors/tcolor-c"+colorNumber+".jpg");

    //string tPhotoSubNumber = ofToString((int)ofRandom(photoBombTotalNumb)+1); todo:: test this
    string tPhotoSubNumber = ofToString(1);

    tPhotoInfoImg.load("tPhotos/tphoto-c"+colorNumber+"-"+tPhotoSubNumber+"-info.jpg");
    tPhotoMonoImg.load("tPhotos/tphoto-c"+colorNumber+"-"+tPhotoSubNumber+"-mono.jpg");
    tPhotoMonoInfoImg.load("tPhotos/tphoto-c"+colorNumber+"-"+tPhotoSubNumber+"-mono-info.jpg");
    tNameImg.load("tNames/tname-c"+colorNumber+".jpg");

}

//--------------------------------------------------------------
void ofApp::updateFadeMask(int _fadeSpeed){
    fadeMaskAlpha+=_fadeSpeed;
    if(fadeMaskAlpha>255) fadeMaskAlpha =255;
    if(fadeMaskAlpha<0)fadeMaskAlpha =0;
    fadeMaskFbo.begin();
    ofPushStyle();
    ofSetColor(fadeMaskAlpha);
    ofDrawRectangle(0, 0, fadeMaskFbo.getWidth(), fadeMaskFbo.getHeight());
    ofPopStyle();
    fadeMaskFbo.end();
}
//--------------------------------------------------------------
void ofApp::resetFadeMask(){
    fadeMaskFbo.begin();
    ofClear(0,0,0,255);
    fadeMaskFbo.end();
}


//--------------------------------------------------------------


void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
