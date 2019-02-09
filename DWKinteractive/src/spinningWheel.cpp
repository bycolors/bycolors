//
//  spinningWheel.cpp
//  taiwanColorFramework
//
//  Created by KJ Wu on 11/6/18.
//
//

#include "spinningWheel.h"
SpinningWheel::SpinningWheel(){
}

void SpinningWheel::setup(float _or, float _ir, float _sd){
    colorWheel[0]=ofColor::fromHex(0xFDE8E9);
    colorWheel[1]=ofColor::fromHex(0xEF95B8);
    colorWheel[2]=ofColor::fromHex(0xE5603B);
    colorWheel[3]=ofColor::fromHex(0xDE7C6B);
    colorWheel[4]=ofColor::fromHex(0xDA9E3C);
    colorWheel[5]=ofColor::fromHex(0xFFCE7B);
    colorWheel[6]=ofColor::fromHex(0x73A533);
    colorWheel[7]=ofColor::fromHex(0x4A5D37);
    colorWheel[8]=ofColor::fromHex(0x6AC9C8);
    colorWheel[9]=ofColor::fromHex(0x7BBFEA);
    colorWheel[10]=ofColor::fromHex(0x00B7CE);
    colorWheel[11]=ofColor::fromHex(0x3A6097);
    colorWheel[12]=ofColor::fromHex(0x61584C);
    colorWheel[13]=ofColor::fromHex(0x9D9087);
    colorWheel[14]=ofColor::fromHex(0x9FA4AE);
    colorWheel[15]=ofColor::fromHex(0x94B0BE);
    
    ofBackground(255, 255, 255);
    ofEnableSmoothing();
    
    sections = 16;
    outterR = _or;
    innerR = _ir;
    
    rotateSpeed = _sd;
    angle = 0;
    
    index = 0;
    mode = 1;
    times = 0;

}

void SpinningWheel::update(){
    if(angle<360){
        angle+=rotateSpeed;
        times++;

    }else{
        angle = 0;
    }
    
    if(index>16){
        index = 0;
    }else{
        index++;
    }
}

void SpinningWheel::draw(float _x, float _y){
    center = ofPoint(_x,_y);
    if(mode == 1){
        for(int i = 0; i < sections; i++){
            ofPath curve2;
            curve2.arc(center, outterR, outterR, i*360/sections+angle, (i+1)*360/sections+angle);
            curve2.arcNegative(center, innerR, innerR, (i+1)*360/sections+angle, i*360/sections+angle);
            curve2.close();
            curve2.setCircleResolution(60);
            ofColor c(255, i*20,0);
            curve2.setFillColor(colorWheel[i]);
            curve2.setFilled(true);
            curve2.draw();
        }
    }else if(mode == 2){
        for(int i = 0; i < sections; i++){
            ofPath curve2;
            if(i == index){
                curve2.arc(center, outterR+10, outterR+10, i*360/sections-5, (i+1)*360/sections+5);
                curve2.arcNegative(center, innerR-10, innerR-10, (i+1)*360/sections+5, i*360/sections-5);
            }else{
                curve2.arc(center, outterR, outterR, i*360/sections, (i+1)*360/sections);
                curve2.arcNegative(center, innerR, innerR, (i+1)*360/sections, i*360/sections);
            }
            curve2.close();
            curve2.setCircleResolution(60);
            ofColor c(255, i*20,0);
            curve2.setFillColor(colorWheel[i]);
            curve2.setFilled(true);
            curve2.draw();
        }
    }
    
}

void SpinningWheel::resetTimes(){
    times = 0;
}
