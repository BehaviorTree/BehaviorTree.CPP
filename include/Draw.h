#ifndef DRAWTREE_H
#define DRAWTREE_H
#include <cstdlib>
#include <GL/glut.h>
#include <GL/glut.h>
#include <math.h>
#include<iostream>
#include<string>
#include <ControlNode.h>
//enum node { SEQUENCE, SELECTOR, PARALLEL,ACTION,CONDITION,ROOT };
enum status {SUCCESS,FAILURE,RUNNING,IDLE };
//enum Node {PARALLEL, SELECTOR, SEQUENCE, SEQUENCESTAR, SELECTORSTAR, ACTION, CONDITION};
void drawTree(BT::ControlNode* tree_);

void resize(int width, int height);

void draw_status(float x, float y, int node_status);

void drawString (void * font, char *s, float x, float y, float z);

void renderBitmapString(float x, float y, void *font,const char *string);

void draw_node(float x, float y, int node_type, const char *leafName, int status);

void draw_edge(GLfloat parent_x, GLfloat parent_y, GLfloat parent_size, GLfloat child_x, GLfloat child_y, GLfloat child_size);

void keyboard(unsigned char key, int x, int y);

void drawCircle(float radius);



//void display();

#endif // DRAWTREE_H
