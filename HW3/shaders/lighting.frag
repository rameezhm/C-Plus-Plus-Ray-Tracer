#version 330 core

in vec4 position; // raw position in the model coord
in vec3 normal;   // raw normal in the model coord

uniform mat4 modelview; // from model coord to eye coord
uniform mat4 view;      // from world coord to eye coord

// Material parameters
uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;
uniform vec4 emision;
uniform float shininess;

// Light source parameters
const int maximal_allowed_lights = 10;
uniform bool enablelighting;
uniform int nlights;
uniform vec4 lightpositions[ maximal_allowed_lights ];
uniform vec4 lightcolors[ maximal_allowed_lights ];

// Output the frag color
out vec4 fragColor;


void main (void){
    if (!enablelighting){
        // Default normal coloring (you don't need to modify anything here)
        vec3 N = normalize(normal);
        fragColor = vec4(0.5f*N + 0.5f , 1.0f);
    } else {
        
        // HW3: You will compute the lighting here.

        // using camera coordinate system
        // eye vector is (0,0,0,(1)) in camera coords
        vec3 eye = vec3(0.f, 0.f ,0.f);
        // get position vector of fragment in camera coords
        vec3 pos = (modelview * position).xyz;
        // view vector in camera coords
        vec3 cview = normalize(eye - pos);
        // normal vector in camera coords
        mat3 normToCam = transpose(inverse(mat3(modelview[0].xyz, modelview[1].xyz, modelview[2].xyz)));
        vec3 cnorm = normalize(normToCam * normal);

        // reflected color contains constant 
        vec4 reflectedColor = emision;
        for (unsigned int j = 0; j < nlights; j++) {
            vec3 l_j = normalize((view *lightpositions[j]).xyz);
            vec3 h_j = normalize(cview + l_j);
            vec4 coef = ambient + max(dot(cnorm, l_j), 0) * diffuse + pow(max(dot(cnorm, h_j), 0), shininess) * specular;
            vec4 L_j = lightcolors[j];
            reflectedColor = reflectedColor + coef * L_j;
        }

        fragColor = reflectedColor;
    }
}
