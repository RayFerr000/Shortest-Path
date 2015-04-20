//
//  lineSegment.cpp
//  shortest
//
//  Created by raymond ferranti on 10/7/14.
//  Copyright (c) 2014 raymond ferranti. All rights reserved.
//
#include "lineSegment.h"
#include <math.h>
#include <iostream>

using namespace std;
lineSegment::lineSegment(point p1 , point p2){
    this -> p1 = p1;
    this -> p2 = p2;


}

void lineSegment::setEndPoints(point p1 , point p2){
    this -> p1 = p1;
    this -> p2 = p2;
}

void lineSegment::printEndPoints(){
    cout<<p1.getx()<< " "<<p1.gety()<<" "<<p2.getx()<<" "<<p2.gety()<<endl;
}

int lineSegment::get_x1(){
    return p1.getx();
}
int lineSegment::get_y1(){
    return p1.gety();
}
int lineSegment::get_x2(){
    return p2.getx();
}
int lineSegment::get_y2(){
    return p2.gety();
}

point lineSegment::getFirstEndpoint(){
    return p1;
}

point lineSegment::getsecondEndpoint(){
    return p2;
}

int lineSegment::length(){
    lineLength =  sqrt((p2.getx()-p1.getx()) * (p2.getx()-p1.getx()) + (p2.gety()-p1.gety()) * (p2.gety()-p1.gety()));
    return lineLength;
}

