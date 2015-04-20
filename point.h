//
//  point.h
//  shortest
//
//  Created by raymond ferranti on 10/7/14.
//  Copyright (c) 2014 raymond ferranti. All rights reserved.
//

#ifndef __shortest__point__
#define __shortest__point__

#include <stdio.h>
class point{
    
public:
    int x;
    int y;
    int getx();
    int gety();
    void setCoordinates(int x , int y);
    point(int x , int y);
    point(){};
    
};

#endif /* defined(__shortest__point__) */




