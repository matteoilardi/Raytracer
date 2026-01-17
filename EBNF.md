``` EBNF
# Scene definition
scene ::= declaration {declaration}

# Declarations
declaration ::= float_decl
              | object_decl
              | "norender" norender_object_decl
              | material_decl
              | camera_decl
              | point_light_decl

# Float declaration
float_decl ::= "float" IDENTIFIER "(" number ")"

# Objects
object_decl ::= "sphere" sphere_args
              | "plane" plane_args
              | "csg" csg_object_args

norender_object_decl ::= "sphere" IDENTIFIER sphere_args
                       | "plane"  IDENTIFIER plane_args
                       | "csg"    IDENTIFIER csg_args

sphere_args ::= "(" IDENTIFIER "," transformation ")"
plane_args  ::= "(" IDENTIFIER "," transformation ")"
csg_object_args ::= "(" IDENTIFIER "," IDENTIFIER "," csg_operation "," transformation ")"

csg_operation ::= "union" | "intersection" | "difference" | "fusion"

# Material
material_decl ::= "material" IDENTIFIER "(" brdf "," pigment ")"

# BRDFs
brdf ::= diffuse_brdf | specular_brdf
diffuse_brdf  ::= "diffuse" "(" pigment ")"
specular_brdf ::= "specular" "(" pigment ")"

# Pigments
pigment ::= uniform_pigment | checkered_pigment | image_pigment
uniform_pigment   ::= "uniform" "(" color ")"
checkered_pigment ::= "checkered" "(" color "," color "," number ")"
image_pigment     ::= "image" "(" LITERAL_STRING ")"

# Lights for point light tracing
point_light_decl ::= "point_light" "(" vector "," color "," number ")"

# Camera
camera_decl ::= "camera" "(" camera_type "," transformation "," aspect_ratio "," number ")"
camera_type ::= "perspective" | "orthogonal"
aspect_ratio ::= "exact_asp_ratio" | number

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

