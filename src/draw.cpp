/* Copyright (C) 2015-2017 Michele Colledanchise - All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/



#include<draw.h>
#include<string>
#include<vector>

#include <X11/Xlib.h>

const float DEG2RAD = 3.14159/180.0;


BT::ControlNode* tree;
bool init = false;


void * font_array[3] = {GLUT_BITMAP_8_BY_13, GLUT_BITMAP_8_BY_13, GLUT_BITMAP_8_BY_13};
void * font = font_array[0];
float additional_spacing_array[10];
bool is_number_pressed_array[10];
unsigned char number_char[4] = {'0', '1', '2', '3'};


float x = 0.0;
float y = 0.4;
float x_offset = 0.01;
float y_offset = 0.15;
float r_color = 1;
float g_color = 1;
float b_color = 1;
GLfloat x_space = 0.06;

int depth;

double zoom = 1.0f;

float fraction = 0.1f;
float zoom_fraction = 0.1f;




void drawEllipse(float xpos, float ypos, float xradius, float yradius)
{
    glBegin(GL_LINE_LOOP);

    for (int i=0; i < 359; i++)
    {
        // convert degrees into radians
        float degInRad = i*DEG2RAD;
        glVertex2d(xpos+cos(degInRad)*xradius,  ypos + sin(degInRad)*yradius);
    }
    glEnd();
}

void drawString(void * font, const char *string, float x, float y, float z)
{
    renderBitmapString(x, y, font, string);
}

int compute_node_lines(const char *string)
{
    const char *c;
    int i = 0;
    int new_line_num = 1;
    glRasterPos2f(x, y);
    for (c=string; *c != '\0'; c++)
    {
        if ((*c == '\n') || ((*c == ' ' && i > 6) || i > 9))
        {
            new_line_num++;
            i = 0;
            continue;
        }
        i++;
    }
    return new_line_num;
}

int compute_max_width(const char *string)
{
    const char *current_char;
    int current_line_width = 0;
    int max_width = 0;

    glRasterPos2f(x, y);
    for (current_char = string; *current_char != '\0'; current_char++)
    {
        if ((*current_char == '\n') || ((*current_char == ' ' && current_line_width > 6) || current_line_width > 9))
        {
            if (current_line_width > max_width)
            {
                max_width = current_line_width;
            }
            current_line_width = 0;
            continue;
        }
        current_line_width++;
    }

    if (max_width == 0)  // if the lable fits in a single line
    {
        max_width = current_line_width;
    }
    return max_width;
}

void renderBitmapString(float x, float y, void *font, const char *string)
{
    const char *c;
    int i = 0;
    int new_line_num = 0;
    glRasterPos2f(x, y);
    for (c=string; *c != '\0'; c++)
    {
        if ((*c == '\n') || ((*c == ' ' && i > 6) || i > 9))
        {
            new_line_num++;
            glRasterPos2f(x, y - 0.025*(new_line_num));
            i = 0;
            continue;
        }
        i++;
        glutBitmapCharacter(font, *c);
    }
}



void draw_node(float x, float y, int node_type, const char *leafName, int status)
{
    float NODE_WIDTH = 0.04;
    float NODE_HEIGHT = 0.02;
    switch (node_type)
    {
    case BT::SELECTORSTAR:
        drawString(font, "?*", (x + NODE_WIDTH/2 -0.005), (y - NODE_HEIGHT/2), 0);
        break;
    case BT::SEQUENCESTAR:
        drawString(font, ">*", (x + NODE_WIDTH/2 -0.0051), (y - NODE_HEIGHT/2), 0);
        break;
    case BT::SELECTOR:
        drawString(font, "?", (x + NODE_WIDTH/2 -0.005), (y - NODE_HEIGHT/2), 0);
        break;
    case BT::SEQUENCE:
        drawString(font, ">", (x + NODE_WIDTH/2 -0.005), (y - NODE_HEIGHT/2), 0);
        break;
    case BT::PARALLEL:
        drawString(font, "=", (x + NODE_WIDTH/2 -0.005), (y - NODE_HEIGHT/2), 0);
        break;
    case BT::DECORATOR:
        drawString(font, "D", (x + NODE_WIDTH/2 -0.005), (y - NODE_HEIGHT/2), 0);
        break;
    case BT::ACTION:
    {
        NODE_HEIGHT = 0.02*(compute_node_lines(leafName));
        std::string st(leafName, 0, 15);
        NODE_WIDTH = 0.02*compute_max_width(leafName);
//            for (unsigned int i = 0; i < st.size(); i++)
//              NODE_WIDTH +=  0.01;
    }
    renderBitmapString((x + 0.015), (y - 0.01), font, leafName);
    // glColor3f(0.2, 1.0, 0.2);
    break;
    case BT::CONDITION:
    {
        NODE_HEIGHT = 0.02*compute_node_lines(leafName);
        std::string st(leafName, 0, 15);
        NODE_WIDTH = 0.02*compute_max_width(leafName);
    }
    renderBitmapString((x  + 2*0.015), (y - 0.01), font, leafName);
    break;
    default:
        break;
    }

    switch (status)
    {
    case BT::RUNNING:
        glColor3f(0.8, 0.8, 0.8);
        break;
    case BT::SUCCESS:
        glColor3f(0.0, 1.0, 0.0);
        break;
    case BT::FAILURE:
        glColor3f(1.0, 0.0, 0.0);
        break;
    case BT::IDLE:
        glColor3f(0.0, 0.0, 0.0);
        break;
    case BT::HALTED:
        glColor3f(0.0, 0.0, 0.0);
        break;
    default:
        break;
    }

    switch (node_type)
    {
    case BT::CONDITION:
        // drawEllipse(x,y,NODE_WIDTH,NODE_HEIGHT);
        // drawEllipse(x,y,0.1,0.021);
        glBegin(GL_LINE_LOOP);
        glVertex2f((GLfloat) (x + NODE_WIDTH), (GLfloat) (y - NODE_HEIGHT - 0.015));
        glVertex2f((GLfloat) (x + NODE_WIDTH), (GLfloat) (y + 0.02));
        glVertex2f((GLfloat) (x), (GLfloat) (y + 0.02));
        glVertex2f((GLfloat) (x), (GLfloat) (y - NODE_HEIGHT - 0.015));
        glColor3f(0.0, 0.0, 0.0);
        glEnd();
        break;

        break;
    case BT::ACTION:

        glBegin(GL_LINE_LOOP);
        glVertex2f((GLfloat) (x + NODE_WIDTH), (GLfloat) (y - NODE_HEIGHT - 0.015));
        glVertex2f((GLfloat) (x + NODE_WIDTH), (GLfloat) (y + 0.02));
        glVertex2f((GLfloat) (x), (GLfloat) (y + 0.02));
        glVertex2f((GLfloat) (x), (GLfloat) (y - NODE_HEIGHT - 0.015));
        glColor3f(0.0, 0.0, 0.0);
        glEnd();
        break;

    default:
        glBegin(GL_LINE_LOOP);
        glVertex2f((GLfloat) (x + NODE_WIDTH), (GLfloat) (y - NODE_HEIGHT));
        glVertex2f((GLfloat) (x + NODE_WIDTH), (GLfloat) (y + NODE_HEIGHT));
        glVertex2f((GLfloat) (x), (GLfloat) (y + NODE_HEIGHT));
        glVertex2f((GLfloat) (x), (GLfloat) (y - NODE_HEIGHT));
        glColor3f(0.0, 0.0, 0.0);
        glEnd();
        break;
    }
}

// draw the edge connecting one node to the other
void draw_edge(GLfloat parent_x, GLfloat parent_y,
               GLfloat parent_size, GLfloat child_x, GLfloat child_y, GLfloat child_size)
{
    glLineWidth(1.5);
    glColor3f(0.0, 0.0, 0.0);
    // GLfloat bottom_spacing = 0.1;  // commented-out as unused variable
    GLfloat above_spacing = 0.04;

    glBegin(GL_LINES);
    glVertex3f(parent_x, parent_y - parent_size, 0);
    glVertex3f(parent_x, child_y + child_size + above_spacing, 0);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(parent_x, child_y + child_size + above_spacing, 0);
    glVertex3f(child_x, child_y + child_size + above_spacing, 0);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(child_x, child_y + child_size + above_spacing, 0);
    glVertex3f(child_x, child_y+child_size, 0);
    glEnd();
}

// draw the edge connecting one node to the other
void draw_straight_edge(GLfloat parent_x, GLfloat parent_y,
                        GLfloat parent_size, GLfloat child_x, GLfloat child_y, GLfloat child_size)
{
    glLineWidth(1.5);
    glColor3f(0.0, 0.0, 0.0);

    glBegin(GL_LINES);
    glVertex3f(parent_x, parent_y-parent_size, 0.0);
    glVertex3f(child_x, child_y+child_size, 0);
    glEnd();
}

// Keyboard callback function ( called on keyboard event handling )
void keyboard(unsigned char key, int x, int y)
{
    for (int i = 1; i < 4; i++)
    {
        if (key == number_char[i])
        {
            is_number_pressed_array[i] = true;
        }
        else
        {
            is_number_pressed_array[i] = false;
        }
    }
}

void keyboard_release(unsigned char key, int x, int y)
{
    for (int i = 1; i < 4; i++)
    {
        if (key == number_char[i])
        {
            is_number_pressed_array[i] = false;
        }
    }
}


void drawCircle(float radius)
{
    glBegin(GL_LINE_LOOP);

    for (int i=0; i<= 360; i++)
    {
        float degInRad = i*3.14142/180;
        glVertex2f(cos(degInRad)*radius, sin(degInRad)*radius);
    }
    glEnd();
}


void updateTree(BT::TreeNode* tree, GLfloat x_pos, GLfloat y_pos, GLfloat y_offset, int depth )
{
    BT::ControlNode* d = dynamic_cast<BT::ControlNode*> (tree);
    if (d == NULL)
    {
        // if it is a leaf node, draw it
        draw_node(x_pos , (GLfloat) y_pos, tree->DrawType(), tree->get_name().c_str(), tree->get_color_status());
    }
    else
    {
        // if it is a control flow node, draw it and its children
        draw_node((GLfloat) x_pos, (GLfloat) y_pos, tree->DrawType(),
                  tree->get_name().c_str(), tree->get_color_status());
        std::vector<BT::TreeNode*> children = d->GetChildren();
        int M = d->GetChildrenNumber();
        std::vector<GLfloat> children_x_end;
        std::vector<GLfloat> children_x_middle_relative;

        GLfloat max_x_end = 0;
        // GLfloat max_x_start = 0;  // commented out as unused variable
        GLfloat current_x_end = 0;

        for (int i = 0; i < M; i++)
        {
            if (children[i]->DrawType() != BT::ACTION && children[i]->DrawType() != BT::CONDITION)
            {
                current_x_end = 0.04;
                children_x_middle_relative.push_back(0.02);
            }
            else
            {
                current_x_end = 0.02*compute_max_width(children[i]->get_name().c_str());
                children_x_middle_relative.push_back(current_x_end/2);
            }

            if (i < M-1)
            {
                max_x_end = max_x_end + current_x_end + x_space + additional_spacing_array[depth];
            }
            else
            {
                max_x_end = max_x_end + current_x_end;
            }
            children_x_end.push_back(max_x_end);
        }

        // GLfloat x_min = 0.0;  // commented-out as unused variable
        // GLfloat x_max = 0.0;  // commented-out as unused variable
        GLfloat x_shift = x_pos - max_x_end/2;
        // GLfloat x_shift_new = 0;  // commented-out as unused variable

        for (int i = 0; i < M; i++)
        {
            if (i > 0)
            {
                updateTree(children[i], x_shift + children_x_end.at(i - 1) , y_pos - y_offset  , y_offset, depth + 1);
                draw_edge(x_pos + 0.015, y_pos, 0.02,
                          x_shift + children_x_end.at(i-1) + children_x_middle_relative.at(i),
                          y_pos - y_offset, 0.02);
            }
            else
            {
                updateTree(children[i], x_shift , y_pos - y_offset  , y_offset, depth + 1);
                draw_edge(x_pos + 0.015, y_pos, 0.02,
                          x_shift + children_x_middle_relative.at(i), y_pos - y_offset, 0.02);
            }
        }
    }
}





void display()
{
    glClearColor(r_color, g_color, b_color, 0.1);    // clear the draw buffer .
    glClear(GL_COLOR_BUFFER_BIT);   // Erase everything
    updateTree(tree, x , y, y_offset, 1);
    glutSwapBuffers();
    glutPostRedisplay();
}


void processSpecialKeys(int key, int xx, int yy)
{
    switch (key)
    {
    case GLUT_KEY_UP :
        y +=  fraction;
        break;
    case GLUT_KEY_DOWN :
        if (y >= 0)
        {
            y -=  fraction;
        }
        break;
    case GLUT_KEY_LEFT:
        x -=  fraction;
        break;
    case GLUT_KEY_RIGHT:
        x +=  fraction;
        break;
    case  GLUT_KEY_PAGE_UP:
        for (int i = 1; i < 10; i++)
        {
            if (is_number_pressed_array[i])
            {
                additional_spacing_array[i] += fraction;
            }
        }
        break;
    case  GLUT_KEY_PAGE_DOWN:
        for (int i = 1; i < 10; i++)
        {
            if (is_number_pressed_array[i] && additional_spacing_array[i] >= 0 )
            {
                additional_spacing_array[i] -= fraction;
            }
        }
        break;
    case  GLUT_KEY_F1:
        if (r_color < 1)  r_color +=  fraction;
        break;
    case  GLUT_KEY_F2:
        if (r_color > 0) r_color -=  fraction;
        break;
    case  GLUT_KEY_F3:
        if (g_color < 1) g_color +=  fraction;
        break;
    case  GLUT_KEY_F4:
        if (g_color > 0) g_color -=  fraction;
        break;
    case  GLUT_KEY_F5:
        if (b_color < 1) b_color +=  fraction;
        break;
    case  GLUT_KEY_F6:
        if (b_color > 0) b_color -=  fraction;
        break;
    case GLUT_KEY_HOME:
        if (zoom < 1.0f)
        {
            glScalef(1.0f + zoom_fraction, 1.0f + zoom_fraction, 1.0f);
            zoom += zoom_fraction;
        }
        else
        {
            glScalef(1.0f, 1.0f, 1.0f);
        }
        break;
    case GLUT_KEY_END:
        glScalef(1.0f  - zoom_fraction, 1.0f  - zoom_fraction, 1.0f);
        zoom -= zoom_fraction;
        break;
    }
}

void drawTree(BT::ControlNode* tree_)
{
    /***************************BT VISUALIZATION****************************/
    int argc = 1;
    char *argv[1] = {const_cast<char*>("")};

    if (!init)
    {
        XInitThreads();
        glutInit(&argc, argv);
        init = true;
        glutInitDisplayMode(GLUT_DEPTH  | GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE );  //  Antialiasing
        glEnable(GL_MULTISAMPLE);
    }
    tree = tree_;
    depth = tree->Depth();

    glutInitWindowSize(1024, 860);

    glutCreateWindow("Behavior Tree");  // Create a window

    glClearColor(0, 0.71, 0.00, 0.1);
    glutDisplayFunc(display);  // Register display callback

    glutKeyboardFunc(keyboard);  // Register keyboard presscallback
    glutKeyboardUpFunc(keyboard_release);  // Register keyboard release callback

    glutSpecialFunc(processSpecialKeys);  // Register keyboard arrow callback

    glutMainLoop();             // Enter main event loop

    /***************************ENDOF BT VISUALIZATION ****************************/
}










