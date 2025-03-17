#include "colors.hpp"

using namespace std;

int main() {

    // Test 1: Creation of a color object, is_close function
    Color color1(1.0, 2.0, 3.0);

    assert( color1.is_close_to(Color(1.0, 2.0, 3.0)) );
    assert( !color1.is_close_to(Color(2.0, 1.0, 3.0)) );
    assert( !color1.is_close_to(Color(1.0, 2.0, 3.0001)) );


    // Test 2: Color operations
    Color color2(4.0, 5.0, 6.0);
    Color color3(1.0, 12.0, 7.0);
    float f1(1.5);

    assert(  (color2 + color3).is_close_to(Color(5.0, 17.0, 13.0)) );
    assert(  !(color2 + color3).is_close_to(Color(5.0, 16.0, 13.5)) );

    assert( (color2*color3).is_close_to(Color(4.0, 60.0, 42.0)) );

    assert( (f1*color2).is_close_to(Color(6.0, 7.5, 9.0)) );
    assert( (color2*f1).is_close_to(Color(6.0, 7.5, 9.0)) );


    // Test 3: HdrImage creation, height and width, valid_indexes method, _pixel_offset method
    HdrImage image1(20,30);
    Color color4(21.0, 18.0, 0.0);

    assert( image1.height == 20 && image1.width == 30 );

    image1.set_pixel(15, 11, color4);
    Color color5 = image1.get_pixel(15, 11);

    assert( color4.is_close_to(color5) );
    assert( color5.is_close_to(Color(21.0, 18.0, 0.0)) );

    assert( image1._valid_indexes(3,4) );
    assert( image1._valid_indexes(20,30) );
    assert( image1._valid_indexes(0,0) );
    assert( !image1._valid_indexes(-1,0) );
    assert( !image1._valid_indexes(0,-1) );
    assert( !image1._valid_indexes(0,30) );
    assert( !image1._valid_indexes(20,0) );

    assert( image1._pixel_offset(9, 5) == (4 + 9*30));

    return EXIT_SUCCESS;
}