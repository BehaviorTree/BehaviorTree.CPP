#ifndef DRAWTREE_H
#define DRAWTREE_H
#include <cstdlib>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <math.h>
#include<iostream>
#include<string>
#include <control_node.h>

//enum status {RUNNING,SUCCESS,FAILURE,IDLE, HALTED };

void drawEllipse(float xradius, float yradius);

void drawTree(BT::ControlNode* tree_);

void resize(int width, int height);

void draw_status(float x, float y, int node_status);

void drawString (void * font, char *string, float x, float y, float z);

void renderBitmapString(float x, float y, void *font,const char *string);

void draw_node(float x, float y, int node_type, const char *leafName, int status);

void draw_edge(GLfloat parent_x, GLfloat parent_y, GLfloat parent_size, GLfloat child_x, GLfloat child_y, GLfloat child_size);

void keyboard(unsigned char key, int x, int y);

void drawCircle(float radius);

int compute_node_lines(const char *string);

int compute_max_width(const char *string);

//void display();

#endif // DRAWTREE_H
