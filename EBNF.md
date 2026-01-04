``` EBNF
# Scene definition
scene ::= declaration {declaration}

# Declarations
declaration ::= float_decl
              | plane_decl
              | sphere_decl
              | material_decl
              | camera_decl
              | point_light_decl

# Float declaration
float_decl ::= "float" IDENTIFIER "(" number ")"

# Geometric primitives
plane_decl  ::= "plane" "(" IDENTIFIER "," transformation ")"
sphere_decl ::= "sphere" "(" IDENTIFIER "," transformation ")"

# Material
material_decl ::= "material" IDENTIFIER "(" brdf "," pigment ")"

# Camera
camera_decl ::= "camera" "(" camera_type "," transformation "," aspect_ratio "," number ")"
camera_type ::= "perspective" | "orthogonal"
aspect_ratio ::= "exact_asp_ratio" | number

# Lights for point light tracing
point_light_decl ::= "point_light" "(" vector "," color "," number ")"

# BRDFs
brdf ::= diffuse_brdf | specular_brdf
diffuse_brdf  ::= "diffuse" "(" pigment ")"
specular_brdf ::= "specular" "(" pigment ")"

# Pigments
pigment ::= uniform_pigment | checkered_pigment | image_pigment
uniform_pigment   ::= "uniform" "(" color ")"
checkered_pigment ::= "checkered" "(" color "," color "," number ")"
image_pigment     ::= "image" "(" LITERAL_STRING ")"

# Colors
color ::= "<" number "," number "," number ">"

# Transformations
transformation ::= basic_transformation {"*" basic_transformation}  # At least one basic_transformation
basic_transformation ::= "identity"
                       | "translation" "(" vector ")"
                       | "rotation_x" "(" number ")"
                       | "rotation_y" "(" number ")"
                       | "rotation_z" "(" number ")"
                       | "scaling" "(" vector ")"

# Numbers and vectors
vector ::= "[" number "," number "," number "]"
number ::= LITERAL_NUMBER | IDENTIFIER
```

