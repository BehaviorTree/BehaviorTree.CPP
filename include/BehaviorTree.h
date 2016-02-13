#ifndef BEHAVIORTREE_H
#define BEHAVIORTREE_H
#include <Draw.h>

#include <ParallelNode.h>
#include <SelectorNode.h>
#include <SequenceNode.h>


#include <SelectorStarNode.h>
#include <SequenceStarNode.h>
#include <DecoratorRetryNode.h>
#include <DecoratorNegationNode.h>

/* System idependent includes */
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif
/******************************/


#include <Exceptions.h>

#include <string>
#include <map>

#include <typeinfo>
#include <math.h>       /* pow */


void Execute(BT::ControlNode* root,int TickPeriod_milliseconds);


#endif
