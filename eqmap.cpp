#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

//#include <gl/gl.h>
//#include <gl/glu.h>
//#include "gl/glut.h"
#include "gl/freeglut.h"

std::string ini_file = "eqmap.ini";

int window_id = 0;

int window_width  = 640;
int window_height = 480;

float map_origin_x = window_width  / 2;
float map_origin_y = window_height / 2;

float MAP_ZOOM_DEFAULT = 1.0;

float MAP_ZOOM_MIN = 0.1;

float map_zoom = MAP_ZOOM_DEFAULT;

float map_zoom_multiplier = 0.25;

float map_offset_x = 0;
float map_offset_y = 0;

float map_offset_multiplier = 10.0;

float map_draw_x = 0;
float map_draw_y = 0;

void *font_name = GLUT_BITMAP_HELVETICA_10;

int font_size = 10;

int num_lines    = 0;
int num_lines_ex = 0;

int num_points    = 0;
int num_points_ex = 0;

struct map_line_t
{
    float from_x;
    float from_y;
    float from_z;

    float to_x;
    float to_y;
    float to_z;

    int r;
    int g;
    int b;
};

std::vector<map_line_t> map_lines;
std::vector<map_line_t>::iterator map_lines_it;

struct map_point_t
{
    float x;
    float y;
    float z;

    int r;
    int g;
    int b;

    int size;

    std::string text;
};

std::vector<map_point_t> map_points;
std::vector<map_point_t>::iterator map_points_it;

float calculate_distance(float x1, float y1, float x2, float y2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void draw_bitmap_characters(float x, float y, void *font, std::string text)
{
    glRasterPos2f(x, y);

    for (unsigned int i = 0; i < text.size(); i++)
    {
        glutBitmapCharacter(font, text[i]);
    }
}

void draw_bitmap_string(float x, float y, void *font, std::string text)
{
    glRasterPos2f(x, y);

    glutBitmapString(font, (const unsigned char*)text.c_str());
}

void map_center()
{
    map_offset_x = 0;
    map_offset_y = 0;
}

void map_scroll_up()
{
    map_offset_y += map_offset_multiplier * map_zoom;
}

void map_scroll_down()
{
    map_offset_y -= map_offset_multiplier * map_zoom;
}

void map_scroll_left()
{
    map_offset_x += map_offset_multiplier * map_zoom;
}

void map_scroll_right()
{
    map_offset_x -= map_offset_multiplier * map_zoom;
}

void map_zoom_reset()
{
    map_zoom = MAP_ZOOM_DEFAULT;
}

void map_zoom_in()
{
    map_zoom -= map_zoom_multiplier * map_zoom;
}

void map_zoom_out()
{
    map_zoom += map_zoom_multiplier * map_zoom;
}

void parse()
{
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(ini_file, pt);
    std::string map_filename = pt.get<std::string>("Map.Filename");

    std::stringstream buffer;
    buffer << map_filename << " - eqmap";

    glutSetWindowTitle(buffer.str().c_str());

    std::fstream file;
    file.open(map_filename.c_str(), std::ios::in);

    std::string line;
    std::vector<std::string> line_data;

    if (file.is_open())
    {
        while (file.good())
        {
            std::getline(file, line);

            boost::replace_all(line, " ", "");

            if (!line.size())
            {
                continue;
            }


            char line_type = line.at(0);

            line = line.substr(1);

            boost::split(line_data, line, boost::is_any_of(","));

            if (!line_data.size())
            {
                continue;
            }

            foreach (std::string line_data_value, line_data)
            {
                //std::cout << "line_data_value: " << line_data_value << std::endl;
            }

            if (line_type == 'L')
            {
                map_line_t map_line;

                map_line.from_x = boost::lexical_cast<float>(line_data.at(0));
                map_line.from_y = boost::lexical_cast<float>(line_data.at(1));
                map_line.from_z = boost::lexical_cast<float>(line_data.at(2));

                map_line.to_x = boost::lexical_cast<float>(line_data.at(3));
                map_line.to_y = boost::lexical_cast<float>(line_data.at(4));
                map_line.to_z = boost::lexical_cast<float>(line_data.at(5));

                map_line.r = boost::lexical_cast<int>(line_data.at(6));
                map_line.g = boost::lexical_cast<int>(line_data.at(7));
                map_line.b = boost::lexical_cast<int>(line_data.at(8));

                map_lines.push_back(map_line);
            }

            if (line_type == 'P')
            {
                map_point_t map_point;

                map_point.x = boost::lexical_cast<float>(line_data.at(0));
                map_point.y = boost::lexical_cast<float>(line_data.at(1));
                map_point.z = boost::lexical_cast<float>(line_data.at(2));

                map_point.r = boost::lexical_cast<int>(line_data.at(3));
                map_point.g = boost::lexical_cast<int>(line_data.at(4));
                map_point.b = boost::lexical_cast<int>(line_data.at(5));

                map_point.size = boost::lexical_cast<int>(line_data.at(6));

                map_point.text = line_data.at(7);

                boost::replace_all(map_point.text, "_", " ");

                map_points.push_back(map_point);
            }
        }
    }

    foreach (map_line_t map_line, map_lines)
    {
        //std::cout << "r" << map_line.r << "g" << map_line.g << "b" << map_line.b << std::endl;
    }

    file.close();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27: // Escape
            glutDestroyWindow(window_id);
            exit(0);
            break;

        case 8: // Backspace
            glutFullScreenToggle();
            break;

        case 32: // Space
            map_zoom_reset();
            map_center();
            break;

        case 56: // Numpad 8
            map_scroll_up();
            break;

        case 50: // Numpad 2
            map_scroll_down();
            break;

        case 52: // Numpad 4
            map_scroll_left();
            break;

        case 54: // Numpad 6
            map_scroll_right();
            break;

        case 53: // Numpad 5
            map_center();
            break;

        case 13: // Enter
            map_zoom_reset();
            break;

        case 43: // Numpad Add
        case 61: // = +
        case 93: // } ]
            map_zoom_in();
            break;

        case 45: // Numpad Subtract or - _
        case 91: // { [
            map_zoom_out();
            break;
    }
}

