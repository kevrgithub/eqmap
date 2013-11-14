// EverQuest Map Viewer

// http://www.cplusplus.com/reference/std/
// http://www.cplusplus.com/reference/stl/
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cassert>

// http://www.boost.org
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm/replace_if.hpp>
#include <boost/range/algorithm/remove_if.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

// http://msdn.microsoft.com/en-us/library/windows/desktop/
#include <windows.h>
#include <shellapi.h>

/*
// http://freeglut.sourceforge.net
// http://freeglut.sourceforge.net/docs/api.php
// http://www.transmissionzero.co.uk/software/freeglut-devel/
//#include <gl/gl.h>
//#include <gl/glu.h>
//#include "gl/glut.h"
#include "gl/freeglut.h"
*/

// http://www.glfw.org/
// http://www.glfw.org/documentation.html
#define GLFW_DLL
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

// http://anttweakbar.sourceforge.net
// http://anttweakbar.sourceforge.net/doc/
#include <AntTweakBar.h>

// http://sourceforge.net/projects/ftgl/
// http://ftgl.sourceforge.net/docs/html/index.html
// http://ftgl.sourceforge.net/docs/html/ftgl-tutorial.html
#include <FTGL/ftgl.h>

// http://www.lonesock.net/soil.html
// https://bitbucket.org/SpartanJ/soil2
#include "SOIL2.h"

#define APPLICATION_NAME "EverQuest Map Viewer"

std::string ini_file = "eqmap.ini";

std::string zones_file = "zones.ini";

std::string zones_search = "";

float frames_per_second = 0.0;

int last_tick = 0;
int frame_count = 0;

GLFWwindow *window_object;

HWND window_hwnd;

WNDPROC window_proc_normal;

