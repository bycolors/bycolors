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

    ofHideCursor();
    

    ofLog()<<"screen resolution: "<<ofGetWidth()<<" x "<<ofGetHeight();
    screenWidth = ofGetWidth();
    screenHeight = ofGetHeight();
    
    state = STATE_START;

    //for testing
    //loadAssets(1);
    //state = STATE_TPHOTO_MONO_IN_2;

    serial.listDevices();
  	vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    int baud = 9600;
  	serial.setup(0, baud); //open the first device
    
    bSendSerialMessage = false;

    detectedColor = 100;

    ofBackground(0, 0, 0);
    ofSetVerticalSync(true);

    //square canvas
    movieFbo.allocate(screenHeight,screenHeight,GL_RGBA);
    kPhotoFbo.allocate(screenHeight, screenHeight,GL_RGBA, 8); // 8 is number of sample
    fadingFbo.allocate(1080, 1080,GL_RGBA, 8);
    fadeMaskFbo.allocate(1080, 1080,GL_RGBA, 8);
    tPhotoFbo.allocate(screenHeight, screenHeight,GL_RGBA, 8); // 8 is number of sample
    tPhotoGridFbo.allocate(1080, 1080,GL_RGBA, 8);


    movieFbo.begin();
    ofClear(255, 255, 255,0);
    movieFbo.end();

    kPhotoFbo.begin();
    ofClear(255, 255, 255,0);
    kPhotoFbo.end();

    fadingFbo.begin();
    ofClear(0, 0, 0,0);
    fadingFbo.end();

    tPhotoFbo.begin();
    ofClear(255, 255, 255,0);
    tPhotoFbo.end();

    tPhotoGridFbo.begin();
    ofClear(255, 255, 255,0);
    tPhotoGridFbo.end();

    fadeMaskFbo.begin();
    ofClear(0,0,0,255);
    fadeMaskFbo.end();

    fadeMaskAlpha = 0;
    backgroundImage=&kPhotoMonoInfoImg;
    foregroundImage=&kPhotoImg;


    searchingFont.load("Mrs Eaves OT Roman.otf", 17);
    searchingFont.setLetterSpacing(0.9);
    frameFont.load("Mrs Eaves OT Roman.otf", 24);
    //searchingImg.load("searching_text.jpg");
    //byColorImg.load("bycolor_text.jpg");

    crossFadeSpeed = 5;
    tClosingAlpha = 255;
    // timing variable
    frameCounter = 0;
    kPhotoSmallHoldTime = 50;
    kPhotoHoldTime = 50;
    kPhotoMonoInfoHoldTime = 550;
    kPhotoMonoHoldTime = 50;
    kColorHoldTime = 150;
    tColorHoldTime = 150;
    tPhotoHoldTime = 50;
    tPhotoMonoHoldTime = 50;
    tPhotoInfoHoldTime = 550;
    tPhotoMonoInfoHoldTime = 200;
    tPhotoMono2HoldTime = 50;
    tColorNameHoldTime = 200;
    
    tClosingHoldTime = 150;
    mapHoldTime = 200;
    mapSpeed = 300;

    endingHoldTime = 200;
    endingAlpha = 255;

    
    
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


    animatedCircleSize.animateFromTo(tPhotoGridFbo.getWidth()*0.01, tPhotoGridFbo.getWidth()*2);
    animatedCircleSize.setDuration(2);
    AnimCurve curve2 = (AnimCurve) (TANH);