void hotkeys(int key, int x, int y)
{
    switch (key)
    {
        case GLUT_KEY_F11:
            glutFullScreenToggle();
            break;

        case GLUT_KEY_UP:
            map_scroll_up();
            break;

        case GLUT_KEY_DOWN:
            map_scroll_down();
            break;

        case GLUT_KEY_LEFT:
            map_scroll_left();
            break;

        case GLUT_KEY_RIGHT:
            map_scroll_right();
            break;

        case GLUT_KEY_HOME:
            map_center();
            break;

        case GLUT_KEY_END:
            map_zoom_reset();
            break;

        case GLUT_KEY_PAGE_UP:
            map_zoom_in();
            break;

        case GLUT_KEY_PAGE_DOWN:
            map_zoom_out();
            break;
    }  
}

void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP)
    {
        map_zoom_reset();
    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
    {
        map_offset_x = map_offset_x + (map_origin_x - x) * map_zoom;
        map_offset_y = map_offset_y + (map_origin_y - y) * map_zoom;
    }
}

void mouse_wheel(int button, int direction, int x, int y)
{
    if (direction > 0)
    {
        map_zoom_in();
    }
    else
    {
        map_zoom_out();
    }
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);

    window_width  = w;
    window_height = h;

    map_origin_x = window_width  / 2;
    map_origin_y = window_height / 2;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, window_width, window_height, 0, 0, 1);
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //glTranslatef(0.375, 0.375, 0);

    glBegin(GL_LINES);

    num_lines = 0;

    foreach (map_line_t map_line, map_lines)
    {
        if (map_line.r == 0 && map_line.g == 0 && map_line.b == 0)
        {
            map_line.r = 255;
            map_line.g = 255;
            map_line.b = 255;
        }

        glColor3ub(map_line.r, map_line.g, map_line.b);

        float line_start_x = map_line.from_x;
        float line_start_y = map_line.from_y;

        float line_stop_x  = map_line.to_x;
        float line_stop_y  = map_line.to_y;

        float line_map_start_x = ((line_start_x / map_zoom) + map_origin_x) + ((map_draw_x + map_offset_x) / map_zoom);
        float line_map_start_y = ((line_start_y / map_zoom) + map_origin_y) + ((map_draw_y + map_offset_y) / map_zoom);

        float line_map_stop_x = ((line_stop_x / map_zoom) + map_origin_x) + ((map_draw_x + map_offset_x) / map_zoom);
        float line_map_stop_y = ((line_stop_y / map_zoom) + map_origin_y) + ((map_draw_y + map_offset_y) / map_zoom);

        float line_map_distance_start = calculate_distance(map_origin_x, map_origin_y, line_map_start_x, line_map_start_y);
        float line_map_distance_stop  = calculate_distance(map_origin_x, map_origin_y, line_map_stop_x,  line_map_stop_y);

        if (line_map_distance_start > map_origin_x && line_map_distance_stop > map_origin_x && line_map_distance_start > map_origin_y && line_map_distance_stop > map_origin_y)
        {
            continue;
        }

        glVertex2f(line_map_start_x, line_map_start_y);
        glVertex2f(line_map_stop_x,  line_map_stop_y);

        num_lines++;
    }

    num_lines_ex = num_lines;

    glEnd();

    num_points = 0;

    foreach (map_point_t map_point, map_points)
    {
        if (map_point.r == 0 && map_point.g == 0 && map_point.b == 0)
        {
            map_point.r = 255;
            map_point.g = 255;
            map_point.b = 255;
        }

        glColor3ub(map_point.r, map_point.g, map_point.b);

        float point_map_x = ((map_point.x / map_zoom) + map_origin_x) + ((map_draw_x + map_offset_x) / map_zoom);
        float point_map_y = ((map_point.y / map_zoom) + map_origin_y) + ((map_draw_y + map_offset_y) / map_zoom);

        float point_map_distance = calculate_distance(map_origin_x, map_origin_y, point_map_x, point_map_y);

        if (point_map_distance > map_origin_x && point_map_distance > map_origin_x)
        {
            continue;
        }

        glBegin(GL_LINES);
            glVertex2f(point_map_x, point_map_y - 4);
            glVertex2f(point_map_x, point_map_y + 5);

            glVertex2f(point_map_x - 4, point_map_y);
            glVertex2f(point_map_x + 5, point_map_y);
        glEnd();

        draw_bitmap_string(point_map_x, point_map_y + (font_size * 1.5), font_name, map_point.text);

        num_points++;
    }

    num_points_ex = num_points;

    glColor3f(1.0, 0, 1.0);

    std::stringstream buffer;

    buffer << "Resolution: " << window_width << "x" << window_height;
    draw_bitmap_string(0, 0 + (font_size * 1), font_name, buffer.str());
    buffer.str("");

    buffer << "Zoom: " << map_zoom;
    draw_bitmap_string(0, 0 + (font_size * 2), font_name, buffer.str());
    buffer.str("");

    buffer << "Lines: " << num_lines_ex;
    draw_bitmap_string(0, 0 + (font_size * 3), font_name, buffer.str());
    buffer.str("");

    buffer << "Points: " << num_points_ex;
    draw_bitmap_string(0, 0 + (font_size * 4), font_name, buffer.str());
    buffer.str("");

    float origin_map_x = map_origin_x + (map_offset_x / map_zoom);
    float origin_map_y = map_origin_y + (map_offset_y / map_zoom);

    draw_bitmap_string(origin_map_x, origin_map_y + font_size, font_name, "Origin (0, 0)");

    glutSwapBuffers();
}

void init()
{
    glDisable(GL_TEXTURE_2D);

    glDisable(GL_DEPTH_TEST);

    glDisable(GL_BLEND);

    glFrontFace(GL_CW);

    glClearColor(0, 0, 0, 0);

    glViewport(0, 0, window_width, window_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, window_width, window_height, 0, 0, 1);
}

int main(int argc, char** argv) 
{
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(ini_file, pt);
    window_width  = pt.get<int>("Window.Width");
    window_height = pt.get<int>("Window.Height");

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - window_width) / 2, (glutGet(GLUT_SCREEN_HEIGHT) - window_height) / 2);

    window_id = glutCreateWindow("eqmap");

    glutDisplayFunc(render);
    glutIdleFunc(render);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(hotkeys);
    glutMouseFunc(mouse);
    glutMouseWheelFunc(mouse_wheel);

    init();
    parse();

    glutMainLoop();

    return 0;
}