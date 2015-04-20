//
//  point.cpp
//  shortest
//
//  Created by raymond ferranti on 10/7/14.
//  Copyright (c) 2014 raymond ferranti. All rights reserved.
//

#include "point.h"


int point::getx(){
    return x;
}

int point::gety(){
    return y;
}

void point::setCoordinates(int x , int y){
    this->x = x;
    this->y = y;
}

point::point(int  x , int y) : x(x), y(y){
   
}