//    AnimCurve curve2 = (AnimCurve) (CUBIC_EASE_IN);
    animatedCircleSize.setCurve( curve2 );
    
    resetGridAnimation();
    
    tPhotoScanBarPos = 0;

    colorWheel.setup(screenHeight*0.051,screenHeight*0.037,2.625); //outter radius, inner radius, rotation speed

    totalMovies = 3;
    nowPlayer = 0;
    loadingMovieId = 0;
    playAndPreload();

    showInfo = false;

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
        
        //--------------------------------------------------------------

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
                //prepare for next scene
                kPhotoFbo.begin();
                ofPushMatrix();
                ofPushStyle();
                
                ofSetColor(0);
                ofDrawRectangle(0, 0, kPhotoFbo.getWidth(), kPhotoFbo.getHeight());
                
                ofTranslate(kPhotoFbo.getWidth()/2, kPhotoFbo.getHeight()/2);
                ofSetRectMode(OF_RECTMODE_CENTER);
                
                ofEnableAlphaBlending();
                ofEnableSmoothing();
                
                ofSetColor(255, 255, 255);
                kPhotoImg.draw(0,kPhotoPosYoffset,animatedPhotoSize.val(),animatedPhotoSize.val());
                
            
                ofSetRectMode(OF_RECTMODE_CORNER);
                drawFrame(-kPhotoFbo.getWidth()*0.11,-kPhotoFbo.getWidth()*0.15, kPhotoFbo.getWidth()*0.22, kPhotoFbo.getWidth()*0.305,ofColor::white);
                
                ofDisableAlphaBlending();
                ofDisableSmoothing();
                
                
                ofPopStyle();
                ofPopMatrix();
                
                kPhotoFbo.end();

            }
        break;

        //--------------------------------------------------------------

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
            
                ofSetColor(255, 255, 255);
                kPhotoImg.draw(0,kPhotoPosYoffset,animatedPhotoSize.val(),animatedPhotoSize.val());


                ofSetRectMode(OF_RECTMODE_CORNER);
                drawFrame(-kPhotoFbo.getWidth()*0.11,-kPhotoFbo.getWidth()*0.15, kPhotoFbo.getWidth()*0.22, kPhotoFbo.getWidth()*0.305,ofColor::white);

                ofDisableAlphaBlending();
                ofDisableSmoothing();


                ofPopStyle();
                ofPopMatrix();

            kPhotoFbo.end();
        break;

        //--------------------------------------------------------------

        case STATE_KPHOTO_MONO_INFO_IN:
            if(frameCounter<kPhotoHoldTime){
                //HOLD KPHOTO
                frameCounter++;
            }else{
                PhotoCrossFade();

                if(fadeMaskAlpha<255){
                    //FADE IN KPHOTO_MONO_INFO
                    updateFadeMask(crossFadeSpeed);
                }else{
                    if(frameCounter<kPhotoHoldTime+kPhotoMonoInfoHoldTime){
                        //HOLD KPHOTO_MONO_INFO
                        frameCounter++;
                    }else{
                        resetFadeMask();
                        state = STATE_KPHOTO_MONO_IN;
                        frameCounter = 0;
                        foregroundImage=&kPhotoMonoInfoImg;
                        backgroundImage=&kPhotoMonoImg;

                    }
                }
            }

        break;

        //--------------------------------------------------------------

        case STATE_KPHOTO_MONO_IN:
            PhotoCrossFade();

            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<kPhotoMonoHoldTime) {
                    frameCounter++;
                }
                else{
                    resetFadeMask();
                    state = STATE_KCOLOR_IN;
                    frameCounter = 0;
                    foregroundImage=&kPhotoMonoImg;
                    backgroundImage=&kColorImg;
                }

            }
        break;

        //--------------------------------------------------------------

        case STATE_KCOLOR_IN:
            PhotoCrossFade();

            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<kColorHoldTime) {
                    frameCounter++;
                }
                else{
                    resetFadeMask();
                    state = STATE_TCOLOR_IN;
                    frameCounter = 0;
                    foregroundImage=&kColorImg;
                    backgroundImage=&tColorImg;
                }

            }
        break;
            
        //--------------------------------------------------------------

        case STATE_TCOLOR_IN:
            //show taiwan color, fade in;
            PhotoCrossFade();

            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<tColorHoldTime) {
                    frameCounter++;
                }
                else{
                    resetFadeMask();
                    state = STATE_TPHOTO_MONO_IN;
                    frameCounter = 0;
                    foregroundImage=&tColorImg;
                    backgroundImage=&tPhotoMonoImg;
                }

            }

        break;

        //--------------------------------------------------------------


        case STATE_TPHOTO_MONO_IN:
            //show taiwan PHOTO MONO, fade in;
            PhotoCrossFade();

            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<tPhotoMonoHoldTime) {
                    frameCounter++;
                }
                else{
                    resetFadeMask();
                    state = STATE_TPHOTO_MONO_INFO_IN;
                    frameCounter = 0;
                    foregroundImage=&tPhotoMonoImg;
                    backgroundImage=&tPhotoMonoInfoImg;
                }

            }

        break;

        //--------------------------------------------------------------

        case STATE_TPHOTO_MONO_INFO_IN:
            //show taiwan PHOTO MONO INFO, fade in;
            PhotoCrossFade();

            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<tPhotoMonoInfoHoldTime) {
                    frameCounter++;
                }
                else{
                    resetFadeMask();
                    state = STATE_TPHOTO_INFO_IN;
                    frameCounter = 0;
                    foregroundImage=&tPhotoMonoInfoImg;
                    backgroundImage=&tPhotoInfoImg;
                }

            }
        break;

        //--------------------------------------------------------------

        case STATE_TPHOTO_INFO_IN:
            //show taiwan PHOTO MONO INFO, fade in;
            PhotoCrossFade();

            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<tPhotoInfoHoldTime) {
                    frameCounter++;
                }
                else{
                    state = STATE_TPHOTO_MONO_IN_2;
                    frameCounter = 0;
                    resetFadeMask();
                    backgroundImage=&tPhotoMonoImg;
                    foregroundImage=&tPhotoInfoImg;

                }

            }
        break;

        //--------------------------------------------------------------

        case STATE_TPHOTO_MONO_IN_2:
            //show taiwan PHOTO MONO INFO, fade in;
            PhotoCrossFade();
            
            if(fadeMaskAlpha<255){
                updateFadeMask(crossFadeSpeed);
            }else{
                if(frameCounter<tPhotoMono2HoldTime) {
                    frameCounter++;
                }
                else{
                    state = STATE_TPHOTO_GRID;
                    frameCounter = 0;
                    resetFadeMask();
                    backgroundImage=&kPhotoMonoInfoImg;
                    foregroundImage=&kPhotoImg;
                    
                    //preload next scene
                    tPhotoGridFbo.begin();
                    
                    ofPushMatrix();
                    ofTranslate(tGridPos.getCurrentPosition().x,tGridPos.getCurrentPosition().y);
                    tPhotoGridImg.draw(0,0,3892,3892);
                    tPhotoMonoImg.draw(allGridLocators[selectedGridIndex].x,allGridLocators[selectedGridIndex].y,1080,1080);
                    
                    ofPopMatrix();
                    tPhotoGridFbo.end();
                }
                
            }
        break;
       
        //--------------------------------------------------------------

        case STATE_TPHOTO_GRID:
            if(!tGridPos.hasFinishedAnimating()){
                float dt = 1.0f / 60.0f;
                tGridPos.update( dt );
                tPhotoGridSizeScale.update( dt );
                for(int i = 0; i<8;i++){
                    tGridPhotoPos[i].update( dt );
                }
                
                gridSize = 3892*(float)tPhotoGridSizeScale;
                photoSize = 1080*(float)tPhotoGridSizeScale;
                
                tPhotoGridFbo.begin();
                ofPushMatrix();
                ofTranslate(tGridPos.getCurrentPosition().x,tGridPos.getCurrentPosition().y);

                tPhotoGridImg.draw(0,0,gridSize,gridSize);
                
                // draw other photos
                for(int i = 0; i < 8; i++ ){
                    if(i!=selectedGridIndex){
                        ofPoint loc = tGridPhotoPos[i].getCurrentPosition()*(float)tPhotoGridSizeScale;
                        tGridPhotos[i].draw(loc.x,loc.y,photoSize,photoSize);
                    }
                }
                
                tPhotoMonoImg.draw(allGridLocators[selectedGridIndex].x*(float)tPhotoGridSizeScale,allGridLocators[selectedGridIndex].y*(float)tPhotoGridSizeScale,photoSize,photoSize);

                ofPopMatrix();

                tPhotoGridFbo.end();

                
            }else{
                if(frameCounter<tColorNameHoldTime){
                    frameCounter++;
                }else{
                    frameCounter = 0;
                    state = STATE_TCOLOR_FILL;
                }
            }

            
        break;
        
        //--------------------------------------------------------------

        case STATE_TCOLOR_FILL:
            //taiwan color fill;
            tPhotoGridFbo.begin();
            

            if(!animatedCircleSize.hasFinishedAnimating()){
                float dt = 1.0f / 60.0f;
                animatedCircleSize.update( dt );

                ofPushMatrix();
                ofPushStyle();
                ofEnableAlphaBlending();
                ofEnableSmoothing();

                ofTranslate(tPhotoGridFbo.getWidth()/2,tPhotoGridFbo.getHeight()/2);

                ofSetCircleResolution(60);

                ofSetColor(ofColor::white);
                ofDrawCircle(0,0,animatedCircleSize.val()+5);

                ofSetColor(allColorPickers[detectedColor-1]);
                ofDrawCircle(0,0,animatedCircleSize.val());

                ofDisableAlphaBlending();
                ofDisableSmoothing();

                ofPopStyle();
                ofPopMatrix();

            }else{
                ofClear(255,255,255,0);
                animatedCircleSize.animateFromTo(tPhotoGridFbo.getWidth()*0.01, tPhotoGridFbo.getWidth()*2);
               // resetGridAnimation();
                state = STATE_TCLOSING_IN;

            }
            tPhotoGridFbo.end();

        break;
            
        //--------------------------------------------------------------

        case STATE_TCLOSING_IN:
            //show tCOLOR NAME;
            if(tClosingAlpha>0){
                tClosingAlpha -= crossFadeSpeed;
            }else{
                if(frameCounter<tClosingHoldTime){
                    frameCounter++;
                }else{
                    frameCounter = 0;
                    tClosingAlpha = 255;
                    state = STATE_MAP;
                }
            }
           
        break;
            
        //--------------------------------------------------------------

        case STATE_MAP:
            //show LAST PHOTO OF PHOTO BOMB : LOGO AND WHITE BACKGROUND;
            if(frameCounter<mapHoldTime){
                frameCounter++;
            }else{
                frameCounter = 0;
                state = STATE_ENDING;
            }

        break;
        
        //--------------------------------------------------------------

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
                    serial.flush();

                    state = STATE_START;

                    ofLog()<<"starting over";
                }
            }
        break;


        default:
          break;
    }

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
            ofSetRectMode(OF_RECTMODE_CENTER);
           // searchingImg.draw(0,-kPhotoFbo.getHeight()*0.2,kPhotoFbo.getWidth()*0.15,kPhotoFbo.getWidth()*0.15*32/146);

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
        case STATE_TPHOTO_MONO_IN_2:
            //show taiwan photo info, FADE in;
            fadingFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
            
        break;
        //--------------------------------------------------------------
        case STATE_TPHOTO_GRID:
            //show taiwan photo info, FADE in;
            tPhotoGridFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);

            
        break;
        //--------------------------------------------------------------
        case STATE_TCOLOR_FILL:
            //show taiwan color;
            tPhotoGridFbo.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);


        break;
        //--------------------------------------------------------------
        case STATE_TCLOSING_IN:
            //show color name
            tClosingImg.draw((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
            
            ofPushStyle();
            ofSetColor(allColorPickers[detectedColor-1],tClosingAlpha);
            ofDrawRectangle((screenWidth-screenHeight)/2+canvasOffsetX,canvasOffsetY,screenHeight,screenHeight);
            ofPopStyle();


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
        ofDrawBitmapString(ofGetTimestampString("%c"),50,50);
        ofDrawBitmapString("Next Movie ID: "+ofToString(loadingMovieId), 50, 90);
        ofDrawBitmapString("Total Frames: "+ofToString(movies[nowPlayer].getTotalNumFrames()), 50, 120);
        ofDrawBitmapString("Current Frame: "+ofToString(movies[nowPlayer].getCurrentFrame()), 50, 150);
        ofDrawBitmapString("Detected color: "+ofToString(detectedColor), 50, 180);
        ofDrawBitmapString("selected TPhoto: "+ofToString(selectedTPhoto), 50, 210);
        ofDrawBitmapString("selected grid index: "+ofToString(selectedGridIndex), 50, 240);

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
            detectedColor = 1;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case '2':
            detectedColor = 2;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case '3':
            detectedColor = 3;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case '4':
            detectedColor = 4;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case '5':
            detectedColor = 5;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case '6':
            detectedColor = 6;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case '7':
            detectedColor = 7;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case '8':
            detectedColor = 8;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;

        case '9':
            detectedColor = 9;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
        break;
        case 'q':
            detectedColor = 10;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
            break;
        case 'w':
            detectedColor = 11;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
            break;
        case 'e':
            detectedColor = 12;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
            break;
        case 'a':
            detectedColor = 13;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
            break;
        case 's':
            detectedColor = 14;
            loadAssets(detectedColor);
            state = STATE_DETECTED;
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

    string timeOfDay;
    
    if (ofGetHours()<11) {
        timeOfDay = "morning";
    }else if(ofGetHours()<17){
        timeOfDay = "afternoon";
    }else{
        timeOfDay = "night";
    }

    // need to load the first movie : should only happen once
    if( !movies[nowPlayer].isLoaded() ){
        
        movies[nowPlayer].load( "movies/"+timeOfDay+"-"+ofToString(loadingMovieId) + ".mp4" );
        //todo:: check time and load morning, afternoon, night movies
    }

    movies[nowPlayer].setLoopState(OF_LOOP_NORMAL);
    movies[nowPlayer].play();

    ofLog()<<"playing: "<<timeOfDay<<"-"<<loadingMovieId<<".mp4";

    // PRELOAD the next movie

    // get a unique random ID
    int preloadId = rand()%totalMovies; //0~totalmovies-1(=2)
    
    while (preloadId == loadingMovieId ) {
        preloadId = rand()%totalMovies;
    }

    // set the loading movie to the new ID
    loadingMovieId = preloadId;

    // load movie in the player that isn't 'nowPlayer'
    int nextPlayer = nowPlayer == 0 ? 1 : 0;
    movies[nextPlayer].load( "movies/"+timeOfDay+"-"+ofToString(loadingMovieId) + ".mp4" );
    ofLog()<<"playing: "<<timeOfDay<<"-"<<loadingMovieId<<".mp4";

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
  if((currentFrame>0) && (totalFrames-currentFrame <20)){
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

        serial.writeByte('a');
    
        if(serial.available()){
            int myByte = 0;
            myByte=serial.readByte();
            
            if ( myByte == OF_SERIAL_NO_DATA ){
                //ofLog()<<"no data";
            }else if ( myByte == OF_SERIAL_ERROR ){
                ofLog()<<"error";
            }else if ( myByte > 14){
                ofLog()<<"no matching color";
            }else{
                ofLog()<<"detected color: "<< myByte;
                detectedColor = myByte;

                //      kPhoto.load("kPhotos/1.jpg");
                //      kPhotoMonoInfo.load("kPhotos/1-info.jpg");
                //      kColor.load("kColors/1.jpg");

                loadAssets(detectedColor);
                state = STATE_DETECTED;

            }
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

void ofApp::PhotoCrossFade(){
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
    frameCounter = 0;
}

//--------------------------------------------------------------


void ofApp::loadAssets(float _numOfColor){

    string colorNumber = ofToString(_numOfColor);

    kPhotoImg.load("kPhotos/kphoto-c"+colorNumber+".jpg");
    kPhotoMonoInfoImg.load("kPhotos/kphoto-c"+colorNumber+"-mono-info.jpg");
    kPhotoMonoImg.load("kPhotos/kphoto-c"+colorNumber+"-mono.jpg");
    kColorImg.load("kColors/kcolor-c"+colorNumber+".jpg");
    tColorImg.load("tColors/tcolor-c"+colorNumber+".jpg");

    
    selectedTPhoto = rand()%4+1; // 1~4
    //selectedTPhoto = 1; // 1~4

    string tPhotoSubNumber = ofToString(selectedTPhoto);
    //string tPhotoSubNumber = ofToString(1);

    tPhotoInfoImg.load("tPhotos/tphoto-c"+colorNumber+"-"+tPhotoSubNumber+"-info.jpg");
    tPhotoMonoImg.load("tPhotos/tphoto-c"+colorNumber+"-"+tPhotoSubNumber+"-mono.jpg");
    tPhotoMonoInfoImg.load("tPhotos/tphoto-c"+colorNumber+"-"+tPhotoSubNumber+"-mono-info.jpg");
    tClosingImg.load("tClosing/tClosing-c"+colorNumber+".jpg");
    
    // using combined color code for grid bg: ex: c1, c2 -> c1
    //int combinedColorNumber = ceil(_numOfColor/2);
    //ofLog()<<"combinedColorNumber: "<<combinedColorNumber;
    //tPhotoGridImg.load("tPhotoGrid/tPhotoGrid-c"+ofToString(combinedColorNumber)+".jpg");

    int combinedColorNumber = ceil(_numOfColor/2);
    //ofLog()<<"combinedColorNumber: "<<combinedColorNumber;
    tPhotoGridImg.load("tPhotoGrid/tPhotoGrid-c"+colorNumber+".jpg");

    ofLog()<<"detected Color: "<<_numOfColor<<" selectedTPhoto: "<<selectedTPhoto<<" selectedGridIndex: "<<selectedGridIndex;

    if(_numOfColor<combinedColorNumber*2){ // EX: 1 < ceil(1/2)*2=2
        for(int i = 0; i < 4; i ++){
            tGridPhotos[i].load("tPhotos/tphoto-c"+colorNumber+"-"+ofToString(i+1)+"-mono.jpg");
            // EX: C1-(1~4)
            tGridPhotos[i+4].load("tPhotos/tphoto-c"+ofToString(_numOfColor+1)+"-"+ofToString(i+1)+"-mono.jpg");
            // EX: C2-(1~4)
            
            ofLog()<<"tPhotos/tphoto-c"+colorNumber+"-"+ofToString(i+1)+"-mono.jpg";
            ofLog()<<"tPhotos/tphoto-c"+ofToString(_numOfColor+1)+"-"+ofToString(i+1)+"-mono.jpg";

        }
        selectedGridIndex = selectedTPhoto-1; //0 1 2 3

        
    }else{
        //EX: 2 !< ceil(2/2)*2=2
        for(int i = 0; i < 4; i ++){
            tGridPhotos[i].load("tPhotos/tphoto-c"+ofToString(_numOfColor-1)+"-"+ofToString(i+1)+"-mono.jpg");
            // EX: C1-(1~4)
            tGridPhotos[i+4].load("tPhotos/tphoto-c"+colorNumber+"-"+ofToString(i+1)+"-mono.jpg");
            // EX: C2-(1~4)

            ofLog()<<"tPhotos/tphoto-c"+ofToString(_numOfColor-1)+"-"+ofToString(i+1)+"-mono.jpg";
            ofLog()<<"tPhotos/tphoto-c"+colorNumber+"-"+ofToString(i+1)+"-mono.jpg";

        }
        
        selectedGridIndex = selectedTPhoto - 1 + 4; //4 5 6 7

    }
    
    resetGridAnimation();
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
    fadeMaskAlpha = 0;
    fadeMaskFbo.begin();
    ofClear(0,0,0,255);
    fadeMaskFbo.end();
}


//--------------------------------------------------------------

void ofApp::resetGridAnimation(){
    gridSize = 3892;
    photoSize = 1080;
    
    tGridPos.setPosition(ofPoint((-allGridLocators[selectedGridIndex].x),(-allGridLocators[selectedGridIndex].y)));
    tGridPos.animateTo(ofPoint(0,0));
    
    tPhotoGridSizeScale.animateFromTo(1, 1080/gridSize);
    
    tGridPos.setDuration(2);
    tPhotoGridSizeScale.setDuration(2);
    
    AnimCurve curve3 = (AnimCurve) (EASE_IN_EASE_OUT);
    
    tGridPos.setCurve( curve3 );
    tPhotoGridSizeScale.setCurve( curve3 );
    
    
    for(int i =0; i<8; i++){
        ofPoint loc = ofPoint(gridSize*cos(i*45),gridSize*sin(i*45));
        tGridPhotoPos[i].setPosition(loc);
        tGridPhotoPos[i].setDuration(2);
        tGridPhotoPos[i].setCurve(curve3);
        tGridPhotoPos[i].animateTo(ofPoint(allGridLocators[i].x,allGridLocators[i].y));
    }
}

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
