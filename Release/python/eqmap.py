import sys, os

import PIL
from PIL import Image, ImageDraw, ImageFont

def main(argv):

    if len(sys.argv) < 2:
        print >>sys.stderr, 'Usage: {0} map.txt'.format(sys.argv[0])
        return 1

    if not os.path.exists(sys.argv[1]):
        print >>sys.stderr, 'Error: File {0} not found!'.format(sys.argv[1])
        return 1

    font_filename = "c:/windows/fonts/arial.ttf"

    font_size1 = 11
    font_size2 = 12
    font_size3 = 18

    map_filename = sys.argv[1]

    map_filename_no_extension = map_filename[0:-4]

    map_lines  = []
    map_points = []

    map_min_x = 0
    map_max_x = 0

    map_min_y = 0
    map_max_y = 0

    map_min_z = 0
    map_max_z = 0

    map_mid_x = 0
    map_mid_y = 0

    map_width  = 0
    map_height = 0
    map_depth  = 0

    map_origin_x = 0
    map_origin_y = 0

    with open(map_filename, 'r') as map_file:

        for map_line in map_file:

            #print map_line

            map_line.split()
            map_line = map_line.replace(' ', '')
            map_line = map_line.replace('_', ' ')

            map_line_type = map_line[:1]

            map_line = map_line[1:-1]

            map_line_split = map_line.split(',')
            
            if map_line_type == 'L':

                map_line_split[0] = float(map_line_split[0])
                map_line_split[1] = float(map_line_split[1])
                map_line_split[2] = float(map_line_split[2])

                map_line_split[3] = float(map_line_split[3])
                map_line_split[4] = float(map_line_split[4])
                map_line_split[5] = float(map_line_split[5])

                map_line_split[6] = int(map_line_split[6])
                map_line_split[7] = int(map_line_split[7])
                map_line_split[8] = int(map_line_split[8])

                map_lines.append(map_line_split)

            elif map_line_type == 'P':

                map_line_split[0] = float(map_line_split[0])
                map_line_split[1] = float(map_line_split[1])
                map_line_split[2] = float(map_line_split[2])

                map_line_split[3] = int(map_line_split[3])
                map_line_split[4] = int(map_line_split[4])
                map_line_split[5] = int(map_line_split[5])

                map_line_split[6] = int(map_line_split[6])

                map_points.append(map_line_split)

    map_file.close()

    #for map_point in map_points:
        #print map_point[7]

    for map_line in map_lines:

        map_line_begin_x = map_line[0]
        map_line_begin_y = map_line[1]
        map_line_begin_z = map_line[2]

        map_line_end_x = map_line[3]
        map_line_end_y = map_line[4]
        map_line_end_z = map_line[5]

        if map_line_begin_x < map_min_x:
            map_min_x = map_line_begin_x

        if map_line_end_x < map_min_x:
            map_min_x = map_line_end_x

        if map_line_begin_x > map_max_x:
            map_max_x = map_line_begin_x

        if map_line_end_x > map_max_x:
            map_max_x = map_line_end_x


        if map_line_begin_y < map_min_y:
            map_min_y = map_line_begin_y

        if map_line_end_y < map_min_y:
            map_min_y = map_line_end_y

        if map_line_begin_y > map_max_y:
            map_max_y = map_line_begin_y

        if map_line_end_y > map_max_y:
            map_max_y = map_line_end_y


        if map_line_begin_z < map_min_z:
            map_min_z = map_line_begin_z

        if map_line_end_z < map_min_z:
            map_min_z = map_line_end_z

        if map_line_begin_z > map_max_z:
            map_max_z = map_line_begin_z

        if map_line_end_z > map_max_z:
            map_max_z = map_line_end_z

    map_mid_x = (map_min_x + map_max_x) / 2
    map_mid_y = (map_min_y + map_max_y) / 2

    map_mid_x = map_mid_x * -1
    map_mid_y = map_mid_y * -1

    map_width  = abs(map_min_x - map_max_x) * 1.5
    map_height = abs(map_min_y - map_max_y) * 1.5
    map_depth  = abs(map_min_z - map_max_z)

    map_origin_x = map_width  / 2
    map_origin_y = map_height / 2

    map_image = Image.new \
    (
        'RGB',
        (
            int(map_width),
            int(map_height)
        ),
        (0, 0, 0)
    )

    map_image_draw = ImageDraw.Draw(map_image)

    map_image_font_size1 = ImageFont.truetype(font_filename, font_size1)
    map_image_font_size2 = ImageFont.truetype(font_filename, font_size2)
    map_image_font_size3 = ImageFont.truetype(font_filename, font_size3)

    for map_line in map_lines:

        map_line_begin_x = map_line[0] + map_origin_x
        map_line_begin_y = map_line[1] + map_origin_y

        map_line_end_x = map_line[3] + map_origin_x
        map_line_end_y = map_line[4] + map_origin_y

        map_line_r = map_line[6]
        map_line_g = map_line[7]
        map_line_b = map_line[8]

        if map_line_r == 0 and map_line_g == 0 and map_line_b == 0:

            map_line_r == 255
            map_line_g == 255
            map_line_b == 255

        map_image_draw.line \
        (
            (
                map_line_begin_x, map_line_begin_y,
                map_line_end_x,   map_line_end_y
            ),
            fill=(map_line_r, map_line_g, map_line_b)
        )

    map_image.save(map_filename_no_extension + '-lines-only.png')

    for map_point in map_points:

        map_point_x = map_point[0] + map_origin_x
        map_point_y = map_point[1] + map_origin_y

        map_point_r = map_point[3]
        map_point_g = map_point[4]
        map_point_b = map_point[5]

        map_point_size = map_point[6]

        map_point_text = map_point[7]

        if map_point_r == 0 and map_point_g == 0 and map_point_b == 0:

            map_point_r == 255
            map_point_g == 255
            map_point_b == 255

        map_point_font = map_image_font_size1

        if map_point_size == 1:
            map_point_font = map_image_font_size1
        elif map_point_size == 2:
            map_point_font = map_image_font_size2
        elif map_point_size == 3:
            map_point_font = map_image_font_size3

        map_point_rectangle_size = 2

        map_image_draw.rectangle \
        (
            (
                map_point_x - map_point_rectangle_size,
                map_point_y - map_point_rectangle_size,
                map_point_x + map_point_rectangle_size,
                map_point_y + map_point_rectangle_size
            ),
            fill=(map_point_r, map_point_g, map_point_b)
        )

        map_point_text_sizes = map_point_font.getsize(map_point_text)

        map_point_text_width  = map_point_text_sizes[0]
        map_point_text_height = map_point_text_sizes[1]
        
        map_image_draw.text \
        (
            (
                map_point_x - (map_point_text_width  / 2),
                map_point_y + (map_point_text_height / 2)
            ),
            map_point_text,
            (map_point_r, map_point_g, map_point_b),
            font=map_point_font
        )

    map_image.save(map_filename_no_extension + '.png')

if __name__ == "__main__":
    sys.exit(main(sys.argv))