LRESULT CALLBACK window_proc_hook(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

int window_width  = 1280;
int window_height = 720;

bool window_start_maximized  = false;
bool window_start_fullscreen = false;

bool window_is_fullscreen = false;

std::string backgrounds_folder = "backgrounds";

bool window_use_background_texture = false;

bool window_use_zone_specific_background_texture = false;

GLuint window_background_texture = 0;

std::string background_file_default = "background.png";

std::string background_file = background_file_default;

std::string anttweakbar_last_error = "";

TwBar *bar_options;
TwBar *bar_zones;

double mouse_x = 0;
double mouse_y = 0;

bool mouse_dragging = false;

bool mouse4_pressed = false;
bool mouse5_pressed = false;

double mouse_dragging_start_x = 0;
double mouse_dragging_start_y = 0;

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

enum map_grid_line_stipple_t
{
    MAP_GRID_LINE_STIPPLE_TYPE_DOTTED        = 0x0101,
    MAP_GRID_LINE_STIPPLE_TYPE_DASHED        = 0x00FF,
    MAP_GRID_LINE_STIPPLE_TYPE_DASH_DOT_DASH = 0x1C47
};

int map_grid_line_stipple_factor = 1;

int map_grid_line_stipple_type = MAP_GRID_LINE_STIPPLE_TYPE_DASH_DOT_DASH;

int map_grid_line_stipple_type_config = 3;

bool map_draw_info_text = true;

bool map_draw_lines  = true;
bool map_draw_points = true;

bool map_draw_origin = true;

std::string origin_text = "Origin (0, 0)";

bool map_draw_layer0 = true;
bool map_draw_layer1 = true;
bool map_draw_layer2 = true;
bool map_draw_layer3 = true;

bool map_draw_lines_layer0 = true;
bool map_draw_lines_layer1 = true;
bool map_draw_lines_layer2 = true;
bool map_draw_lines_layer3 = true;

bool map_draw_points_layer0 = true;
bool map_draw_points_layer1 = true;
bool map_draw_points_layer2 = true;
bool map_draw_points_layer3 = true;

bool map_zoom_to_fit = false;

float map_lines_width          = 1.0;
bool  map_lines_black_to_white = true;

bool map_points_show_coordinates  = false;
bool map_points_opaque_background = false;
bool map_points_ignore_size       = false;
bool map_points_black_to_white    = true;

#define FONT_NAME_DEFAULT "arial"

FTFont *font_object;

FTFont *font_size1_object;
FTFont *font_size2_object;
FTFont *font_size3_object;

std::string fonts_folder = "fonts";

std::string font_file = "c:/windows/fonts/arial.ttf";

int font_size  = 10;
int font_size1 = 11;
int font_size2 = 12;
int font_size3 = 18;

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

void init();
void done();
void render();

float calculate_distance(float x1, float y1, float x2, float y2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

bool file_exists(std::string filename)
{
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

float get_string_width(FTFont* font, std::string text)
{
    return font->Advance(text.c_str());
}

float get_font_height(FTFont* font)
{
    return font->Ascender();
}

void draw_string(FTFont* font, int size, float x, float y, std::string text)
{
    if (font->FaceSize() != size)
    {
        font->FaceSize(size);
    }

    glRasterPos2f(x, y);

    //glPushMatrix();
    //glTranslatef(x, y, 0);
    //glScalef(1, -1, 1);

    font->Render(text.c_str());

    //glScalef(1, 1, 1);
    //glTranslatef(-x, -y, 0);
    //glPopMatrix();
}

void draw_string_background(FTFont* font, int size, float x, float y, std::string text)
{
    float string_width = get_string_width(font, text);

    float font_height = get_font_height(font);

    glBegin(GL_QUADS);
        glVertex2f(x                - 1, y                          - 0);
        glVertex2f(x + string_width + 0, y                          - 0);
        glVertex2f(x + string_width + 0, y + font_height + (size * 0.1));
        glVertex2f(x                - 1, y + font_height + (size * 0.1));
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

std::string get_windows_font_file(std::string name)
{
    char windows_folder[MAX_PATH];
    GetWindowsDirectoryA(windows_folder, MAX_PATH);

    std::stringstream windows_font_file_arial;
    windows_font_file_arial << windows_folder << "\\Fonts\\" << name << ".ttf";

    return windows_font_file_arial.str();
}

void font_load()
{
/*
    if (!font_file.size())
    {
        return;
    }
*/

    if (string_contains(font_file, ".ttf") == false)
    {
        return;
    }

    if (file_exists(font_file) == false)
    {
        font_file = get_windows_font_file(FONT_NAME_DEFAULT);
    }

    delete font_object;
    font_object = new FTBitmapFont(font_file.c_str());

    if (font_object->Error())
    {
        std::stringstream error_text;
        error_text
            << "An error occurred with FTGL."
            << "\r\n\r\n"
            << "Font could not be loaded."
            << "\r\n"
            << "File: " << font_file;

        message_box_show_error(error_text.str());

        return;
    }

    delete font_size1_object;
    delete font_size2_object;
    delete font_size3_object;
    font_size1_object = new FTBitmapFont(font_file.c_str());
    font_size2_object = new FTBitmapFont(font_file.c_str());
    font_size3_object = new FTBitmapFont(font_file.c_str());

    font_object->FaceSize(font_size);

    font_size1_object->FaceSize(font_size1);
    font_size2_object->FaceSize(font_size2);
    font_size3_object->FaceSize(font_size3);
}

void window_load_background_texture()
{
    if (!background_file.size())
    {
        return;
    }

    window_background_texture = SOIL_load_OGL_texture
    (
        background_file.c_str(),

        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_COMPRESS_TO_DXT //| SOIL_FLAG_INVERT_Y
    );
}

void window_save_screenshot()
{
    int i = 0;
    
    while (true)
    {
        std::stringstream screenshot_index;
        screenshot_index << std::setw(6) << std::setfill('0') << i;

        std::stringstream screenshot_filename;
        screenshot_filename << "eqmap" << screenshot_index.str() << ".png";

        if (file_exists(screenshot_filename.str()) == false)
        {
            int screenshot_result = SOIL_save_screenshot
            (
                screenshot_filename.str().c_str(),
                SOIL_SAVE_TYPE_PNG,
                0, 0, window_width, window_height
            );

            break;
        }

        i++;
    }

    Sleep(1000);
}

void window_set_border(HWND hwnd, bool b)
{
    LONG window_style = GetWindowLong(hwnd, GWL_STYLE);

    if (b == true)
    {
        window_style |= (WS_CAPTION | WS_THICKFRAME);
    }
    else
    {
        window_style &= ~(WS_CAPTION | WS_THICKFRAME);
    }

    SetWindowLongPtr(hwnd, GWL_STYLE, window_style);

    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
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

void map_calculate_grid_size()
{
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
}

void map_parse_file(std::string filename, int layer)
{
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
        glfwSetWindowTitle(window_object, buffer.str().c_str());
        buffer.str("");

        map_parse_file(map_filename, 0);
    }
    else
    {
        buffer << zone_name << " - " << APPLICATION_NAME;
        glfwSetWindowTitle(window_object, buffer.str().c_str());
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

    map_calculate_grid_size();

    map_zoom_reset();
    map_center();

    map_zoom_to_fit = true;

    if (window_use_zone_specific_background_texture == true)
    {
        std::stringstream zone_specific_background_file;
        zone_specific_background_file << backgrounds_folder << "/" << map_zone_name << ".png";

        if (file_exists(zone_specific_background_file.str()) == true)
        {
            background_file = zone_specific_background_file.str();

            window_load_background_texture();
        }
        else
        {
            background_file = background_file_default;

            window_load_background_texture();
        }
    }
}

void parse_zone_info()
{
    if (file_exists(zones_file) == false)
    {
        return;
    }

    zone_infos.clear();

    std::ifstream file(zones_file.c_str());

    if (!file.is_open())
    {
        return;
    }

    std::string line;

    while (std::getline(file, line))
    {
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

        std::vector<std::string> tokens;

        boost::split(tokens, line, boost::is_any_of("="));

        if (tokens.size() == 2)
        {
            zone_info_t zone_info;

            zone_info.name_short = tokens.at(0);
            zone_info.name_long  = tokens.at(1);

            zone_info.group = zone_info_group;

            zone_infos.push_back(zone_info);
        }
    }
}

static void error_callback(int error, const char* description)
{
    std::stringstream error_text;
    error_text
        << "An error occurred with GLFW."
        << "\r\n\r\n"
        << error << ": " << description;

    message_box_show_error(error_text.str());
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
        case GLFW_KEY_UP:
            map_scroll_up();
            break;

        case GLFW_KEY_DOWN:
            map_scroll_down();
            break;

        case GLFW_KEY_LEFT:
            map_scroll_left();
            break;

        case GLFW_KEY_RIGHT:
            map_scroll_right();
            break;

        case GLFW_KEY_0:
            map_center_on_origin();
            break;

        case GLFW_KEY_1:
            map_zoom = 1.0;
            break;

        case GLFW_KEY_2:
            map_zoom = 2.0;
            break;

        case GLFW_KEY_3:
            map_zoom = 3.0;
            break;

        case GLFW_KEY_4:
            map_zoom = 4.0;
            break;

        case GLFW_KEY_5:
            map_zoom = 5.0;
            break;

        case GLFW_KEY_6:
            map_zoom = 6.0;
            break;

        case GLFW_KEY_7:
            map_zoom = 7.0;
            break;

        case GLFW_KEY_8:
            map_zoom = 8.0;
            break;

        case GLFW_KEY_9:
            map_zoom = 9.0;
            break;
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (!TwEventMouseButtonGLFW(button, action))
    {

        if (action == GLFW_RELEASE)
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            mouse_dragging = false;
        }

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            SetCursor(LoadCursor(NULL, IDC_SIZEALL));

            mouse_dragging_start_x = mouse_x;
            mouse_dragging_start_y = mouse_y;

            mouse_dragging = true;
        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        {
            SetCursor(LoadCursor(NULL, IDC_CROSS));
        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        {
            map_offset_x = map_offset_x + (map_origin_x - mouse_x) * map_zoom;
            map_offset_y = map_offset_y + (map_origin_y - mouse_y) * map_zoom;
        }

        if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
        {
            map_zoom_reset();
            map_center();
        }

        if (button == GLFW_MOUSE_BUTTON_4)
        {
            if (action == GLFW_PRESS)
            {
                mouse4_pressed = true;
            }
            else
            {
                mouse4_pressed = false;
            }
        }

        if (button == GLFW_MOUSE_BUTTON_5)
        {
            if (action == GLFW_PRESS)
            {
                mouse5_pressed = true;
            }
            else
            {
                mouse5_pressed = false;
            }
        }

    }
}

static void scroll_callback(GLFWwindow* window, double x, double y)
{
    static int mouse_wheel_pos = 0;
    mouse_wheel_pos += (int)y;

    int result = TwMouseWheel(mouse_wheel_pos);

    if (!result)
    {

        if (mouse4_pressed == true)
        {
            if (y > 0)
            {
                map_max_z += 1;
            }
            else
            {
                map_max_z -= 1;
            }
        }

        if (mouse5_pressed == true)
        {
            if (y > 0)
            {
                map_min_z += 1;
            }
            else
            {
                map_min_z -= 1;
            }
        }

        if (mouse4_pressed == false && mouse5_pressed == false)
        {
            if (y > 0)
            {
                map_zoom_in();
            }
            else
            {
                map_zoom_out();
            }
        }

    }
}

static void cursor_pos_callback(GLFWwindow* window, double x, double y)
{
    mouse_x = x;
    mouse_y = y;

    if (!TwEventMousePosGLFW(x, y))
    {

        if (mouse_dragging == true)
        {
            SetCursor(LoadCursor(NULL, IDC_SIZEALL));

            map_offset_x += (x - mouse_dragging_start_x) * map_zoom;
            map_offset_y += (y - mouse_dragging_start_y) * map_zoom;

            mouse_dragging_start_x = x;
            mouse_dragging_start_y = y;
        }

    }
}

void viewport_and_ortho()
{
    glViewport(0, 0, window_width, window_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //glOrtho(0, window_width, 0, window_height, -1, 1);
    glOrtho(0, window_width, window_height, 0, -1, 1); // y-axis is flipped, origin is top-left of window

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    window_width  = width;
    window_height = height;

    map_origin_x = window_width  / 2;
    map_origin_y = window_height / 2;

    viewport_and_ortho();

    TwWindowSize(window_width, window_height);

    render();
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if (window_use_background_texture == true)
    {
        if (window_background_texture != 0)
        {
            glColor3ub(255, 255, 255);

            glEnable(GL_TEXTURE_2D);

            glBindTexture(GL_TEXTURE_2D, window_background_texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glBegin(GL_QUADS);
                glTexCoord2f(0, 0); glVertex2f(0.0f                 , 0.0f                  );
                glTexCoord2f(1, 0); glVertex2f((GLfloat)window_width, 0.0f                  );
                glTexCoord2f(1, 1); glVertex2f((GLfloat)window_width, (GLfloat)window_height);
                glTexCoord2f(0, 1); glVertex2f(0.0f                 , (GLfloat)window_height);
            glEnd();

            glDisable(GL_TEXTURE_2D);
        }
    }

    if (map_draw_lines == true)
    {
        glLineWidth(map_lines_width);

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

                if (map_draw_lines_layer0 == false)
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

                if (map_draw_lines_layer1 == false)
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

                if (map_draw_lines_layer2 == false)
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

                if (map_draw_lines_layer3 == false)
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

            if (map_lines_black_to_white == true)
            {
                if (map_line.r == 0 && map_line.g == 0 && map_line.b == 0)
                {
                    map_line.r = 255;
                    map_line.g = 255;
                    map_line.b = 255;
                }
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
        float font_height = get_font_height(font_object);

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

            float origin_label_length = get_string_width(font_object, origin_label);

            // north
            draw_string(font_object, font_size, origin_map_x - (origin_label_length / 2), min_map_y - font_height,                     origin_label);

            // south
            draw_string(font_object, font_size, origin_map_x - (origin_label_length / 2), max_map_y + font_height + (font_height / 2), origin_label);

            // west
            draw_string(font_object, font_size, min_map_x - font_height - origin_label_length, origin_map_y + (font_height / 2), origin_label);

            // east
            draw_string(font_object, font_size, max_map_x + font_height,                       origin_map_y + (font_height / 2), origin_label);
        }

        glColor3f(map_grid_color[0] / 2, map_grid_color[1] / 2, map_grid_color[2] / 2);

        glEnable(GL_LINE_STIPPLE);

        glLineStipple(map_grid_line_stipple_factor, map_grid_line_stipple_type);

        //glBegin(GL_LINES);

        int grid_num_lines = 0;

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

                float grid_size_label_length = get_string_width(font_object, grid_size_label.str());

                // west
                draw_string(font_object, font_size, min_map_x - font_height - grid_size_label_length, next_y + (font_height / 2), grid_size_label.str());

                // east
                draw_string(font_object, font_size, max_map_x + font_height,                          next_y + (font_height / 2), grid_size_label.str());
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

                float grid_size_label_length = get_string_width(font_object, grid_size_label.str());

                // west
                draw_string(font_object, font_size, min_map_x - font_height - grid_size_label_length, next_y + (font_height / 2), grid_size_label.str());

                // east
                draw_string(font_object, font_size, max_map_x + font_height,                          next_y + (font_height / 2), grid_size_label.str());
            }
        }

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

                float grid_size_label_length = get_string_width(font_object, grid_size_label.str());

                // north
                draw_string(font_object, font_size, next_x - (grid_size_label_length / 2), min_map_y - font_height,                     grid_size_label.str());

                // south
                draw_string(font_object, font_size, next_x - (grid_size_label_length / 2), max_map_y + font_height + (font_height / 2), grid_size_label.str());
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

                float grid_size_label_length = get_string_width(font_object, grid_size_label.str());

                // north
                draw_string(font_object, font_size, next_x - (grid_size_label_length / 2), min_map_y - font_height,                     grid_size_label.str());

                // south
                draw_string(font_object, font_size, next_x - (grid_size_label_length / 2), max_map_y + font_height + (font_height / 2), grid_size_label.str());
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

            if (map_points_black_to_white == true)
            {
                if (map_point.r == 0 && map_point.g == 0 && map_point.b == 0)
                {
                    map_point.r = 255;
                    map_point.g = 255;
                    map_point.b = 255;
                }
            }

            FTFont* point_font_object = font_size1_object;

            int point_font_size = font_size1;

            float point_font_height = get_font_height(font_size1_object);

            if (map_points_ignore_size == false)
            {
                switch (map_point.size)
                {
                    case 1:
                        point_font_object = font_size1_object;
                        point_font_size   = font_size1;
                        point_font_height = get_font_height(font_size1_object);
                        break;

                    case 2:
                        point_font_object = font_size1_object;
                        point_font_size   = font_size2;
                        point_font_height = get_font_height(font_size2_object);
                        break;

                    case 3:
                        point_font_object = font_size2_object;
                        point_font_size   = font_size3;
                        point_font_height = get_font_height(font_size3_object);
                        break;
                }
            }

            if (map_points_opaque_background == true)
            {
                float point_background_x = point_map_x;
                float point_background_y = point_map_y + (point_font_height + (point_font_size * 0.1)) - point_font_height;

                glColor3ub(0, 0, 128);

                draw_string_background(point_font_object, point_font_size, point_background_x, point_background_y, map_point.text);

                glColor3ub(255, 255, 255);
            }
            else
            {
                glColor3ub(map_point.r, map_point.g, map_point.b);
            }

            draw_plus(point_map_x, point_map_y, 4);

            draw_string(point_font_object, point_font_size, point_map_x, point_map_y + (point_font_height + (point_font_size * 0.1)), map_point.text);

            if (map_points_show_coordinates == true)
            {
                float loc_y = reverse_sign(map_point.y);
                float loc_x = reverse_sign(map_point.x);

                std::stringstream map_point_coordinates_text;
                map_point_coordinates_text << std::setprecision(2) << std::fixed << "(" << loc_y << ", " << loc_x << ")";

                if (map_points_opaque_background == true)
                {
                    float point_background_x = point_map_x;
                    float point_background_y = point_map_y + ((point_font_height + (point_font_size * 0.1)) * 2) - point_font_height;

                    glColor3ub(0, 0, 128);

                    draw_string_background(point_font_object, point_font_size, point_background_x, point_background_y, map_point_coordinates_text.str());

                    glColor3ub(255, 255, 255);
                }
                //else
                //{
                    //glColor3ub(map_point.r, map_point.g, map_point.b);
                //}

                draw_string(point_font_object, point_font_size, point_map_x, point_map_y + ((point_font_height + (point_font_size * 0.1)) * 2), map_point_coordinates_text.str());
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

        float font_height = get_font_height(font_object);

        draw_point(origin_map_x, origin_map_y, 4);

        draw_string(font_object, font_size, origin_map_x, origin_map_y + (font_height + (font_size * 0.1)), origin_text);
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
        glColor3f(1.0, 0.0, 1.0);

        std::vector<std::string> map_info_text;

        std::stringstream map_info_text_buffer;

        map_info_text_buffer << "FPS: " << std::setprecision(2) << std::fixed << frames_per_second;
        map_info_text.push_back(map_info_text_buffer.str());
        map_info_text_buffer.str("");

        map_info_text_buffer << "Resolution: " << window_width << "x" << window_height;
        map_info_text.push_back(map_info_text_buffer.str());
        map_info_text_buffer.str("");

        map_info_text_buffer << "Zoom: " << std::setprecision(2) << std::fixed << map_zoom;
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

        float font_height = get_font_height(font_object);

        int map_info_text_index = 1;
        foreach (std::string map_info_text_value, map_info_text)
        {
            draw_string(font_object, font_size, 0 + (font_size * 0.1), 0 + ((font_height + (font_size * 0.1)) * map_info_text_index), map_info_text_value);
            map_info_text_index++;
        }
    }

    TwDraw();

    glfwSwapBuffers(window_object);
}

void init()
{
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    glClearColor(0, 0, 0, 1);

    viewport_and_ortho();
}

void done()
{
    delete font_object;

    delete font_size1_object;
    delete font_size2_object;
    delete font_size3_object;

    glfwDestroyWindow(window_object);

    glfwTerminate();

    TwTerminate();
}

void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString)
{
    destinationClientString = sourceLibraryString;
}

void TW_CALL bar_error_handler(const char *error_message)
{
    anttweakbar_last_error = error_message;
}

void TW_CALL bar_options_button_save_screenshot(void *)
{
    window_save_screenshot();
}

/*
void TW_CALL bar_options_button_toggle_fullscreen(void *)
{
    //
}
*/

void TW_CALL bar_options_button_load_font(void *)
{
    font_load();
}

void TW_CALL bar_options_button_load_background_texture(void *)
{
    window_load_background_texture();
}

void TW_CALL bar_options_button_load_zone(void *)
{
    map_load_zone(map_zone_name);
}

void TW_CALL bar_options_button_offset_up(void *)
{
    map_scroll_up();
}

void TW_CALL bar_options_button_offset_down(void *)
{
    map_scroll_down();
}

void TW_CALL bar_options_button_offset_left(void *)
{
    map_scroll_left();
}

void TW_CALL bar_options_button_offset_right(void *)
{
    map_scroll_right();
}

void TW_CALL bar_options_button_reset_height_filter(void *)
{
    map_calculate_bounds();
}

void TW_CALL bar_options_button_center_on_origin(void *)
{
    map_center_on_origin();
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
    glfwSetWindowShouldClose(window_object, GL_TRUE);
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

    std::vector<std::string> zone_short_names;

    foreach (zone_info_t zone_info, zone_infos)
    {
        zone_short_names.push_back(zone_info.name_short);
    }

    std::sort(zone_short_names.begin(), zone_short_names.end());

    foreach (std::string zone_short_name, zone_short_names)
    {
        vp = static_cast<void *>(new std::string(zone_short_name));

        std::stringstream button_name;
        button_name << "ZoneButton_" << zone_short_name << "_ShortNames";

        std::stringstream button_def;
        button_def << " label='" << zone_short_name << "' group=ShortNames";

        TwAddButton(bar_zones, button_name.str().c_str(), bar_zones_button_zone_name, vp, button_def.str().c_str());
    }

    TwDefine(" Zones/ShortNames label='Short Names' opened=false ");

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

        vp = static_cast<void *>(new std::string(zone_info.name_short));

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
    TwAddButton(bar_options, "SaveScreenshot", bar_options_button_save_screenshot, NULL,
        " group=Window label='Save Screenshot' help='Save a screenshot of the window to file' key='F9' ");
/*
    TwAddButton(bar_options, "ToggleFullscreen", bar_options_button_toggle_fullscreen, NULL,
        " group=Window label='Toggle Fullscreen' help='Switch between windowed mode and fullscreen mode' key='F11' ");
*/
    TwAddVarRW(bar_options, "FontFile", TW_TYPE_STDSTRING, &font_file,
        " group=Font label='File: ' help='Font file used for drawing text' ");
    TwAddButton(bar_options, "FontLoad", bar_options_button_load_font, NULL,
        " group=Font label='Load Font' help='Load the font' key='f' ");
    TwAddVarRW(bar_options, "FontSize", TW_TYPE_INT32, &font_size,
        " group=Font label='Size: ' help='Size of drawn text' min=1 ");
    TwAddVarRW(bar_options, "FontSize1", TW_TYPE_INT32, &font_size1,
        " group=Font label='Point Size 1: ' help='Size of drawn text for points' min=1 ");
    TwAddVarRW(bar_options, "FontSize2", TW_TYPE_INT32, &font_size2,
        " group=Font label='Point Size 2: ' help='Size of drawn text for points' min=1 ");
    TwAddVarRW(bar_options, "FontSize3", TW_TYPE_INT32, &font_size3,
        " group=Font label='Point Size 3: ' help='Size of drawn text for points' min=1 ");

    TwDefine(" Options/Font group=Window ");

    TwAddVarRW(bar_options, "WindowUseBackgroundTexture", TW_TYPE_BOOLCPP, &window_use_background_texture,
        " group=Background label='Enabled' help='Use an image as the background for the window' key='b' ");
    TwAddVarRW(bar_options, "WindowUseZoneSpecificBackgroundTexture", TW_TYPE_BOOLCPP, &window_use_zone_specific_background_texture,
        " group=Background label='Use Zone Specific Textures' help='Use an image that is specific to the current map as the background for the window' key='B' ");
    TwAddVarRW(bar_options, "BackgroundFile", TW_TYPE_STDSTRING, &background_file,
        " group=Background label='Texture File: ' help='Image file used as the background for the window' ");
    TwAddButton(bar_options, "WindowLoadBackgroundTexture", bar_options_button_load_background_texture, NULL,
        " group=Background label='Load Texture' help='Load the background texture' key='t' ");

    TwDefine(" Options/Background group=Window ");

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

    TwAddButton(bar_options, "MapOffsetUp", bar_options_button_offset_up, NULL,
        " group=Offset label='Up' help='Move around the map' key='Up' ");
    TwAddButton(bar_options, "MapOffsetDown", bar_options_button_offset_down, NULL,
        " group=Offset label='Down' help='Move around the map' key='Down' ");
    TwAddButton(bar_options, "MapOffsetLeft", bar_options_button_offset_left, NULL,
        " group=Offset label='Left' help='Move around the map' key='Left' ");
    TwAddButton(bar_options, "MapOffsetRight", bar_options_button_offset_right, NULL,
        " group=Offset label='Right' help='Move around the map' key='Right' ");

    TwDefine(" Options/Offset group=Map ");

    TwDefine(" Options/Offset opened=false ");

    TwAddVarRW(bar_options, "MapDrawLines", TW_TYPE_BOOLCPP, &map_draw_lines,
        " group=Draw label='Lines' help='Draw the lines of the map' key='l' ");
    TwAddVarRW(bar_options, "MapDrawPoints", TW_TYPE_BOOLCPP, &map_draw_points,
        " group=Draw label='Points' help='Draw the points of the map' key='p' ");
    TwAddVarRW(bar_options, "MapDrawOrigin", TW_TYPE_BOOLCPP, &map_draw_origin,
        " group=Draw label='Origin' help='Draw a point at the origin coordinates 0,0' key='o' ");

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

    TwAddVarRW(bar_options, "MapLinesWidth", TW_TYPE_FLOAT, &map_lines_width,
        " group=Lines label='Width: ' help='How wide or thick the map lines are drawn' min=1 max=10 step=1 precision=0 ");
    TwAddVarRW(bar_options, "MapLinesBlackToWhite", TW_TYPE_BOOLCPP, &map_lines_black_to_white,
        " group=Lines label='Black -> White' help='Change the color of lines from black to white for visibility on dark backgrounds' ");

    TwAddSeparator(bar_options, NULL, " group=Lines ");

    TwAddVarRW(bar_options, "MapDrawLinesLayer0", TW_TYPE_BOOLCPP, &map_draw_lines_layer0,
        " group=Lines label='Base' help='Draw the lines of the base map layer' ");
    TwAddVarRW(bar_options, "MapDrawLinesLayer1", TW_TYPE_BOOLCPP, &map_draw_lines_layer1,
        " group=Lines label='Layer 1' help='Draw the lines of the first map layer' ");
    TwAddVarRW(bar_options, "MapDrawLinesLayer2", TW_TYPE_BOOLCPP, &map_draw_lines_layer2,
        " group=Lines label='Layer 2' help='Draw the lines of the second map layer' ");
    TwAddVarRW(bar_options, "MapDrawLinesLayer3", TW_TYPE_BOOLCPP, &map_draw_lines_layer3,
        " group=Lines label='Layer 3' help='Draw the lines of the third map layer' ");

    TwDefine(" Options/Lines group=Map ");

    TwAddVarRW(bar_options, "MapPointsIgnoreSize", TW_TYPE_BOOLCPP, &map_points_ignore_size,
        " group=Points label='Ignore Size' help='Always use the smallest size for points of the map' ");
    TwAddVarRW(bar_options, "MapPointsShowCoordinates", TW_TYPE_BOOLCPP, &map_points_show_coordinates,
        " group=Points label='Show Coordinates' help='Show the /loc coordinates for points of the map' key='P' ");
    TwAddVarRW(bar_options, "MapPointsOpaqueBackground", TW_TYPE_BOOLCPP, &map_points_opaque_background,
        " group=Points label='Opaque Background' help='Draw an opaque background behind points of the map' ");
    TwAddVarRW(bar_options, "MapPointsBlackToWhite", TW_TYPE_BOOLCPP, &map_points_black_to_white,
        " group=Points label='Black -> White' help='Change the color of points from black to white for visibility on dark backgrounds' ");

    TwAddSeparator(bar_options, NULL, " group=Points ");

    TwAddVarRW(bar_options, "MapDrawPointsLayer0", TW_TYPE_BOOLCPP, &map_draw_points_layer0,
        " group=Points label='Base' help='Draw the points of the base map layer' ");
    TwAddVarRW(bar_options, "MapDrawPointsLayer1", TW_TYPE_BOOLCPP, &map_draw_points_layer1,
        " group=Points label='Layer 1' help='Draw the points of the first map layer' ");
    TwAddVarRW(bar_options, "MapDrawPointsLayer2", TW_TYPE_BOOLCPP, &map_draw_points_layer2,
        " group=Points label='Layer 2' help='Draw the points of the second map layer' ");
    TwAddVarRW(bar_options, "MapDrawPointsLayer3", TW_TYPE_BOOLCPP, &map_draw_points_layer3,
        " group=Points label='Layer 3' help='Draw the points of the third map layer' ");

    TwDefine(" Options/Points group=Map ");

    TwAddVarRW(bar_options, "MinZ", TW_TYPE_FLOAT, &map_min_z,
        " group=HeightFilter label='Minimum Z-Axis: ' help='Filters out lines below this height' keyIncr='PGUP' keyDecr='PGDOWN' step=1 precision=2 ");
    TwAddVarRW(bar_options, "MaxZ", TW_TYPE_FLOAT, &map_max_z,
        " group=HeightFilter label='Maximum Z-Axis: ' help='Filters out lines above this height' keyIncr='SHIFT+PGUP' keyDecr='SHIFT+PGDOWN' step=1 precision=2 ");

    TwAddButton(bar_options, "ResetHeightFilter", bar_options_button_reset_height_filter, NULL,
        " group=HeightFilter label='Reset Height Filter' help='Reset the Z-axis values for the height filter' key='h' ");

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
        " group=Grid label='Size: ' help='Interval between grid lines and coordinates' min=10 precision=0 ");
    TwAddVarRW(bar_options, "MapGridColor", TW_TYPE_COLOR3F, &map_grid_color,
        " group=Grid label='Color: ' help='Change the color of the grid' ");

    TwAddVarRW(bar_options, "MapGridLineStippleFactor", TW_TYPE_INT32, &map_grid_line_stipple_factor,
        " group=Grid label='Line Stipple Factor: ' help='Change the appearance of the grid lines' min=1 ");

    TwEnumVal map_grid_line_stipple_tw_ev[] =
    {
        {MAP_GRID_LINE_STIPPLE_TYPE_DOTTED,        "Dotted"},
        {MAP_GRID_LINE_STIPPLE_TYPE_DASHED,        "Dashed"},
        {MAP_GRID_LINE_STIPPLE_TYPE_DASH_DOT_DASH, "Dash Dot Dash"}
    };

    TwType map_grid_line_stipple_tw_type = TwDefineEnum("MapGridLineStippleTypeEnum", map_grid_line_stipple_tw_ev, 3);

    TwAddVarRW(bar_options, "MapGridLineStippleType", map_grid_line_stipple_tw_type, &map_grid_line_stipple_type,
        " group=Grid label='Line Stipple Type: ' help='Change the appearance of the grid lines' ");

    TwDefine(" Options/Grid group=Map ");

    TwAddButton(bar_options, "CenterOnOrigin", bar_options_button_center_on_origin, NULL,
        " group=Map label='Center Map on Origin' help='Center the map on the origin coordiantes' key='0' ");

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

    window_use_background_texture = pt.get<bool>("Window.UseBackgroundTexture", window_use_background_texture);

    window_use_zone_specific_background_texture = pt.get<bool>("Window.UseZoneSpecificBackgroundTexture", window_use_zone_specific_background_texture);

    backgrounds_folder = pt.get<std::string>("Backgrounds.Folder", backgrounds_folder);
    background_file    = pt.get<std::string>("Backgrounds.File",   background_file);

    fonts_folder = pt.get<std::string>("Fonts.Folder", fonts_folder);
    font_file    = pt.get<std::string>("Fonts.File",   font_file);

    font_size  = pt.get<int>("Fonts.Size",       font_size);
    font_size1 = pt.get<int>("Fonts.PointSize1", font_size1);
    font_size2 = pt.get<int>("Fonts.PointSize2", font_size2);
    font_size3 = pt.get<int>("Fonts.PointSize3", font_size3);

    map_folder = pt.get<std::string>("Map.Folder", map_folder);

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

    map_draw_lines_layer0 = pt.get<bool>("Lines.Base",   map_draw_lines_layer0);
    map_draw_lines_layer1 = pt.get<bool>("Lines.Layer1", map_draw_lines_layer1);
    map_draw_lines_layer2 = pt.get<bool>("Lines.Layer2", map_draw_lines_layer2);
    map_draw_lines_layer3 = pt.get<bool>("Lines.Layer3", map_draw_lines_layer3);

    map_draw_points_layer0 = pt.get<bool>("Points.Base",   map_draw_points_layer0);
    map_draw_points_layer1 = pt.get<bool>("Points.Layer1", map_draw_points_layer1);
    map_draw_points_layer2 = pt.get<bool>("Points.Layer2", map_draw_points_layer2);
    map_draw_points_layer3 = pt.get<bool>("Points.Layer3", map_draw_points_layer3);

    map_lines_width          = pt.get<float>("Lines.Width",        map_lines_width);
    map_lines_black_to_white = pt.get<bool> ("Lines.BlackToWhite", map_lines_black_to_white);

    map_points_show_coordinates  = pt.get<bool>("Points.ShowCoordinates",  map_points_show_coordinates);
    map_points_opaque_background = pt.get<bool>("Points.OpaqueBackground", map_points_opaque_background);
    map_points_ignore_size       = pt.get<bool>("Points.IgnoreSize",       map_points_ignore_size);
    map_points_black_to_white    = pt.get<bool>("Points.BlackToWhite",     map_points_black_to_white);

    map_draw_grid             = pt.get<bool>("Grid.Enabled",     map_draw_grid);
    map_draw_grid_coordinates = pt.get<bool>("Grid.Coordinates", map_draw_grid_coordinates);

    map_grid_color[0] = pt.get<int>("Grid.ColorRed",   map_grid_color[0]);
    map_grid_color[1] = pt.get<int>("Grid.ColorGreen", map_grid_color[1]);
    map_grid_color[2] = pt.get<int>("Grid.ColorBlue",  map_grid_color[2]);

    map_grid_color[0] /= 256;
    map_grid_color[1] /= 256;
    map_grid_color[2] /= 256;

    map_grid_line_stipple_factor = pt.get<int>("Grid.LineStippleFactor", map_grid_line_stipple_factor);

    map_grid_line_stipple_type_config = pt.get<int>("Grid.LineStippleType", map_grid_line_stipple_type_config);

    switch (map_grid_line_stipple_type_config)
    {
        case 1:
            map_grid_line_stipple_type = MAP_GRID_LINE_STIPPLE_TYPE_DOTTED;
            break;

        case 2:
            map_grid_line_stipple_type = MAP_GRID_LINE_STIPPLE_TYPE_DOTTED;
            break;

        case 3:
        default:
            map_grid_line_stipple_type = MAP_GRID_LINE_STIPPLE_TYPE_DASH_DOT_DASH;
            break;
    }

    backgrounds_folder = pt.get<std::string>("Backgrounds.Folder", backgrounds_folder);
}

LRESULT CALLBACK window_proc_hook(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    int handled = 0;

    static unsigned int s_PrevKeyDown = 0;
    static int s_PrevKeyDownMod = 0;
    static int s_PrevKeyDownHandled = 0;

    switch (message)
    {
        case WM_DROPFILES:
        {
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

                std::string drop_filename_stdstring = (char *)drop_filename;

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

                if (string_contains(drop_filename_stdstring, ".png") == true)
                {
                    background_file = drop_filename_stdstring;

                    window_load_background_texture();

                    window_use_background_texture = true;
                }

                if (string_contains(drop_filename_stdstring, ".ttf") == true)
                {
                    font_file = drop_filename_stdstring;

                    font_load();
                }
            }

            DragFinish(drop);
            break;
            }

////////////////////////////////////////////////////////////////////////////////////////////////////

        case WM_CHAR:
        case WM_SYSCHAR:
        {
            int key = (int)(wparam&0xff);
            int kmod = 0;

            if( GetAsyncKeyState(VK_SHIFT)<0 )
                kmod |= TW_KMOD_SHIFT;
            if( GetAsyncKeyState(VK_CONTROL)<0 )
            {
                kmod |= TW_KMOD_CTRL;
                if( key>0 && key<27 )
                    key += 'a'-1;
            }
            if( GetAsyncKeyState(VK_MENU)<0 )
                kmod |= TW_KMOD_ALT;
            if( key>0 && key<256 )
                handled = TwKeyPressed(key, kmod);
        }
        break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            int kmod = 0;
            int testkp = 0;
            int k = 0;

            if( GetAsyncKeyState(VK_SHIFT)<0 )
                kmod |= TW_KMOD_SHIFT;
            if( GetAsyncKeyState(VK_CONTROL)<0 )
            {
                kmod |= TW_KMOD_CTRL;
                testkp = 1;
            }
            if( GetAsyncKeyState(VK_MENU)<0 )
            {
                kmod |= TW_KMOD_ALT;
                testkp = 1;
            }
            if( wparam>=VK_F1 && wparam<=VK_F15 )
                k = TW_KEY_F1 + ((int)wparam-VK_F1);
            else if( testkp && wparam>=VK_NUMPAD0 && wparam<=VK_NUMPAD9 )
                k = '0' + ((int)wparam-VK_NUMPAD0);
            else
            {
                switch( wparam )
                {
                case VK_UP:
                    k = TW_KEY_UP;
                    break;
                case VK_DOWN:
                    k = TW_KEY_DOWN;
                    break;
                case VK_LEFT:
                    k = TW_KEY_LEFT;
                    break;
                case VK_RIGHT:
                    k = TW_KEY_RIGHT;
                    break;
                case VK_INSERT:
                    k = TW_KEY_INSERT;
                    break;
                case VK_DELETE:
                    k = TW_KEY_DELETE;
                    break;
                case VK_PRIOR:
                    k = TW_KEY_PAGE_UP;
                    break;
                case VK_NEXT:
                    k = TW_KEY_PAGE_DOWN;
                    break;
                case VK_HOME:
                    k = TW_KEY_HOME;
                    break;
                case VK_END:
                    k = TW_KEY_END;
                    break;
                case VK_DIVIDE:
                    if( testkp )
                        k = '/';
                    break;
                case VK_MULTIPLY:
                    if( testkp )
                        k = '*';
                    break;
                case VK_SUBTRACT:
                    if( testkp )
                        k = '-';
                    break;
                case VK_ADD:
                    if( testkp )
                        k = '+';
                    break;
                case VK_DECIMAL:
                    if( testkp )
                        k = '.';
                    break;
                default:
                    if( (kmod&TW_KMOD_CTRL) && (kmod&TW_KMOD_ALT) )
                        k = MapVirtualKey( (UINT)wparam, 2 ) & 0x0000FFFF;
                }
            }
            if( k!=0 )
                handled = TwKeyPressed(k, kmod);
            else
            {
                int key = (int)(wparam&0xff);
                if( kmod&TW_KMOD_CTRL && key>0 && key<27 )
                    key += 'a'-1;
                if( key>0 && key<256 )
                    handled = TwKeyTest(key, kmod);
            }
            s_PrevKeyDown = wparam;
            s_PrevKeyDownMod = kmod;
            s_PrevKeyDownHandled = handled;
        }
        break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            int kmod = 0;
            if( GetAsyncKeyState(VK_SHIFT)<0 )
                kmod |= TW_KMOD_SHIFT;
            if( GetAsyncKeyState(VK_CONTROL)<0 )
                kmod |= TW_KMOD_CTRL;
            if( GetAsyncKeyState(VK_MENU)<0 )
                kmod |= TW_KMOD_ALT;
            if( s_PrevKeyDown==wparam && s_PrevKeyDownMod==kmod )
                handled = s_PrevKeyDownHandled;
            else 
            {
                int key = (int)(wparam&0xff);
                if( kmod&TW_KMOD_CTRL && key>0 && key<27 )
                    key += 'a'-1;
                if( key>0 && key<256 )
                    handled = TwKeyTest(key, kmod);
            }
            s_PrevKeyDown = 0;
            s_PrevKeyDownMod = 0;
            s_PrevKeyDownHandled = 0;
        }
        break;

////////////////////////////////////////////////////////////////////////////////////////////////////

    }

    LRESULT result = CallWindowProc(window_proc_normal, hwnd, message, wparam, lparam);

    return result;
}

int main(int argc, char **argv)
{
    atexit(done);

    //font_file = get_windows_font_file(FONT_NAME_DEFAULT);

    map_load_config();

    if (!glfwInit())
    {
        message_box_show_error("Function glfwInit failed");

        done();

        exit(EXIT_FAILURE);
    }

    GLFWmonitor *monitor = NULL;

    if (window_start_fullscreen == true)
    {
        monitor = glfwGetPrimaryMonitor();

        window_is_fullscreen = true;
    }

    const GLFWvidmode *vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    if (window_start_fullscreen == true)
    {
        window_width  = vidmode->width;
        window_height = vidmode->height;
    }

    window_object = glfwCreateWindow(window_width, window_height, APPLICATION_NAME, monitor, NULL);
    if (!window_object)
    {
        message_box_show_error("Function glfwCreateWindow failed");

        done();

        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window_object);

    if (window_start_fullscreen == false)
    {
        //glfwSetWindowSize(window, window_width, window_height);
        glfwSetWindowPos
        (
            window_object,
            (vidmode->width  - window_width)  / 2,
            (vidmode->height - window_height) / 2
        );
    }

    window_hwnd = glfwGetWin32Window(window_object);

    window_proc_normal = (WNDPROC)GetWindowLongPtr(window_hwnd, GWLP_WNDPROC);
    SetWindowLongPtr(window_hwnd, GWLP_WNDPROC, (LONG_PTR)window_proc_hook);

    DragAcceptFiles(window_hwnd, true);

    if (window_start_maximized == true)
    {
        ShowWindow(window_hwnd, SW_MAXIMIZE);
    }

    glfwSetErrorCallback(error_callback);

    glfwSetWindowSizeCallback (window_object, window_size_callback);
    //glfwSetKeyCallback        (window_object, key_callback);
    //glfwSetCharCallback       (window_object, char_callback);
    glfwSetMouseButtonCallback(window_object, mouse_button_callback);
    glfwSetScrollCallback     (window_object, scroll_callback);
    glfwSetCursorPosCallback  (window_object, cursor_pos_callback);

    init();

    TwInit(TW_OPENGL, NULL);

    TwCopyStdStringToClientFunc(CopyStdStringToClient);
    TwHandleErrors(bar_error_handler);

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

        if (string_contains(argv[1], ".png") == true)
        {
            background_file = argv[1];

            window_use_background_texture = true;
        }

        if (string_contains(argv[1], ".ttf") == true)
        {
            font_file = argv[1];
        }
    }

    font_load();

    window_load_background_texture();

    map_load_zone(map_zone_name);

    last_tick = GetTickCount();

    while (!glfwWindowShouldClose(window_object))
    {
        render();

        if (map_zoom_to_fit == true)
        {
            glfwPollEvents();
        }
        else
        {
            glfwWaitEvents();
        }
    }

    done();

    exit(EXIT_SUCCESS);

    return 0;
}
