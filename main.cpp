/* compiles with command line  gcc xlibdemo.c -lX11 -lm -L/usr/X11R6/lib */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include "point.h"
#include "lineSegment.h"
#include <vector>
using namespace std;

Display *display_ptr;
Screen *screen_ptr;
int screen_num;
char *display_name = NULL;
unsigned int display_width, display_height;

Window win;
int border_width;
unsigned int win_width, win_height;
int win_x, win_y;

XWMHints *wm_hints;
XClassHint *class_hints;
XSizeHints *size_hints;
XTextProperty win_name, icon_name;
char const *win_name_string = "Example Window";
char const *icon_name_string = "Icon for Example Window";

XEvent report;

GC gc, gc_yellow, gc_red, gc_grey;
unsigned long valuemask = 0;
XGCValues gc_values, gc_yellow_values, gc_red_values, gc_grey_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;


int timesClicked = 0;
bool flag = true;
point firstClick;
point secondClick;

int orientation(point p1 , point p2 , point p3);
int distance(point p1 , point p2);
bool onSegment(point p, point q, point r);
bool doIntersect(point p1, point q1, point p2, point q2);
bool shareCoordinate(point p1 , point p2 , point p3 , point p4);
void floyd();
void computePathMatrix();
void path(int q , int r);

vector<lineSegment> obstacles;                          //The list of all line segments.
vector<point> endPoints;                                //The list of all points.
vector<vector<int> > possiblePaths;                      //A matrix that tells you wether a path from point i to point point j is possible.
vector<vector<int> > shortestPathMatrix;                 //A matrix that lists the intermediate points on the shortest path from point i to point j.
vector<int> shortestPath;                               //The list of points on the shortest path from point i to point j.



