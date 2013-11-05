#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm/replace_if.hpp>
#include <boost/range/algorithm/remove_if.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

//#include <gl/gl.h>
//#include <gl/glu.h>
//#include "gl/glut.h"
#include "gl/freeglut.h"

#include <AntTweakBar.h>

#include <windows.h>
#include <shellapi.h>

#define APPLICATION_NAME "eqmap"

std::string ini_file = "eqmap.ini";

std::string zones_file = "zones.ini";

std::string zones_search = "";

float frames_per_second = 0.0;

int last_tick = 0;
int frame_count = 0;

int window_id = 0;

HWND window_hwnd;

WNDPROC window_proc_freeglut;

LRESULT CALLBACK window_proc_hook(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

int window_width  = 1280;
int window_height = 720;

bool window_start_maximized  = false;
bool window_start_fullscreen = false;

TwBar *bar_options;
TwBar *bar_zones;

bool mouse_dragging = false;

int mouse_dragging_start_x = 0;
int mouse_dragging_start_y = 0;

std::string map_folder = "maps";
std::string map_zone_name = "qeynos";

float map_origin_x = window_width  / 2;
float map_origin_y = window_height / 2;

const float MAP_ZOOM_DEFAULT = 1.0;

const float MAP_ZOOM_MIN = 0.01;

float map_zoom = MAP_ZOOM_DEFAULT;

float map_zoom_multiplier = 0.25;

float map_offset_x = 0;
float map_offset_y = 0;

float map_offset_multiplier = 10.0;

float map_draw_x = 0;
float map_draw_y = 0;

float map_min_x = 0;
float map_min_y = 0;
float map_min_z = 0;

float map_max_x = 0;
float map_max_y = 0;
float map_max_z = 0;

float map_width  = 0;
float map_height = 0;
float map_depth  = 0;

float map_mid_x = 0;
float map_mid_y = 0;

bool map_draw_grid             = false;
bool map_draw_grid_coordinates = true;

float map_grid_size = 100;

float map_grid_color[3] = {1.0, 0.0, 0.0};

bool map_draw_info_text = true;

bool map_draw_lines  = true;
bool map_draw_points = true;

bool map_draw_origin = true;

bool map_draw_layer0 = true;
bool map_draw_layer1 = true;
bool map_draw_layer2 = true;
bool map_draw_layer3 = true;

bool map_points_ignore_size = false;

bool map_draw_points_coordinates = false;
bool map_draw_points_opaque      = false;

bool map_draw_points_layer0 = true;
bool map_draw_points_layer1 = true;
bool map_draw_points_layer2 = true;
bool map_draw_points_layer3 = true;

bool map_zoom_to_fit = false;

/*
GLUT_BITMAP_9_BY_15
GLUT_BITMAP_8_BY_13
GLUT_BITMAP_HELVETICA_18
GLUT_BITMAP_HELVETICA_12
GLUT_BITMAP_HELVETICA_10
GLUT_BITMAP_TIMES_ROMAN_24
GLUT_BITMAP_TIMES_ROMAN_10
*/
void *font_name = GLUT_BITMAP_HELVETICA_10;

int font_size = 10;

float font_offset_x = 1.5;
float font_offset_y = 1.5;

int map_lines_total  = 0;
int map_points_total = 0;

int map_lines_visible    = 0;
int map_lines_visible_ex = 0;

int map_points_visible    = 0;
int map_points_visible_ex = 0;

struct map_line_t
{
    int layer;

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
    int layer;

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

struct zone_info_t
{
    std::string name_short;
    std::string name_long;
    std::string group;
};

std::vector<zone_info_t> zone_infos;
std::vector<zone_info_t>::iterator zone_infos_it;

std::string zone_info_group = "Other";

float calculate_distance(float x1, float y1, float x2, float y2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

bool file_exists(std::string filename)
{
    //std::ifstream file(filename.c_str());
    //return file.good();

    return std::ifstream(filename.c_str()).is_open();
}

bool string_contains(std::string s, std::string text)
{
    std::size_t found = s.find(text);

    if (found != std::string::npos)
    {
        return true;
    }
   
    return false;
}

template <class T>
T reverse_sign(T value)
{
    T result;

    if (value == 0)
    {
        return value;
    }

    if (value > 0)
    {
        result = value - (value * 2);
    }
    else
    {
        result = abs(value);
    }

    return result;
}

void toggle_bool(bool &b)
{
    b = !b;
}

std::string get_bool_string(bool b)
{
    return b ? "True" : "False";
}

void message_box_show_error(std::string text)
{
    MessageBoxA(window_hwnd, text.c_str(), APPLICATION_NAME, MB_ICONERROR);
}

template <class T>
bool map_line_data_value_can_lexical_cast(std::string filename, int line_number, int line_index, T &output, std::string value, std::string value_name)
{
    try
    {
        output = boost::lexical_cast<T>(value);
    }
    catch (const boost::bad_lexical_cast &e)
    {
        std::stringstream error_text;
        error_text
            << "Error while parsing map"
            << "\r\n\r\n"
            << "File: "  << filename    << "\r\n"
            << "Line: "  << line_number << "\r\n"
            << "Index: " << line_index  << "\r\n"
            << "Value: " << value       << "\r\n"
            << "Name: "  << value_name
            << "\r\n\r\n"
            << e.what();

        message_box_show_error(error_text.str());

        return false;
    }
    catch (...)
    {
        std::stringstream error_text;
        error_text
            << "Unknown exception caught while parsing map"
            << "\r\n\r\n"
            << "File: "  << filename    << "\r\n"
            << "Line: "  << line_number << "\r\n"
            << "Index: " << line_index  << "\r\n"
            << "Value: " << value       << "\r\n"
            << "Name: "  << value_name;

        message_box_show_error(error_text.str());

        return false;
    }

    return true;
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

void draw_bitmap_string_background(float x, float y, void *font, int font_size, std::string text)
{
    glBegin(GL_QUADS);
        glVertex2f(x                                                              - 1, y             - 0);
        glVertex2f(x + glutBitmapLength(font, (const unsigned char*)text.c_str()) + 0, y             - 0);
        glVertex2f(x + glutBitmapLength(font, (const unsigned char*)text.c_str()) + 0, y + font_size + 2);
        glVertex2f(x                                                              - 1, y + font_size + 2);
    glEnd();
}

void draw_plus(float x, float y, float size)
{
    glBegin(GL_LINES);
        glVertex2f(x - size, y);
        glVertex2f(x + size, y);

        glVertex2f(x, y - size);
        glVertex2f(x, y + size);
    glEnd();
}

void draw_point(float x, float y, float size)
{
    glPointSize(size);

    glBegin(GL_POINTS);
        glVertex2f(x, y);
    glEnd();
}

void map_draw_info_text_toggle()
{
    map_draw_info_text = !map_draw_info_text;
}

void map_draw_layer0_toggle()
{
    map_draw_layer0 = !map_draw_layer0;
}

void map_draw_layer1_toggle()
{
    map_draw_layer1 = !map_draw_layer1;
}

void map_draw_layer2_toggle()
{
    map_draw_layer2 = !map_draw_layer2;
}

void map_draw_layer3_toggle()
{
    map_draw_layer3 = !map_draw_layer3;
}

void map_center()
{
    map_offset_x = map_mid_x;
    map_offset_y = map_mid_y;
}

void map_center_on_origin()
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

    if (map_zoom < MAP_ZOOM_MIN)
    {
        map_zoom = MAP_ZOOM_MIN;
    }
}

void map_zoom_out()
{
    map_zoom += map_zoom_multiplier * map_zoom;
}

void map_calculate_bounds()
{
    if (!map_lines.size())
    {
        return;
    }

    map_min_x = 0;
    map_min_y = 0;
    map_min_z = 0;

    map_max_x = 0;
    map_max_y = 0;
    map_max_z = 0;

    map_width  = 0;
    map_height = 0;
    map_depth  = 0;

    map_mid_x = 0;
    map_mid_y = 0;

    foreach (map_line_t map_line, map_lines)
    {
        if (map_line.from_x < map_min_x)
        {
            map_min_x = map_line.from_x;
        }

        if (map_line.to_x < map_min_x)
        {
            map_min_x = map_line.to_x;
        }

        if (map_line.from_x > map_max_x)
        {
            map_max_x = map_line.from_x;
        }

        if (map_line.to_x > map_max_x)
        {
            map_max_x = map_line.to_x;
        }


        if (map_line.from_y < map_min_y)
        {
            map_min_y = map_line.from_y;
        }

        if (map_line.to_y < map_min_y)
        {
            map_min_y = map_line.to_y;
        }

        if (map_line.from_y > map_max_y)
        {
            map_max_y = map_line.from_y;
        }

        if (map_line.to_y > map_max_y)
        {
            map_max_y = map_line.to_y;
        }


        if (map_line.from_z < map_min_z)
        {
            map_min_z = map_line.from_z;
        }

        if (map_line.to_z < map_min_z)
        {
            map_min_z = map_line.to_z;
        }

        if (map_line.from_z > map_max_z)
        {
            map_max_z = map_line.from_z;
        }

        if (map_line.to_z > map_max_z)
        {
            map_max_z = map_line.to_z;
        }
    }

    map_width  = abs(map_min_x - map_max_x);
    map_height = abs(map_min_y - map_max_y);
    map_depth  = abs(map_min_z - map_max_z);

    map_mid_x = (map_min_x + map_max_x) / 2;
    map_mid_y = (map_min_y + map_max_y) / 2;

    map_mid_x = reverse_sign(map_mid_x);
    map_mid_y = reverse_sign(map_mid_y);
}

void map_parse_file(std::string filename, int layer)
{
    if (file_exists(filename) == false)
    {
        return;
    }

    std::ifstream file(filename.c_str());

    if (!file.is_open())
    {
        return;
    }

/*
    file.seekg(0, std::ios::beg);

    char first_three[3];
    file.read(first_three, 3 * sizeof(char));

    if
    (
        (BYTE)first_three[0] == 0xEF &&
        (BYTE)first_three[1] == 0xBB &&
        (BYTE)first_three[2] == 0xBF
    )
    {
        message_box_show_error("Error while parsing map\r\n\r\nUTF-8 encoding is not supported");

        return;
    }
*/

    file.seekg(0, std::ios::beg);

    char first_char;
    file >> first_char;

    int num_char_checks = 0;

    while (first_char != 'P' && first_char != 'L')
    {
        file >> first_char;

        num_char_checks++;

        if (num_char_checks > 256)
        {
            return;
        }
    }

    file.seekg(num_char_checks, std::ios::beg);

    std::string line;
    std::vector<std::string> line_data;

    int line_number = 0;

    while (std::getline(file, line))
    {
        line_number++;

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

        if (line_type == 'L')
        {
            if (line_data.size() == 9)
            {
                map_line_t map_line;

                map_line.layer = layer;

                if (map_line_data_value_can_lexical_cast<float>(filename, line_number, 1, map_line.from_x, line_data.at(0), "Line From X") == false) continue;
                if (map_line_data_value_can_lexical_cast<float>(filename, line_number, 2, map_line.from_y, line_data.at(1), "Line From Y") == false) continue;
                if (map_line_data_value_can_lexical_cast<float>(filename, line_number, 3, map_line.from_z, line_data.at(2), "Line From Z") == false) continue;

                if (map_line_data_value_can_lexical_cast<float>(filename, line_number, 4, map_line.to_x, line_data.at(3), "Line To X") == false) continue;
                if (map_line_data_value_can_lexical_cast<float>(filename, line_number, 5, map_line.to_y, line_data.at(4), "Line To Y") == false) continue;
                if (map_line_data_value_can_lexical_cast<float>(filename, line_number, 6, map_line.to_z, line_data.at(5), "Line To Z") == false) continue;

                if (map_line_data_value_can_lexical_cast<int>(filename, line_number, 7, map_line.r, line_data.at(6), "Line Red")   == false) continue;
                if (map_line_data_value_can_lexical_cast<int>(filename, line_number, 8, map_line.g, line_data.at(7), "Line Green") == false) continue;
                if (map_line_data_value_can_lexical_cast<int>(filename, line_number, 9, map_line.b, line_data.at(8), "Line Blue")  == false) continue;

/*
                map_line.from_x = boost::lexical_cast<float>(line_data.at(0));
                map_line.from_y = boost::lexical_cast<float>(line_data.at(1));
                map_line.from_z = boost::lexical_cast<float>(line_data.at(2));

                map_line.to_x = boost::lexical_cast<float>(line_data.at(3));
                map_line.to_y = boost::lexical_cast<float>(line_data.at(4));
                map_line.to_z = boost::lexical_cast<float>(line_data.at(5));

                map_line.r = boost::lexical_cast<int>(line_data.at(6));
                map_line.g = boost::lexical_cast<int>(line_data.at(7));
                map_line.b = boost::lexical_cast<int>(line_data.at(8));
*/

                map_lines.push_back(map_line);
            }
        }

        if (line_type == 'P')
        {
            if (line_data.size() == 8)
            {
                map_point_t map_point;

                map_point.layer = layer;

                if (map_line_data_value_can_lexical_cast<float>(filename, line_number, 1, map_point.x, line_data.at(0), "Point X") == false) continue;
                if (map_line_data_value_can_lexical_cast<float>(filename, line_number, 2, map_point.y, line_data.at(1), "Point Y") == false) continue;
                if (map_line_data_value_can_lexical_cast<float>(filename, line_number, 3, map_point.z, line_data.at(2), "Point Z") == false) continue;

                if (map_line_data_value_can_lexical_cast<int>(filename, line_number, 4, map_point.r, line_data.at(3), "Point Red")   == false) continue;
                if (map_line_data_value_can_lexical_cast<int>(filename, line_number, 5, map_point.g, line_data.at(4), "Point Green") == false) continue;
                if (map_line_data_value_can_lexical_cast<int>(filename, line_number, 6, map_point.b, line_data.at(5), "Point Blue")  == false) continue;

                if (map_line_data_value_can_lexical_cast<int>(filename, line_number, 7, map_point.size, line_data.at(6), "Point Size") == false) continue;

/*
                map_point.x = boost::lexical_cast<float>(line_data.at(0));
                map_point.y = boost::lexical_cast<float>(line_data.at(1));
                map_point.z = boost::lexical_cast<float>(line_data.at(2));

                map_point.r = boost::lexical_cast<int>(line_data.at(3));
                map_point.g = boost::lexical_cast<int>(line_data.at(4));
                map_point.b = boost::lexical_cast<int>(line_data.at(5));

                map_point.size = boost::lexical_cast<int>(line_data.at(6));
*/

                map_point.text = line_data.at(7);

                boost::replace_all(map_point.text, "_", " ");

                map_points.push_back(map_point);
            }
        }
    }

    file.close();
}

void map_load_zone(std::string zone_name)
{
    map_lines.clear();
    map_points.clear();

    std::stringstream buffer;

    std::string map_filename;

    bool use_map_folder = true;

    if (string_contains(zone_name, "\\") == true)
    {
        use_map_folder = false;
    }

    if (string_contains(zone_name, "/") == true)
    {
        use_map_folder = false;
    }

    if (use_map_folder == true)
    {
        buffer << map_folder << "/" << zone_name << ".txt";
        map_filename = buffer.str();
        buffer.str("");
    }
    else
    {
        map_filename = zone_name;
    }

    if (file_exists(map_filename) == true)
    {
        buffer << map_filename << " - " << APPLICATION_NAME;
        glutSetWindowTitle(buffer.str().c_str());
        buffer.str("");

        map_parse_file(map_filename, 0);
    }
    else
    {
        buffer << zone_name << " - " << APPLICATION_NAME;
        glutSetWindowTitle(buffer.str().c_str());
        buffer.str("");
    }

    int i = 1;
    for (i = 1; i < 4; i++)
    {
        buffer << map_folder << "/" << zone_name << "_" << i << ".txt";
        std::string map_layer_filename = buffer.str();
        buffer.str("");

        if (file_exists(map_layer_filename) == true)
        {
            map_parse_file(map_layer_filename, i);
        }
    }

    map_zone_name = zone_name;

    map_origin_x = window_width  / 2;
    map_origin_y = window_height / 2;

    map_lines_total  = map_lines.size();
    map_points_total = map_points.size();

    map_calculate_bounds();

    if (map_width > 2000 || map_height > 2000)
    {
        map_grid_size = 1000;
    }
    else
    {
        if (map_width > 1000 || map_height > 1000)
        {
            map_grid_size = 500;
        }
        else
        {
            map_grid_size = 100;
        }
    }

    map_zoom_reset();
    map_center();

    map_zoom_to_fit = true;
}

void parse_zone_info()
{
    if (file_exists(zones_file) == false)
    {
        return;
    }

    zone_infos.clear();

    std::ifstream file(zones_file.c_str());

    if (file.is_open())
    {
        if (file.good() == false)
        {
            return;
        }

        while (file.good())
        {
            std::string line;
            std::getline(file, line);

            if (line.size() == 0)
            {
                continue;
            }

            if (string_contains(line, "#") == true)
            {
                continue;
            }

            if (string_contains(line, "//") == true)
            {
                continue;
            }

            if (string_contains(line, "[") == true)
            {
                if (string_contains(line, "]") == true)
                {
                    line.erase(boost::remove_if(line, boost::is_any_of("[]")), line.end());
                    zone_info_group = line;

                    continue;
                }
            }

            std::vector<std::string>tokens;

            boost::split(tokens, line, boost::is_any_of("="));

            if (tokens.size() == 2)
            {
                zone_info_t zone_info;

                zone_info.name_short = tokens.at(0);
                zone_info.name_long  = tokens.at(1);

                zone_info.group      = zone_info_group;

                zone_infos.push_back(zone_info);
            }
        }
    }
}

void keyboard(unsigned char key, int x, int y)
{
    if (!TwEventKeyboardGLUT(key, x, y))
    {

        switch (key)
        {
/*
            case 27: // Escape
                glutDestroyWindow(window_id);
                exit(0);
                break;
*/

            case 8: // Backspace
                glutFullScreenToggle();
                break;

/*
            case 32: // Space
                map_zoom_reset();
                map_center();
                break;

            case 119: // w
                map_scroll_up();
                break;

            case 115: // s
                map_scroll_down();
                break;

            case 97:  // a
                map_scroll_left();
                break;

            case 100: // d
                map_scroll_right();
                break;
*/

            case 48: // 0
                map_zoom = 0.1;
                break;

            case 49: // 1
                map_zoom = 1.0;
                break;

            case 50: // 2
                map_zoom = 2.0;
                break;

            case 51: // 3
                map_zoom = 3.0;
                break;

            case 52: // 4
                map_zoom = 4.0;
                break;

            case 53: // 5
                map_zoom = 5.0;
                break;

            case 54: // 6
                map_zoom = 6.0;
                break;

            case 55: // 7
                map_zoom = 7.0;
                break;

            case 56: // 8
                map_zoom = 8.0;
                break;

            case 57: // 9
                map_zoom = 9.0;
                break;
/*
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
*/
        }

    }
}

void special(int key, int x, int y)
{
    if (!TwEventSpecialGLUT(key, x, y))
    {

        switch (key)
        {
/*
            case GLUT_KEY_F5:
                map_zoom_reset();
                map_center();
                break;

            case GLUT_KEY_F10:
                map_draw_info_text_toggle();
                break;

            case GLUT_KEY_F11:
                glutFullScreenToggle();
                break;
*/

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

/*
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
*/
        }

    }
}

void mouse(int button, int state, int x, int y)
{
    if (!TwEventMouseButtonGLUT(button, state, x, y))
    {

        if (state == GLUT_UP)
        {
            glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
        }

        if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
        {
            mouse_dragging = false;
        }

        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            glutSetCursor(GLUT_CURSOR_CYCLE);

            mouse_dragging_start_x = x;
            mouse_dragging_start_y = y;

            mouse_dragging = true;
        }

        if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
        {
            glutSetCursor(GLUT_CURSOR_CROSSHAIR);
        }

        if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
        {
            map_offset_x = map_offset_x + (map_origin_x - x) * map_zoom;
            map_offset_y = map_offset_y + (map_origin_y - y) * map_zoom;
        }

        if (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP)
        {
            map_zoom_reset();
            map_center();
        }

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

void motion(int x, int y)
{
    if (!TwEventMouseMotionGLUT(x, y))
    {

        if (mouse_dragging == true)
        {
            glutSetCursor(GLUT_CURSOR_CYCLE);

            map_offset_x += (x - mouse_dragging_start_x) * map_zoom;
            map_offset_y += (y - mouse_dragging_start_y) * map_zoom;

            mouse_dragging_start_x = x;
            mouse_dragging_start_y = y;
        }

    }
}

void reshape(int w, int h)
{
    window_width  = w;
    window_height = h;

    map_origin_x = window_width  / 2;
    map_origin_y = window_height / 2;

    glViewport(0, 0, window_width, window_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, window_width, window_height, 0, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    TwWindowSize(window_width, window_height);
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if (map_draw_lines == true)
    {
        glBegin(GL_LINES);

        map_lines_visible = 0;

        foreach (map_line_t map_line, map_lines)
        {
            int layer = map_line.layer;

            if (layer == 0)
            {
                if (map_draw_layer0 == false)
                {
                    continue;
                }
            }

            if (layer == 1)
            {
                if (map_draw_layer1 == false)
                {
                    continue;
                }
            }

            if (layer == 2)
            {
                if (map_draw_layer2 == false)
                {
                    continue;
                }
            }

            if (layer == 3)
            {
                if (map_draw_layer3 == false)
                {
                    continue;
                }
            }

            if (map_min_z != 0)
            {
                if
                (
                    map_line.from_z < map_min_z ||
                    map_line.to_z   < map_min_z
                )
                {
                    continue;
                }
            }

            if (map_max_z != 0)
            {
                if
                (
                    map_line.from_z > map_max_z ||
                    map_line.to_z   > map_max_z
                )
                {
                    continue;
                }
            }

            float line_start_x = map_line.from_x;
            float line_start_y = map_line.from_y;

            float line_stop_x  = map_line.to_x;
            float line_stop_y  = map_line.to_y;

            float line_map_start_x = ((line_start_x / map_zoom) + map_origin_x) + ((map_draw_x + map_offset_x) / map_zoom);
            float line_map_start_y = ((line_start_y / map_zoom) + map_origin_y) + ((map_draw_y + map_offset_y) / map_zoom);

            float line_map_stop_x = ((line_stop_x / map_zoom) + map_origin_x) + ((map_draw_x + map_offset_x) / map_zoom);
            float line_map_stop_y = ((line_stop_y / map_zoom) + map_origin_y) + ((map_draw_y + map_offset_y) / map_zoom);

            if
            (
                line_map_start_x > window_width &&
                line_map_stop_x  > window_width ||
                line_map_start_x < 0            &&
                line_map_stop_x  < 0
            )
            {
                continue;
            }

            if
            (
                line_map_start_y > window_height &&
                line_map_stop_y  > window_height ||
                line_map_start_y < 0             &&
                line_map_stop_y  < 0
            )
            {
                continue;
            }

            if (map_line.r == 0 && map_line.g == 0 && map_line.b == 0)
            {
                map_line.r = 255;
                map_line.g = 255;
                map_line.b = 255;
            }

            glColor3ub(map_line.r, map_line.g, map_line.b);

            glVertex2f(line_map_start_x, line_map_start_y);
            glVertex2f(line_map_stop_x,  line_map_stop_y);

            map_lines_visible++;
        }

        map_lines_visible_ex = map_lines_visible;

        glEnd();
    }
    else
    {
        map_lines_visible    = 0;
        map_lines_visible_ex = 0;
    }

    if (map_draw_grid == true)
    {
        float origin_map_x = map_origin_x + (map_offset_x / map_zoom);
        float origin_map_y = map_origin_y + (map_offset_y / map_zoom);

        float min_map_x = ((map_min_x / map_zoom) + map_origin_x) + ((map_draw_x + map_offset_x) / map_zoom);
        float max_map_x = ((map_max_x / map_zoom) + map_origin_x) + ((map_draw_x + map_offset_x) / map_zoom);

        float min_map_y = ((map_min_y / map_zoom) + map_origin_y) + ((map_draw_y + map_offset_y) / map_zoom);
        float max_map_y = ((map_max_y / map_zoom) + map_origin_y) + ((map_draw_y + map_offset_y) / map_zoom);

        glColor3f(map_grid_color[0], map_grid_color[1], map_grid_color[2]);

        glBegin(GL_LINES);
            glVertex2f(origin_map_x, min_map_y);
            glVertex2f(origin_map_x, max_map_y);

            glVertex2f(min_map_x, origin_map_y);
            glVertex2f(max_map_x, origin_map_y);
        glEnd();

        if (map_draw_grid_coordinates == true)
        {
            std::string origin_label = "0";

            int origin_label_length = glutBitmapLength(font_name, (const unsigned char*)origin_label.c_str());

            draw_bitmap_string(origin_map_x - (origin_label_length / 2), min_map_y - (font_size * font_offset_y), font_name, origin_label);

            draw_bitmap_string(origin_map_x - (origin_label_length / 2), max_map_y + (font_size * font_offset_y), font_name, origin_label);

            draw_bitmap_string(min_map_x - (font_size * font_offset_y) - origin_label_length, origin_map_y + (font_size / 2), font_name, origin_label);

            draw_bitmap_string(max_map_x + (font_size * font_offset_y), origin_map_y + (font_size / 2), font_name, origin_label);
        }

        glColor3f(map_grid_color[0] / 2, map_grid_color[1] / 2, map_grid_color[2] / 2);

        glEnable(GL_LINE_STIPPLE);

        glLineStipple(1, 0x1C47);

        //glBegin(GL_LINES);

        int grid_num_lines = 0;

        float distance_west = abs(map_min_x) / map_zoom;

        grid_num_lines = (int)(distance_west / (map_grid_size / map_zoom));

        for (int i = 1; i < grid_num_lines + 1; i++)
        {
            float next_x = origin_map_x - ((i * map_grid_size) / map_zoom);

            glBegin(GL_LINES);
                glVertex2f(next_x, min_map_y);
                glVertex2f(next_x, max_map_y);
            glEnd();

            if (map_draw_grid_coordinates == true)
            {
                std::stringstream grid_size_label;
                grid_size_label << i * map_grid_size;

                int grid_size_label_length = glutBitmapLength(font_name, (const unsigned char*)grid_size_label.str().c_str());

                draw_bitmap_string(next_x - (grid_size_label_length / 2), min_map_y - (font_size * font_offset_y), font_name, grid_size_label.str());
                draw_bitmap_string(next_x - (grid_size_label_length / 2), max_map_y + (font_size * font_offset_y), font_name, grid_size_label.str());
            }
        }

        float distance_east = abs(map_max_x) / map_zoom;

        grid_num_lines = (int)(distance_east / (map_grid_size / map_zoom));

        for (int i = 1; i < grid_num_lines + 1; i++)
        {
            float next_x = origin_map_x + ((i * map_grid_size) / map_zoom);

            glBegin(GL_LINES);
                glVertex2f(next_x, min_map_y);
                glVertex2f(next_x, max_map_y);
            glEnd();

            if (map_draw_grid_coordinates == true)
            {
                std::stringstream grid_size_label;
                grid_size_label << "-" << i * map_grid_size;

                int grid_size_label_length = glutBitmapLength(font_name, (const unsigned char*)grid_size_label.str().c_str());

                draw_bitmap_string(next_x - (grid_size_label_length / 2), min_map_y - (font_size * font_offset_y), font_name, grid_size_label.str());
                draw_bitmap_string(next_x - (grid_size_label_length / 2), max_map_y + (font_size * font_offset_y), font_name, grid_size_label.str());
            }
        }

        float distance_north = abs(map_min_y) / map_zoom;

        grid_num_lines = (int)(distance_north / (map_grid_size / map_zoom));

        for (int i = 1; i < grid_num_lines + 1; i++)
        {
            float next_y = origin_map_y - ((i * map_grid_size) / map_zoom);

            glBegin(GL_LINES);
                glVertex2f(min_map_x, next_y);
                glVertex2f(max_map_x, next_y);
            glEnd();

            if (map_draw_grid_coordinates == true)
            {
                std::stringstream grid_size_label;
                grid_size_label << i * map_grid_size;

                int grid_size_label_length = glutBitmapLength(font_name, (const unsigned char*)grid_size_label.str().c_str());

                draw_bitmap_string(min_map_x - (font_size * font_offset_y) - grid_size_label_length, next_y + (font_size / 2), font_name, grid_size_label.str());
                draw_bitmap_string(max_map_x + (font_size * font_offset_y), next_y + (font_size / 2), font_name, grid_size_label.str());
            }
        }

        float distance_south = abs(map_max_y) / map_zoom;

        grid_num_lines = (int)(distance_south / (map_grid_size / map_zoom));

        for (int i = 1; i < grid_num_lines + 1; i++)
        {
            float next_y = origin_map_y + ((i * map_grid_size) / map_zoom);

            glBegin(GL_LINES);
                glVertex2f(min_map_x, next_y);
                glVertex2f(max_map_x, next_y);
            glEnd();

            if (map_draw_grid_coordinates == true)
            {
                std::stringstream grid_size_label;
                grid_size_label << "-" << i * map_grid_size;

                int grid_size_label_length = glutBitmapLength(font_name, (const unsigned char*)grid_size_label.str().c_str());

                draw_bitmap_string(min_map_x - (font_size * font_offset_y) - grid_size_label_length, next_y + (font_size / 2), font_name, grid_size_label.str());
                draw_bitmap_string(max_map_x + (font_size * font_offset_y), next_y + (font_size / 2), font_name, grid_size_label.str());
            }
        }

        //glEnd();

        glDisable(GL_LINE_STIPPLE);
    }

    if (map_draw_points == true)
    {
        map_points_visible = 0;

        foreach (map_point_t map_point, map_points)
        {
            int layer = map_point.layer;

            if (layer == 0)
            {
                if (map_draw_layer0 == false)
                {
                    continue;
                }

                if (map_draw_points_layer0 == false)
                {
                    continue;
                }
            }

            if (layer == 1)
            {
                if (map_draw_layer1 == false)
                {
                    continue;
                }

                if (map_draw_points_layer1 == false)
                {
                    continue;
                }
            }

            if (layer == 2)
            {
                if (map_draw_layer2 == false)
                {
                    continue;
                }

                if (map_draw_points_layer2 == false)
                {
                    continue;
                }
            }

            if (layer == 3)
            {
                if (map_draw_layer3 == false)
                {
                    continue;
                }

                if (map_draw_points_layer3 == false)
                {
                    continue;
                }
            }

            float point_map_x = ((map_point.x / map_zoom) + map_origin_x) + ((map_draw_x + map_offset_x) / map_zoom);
            float point_map_y = ((map_point.y / map_zoom) + map_origin_y) + ((map_draw_y + map_offset_y) / map_zoom);

            if
            (
                point_map_x > window_width  ||
                point_map_y > window_height ||
                point_map_x < 0             ||
                point_map_y < 0
            )
            {
                continue;
            }

            if (map_point.r == 0 && map_point.g == 0 && map_point.b == 0)
            {
                map_point.r = 255;
                map_point.g = 255;
                map_point.b = 255;
            }

            int point_font_size   = font_size;
            void *point_font_name = font_name;

            if (map_points_ignore_size == false)
            {
                if (map_point.size == 1)
                {
                    point_font_size = 10;
                    point_font_name = GLUT_BITMAP_HELVETICA_10;
                }

                if (map_point.size == 2)
                {
                    point_font_size = 12;
                    point_font_name = GLUT_BITMAP_HELVETICA_12;
                }

                if (map_point.size == 3)
                {
                    point_font_size = 18;
                    point_font_name = GLUT_BITMAP_HELVETICA_18;
                }
            }

            if (map_draw_points_opaque == true)
            {
                float point_background_x = point_map_x;
                float point_background_y = point_map_y + (point_font_size * font_offset_y) - point_font_size;

                glColor3ub(0, 0, 128);

                draw_bitmap_string_background(point_background_x, point_background_y, point_font_name, point_font_size, map_point.text);

                glColor3ub(255, 255, 255);
            }
            else
            {
                glColor3ub(map_point.r, map_point.g, map_point.b);
            }

            draw_plus(point_map_x, point_map_y, 4);

            draw_bitmap_string(point_map_x, point_map_y + (point_font_size * font_offset_y), point_font_name, map_point.text);

            if (map_draw_points_coordinates == true)
            {
                float loc_y = reverse_sign(map_point.y);
                float loc_x = reverse_sign(map_point.x);

                std::stringstream map_point_coordinates_text;
                map_point_coordinates_text << std::setprecision(2) << std::fixed << "(" << loc_y << ", " << loc_x << ")";

                if (map_draw_points_opaque == true)
                {
                    float point_background_x = point_map_x;
                    float point_background_y = point_map_y + ((point_font_size * font_offset_y) * 2) - point_font_size;

                    glColor3ub(0, 0, 128);

                    draw_bitmap_string_background(point_background_x, point_background_y, point_font_name, point_font_size, map_point_coordinates_text.str());

                    glColor3ub(255, 255, 255);
                }
                //else
                //{
                    //glColor3ub(map_point.r, map_point.g, map_point.b);
                //}

                draw_bitmap_string(point_map_x, point_map_y + ((point_font_size * font_offset_y) * 2), point_font_name, map_point_coordinates_text.str());
            }

            map_points_visible++;
        }

        map_points_visible_ex = map_points_visible;
    }
    else
    {
        map_points_visible    = 0;
        map_points_visible_ex = 0;
    }

    if (map_draw_origin == true)
    {
        glColor3f(1.0, 0, 1.0);

        float origin_map_x = map_origin_x + (map_offset_x / map_zoom);
        float origin_map_y = map_origin_y + (map_offset_y / map_zoom);

        draw_point(origin_map_x, origin_map_y, 4);

        draw_bitmap_string(origin_map_x, origin_map_y + (font_size * font_offset_y), font_name, "Origin (0, 0)");
    }

    if (map_zoom_to_fit == true)
    {
        if (map_lines_visible == map_lines_total)
        {
            if (map_points_visible == map_points_total && map_points_total > 0)
            {
                //map_zoom += 0.5;

                map_zoom_to_fit = false;
            }

            if (map_points_total > 0 && map_points_visible < map_points_total)
            {
                map_zoom += 1.0;
            }
            else
            {
                //map_zoom += 0.5;

                map_zoom_to_fit = false;
            }
        }

        if (map_lines_total > 0 && map_lines_visible < map_lines_total)
        {
            map_zoom += 1.0;
        }

        map_center();
    }

    int current_tick = GetTickCount();

    int difference_tick = current_tick - last_tick;

    frame_count++;

    if (difference_tick >= 10000)
    {
        map_zoom_to_fit = false;
    }

    if (difference_tick >= 1000)
    {
        frames_per_second = (float)(frame_count / (difference_tick / 100.0f));

        last_tick = current_tick;
        frame_count = 0;
    }

    if (map_draw_info_text == true)
    {
        glColor3f(1.0, 0, 1.0);

        std::vector<std::string> map_info_text;

        std::stringstream map_info_text_buffer;

        map_info_text_buffer << "FPS: " << frames_per_second;
        map_info_text.push_back(map_info_text_buffer.str());
        map_info_text_buffer.str("");

        map_info_text_buffer << "Resolution: " << window_width << "x" << window_height;
        map_info_text.push_back(map_info_text_buffer.str());
        map_info_text_buffer.str("");

        map_info_text_buffer << "Zoom: " << map_zoom;
        map_info_text.push_back(map_info_text_buffer.str());
        map_info_text_buffer.str("");

        map_info_text_buffer << "Lines: ";
        if (map_draw_lines == true)
        {
            map_info_text_buffer << map_lines_visible_ex;
        }
        else
        {
            map_info_text_buffer << get_bool_string(map_draw_lines);
        }
        map_info_text.push_back(map_info_text_buffer.str());
        map_info_text_buffer.str("");

        map_info_text_buffer << "Points: ";
        if (map_draw_points == true)
        {
            map_info_text_buffer << map_points_visible_ex;
        }
        else
        {
            map_info_text_buffer << get_bool_string(map_draw_points);
        }
        map_info_text.push_back(map_info_text_buffer.str());
        map_info_text_buffer.str("");

        map_info_text_buffer << "Base: " << get_bool_string(map_draw_layer0);
        map_info_text.push_back(map_info_text_buffer.str());
        map_info_text_buffer.str("");

        map_info_text_buffer << "Layer 1: " << get_bool_string(map_draw_layer1);
        map_info_text.push_back(map_info_text_buffer.str());
        map_info_text_buffer.str("");

        map_info_text_buffer << "Layer 2: " << get_bool_string(map_draw_layer2);
        map_info_text.push_back(map_info_text_buffer.str());
        map_info_text_buffer.str("");

        map_info_text_buffer << "Layer 3: " << get_bool_string(map_draw_layer3);
        map_info_text.push_back(map_info_text_buffer.str());
        map_info_text_buffer.str("");

        int map_info_text_index = 1;
        foreach (std::string map_info_text_value, map_info_text)
        {
            draw_bitmap_string(0, 0 + (font_size * map_info_text_index), font_name, map_info_text_value);
            map_info_text_index++;
        }
    }

    TwDraw();

    glutSwapBuffers();

    glutPostRedisplay();
}

void idle()
{
    glutPostRedisplay();
}

void init_gl()
{
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    glFrontFace(GL_CW);

    glClearColor(0, 0, 0, 1);

    glViewport(0, 0, window_width, window_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, window_width, window_height, 0, 0, 1);
}

void done()
{
    TwTerminate();
}

void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString)
{
    destinationClientString = sourceLibraryString;
}

void TW_CALL bar_error_handler(const char *error_message)
{
    assert(error_message == NULL);
}

void TW_CALL bar_options_button_toggle_fullscreen(void *)
{
    glutFullScreenToggle();
}

void TW_CALL bar_options_button_load_zone(void *)
{
    map_load_zone(map_zone_name);
}

void TW_CALL bar_options_button_reset_height_filter(void *)
{
    map_calculate_bounds();
}

void TW_CALL bar_options_button_zoom_to_fit(void *)
{
    map_zoom_reset();

    map_zoom_to_fit = true;
}

void TW_CALL bar_options_button_reset_zoom_and_center_map(void *)
{
    map_zoom_reset();
    map_center();
}

void TW_CALL bar_options_button_exit(void *)
{
    glutLeaveMainLoop();
}

void bar_zones_create();

void bar_zones_refresh()
{
    TwDeleteBar(bar_zones);

    bar_zones_create();

    TwDefine(" Zones iconified=false ");
}

void TW_CALL bar_zones_button_refresh_zones(void *)
{
    bar_zones_refresh();
}

void TW_CALL bar_zones_button_zone_name(void *client_data)
{
    std::string *ps = static_cast<std::string *>(client_data);

    std::string s = *ps;

    map_load_zone(s);
}

void bar_zones_create()
{
    parse_zone_info();

    if (zone_infos.size() == 0)
    {
        return;
    }

    bar_zones = TwNewBar("Zones");
    TwDefine(" Zones iconified=true refresh=0.1 position='512 16' size='480 640' valueswidth=240 ");

    TwAddVarRW(bar_zones, "ZonesFile", TW_TYPE_STDSTRING, &zones_file,
        " label='File: ' help='File that contains the zones information' ");

    TwAddVarRW(bar_zones, "ZonesSearch", TW_TYPE_STDSTRING, &zones_search,
        " label='Search: ' help='Filter out zones from the list by name' ");

    TwAddButton(bar_zones, "RefreshZones", bar_zones_button_refresh_zones, NULL,
        " label='Refresh Zones List' help='Refresh the list of zones by reading the zones file again' ");

    TwAddSeparator(bar_zones, NULL, "");

    void *vp;

    foreach (zone_info_t zone_info, zone_infos)
    {
        bool skip_zone = false;

        if (zones_search.size() > 0)
        {
            std::string zones_search_to_lower = boost::to_lower_copy(zones_search);

            std::string zone_info_name_long_to_lower = boost::to_lower_copy(zone_info.name_long);

            if (string_contains(zone_info_name_long_to_lower, zones_search_to_lower) == false)
            {
                skip_zone = true;
            }

            if (zones_search_to_lower == zone_info.name_short)
            {
                skip_zone = false;
            }
        }

        if (skip_zone == true)
        {
            continue;
        }

        vp = static_cast<void*>(new std::string(zone_info.name_short));

        std::string zone_info_group_no_spaces = boost::replace_all_copy(zone_info.group, " ", "");

        std::stringstream button_name;
        button_name << "ZoneButton_" << zone_info.name_short;

        std::stringstream button_def;
        button_def << " label='" << zone_info.name_long << "' group=" << zone_info_group_no_spaces;

        TwAddButton(bar_zones, button_name.str().c_str(), bar_zones_button_zone_name, vp, button_def.str().c_str());

        std::stringstream define_group_label;
        define_group_label << " Zones/" << zone_info_group_no_spaces << " label='" << zone_info.group << "' ";

        TwDefine(define_group_label.str().c_str());
    }
}

void bar_options_create()
{
    bar_options = TwNewBar("Options");
    TwDefine(" Options iconified=false refresh=0.1 position='16 16' size='480 640' valueswidth=240 ");

    TwAddVarRW(bar_options, "InfoText", TW_TYPE_BOOLCPP, &map_draw_info_text,
        " label='Information Text' help='Draw the information text at the top left of the screen' key='F10' ");

    TwAddSeparator(bar_options, NULL, "");

    TwAddVarRO(bar_options, "WindowWidth", TW_TYPE_UINT32, &window_width,
        " group=Window label='Width: ' help='Width of the map window' ");
    TwAddVarRO(bar_options, "WindowHeight", TW_TYPE_UINT32, &window_height,
        " group=Window label='Height: ' help='Height of the map window' ");
    TwAddButton(bar_options, "ToggleFullscreen", bar_options_button_toggle_fullscreen, NULL,
        " group=Window label='Toggle Fullscreen' help='Switch between windowed mode and fullscreen mode' key='F11' ");

    //TwDefine(" Options/Window opened=false ");

    TwAddVarRW(bar_options, "MapFolder", TW_TYPE_STDSTRING, &map_folder,
        " group=Map label='Folder: ' help='Folder that contains the map files' ");

    TwAddVarRW(bar_options, "ZoneName", TW_TYPE_STDSTRING, &map_zone_name,
        " group=Zone label='Name: ' help='Name of the zone' ");

    TwAddButton(bar_options, "LoadZone", bar_options_button_load_zone, NULL,
        " group=Zone label='Load Zone' help='Load the map for this zone' key='F5' ");

    TwDefine(" Options/Zone group=Map ");

    TwAddVarRO(bar_options, "FramesPerSecond", TW_TYPE_FLOAT, &frames_per_second,
        " group=Map label='FPS: ' help='The amount of time it takes to draw the map each frame' precision=2 ");
    TwAddVarRW(bar_options, "MapZoom", TW_TYPE_FLOAT, &map_zoom,
        " group=Map label='Zoom: ' help='Scales the size of the map' keyIncr='-' keyDecr='+' min=0.01 max=1000 step=0.25 precision=2 ");
    TwAddVarRW(bar_options, "MapZoomMultiplier", TW_TYPE_FLOAT, &map_zoom_multiplier,
        " group=Map label='Zoom Multiplier: ' help='Amount to zoom when scaling the map' min=0.01 max=10 step=0.01 precision=2 ");

    TwAddVarRO(bar_options, "MapLinesTotal", TW_TYPE_UINT32, &map_lines_total,
        " group=Map label='Lines: ' help='Number of lines the map has' ");
    TwAddVarRO(bar_options, "MapPointsTotal", TW_TYPE_UINT32, &map_points_total,
        " group=Map label='Points: ' help='Number of points the map has' ");

    TwAddVarRO(bar_options, "MapLinesVisible", TW_TYPE_UINT32, &map_lines_visible,
        " group=Map label='Lines Visible: ' help='Number of lines currently visible' ");
    TwAddVarRO(bar_options, "MapPointsVisible", TW_TYPE_UINT32, &map_points_visible,
        " group=Map label='Points Visible: ' help='Number of points currently visible' ");

    TwAddVarRW(bar_options, "MapOffsetX", TW_TYPE_FLOAT, &map_offset_x,
        " group=Map label='Offset X: ' help='Offset from the origin on the X-axis' step=1 precision=2 ");
    TwAddVarRW(bar_options, "MapOffsetY", TW_TYPE_FLOAT, &map_offset_y,
        " group=Map label='Offset Y: ' help='Offset from the origin on the Y-axis' step=1 precision=2 ");

    TwAddVarRW(bar_options, "MapDrawLines", TW_TYPE_BOOLCPP, &map_draw_lines,
        " group=Draw label='Lines' help='Draw the lines of the map' key='l' ");
    TwAddVarRW(bar_options, "MapDrawPoints", TW_TYPE_BOOLCPP, &map_draw_points,
        " group=Draw label='Points' help='Draw the points or text labels of the map' key='p' ");
    TwAddVarRW(bar_options, "MapDrawOrigin", TW_TYPE_BOOLCPP, &map_draw_origin,
        " group=Draw label='Origin' help='Draw a label at the origin coordinates 0,0' key='o' ");

    TwDefine(" Options/Draw group=Map ");

    TwAddVarRW(bar_options, "MapLayer0", TW_TYPE_BOOLCPP, &map_draw_layer0,
        " group=Layers label='Base' help='Draw the base map layer' key='F4' ");
    TwAddVarRW(bar_options, "MapLayer1", TW_TYPE_BOOLCPP, &map_draw_layer1,
        " group=Layers label='Layer 1' help='Draw the first map layer' key='F1' ");
    TwAddVarRW(bar_options, "MapLayer2", TW_TYPE_BOOLCPP, &map_draw_layer2,
        " group=Layers label='Layer 2' help='Draw the second map layer' key='F2' ");
    TwAddVarRW(bar_options, "MapLayer3", TW_TYPE_BOOLCPP, &map_draw_layer3,
        " group=Layers label='Layer 3' help='Draw the third map layer' key='F3' ");

    TwDefine(" Options/Layers group=Map ");

    TwAddVarRW(bar_options, "MapPointsIgnoreSize", TW_TYPE_BOOLCPP, &map_points_ignore_size,
        " group=Points label='Ignore Size' help='Always use the smallest size for points or text labels of the map' ");
    TwAddVarRW(bar_options, "MapDrawPointsCoordinates", TW_TYPE_BOOLCPP, &map_draw_points_coordinates,
        " group=Points label='Show /loc Coordinates' help='Show the /loc coordinates for points or text labels of the map' key='P' ");
    TwAddVarRW(bar_options, "MapDrawPointsOpaque", TW_TYPE_BOOLCPP, &map_draw_points_opaque,
        " group=Points label='Opaque Background' help='Draw an opaque background behind points or text labels of the map' ");

    TwAddSeparator(bar_options, NULL, " group=Points ");

    TwAddVarRW(bar_options, "MapDrawPointsLayer0", TW_TYPE_BOOLCPP, &map_draw_points_layer0,
        " group=Points label='Base' help='Draw the points or text labels of the base map layer' ");
    TwAddVarRW(bar_options, "MapDrawPointsLayer1", TW_TYPE_BOOLCPP, &map_draw_points_layer1,
        " group=Points label='Layer 1' help='Draw the points or text labels of the first map layer' ");
    TwAddVarRW(bar_options, "MapDrawPointsLayer2", TW_TYPE_BOOLCPP, &map_draw_points_layer2,
        " group=Points label='Layer 2' help='Draw the points or text labels of the second map layer' ");
    TwAddVarRW(bar_options, "MapDrawPointsLayer3", TW_TYPE_BOOLCPP, &map_draw_points_layer3,
        " group=Points label='Layer 3' help='Draw the points or text labels of the third map layer' ");

    TwDefine(" Options/Points group=Map ");

    TwAddVarRW(bar_options, "MinZ", TW_TYPE_FLOAT, &map_min_z,
        " group=HeightFilter label='Minimum Z-Axis: ' help='Filters out lines below this height' keyIncr='PGUP' keyDecr='PGDOWN' step=1 precision=2 ");
    TwAddVarRW(bar_options, "MaxZ", TW_TYPE_FLOAT, &map_max_z,
        " group=HeightFilter label='Maximum Z-Axis: ' help='Filters out lines above this height' keyIncr='SHIFT+PGUP' keyDecr='SHIFT+PGDOWN' step=1 precision=2 ");

    TwAddButton(bar_options, "ResetHeightFilter", bar_options_button_reset_height_filter, NULL,
        " group=HeightFilter label='Reset Height Filter' help='Reset the Z-axis values for the height filter' key='F9' ");

    TwDefine(" Options/HeightFilter label='Height Filter' group=Map ");

    TwAddVarRO(bar_options, "MapWidth", TW_TYPE_FLOAT, &map_width,
        " group=Bounds label='Width: ' help='How wide the map is on the X-axis' precision=2 ");
    TwAddVarRO(bar_options, "MapHeight", TW_TYPE_FLOAT, &map_height,
        " group=Bounds label='Height: ' help='How tall the map is on the Y-axis' precision=2 ");

    TwAddVarRO(bar_options, "MapDepth", TW_TYPE_FLOAT, &map_depth,
        " group=Bounds label='Depth: ' help='How deep the map is on the Z-axis' precision=2 ");

    TwAddSeparator(bar_options, NULL, " group=Bounds ");

    TwAddVarRO(bar_options, "MapMinX", TW_TYPE_FLOAT, &map_min_x,
        " group=Bounds label='Minimum X-Axis: ' help='Lowest X-axis value' precision=2 ");
    TwAddVarRO(bar_options, "MapMaxX", TW_TYPE_FLOAT, &map_max_x,
        " group=Bounds label='Maximum X-Axis: ' help='Highest X-axis value' precision=2 ");

    TwAddVarRO(bar_options, "MapMinY", TW_TYPE_FLOAT, &map_min_y,
        " group=Bounds label='Minimum Y-Axis: ' help='Lowest Y-axis value' precision=2 ");
    TwAddVarRO(bar_options, "MapMaxY", TW_TYPE_FLOAT, &map_max_y,
        " group=Bounds label='Maximum Y-Axis: ' help='Highest Y-axis value' precision=2 ");

    TwAddVarRO(bar_options, "MapMidX", TW_TYPE_FLOAT, &map_mid_x,
        " group=Bounds label='Midpoint X-Axis: ' help='Difference between lowest and highest X-axis values' precision=2 ");
    TwAddVarRO(bar_options, "MapMidY", TW_TYPE_FLOAT, &map_mid_y,
        " group=Bounds label='Midpoint Y-Axis: ' help='Difference between lowest and highest Y-axis values' precision=2 ");

    TwDefine(" Options/Bounds group=Map ");

    TwDefine(" Options/Bounds opened=false ");

    TwAddVarRW(bar_options, "MapDrawGrid", TW_TYPE_BOOLCPP, &map_draw_grid,
        " group=Grid label='Enabled' help='Draw the grid lines' key='g' ");
        TwAddVarRW(bar_options, "MapDrawGridCoordinates", TW_TYPE_BOOLCPP, &map_draw_grid_coordinates,
        " group=Grid label='Coordinates' help='Draw the grid coordinates' key='G' ");
    TwAddVarRW(bar_options, "MapGridSize", TW_TYPE_FLOAT, &map_grid_size,
        " group=Grid label='Size: ' ");
    TwAddVarRW(bar_options, "MapGridColor", TW_TYPE_COLOR3F, &map_grid_color,
        " group=Grid label='Color: ' help='Change the color of the grid' ");

    TwDefine(" Options/Grid group=Map ");

    TwAddButton(bar_options, "ZoomToFit", bar_options_button_zoom_to_fit, NULL,
        " group=Map label='Zoom Map to Fit Window' help='Zoom out until the entire map is visible inside the window' key='z' ");

    TwAddButton(bar_options, "ResetZoomAndCenterMap", bar_options_button_reset_zoom_and_center_map, NULL,
        " group=Map label='Reset Zoom and Center Map' help='Set the zoom amount to 1.0 and center the map' key='SPACE' ");

    TwAddSeparator(bar_options, NULL, "");

    TwAddButton(bar_options, "Exit", bar_options_button_exit, NULL,
        " label='Exit' help='Close this application' key='ESCAPE' ");
}

void map_load_config()
{
    if (file_exists(ini_file) == false)
    {
        return;
    }

    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(ini_file, pt);

    window_width  = pt.get<int>("Window.Width",  window_width);
    window_height = pt.get<int>("Window.Height", window_height);

    window_start_maximized  = pt.get<bool>("Window.Maximized",  window_start_maximized);
    window_start_fullscreen = pt.get<bool>("Window.Fullscreen", window_start_fullscreen);

    zones_file = pt.get<std::string>("Zones.File", zones_file);

    map_folder    = pt.get<std::string>("Map.Folder", map_folder);
    map_zone_name = pt.get<std::string>("Map.Zone",   map_zone_name);

    map_draw_info_text = pt.get<bool>("InformationText.Enabled", map_draw_info_text);

    map_draw_lines  = pt.get<bool>("Draw.Lines",  map_draw_lines);
    map_draw_points = pt.get<bool>("Draw.Points", map_draw_points);
    map_draw_origin = pt.get<bool>("Draw.Origin", map_draw_origin);

    map_draw_layer0 = pt.get<bool>("Layers.Base",   map_draw_layer0);
    map_draw_layer1 = pt.get<bool>("Layers.Layer1", map_draw_layer1);
    map_draw_layer2 = pt.get<bool>("Layers.Layer2", map_draw_layer2);
    map_draw_layer3 = pt.get<bool>("Layers.Layer3", map_draw_layer3);

    map_points_ignore_size = pt.get<bool>("Points.IgnoreSize", map_points_ignore_size);

    map_draw_points_coordinates = pt.get<bool>("Points.ShowCoordinates",  map_draw_points_coordinates);
    map_draw_points_opaque      = pt.get<bool>("Points.OpaqueBackground", map_draw_points_opaque);
    map_draw_points_layer0      = pt.get<bool>("Points.Base",             map_draw_points_layer0);
    map_draw_points_layer1      = pt.get<bool>("Points.Layer1",           map_draw_points_layer1);
    map_draw_points_layer2      = pt.get<bool>("Points.Layer2",           map_draw_points_layer2);
    map_draw_points_layer3      = pt.get<bool>("Points.Layer3",           map_draw_points_layer3);

    map_draw_grid             = pt.get<bool>("Grid.Enabled",     map_draw_grid);
    map_draw_grid_coordinates = pt.get<bool>("Grid.Coordinates", map_draw_grid_coordinates);

    map_grid_color[0] = pt.get<int>("Grid.ColorRed",   map_grid_color[0]);
    map_grid_color[1] = pt.get<int>("Grid.ColorGreen", map_grid_color[1]);
    map_grid_color[2] = pt.get<int>("Grid.ColorBlue",  map_grid_color[2]);

    map_grid_color[0] /= 256;
    map_grid_color[1] /= 256;
    map_grid_color[2] /= 256;
}

LRESULT CALLBACK window_proc_hook(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
        case WM_DROPFILES:
            HDROP drop = (HDROP)wparam;

            UINT drop_file = DragQueryFile(drop, 0xFFFFFFFF, NULL, NULL);

            if (drop_file != 1)
            {
                message_box_show_error("Drag and drop multiple files is not supported.");

                DragFinish(drop);
                break;
            }

            LPSTR drop_filename[MAX_PATH] = {0};
            drop_filename[0] = '\0';

            if (DragQueryFileA(drop, 0, (LPSTR)drop_filename, MAX_PATH))
            {
                //MessageBoxA(hwnd, (LPCSTR)drop_filename, APPLICATION_NAME, MB_ICONINFORMATION);

                std::string drop_filename_stdstring = (char*)drop_filename;

                if (string_contains(drop_filename_stdstring, ".txt") == true)
                {
                    map_zone_name = drop_filename_stdstring;

                    map_load_zone(map_zone_name);
                }

                if (string_contains(drop_filename_stdstring, ".ini") == true)
                {
                    if (string_contains(drop_filename_stdstring, ini_file) == false)
                    {
                        zones_file = drop_filename_stdstring;

                        bar_zones_refresh();
                    }
                }
            }

            DragFinish(drop);
            break;
    }

    LRESULT result = CallWindowProc(window_proc_freeglut, hwnd, message, wparam, lparam);

    return result;
}

int main(int argc, char** argv)
{
    map_load_config();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition
    (
        (glutGet(GLUT_SCREEN_WIDTH)  - window_width)  / 2,
        (glutGet(GLUT_SCREEN_HEIGHT) - window_height) / 2
    );

    window_id = glutCreateWindow(APPLICATION_NAME);

    window_hwnd = FindWindowA("FREEGLUT", APPLICATION_NAME);

    window_proc_freeglut = (WNDPROC)GetWindowLongPtr(window_hwnd, GWLP_WNDPROC);

    SetWindowLongPtr(window_hwnd, GWLP_WNDPROC, (LONG_PTR)window_proc_hook);

    DragAcceptFiles(window_hwnd, true);

    if (window_start_maximized == true)
    {
        ShowWindow(window_hwnd, SW_MAXIMIZE);
    }

    if (window_start_fullscreen == true)
    {
        glutFullScreen();
    }

    glutDisplayFunc(render);
    //glutIdleFunc(idle);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMouseWheelFunc(mouse_wheel);
    glutMotionFunc(motion);
    glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);

    atexit(done);

    init_gl();

    TwInit(TW_OPENGL, NULL);

    TwCopyStdStringToClientFunc(CopyStdStringToClient);
    TwHandleErrors(bar_error_handler);
    TwGLUTModifiersFunc(glutGetModifiers);

    TwDefine(" GLOBAL fontsize=2 contained=true help='EverQuest Map Viewer' ");

    TwDefine(" TW_HELP position='512 16' size='480 640' ");

    bar_options_create();
    bar_zones_create();

    TwWindowSize(window_width, window_height);

    if (argc == 2)
    {
        if (string_contains(argv[1], ".txt") == true)
        {
            map_zone_name = argv[1];
        }

        if (string_contains(argv[1], ".ini") == true)
        {
            zones_file = argv[1];

            bar_zones_refresh();
        }
    }

    map_load_zone(map_zone_name);

    last_tick = GetTickCount();

    glutMainLoop();

    return 0;
}