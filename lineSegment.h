//
//  lineSegment.h
//  shortest
//
//  Created by raymond ferranti on 10/7/14.
//  Copyright (c) 2014 raymond ferranti. All rights reserved.
//

#ifndef __shortest__lineSegment__
#define __shortest__lineSegment__

#include <stdio.h>
#include "point.h"

class lineSegment{
public:
    point p1, p2;
    int lineLength;
    int length();
    void setEndPoints(point p1 , point p2);
    void printEndPoints();
    point getFirstEndpoint();
    point getsecondEndpoint();
    int get_x1();
    int get_x2();
    int get_y1();
    int get_y2();

    lineSegment(point p1 , point p2);
    lineSegment(){};
    
    
};


#endif /* defined(__shortest__lineSegment__) */