int main(int argc, char **argv)
{
    /* opening display: basic connection to X Server */
    if( (display_ptr = XOpenDisplay(display_name)) == NULL )
    { printf("Could not open display. \n"); exit(-1);}
    printf("Connected to X server  %s\n", XDisplayName(display_name) );
    screen_num = DefaultScreen( display_ptr );
    screen_ptr = DefaultScreenOfDisplay( display_ptr );
    color_map  = XDefaultColormap( display_ptr, screen_num );
    display_width  = DisplayWidth( display_ptr, screen_num );
    display_height = DisplayHeight( display_ptr, screen_num );
    
    printf("Width %d, Height %d, Screen Number %d\n",
           display_width, display_height, screen_num);
    /* creating the window */
    border_width = 10;
    win_x = 0; win_y = 0;
    win_width = display_width/2;
    win_height = (int) (win_width / 1.7); /*rectangular window*/
    
    win= XCreateSimpleWindow( display_ptr, RootWindow( display_ptr, screen_num),
                             win_x, win_y, win_width, win_height, border_width,
                             BlackPixel(display_ptr, screen_num),
                             WhitePixel(display_ptr, screen_num) );
    /* now try to put it on screen, this needs cooperation of window manager */
    size_hints = XAllocSizeHints();
    wm_hints = XAllocWMHints();
    class_hints = XAllocClassHint();
    if( size_hints == NULL || wm_hints == NULL || class_hints == NULL )
    { printf("Error allocating memory for hints. \n"); exit(-1);}
    
    size_hints -> flags = PPosition | PSize | PMinSize  ;
    size_hints -> min_width = 60;
    size_hints -> min_height = 60;
    
    XStringListToTextProperty( (char**)&win_name_string,1,&win_name);
    XStringListToTextProperty( (char**)&icon_name_string,1,&icon_name);
    
    wm_hints -> flags = StateHint | InputHint ;
    wm_hints -> initial_state = NormalState;
    wm_hints -> input = False;
    
    class_hints -> res_name = (char*)"x_use_example";
    class_hints -> res_class =(char*)"examples";
    
    XSetWMProperties( display_ptr, win, &win_name, &icon_name, argv, argc,
                     size_hints, wm_hints, class_hints );
    
    /* what events do we want to receive */
    XSelectInput( display_ptr, win,
                 ExposureMask | StructureNotifyMask | ButtonPressMask );
    
    /* finally: put window on screen */
    XMapWindow( display_ptr, win );
    
    XFlush(display_ptr);
    
    /* create graphics context, so that we may draw in this window */
    gc = XCreateGC( display_ptr, win, valuemask, &gc_values);
    XSetForeground( display_ptr, gc, BlackPixel( display_ptr, screen_num ) );
    XSetLineAttributes( display_ptr, gc, 4, LineSolid, CapRound, JoinRound);
    /* and three other graphics contexts, to draw in yellow and red and grey*/
    gc_yellow = XCreateGC( display_ptr, win, valuemask, &gc_yellow_values);
    XSetLineAttributes(display_ptr, gc_yellow, 6, LineSolid,CapRound, JoinRound);
    if( XAllocNamedColor( display_ptr, color_map, "yellow",
                         &tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color yellow\n"); exit(-1);}
    else
        XSetForeground( display_ptr, gc_yellow, tmp_color1.pixel );
    gc_red = XCreateGC( display_ptr, win, valuemask, &gc_red_values);
    XSetLineAttributes( display_ptr, gc_red, 6, LineSolid, CapRound, JoinRound);
    if( XAllocNamedColor( display_ptr, color_map, "red",
                         &tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color red\n"); exit(-1);}
    else
        XSetForeground( display_ptr, gc_red, tmp_color1.pixel );
    gc_grey = XCreateGC( display_ptr, win, valuemask, &gc_grey_values);
    if( XAllocNamedColor( display_ptr, color_map, "light grey",
                         &tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color grey\n"); exit(-1);}
    else
        XSetForeground( display_ptr, gc_grey, tmp_color1.pixel );
    
    /* and now it starts: the event loop */
    while(1)
    { XNextEvent( display_ptr, &report );
        switch( report.type )
        {
            case Expose:{
            if (flag == true)
            {
                int i = 0;
                int a,b,c,d;
                point p1;
                point p2;
                lineSegment current;
                
                FILE *fp=fopen("/Users/raymondferranti/Desktop/obstacles","r");
                char linebuff[32];
                
                while(fgets(linebuff, sizeof(linebuff), fp)){
                    
                    sscanf(linebuff,"S (%d,%d) (%d,%d)",&a,&b,&c,&d);
                    p1.setCoordinates(a,b);
                    p2.setCoordinates(c, d);
                    current.setEndPoints(p1, p2);                               
                    obstacles.push_back(current);
                    lineSegment tmp = obstacles.at(i);
                    XDrawLine(display_ptr,win,gc,tmp.get_x1(),tmp.get_y1(),tmp.get_x2(),tmp.get_y2());
                    i++;
                }
                flag = false;

                fclose(fp);
                

            
                int j;
                point tmp;
                /* Store the endpoints of each line segments into a vector of points */
                for (j = 0 ; j<obstacles.size(); j++) {
                    tmp.setCoordinates(obstacles.at(j).getFirstEndpoint().getx(), obstacles.at(j).getFirstEndpoint().gety());
                    endPoints.push_back(tmp);
                    tmp.setCoordinates(obstacles.at(j).getsecondEndpoint().getx(), obstacles.at(j).getsecondEndpoint().gety());
                    endPoints.push_back(tmp);
                    
                }
              
            }
        }

                break;
            case ConfigureNotify:
                /* This event happens when the user changes the size of the window*/
                win_width = report.xconfigure.width;
                win_height = report.xconfigure.height;
                break;
            case ButtonPress:
                /* This event happens when the user pushes a mouse button. I draw
                 a circle to show the point where it happened, but do not save
                 the position; so when the next redraw event comes, these circles
                 disappear again. */
            {
                int x, y;
                if (timesClicked == 0 ) {
                    firstClick.setCoordinates(report.xbutton.x, report.xbutton.y);
                    endPoints.insert(endPoints.begin(), firstClick);

                }
                if (timesClicked == 1 ) {
                    secondClick.setCoordinates(report.xbutton.x, report.xbutton.y);
                    endPoints.push_back(secondClick);

                    computePathMatrix();
                    
                    floyd();

                    path(0, (int)endPoints.size()-1);
                    if (shortestPath.size() == 0) {
                        XDrawLine(display_ptr, win, gc_red, endPoints[0].getx(), endPoints[0].gety(), endPoints[endPoints.size()-1].getx(), endPoints[endPoints.size()-1].gety());
                    }
                    else if(shortestPath.size()==1){
                        
                        XDrawLine(display_ptr, win, gc_red, endPoints[0].getx(), endPoints[0].gety(), endPoints[shortestPath[0]].getx(), endPoints[shortestPath[0]].gety());
                        XDrawLine(display_ptr, win, gc_red, endPoints[endPoints.size()-1].getx(), endPoints[endPoints.size()-1].gety(),
                                  endPoints[shortestPath[shortestPath.size()-1]].getx() , endPoints[shortestPath[shortestPath.size()-1]].gety());
                    }
                    else if(shortestPath.size() == 2){
                        XDrawLine(display_ptr, win, gc_red, endPoints[shortestPath[0]].getx(), endPoints[shortestPath[0]].gety(),
                                  endPoints[shortestPath[1]].getx(), endPoints[shortestPath[1]].gety());
                        XDrawLine(display_ptr, win, gc_red, endPoints[0].getx(), endPoints[0].gety(), endPoints[shortestPath[0]].getx(), endPoints[shortestPath[0]].gety());
                        XDrawLine(display_ptr, win, gc_red, endPoints[endPoints.size()-1].getx(), endPoints[endPoints.size()-1].gety(),
                                  endPoints[shortestPath[shortestPath.size()-1]].getx() , endPoints[shortestPath[shortestPath.size()-1]].gety());
                    }
                    else if (shortestPath.size()>=3){

                        for (int i = 0; i<=shortestPath.size()-2; i++) {
                            
                            XDrawLine(display_ptr, win, gc_red, endPoints[shortestPath[i]].getx(), endPoints[shortestPath[i]].gety(),
                                      endPoints[shortestPath[i+1]].getx(), endPoints[shortestPath[i+1]].gety());
                        }
                        XDrawLine(display_ptr, win, gc_red, endPoints[0].getx(), endPoints[0].gety(), endPoints[shortestPath[0]].getx(), endPoints[shortestPath[0]].gety());
                        XDrawLine(display_ptr, win, gc_red, endPoints[endPoints.size()-1].getx(), endPoints[endPoints.size()-1].gety(),
                                  endPoints[shortestPath[shortestPath.size()-1]].getx() , endPoints[shortestPath[shortestPath.size()-1]].gety());
                    }
                    timesClicked = -1;
                    possiblePaths.clear();
                    shortestPathMatrix.clear();
                    shortestPath.clear();
                }
                timesClicked++;
                x = report.xbutton.x;
                y = report.xbutton.y;
                if (report.xbutton.button == Button1 )
                    XFillArc( display_ptr, win, gc_red,
                             x -win_height/40, y- win_height/40,
                             win_height/20, win_height/20, 0, 360*64);
                else
                    exit(0);
                
            }
                break;
            default:
                /* this is a catch-all for other events; it does not do anything.
                 One could look at the report type to see what the event was */
                break;
        }
        
    }
}

int orientation(point p1 , point p2 , point p3){
    int val = ( (p2.gety() - p1.gety()) * (p3.getx() - p2.getx()) - (p2.getx() - p1.getx()) * (p3.gety() - p2.gety()));
    if( val == 0)
        return 0; // If val == 0 then the three points are colinnear.
    
    if ( val > 0){
        return 1;
    }
    else
        return 2;
}

bool onSegment(point p, point q, point r)
{
    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
        q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
        return true;
    
    return false;
}

bool doIntersect(point p1, point q1, point p2, point q2)
{
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);
    
    // General case
    if (o1 != o2 && o3 != o4)
        return true;
    
    // Special Cases
    // p1, q1 and p2 are colinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;
    
    // p1, q1 and p2 are colinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;
    
    // p2, q2 and p1 are colinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;
    
    // p2, q2 and q1 are colinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;
    
    return false; // Doesn't fall in any of the above cases
}


int distance(point p1 , point p2){
    int distance = sqrt((p2.getx()-p1.getx()) * (p2.getx()-p1.getx()) + (p2.gety()-p1.gety()) * (p2.gety()-p1.gety()));
    return distance;
}

bool shareCoordinate(point p1 , point p2 , point p3 , point p4){
    if (((p1.getx() == p2.getx()) && (p1.gety() == p2.gety())) ||
        ((p1.getx() == p3.getx()) && (p1.gety() == p3.gety())) ||
        ((p1.getx() == p4.getx()) && (p1.gety() == p4.gety())) ||
        ((p2.getx() == p3.getx()) && (p2.gety() == p3.gety())) ||
        ((p2.getx() == p4.getx()) && (p2.gety() == p4.gety())) ||
        ((p3.getx() == p4.getx()) && (p3.gety() == p4.gety()))) {
            return true;
    }
    else
        return false;
}


void path(int q , int r){

    if (shortestPathMatrix[q][r] != 0) {
        path(q, shortestPathMatrix[q][r]);
        shortestPath.push_back(shortestPathMatrix[q][r]);
        path(shortestPathMatrix[q][r], r);
    }
}
void floyd(){
    int k,i,j ;
    int intermediate1 , intermediate2;
    vector<int> tmp;
    bool flag = false;
    
    for (i = 0 ; i <possiblePaths.size(); i++) {
        for (j = 0; j <possiblePaths.size(); j++) {
            tmp.push_back(0);
        }
        shortestPathMatrix.push_back(tmp);
    }
    for (k = 0 ; k <possiblePaths.size() ; k++) {
        for (i = 0; i<possiblePaths.size(); i++) {
            for (j = 0; j<possiblePaths.size(); j++) {
                if ((i == j) && j!=possiblePaths.size()-1)j++;
                intermediate1 = possiblePaths.at(i).at(k);
                intermediate2 = possiblePaths.at(k).at(j);
                while ((intermediate1 == 0 || intermediate2 == 0) || (i==j)) {
                    if (j == possiblePaths.size()-1) {
                        flag = true;
                        break;
                    }
                    j++;
                    intermediate1 = possiblePaths.at(i).at(k);
                    intermediate2 = possiblePaths.at(k).at(j);
                }
                if(flag == true)break;
                if (possiblePaths[i][j] == 0){
                    shortestPathMatrix[i][j] = k;
                    possiblePaths[i][j] = possiblePaths[i][k] + possiblePaths[k][j];

                }
                else if (possiblePaths[i][j] > possiblePaths[i][k] + possiblePaths[k][j]) {
                        shortestPathMatrix[i][j] = k;
                        possiblePaths[i][j] = possiblePaths[i][k] + possiblePaths[k][j];
                }
            
            }
            flag = false;
        }
    }
}


void computePathMatrix(){  //This function if there is a path from point i to point j. This will prune the search space.
    int i;
    int j;
    int k;
    vector<int> tmp;
    bool flag1 = false;
    bool flag2 = false;
    bool intersects ;
    for (i = 0 ; i<endPoints.size() ; i++) {
        for (j = 0 ; j<endPoints.size() ; j++) {
            if ( i == j ){
                tmp.push_back(0); //0 signifys that a path from i to j is not valid. There is no path from point i to itself.

                if (j == endPoints.size()-1) {
                    flag1 = true;
                    break;
                }
                else
                    j++;
            }
            intersects = false;
            for (k = 0 ; k<obstacles.size(); k++) {
                while (shareCoordinate(endPoints.at(i), endPoints.at(j), obstacles.at(k).getFirstEndpoint(), obstacles.at(k).getsecondEndpoint())) {
                    if(k == obstacles.size()-1){
                        flag2 = true;
                        break;
                    }
                    else
                        k++;
                }
                if (flag2 == true) break;
                intersects = doIntersect(endPoints.at(i), endPoints.at(j), obstacles.at(k).getFirstEndpoint(), obstacles.at(k).getsecondEndpoint());
                
                if (intersects == true) {
                    break;
                }
            }
            if (intersects == true) {
                tmp.push_back(0); //There is no path from i to j.
            }
            else{
                tmp.push_back(distance(endPoints.at(i), endPoints.at(j)));   // There is a path from i to j so store a one at the ith, jth spot
            }
            flag2 = false;
        }
        
        possiblePaths.push_back(tmp);
        tmp.clear();
        intersects = false;
        if (flag1 == true) break;

    }

}






